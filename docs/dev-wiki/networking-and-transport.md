# Networking and Transport

This page explains how participant discovery, connection setup, and transport fallback work in the current SIL Kit implementation.
It is focused on the runtime internals behind connectivity, not on user-level setup alone.

## Big Picture

The transport model follows this sequence:

1. A participant opens local acceptors for incoming connections.
2. The participant connects to the registry.
3. The participant announces its reachable endpoints to the registry.
4. The registry informs participants about each other.
5. Participants try to establish direct peer-to-peer connections.
6. If direct connection fails, SIL Kit may try a remote-connect request.
7. If that still fails, SIL Kit may fall back to routing messages through the registry as a proxy.

Conceptually, the registry is always required for discovery.
In the normal case, it is not part of the steady-state data path between participants.

## Core Transport Implementation

The concrete transport backend in this repository is VAsio.

The most important implementation area is:

- `SilKit/source/core/vasio/`

Key files include:

- `VAsioConnection.*`: participant-side transport orchestration
- `VAsioRegistry.*`: registry-side transport runtime
- `VAsioPeer.*`: a direct peer connection
- `VAsioProxyPeer.*`: a proxy-backed peer routed through the registry
- `ConnectPeer.*`: low-level connect attempts to a specific peer
- `ConnectKnownParticipants.*`: higher-level coordination of connecting to all discovered peers
- `RemoteConnectionManager.*`: handles remote connection attempts initiated via the registry
- `TransformAcceptorUris.*`: rewrites advertised endpoints for a specific audience participant

If you are debugging connectivity, this directory is usually more important than any single service implementation directory.

## What The Registry Actually Does

The registry has two distinct roles:

- mandatory discovery and initial connection brokering
- optional proxy fallback when direct participant-to-participant communication is not possible

That distinction matters.

Under normal operation:

- the registry is the first thing a participant connects to
- the registry shares information about already connected participants
- the participants then attempt to connect directly to each other

This matches the intended runtime model:

- the registry is a central process for discovery
- communication between participants is peer-to-peer
- if direct transport is unavailable, the registry can be used as a proxy

The best mental model is: discovery first, direct transport preferred, proxy only as fallback.

## Participant Join Sequence In Code

The main participant-side setup lives in `VAsioConnection::JoinSimulation(...)`.

In the current implementation, it performs these steps in order:

1. determine the simulation name from the connect URI
2. open participant acceptors
3. connect to the registry and start the IO worker
4. wait for the registry handshake to complete
5. connect to all known participants
6. wait for all participant handshakes to complete

That flow is a good entry point for debugging startup failures because it separates:

- local listen setup failures
- registry connection failures
- known-participant handshake failures

## Acceptor Endpoints

Participants must be reachable by other participants.
To do that, they open acceptors before connecting to the registry.

By default, `VAsioConnection` prepares acceptors like this:

- TCP IPv4 catch-all: `tcp://0.0.0.0:0`
- TCP IPv6 catch-all: `tcp://[::]:0`
- local domain socket path derived from temp-directory state and participant identity

Important details:

- `:0` means the operating system chooses a free port
- local domain sockets are enabled only when `EnableDomainSockets` is true
- if all acceptors fail, participant join fails immediately

Relevant code paths:

- `PrepareAcceptorEndpointUris(...)`
- `OpenParticipantAcceptors(...)`
- `OpenTcpAcceptors(...)`
- `OpenLocalAcceptors(...)`

## Middleware Configuration Knobs

The most relevant participant transport settings are in the middleware configuration section:

- `RegistryUri`
- `ConnectAttempts`
- `ConnectTimeoutSeconds`
- `EnableDomainSockets`
- `RegistryAsFallbackProxy`
- `AcceptorUris`
- `TcpNoDelay`
- `TcpQuickAck`
- `TcpSendBufferSize`
- `TcpReceiveBufferSize`

These matter in different ways:

- `RegistryUri` controls how participants find the registry
- `ConnectAttempts` and `ConnectTimeoutSeconds` shape connection and handshake behavior
- `EnableDomainSockets` changes whether local-domain paths are used at all
- `RegistryAsFallbackProxy` controls whether proxy fallback capability is advertised
- `AcceptorUris` lets developers replace default ephemeral listen endpoints with explicit ones
- the TCP settings tune socket behavior rather than discovery logic

The most important "special case" setting is `AcceptorUris`, because it can make peer-to-peer connectivity deterministic across firewalls, containers, VMs, or routed networks.

## Why Endpoint Transformation Exists

Participants may advertise endpoints that are not directly useful to every other participant.
Typical examples:

- catch-all addresses like `0.0.0.0`
- loopback addresses only useful to local peers
- local-domain socket paths meaningful only on the same host

`TransformAcceptorUris(...)` exists to adapt the advertised participant endpoints to a specific audience participant.

What it does at a high level:

- inspects the source connection path and audience connection path
- rewrites catch-all TCP acceptors into concrete addresses where appropriate
- preserves local-domain endpoints when they make sense
- orders resulting endpoints by preference

The current ordering policy prefers:

- local-domain endpoints first
- then loopback or non-local TCP depending on whether the audience is local

This is an important file when debugging "the registry told me to connect somewhere unusable" type issues.

## Registry Connection Step

Before a participant can talk to peers, it must connect to the registry.

The participant builds a synthetic `VAsioPeerInfo` for the registry using:

- a local-domain endpoint if domain sockets are enabled
- the configured `silkit://host:port` connect URI converted to a TCP endpoint

It then tries to connect using `ConnectPeer`.

If registry connection fails, the current implementation logs:

- that the registry could not be reached
- which URIs were attempted
- hints about domain sockets, host resolution, and configuration

