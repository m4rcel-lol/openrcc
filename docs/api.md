# API Overview

The canonical API is defined in [`proto/rcc_control.proto`](../proto/rcc_control.proto).

## Service: `openrcc.v1.OpenRccControl`

### `OpenJob(OpenJobRequest) -> OpenJobResponse`
Creates a new job context and returns assigned `job_id`.

### `ExecuteScript(ExecuteScriptRequest) -> ExecuteScriptResponse`
Submits Luau source to a target job. Response indicates acceptance.

### `CloseJob(CloseJobRequest) -> CloseJobResponse`
Requests close behavior:
- `force=false`: drain path
- `force=true`: forced fault/teardown path

### `GetJobStatus(JobStatusRequest) -> JobStatusResponse`
Returns state for a specific job.

### `ServerStatus(Empty) -> ServerStatusResponse`
Returns aggregate counts and service uptime.

## State values

Job state strings are generated from the FSM state enum:
- `PENDING`
- `RUNNING`
- `DRAINING`
- `CLOSED`
- `FAULTED`

## Request/response IDs

Most RPC messages include `request_id` for trace correlation and request/response pairing.
