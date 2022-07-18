// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

namespace SilKit {
namespace Services {
namespace Orchestration {

struct NextSimTask;

struct ParticipantCommand;
struct SystemCommand;
struct ParticipantStatus;
struct WorkflowConfiguration;

class ILifecycleServiceNoTimeSync;
class ILifecycleServiceWithTimeSync;
class ITimeSyncService;

class ISystemMonitor;

class ISystemController;

class ITimeProvider;
} // namespace Orchestration
} // namespace Services
} // namespace SilKit