The participant then fails fast with a transport-level error.

## Discovery Of Known Participants

Once connected to the registry, the participant announces itself.
The registry handshake gives the participant the set of already known peers.

The next stage is handled by `ConnectKnownParticipants`.

Its responsibilities are:

- store the known participant set
- create a peer state tracker for each discovered participant
- start direct connection attempts to all of them
- track progress until either all replies are received or at least one peer fails completely

The participant-side promises in `VAsioConnection` then translate this into user-visible success or failure of simulation join.

## Direct Connect, Remote Connect, Proxy Fallback

For each known participant, the current connection order is:

1. direct connect
2. remote connect request
3. registry proxy fallback

### Direct Connect

`ConnectKnownParticipants::Peer::StartConnecting()` starts a direct connect with `ConnectPeer`.
On success:

- a `VAsioPeer` is created
- the peer is registered with the connection
- the handshake proceeds while waiting for the participant announcement reply

This is the preferred and expected path.

### Remote Connect Request

If direct connect fails, SIL Kit may request that the remote participant connects back instead.

This only works when:

- local configuration enables the capability
- the remote participant also advertises support for it

In code, this is the `RequestParticipantConnection` capability path.

If supported:

- a `RemoteParticipantConnectRequest` is sent through the registry
- a timer is started while waiting for the remote side to act
- a successful remote connection becomes a regular `VAsioPeer`

This path matters in asymmetric networking setups where one side can initiate but the other side cannot be reached directly from the first side.

### Registry Proxy Fallback

If direct connect and remote connect are not available or do not succeed, SIL Kit may fall back to proxy messaging through the registry.

This only works when:

- proxy fallback is enabled in local configuration
- the remote participant also advertises proxy capability

In code, this is the `ProxyMessage` capability path.

If proxy fallback is possible:

- a `VAsioProxyPeer` is created
- the registry remains in the message path for that peer relation
- handshake completion can still proceed, but the communication path is now slower and less direct

This is why the runtime emits warnings when direct connect fails and the registry must proxy messages.

## Proxy Message Behavior

Proxy messages are handled explicitly in `VAsioConnection::ReceiveProxyMessage(...)`.

Important behavior:

- proxy messages are ignored if the feature is disabled by configuration
- when acting as proxy, SIL Kit forwards the payload to the target peer
- proxy source-to-destination mappings are tracked
- on disconnect, empty proxy messages can be emitted so destinations learn that the proxied source disappeared

This disconnect propagation is easy to miss and matters when debugging cleanup behavior for proxied peers.

## Capabilities Drive Fallback Decisions

Connection fallback is not based only on local preference. It also depends on advertised capabilities.

Current capability decisions include:

- `ProxyMessage` if `registryAsFallbackProxy` is enabled
- `RequestParticipantConnection` if remote participant connection is enabled in configuration

That means many "why didn't it use proxy" or "why didn't it request reverse connection" questions are really capability-advertising questions.

Start in these places:

- `MakeCapabilitiesFromConfiguration(...)`
- `TryRemoteConnectRequest(...)`
- `TryProxyConnect(...)`

## Failure Modes To Recognize

There are three common classes of failures.

### Registry Reachability Failure

Symptoms:

- participant cannot connect to the registry at all
- logs mention failed attempts to connect to the registry

Typical causes:

- wrong `RegistryUri`
- hostname not resolvable from the participant host
- registry bound only to local sockets or wrong interface
- firewall, NAT, VM, Docker, or WSL boundary issues

### Participant Reachability Failure

Symptoms:

- registry connection succeeds
- timeout occurs while waiting for known participants
- logs list specific unreachable participants and their advertised endpoints

Typical causes:

- advertised ports not reachable from peer hosts
- domain sockets advertised where only TCP would work
- loopback or catch-all addressing mismatched to the actual network topology

### Proxy Fallback Warning

Symptoms:

- direct connect warning appears
- communication still works, but via registry proxy

Typical causes:

- direct peer connectivity blocked by topology or firewall
- remote-connect path unavailable or unsupported
- proxy capability still enabled, allowing degraded operation

This is a degraded but not necessarily fatal mode.

## Practical Debugging Order

When diagnosing transport issues, this order is usually effective:

1. Verify the participant can reach the registry.
2. Verify the registry is listening on the intended address family and interface.
3. Inspect advertised acceptor URIs for the failing participant.
4. Check whether domain sockets, loopback, or catch-all addresses make sense for the topology.
5. Check whether direct connect, remote connect, or proxy capability should be available.
6. Only after that inspect protocol-specific service code.

In practice, many “PubSub is broken” or “CAN does not work” issues are actually transport reachability problems below the service layer.

## Where To Start For Specific Changes

If you need to change participant startup transport behavior:

- start in `VAsioConnection::JoinSimulation(...)`

If you need to change how peers are connected:

- start in `ConnectKnownParticipants.*`
- then inspect `ConnectPeer.*`

If you need to change remote-connect fallback:

- start in `RemoteConnectionManager.*`
- then inspect `TryRemoteConnectRequest(...)`

If you need to change proxy behavior:

- inspect `TryProxyConnect(...)`
- inspect `VAsioProxyPeer.*`
- inspect `ReceiveProxyMessage(...)`

If you need to change endpoint rewriting or address selection:

- inspect `TransformAcceptorUris.*`
- inspect acceptor preparation in `VAsioConnection.*`

## Related Pages

- [Core Architecture](./core-architecture.md)
- [Repository Layout](./repository-layout.md)
- [Build and Test](./build-and-test.md)
- [Developer Wiki Front Page](./README.md)
