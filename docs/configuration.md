# Configuration

OpenRCC reads runtime configuration from:

- `/etc/openrcc/service.toml`

## Supported keys

### `bind_address`
- Type: string
- Default: `"0.0.0.0:50051"`
- Purpose: gRPC listening address.

### `default_tick_rate_hz`
- Type: unsigned integer
- Default: `30`
- Purpose: default simulation/update tick target.
- Must be greater than zero.

### `json_logs`
- Type: boolean
- Default: `true`
- Purpose: toggles logging mode setup.
- Accepted values: `true`, `false`, `1`, `0`.

Invalid supported values fail startup with a configuration error instead of
silently falling back to an unsafe or surprising value.

## Example

```toml
bind_address = "0.0.0.0:50051"
default_tick_rate_hz = 30
json_logs = true
```

## systemd

A unit file is provided at:

- [`/systemd/openrcc.service`](../systemd/openrcc.service)

It runs `/usr/bin/openrcc` with `Type=notify` and restart-on-failure behavior.
