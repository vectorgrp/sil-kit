# I2C Bus Support for SIL Kit — Design Document

## Context

I2C (Inter-Integrated Circuit) is a two-wire (SDA + SCL) synchronous serial bus widely used in automotive ECUs for sensor/actuator communication. Adding I2C to SIL Kit allows ECU software that drives I2C peripherals to participate in distributed HIL/SIL simulations. The goal is a functional-level simulation (correct data, address routing, ACK/NACK presence) — not timing-accurate bit-level simulation. This mirrors the fidelity trade-off already accepted for CAN and LIN.

### Design Decisions (resolved)
1. **Slave read-response**: pre-loaded buffer via `SetReadResponse()` (like LIN's `UpdateTxBuffer`)
2. **Combined transactions (Sr)**: two separate `SendFrame` calls — no dedicated combined type
3. **Speed mode**: included in `I2cControllerConfig` for documentation/tracing; ignored by trivial sim, useful for future network simulators
4. **General Call (0x00)**: implemented in trivial sim — broadcast to all slave controllers on the network
5. **Internal transport**: dedicated typed wire messages (same pattern as CAN/LIN), **not** SIL Kit's DataPublisher/RpcClient infrastructure

### Why not use internal pub/sub or RPC as transport
It was considered to implement the internal message routing using SIL Kit's own DataPublisher/DataSubscriber or RpcClient/RpcServer (with a reserved topic naming convention and special media type) to avoid writing new wire message types and serdes. Rejected for three reasons:
- **Netsim interception breaks**: `SimBehaviorDetailed` redirects `SendMsg()` to the simulator participant; if the controller internally owns a DataPublisher, the netsim hook would need to intercept at the DataPublisher level — the wrong abstraction layer.
- **Tracing is degraded**: DataPublisher payloads appear as opaque blobs in ETF/MDF4 traces; dedicated wire message types are recognized and decoded as I2C frames automatically.
- **Service identity is confused**: the I2C controller would appear in service discovery as both a `Controller` (for netsim) and as internal DataPublisher/RpcClient services, polluting the topology view.
The code saved (mainly `WireI2cMessages.hpp` ~50 lines, serdes ~100 lines, `SilKitMsgTraits` entries ~10 lines) does not justify these costs.

---

## I2C Protocol Summary

- **7-bit addressing** (standard) + **10-bit** (extended); address 0x00 = General Call (broadcast)
- Transaction: `START | ADDR[6:0] R/W̄ | ACK | [DATA | ACK]… | STOP`
- Write = master sends data to slave; Read = slave sends data to master (pre-loaded)
- Combined read = two `SendFrame` calls: `Write(reg_addr)` then `Read(len)` — no Sr atomicity in simulation
- Multi-master arbitration and clock stretching are **not modeled** in trivial sim

---

## Public API

### `I2cDatatypes.hpp`

```cpp
using I2cAddress = uint16_t;  // 0x00–0x7F (7-bit) or 0x000–0x3FF (10-bit)

enum class I2cAddressMode : uint8_t {
    AddressMode7Bit  = 0,
    AddressMode10Bit = 1,
};

enum class I2cTransferDirection : uint8_t {
    Write = 0,
    Read  = 1,
};

enum class I2cSpeedMode : uint8_t {
    Standard  = 0,  // 100 kbit/s
    Fast      = 1,  // 400 kbit/s
    FastPlus  = 2,  // 1 Mbit/s
    HighSpeed = 3,  // 3.4 Mbit/s
};

enum class I2cTransmitStatus : uint8_t {
    Transmitted      = 0,  // slave ACKed all bytes
    AddressNak       = 1,  // no slave at this address
    DataNak          = 2,  // slave NACKed a data byte (transfer-level only)
    ArbitrationLost  = 3,  // only injectable via network simulator
    BusError         = 4,  // only injectable via network simulator
};

struct I2cFrame {
    I2cAddress           address;
    I2cAddressMode       addressMode;
    I2cTransferDirection direction;
    Util::Span<const uint8_t> data;  // empty for Read requests
};

struct I2cFrameEvent {
    std::chrono::nanoseconds timestamp;
    I2cFrame                 frame;
    TransmitDirection        direction;   // Tx or Rx from this controller's perspective
    void*                    userContext;
};

struct I2cFrameTransmitEvent {
    std::chrono::nanoseconds timestamp;
    I2cAddress               address;
    I2cTransmitStatus        status;
    void*                    userContext;
};

struct I2cControllerConfig {
    I2cControllerMode mode;             // Master or Slave
    I2cAddress        slaveAddress;     // only for Slave mode (ignored for Master)
    I2cAddressMode    slaveAddressMode; // only for Slave mode
    I2cSpeedMode      speedMode;        // informational; trivial sim ignores
};
```

### `II2cController.hpp`

```cpp
class II2cController {
public:
    // Called once before use; broadcasts WireI2cControllerConfig to the network
    virtual void Init(I2cControllerConfig config) = 0;

    // Master: initiate a write or read transfer
    virtual void SendFrame(const I2cFrame& frame, void* userContext = nullptr) = 0;

    // Slave: pre-load data to return when master issues a Read to this address
    virtual void SetReadResponse(Util::Span<const uint8_t> data) = 0;

    virtual HandlerId AddFrameHandler(FrameHandler handler,
                                      DirectionMask directionMask = DirectionMask::RxTx) = 0;
    virtual HandlerId AddFrameTransmitHandler(FrameTransmitHandler handler) = 0;
    virtual void RemoveFrameHandler(HandlerId) = 0;
    virtual void RemoveFrameTransmitHandler(HandlerId) = 0;
};
```

---

## Wire Messages (`WireI2cMessages.hpp`)

```cpp
struct WireI2cFrame {
    I2cAddress               address;
    I2cAddressMode           addressMode;
    I2cTransferDirection     direction;
    Util::SharedVector<uint8_t> data;  // payload for Write or slave Read response
};

struct WireI2cFrameEvent {
    std::chrono::nanoseconds timestamp;
    WireI2cFrame             frame;
    TransmitDirection        direction;
    void*                    userContext;
};

struct I2cAcknowledge {
    std::chrono::nanoseconds timestamp;
    I2cAddress               address;
    I2cTransmitStatus        status;
    void*                    userContext;
};

struct WireI2cControllerConfig {
    I2cControllerMode mode;
    I2cAddress        slaveAddress;
    I2cAddressMode    slaveAddressMode;
    I2cSpeedMode      speedMode;
};
```

---

## Internal Message Interfaces

### `IMsgForI2cController.hpp`
Controller receives: `WireI2cFrameEvent`, `I2cAcknowledge`, `WireI2cControllerConfig`
Controller sends: `WireI2cFrameEvent`, `I2cAcknowledge`, `WireI2cControllerConfig`

### `IMsgForI2cSimulator.hpp`
Simulator receives: `WireI2cFrameEvent`, `WireI2cControllerConfig`
Simulator sends: `WireI2cFrameEvent`, `I2cAcknowledge`

---

## SimBehavior — Trivial Mode

The key difference from CAN's trivial sim: I2C requires **address-routing** and **slave read-response injection**.

The `SimBehaviorTrivial` tracks all `WireI2cControllerConfig` announcements for slave controllers on the same network (populated via `ReceiveMsg(WireI2cControllerConfig)`) — this is the slave address registry.

### Write transfer (`SendFrame({addr, Write, data})`)
1. Broadcast `WireI2cFrameEvent(direction=Tx)` to all on network
2. Slaves whose `slaveAddress == addr` (or `addr == 0x00` for General Call) fire `FrameHandler(direction=Rx)`
3. If any matching slave is known in registry → auto-generate `I2cAcknowledge{Transmitted}` to sender
4. If `addr == 0x00` → always generate `I2cAcknowledge{Transmitted}` (General Call, no NACK possible)
5. If no slave matched → auto-generate `I2cAcknowledge{AddressNak}`
6. Master fires `FrameTransmitHandler`

### Read transfer (`SendFrame({addr, Read, empty_data})`)
1. Broadcast `WireI2cFrameEvent(direction=Tx, Read)` to all on network
2. Matching slave fires `FrameHandler(direction=Rx)` — slave's pre-loaded `SetReadResponse()` buffer is used
3. Trivial sim injects `WireI2cFrameEvent(Tx from slave, data=slave_read_buf)` back onto bus
4. Master receives that event → fires `FrameHandler(direction=Rx)` with the read data
5. Auto-generate `I2cAcknowledge{Transmitted}` to master
6. Master fires `FrameTransmitHandler`

If no slave is registered: auto-generate `I2cAcknowledge{AddressNak}`, master fires `FrameTransmitHandler`.
If slave has no pre-loaded buffer: auto-generate `I2cAcknowledge{AddressNak}` (slave can't respond).

### General Call (address 0x00, Write only)
- Broadcast reaches all slave controllers regardless of their configured address
- Auto-generates `I2cAcknowledge{Transmitted}` unconditionally (per spec, slaves cannot NACK a General Call)
- Read to 0x00 is invalid per I2C spec — treat as `I2cAcknowledge{BusError}`

---

## SimBehavior — Detailed Mode

Identical routing to CAN/LIN detailed mode: all `SendMsg()` variants unicast to the network simulator participant. `AllowReception()` only allows from that simulator.

Network simulator hook struct:
```cpp
struct SimulatedI2cControllerFunctions {
    std::function<void(II2cController*, const I2cFrame&)>           OnFrameRequest;
    std::function<void(II2cController*, const I2cControllerConfig&)> OnControllerConfig;
};
```

---

## Accuracy Constraints

| Feature | Trivial sim | Detailed sim |
|---|---|---|
| Data transfer (write) | ✅ | ✅ |
| Address routing + ACK presence | ✅ | ✅ |
| General Call broadcast | ✅ | ✅ |
| Byte-level ACK/NACK | ❌ transfer-level only | ✅ |
| Clock stretching | ❌ | ✅ |
| Multi-master arbitration | ❌ | ✅ |
| Real-time bit timing | ❌ (virtual time only) | ❌ (virtual time only) |
| Sr atomicity | ❌ (two separate calls) | implementation-defined |

The simulation is appropriate for **functional software testing** of I2C drivers, not for timing compliance verification.

---

## Files to Create / Modify

### New files

```
SilKit/include/silkit/services/i2c/
  I2cDatatypes.hpp
  II2cController.hpp
  string_utils.hpp
  fwd_decl.hpp
  all.hpp

SilKit/include/silkit/capi/
  I2c.h

SilKit/source/services/i2c/
  I2cController.hpp / .cpp
  ISimBehavior.hpp
  SimBehavior.hpp / .cpp
  SimBehaviorTrivial.hpp / .cpp   ← address registry, Write/Read routing, General Call
  SimBehaviorDetailed.hpp / .cpp
  IMsgForI2cController.hpp
  IMsgForI2cSimulator.hpp
  I2cSerdes.hpp / .cpp
  I2cDatatypesUtils.hpp / .cpp
  CMakeLists.txt

SilKit/source/wire/i2c/
  WireI2cMessages.hpp

SilKit/source/capi/
  CapiI2c.cpp
```

### Files to modify

| File | Change |
|---|---|
| `SilKit/source/core/internal/ServiceConfigKeys.hpp` | Add `controllerTypeI2c = "I2C"` |
| `SilKit/source/core/participant/Participant.hpp` | Add `CreateI2cController(name, network)` |
| `SilKit/source/core/participant/Participant_impl.hpp` | Implement `CreateI2cController()` |
| `SilKit/source/core/internal/traits/SilKitMsgTraits.hpp` | Register wire message types |
| `SilKit/source/services/CMakeLists.txt` | Link `SilKitServiceI2c` |
| `SilKit/include/silkit/services/all.hpp` | Include `i2c/all.hpp` |
| `SilKit/include/silkit/capi/SilKit.h` | Include `I2c.h` |
| `SilKit/source/capi/CMakeLists.txt` | Add `CapiI2c.cpp` |
| `SilKit/include/silkit/experimental/netsim/` | Add `ISimulatedI2cController` + functions struct |
