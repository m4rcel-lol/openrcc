# Architecture Overview

## Core components

- **Control-plane API (`OpenRccControl`)**
  - Defined in `proto/rcc_control.proto`
  - Exposed by `RccControlServiceImpl`

- **JobManager + JobFsm**
  - Owns tracked jobs and worker threads
  - Enforces legal job state transitions:
    - `PENDING -> RUNNING`
    - `RUNNING -> DRAINING`
    - `DRAINING -> CLOSED`
    - `RUNNING -> FAULTED`
    - `DRAINING -> FAULTED`
    - `PENDING -> FAULTED`

- **Luau sandbox**
  - Initializes a VM when Lua/Luau headers are available
  - Removes unsafe globals (`os`, `io`, `require`)
  - Installs limited `rcc` helper APIs

- **SOAP bridge**
  - Demonstrates XML extraction + mapping into protobuf requests
  - Uses local gRPC channel to keep one canonical service path

- **Tutorial mode**
  - Provides deterministic scripted walkthrough of startup, RPC, FSM, and teardown

## Runtime flow (high level)

1. Service starts and parses `/etc/openrcc/service.toml`.
2. gRPC server listens on configured bind address.
3. `OpenJob` allocates a job and transitions it to `RUNNING`.
4. `ExecuteScript` queues script source if job is runnable.
5. `CloseJob` drains or force-faults based on request.
6. `GetJobStatus` returns current state, uptime, and memory estimate.
7. `ServerStatus` returns aggregate counters + uptime.

## Shutdown behavior

- SIGINT/SIGTERM trigger graceful shutdown path.
- Service calls `DrainAll()` then waits for bounded gRPC shutdown.
- systemd notifications are used when available (`READY=1`, `STOPPING=1`).
