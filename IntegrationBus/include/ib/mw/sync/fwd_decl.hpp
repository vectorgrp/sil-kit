// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

namespace ib {
namespace mw {
namespace sync {

struct NextSimTask;
struct Tick;
struct TickDone;
struct QuantumRequest;
struct QuantumGrant;

struct ParticipantCommand;
struct SystemCommand;
struct ParticipantStatus;

class IParticipantController;
class IIbToParticipantController;

class ISystemMonitor;
class IIbToSystemMonitor;

class ISystemController;
class IIbToSystemController;

class ISyncMaster;
class IIbToSyncMaster;

class ITimeProvider;
} // namespace sync
} // namespace mw
} // namespace ib
