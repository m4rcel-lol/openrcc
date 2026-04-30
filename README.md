# OpenRCC

OpenRCC is an educational clean-room scaffold for a distributed game server control architecture.

It includes:
- A canonical **gRPC control-plane service** for job lifecycle operations.
- A **job manager + FSM** model for predictable state transitions.
- A **Luau sandbox** skeleton with basic capability restrictions.
- A lightweight **SOAP bridge** executable showing SOAP-to-gRPC mapping.
- A deterministic **tutorial mode** for architecture walkthroughs.

## Documentation

- [Docs index](./docs/README.md)
- [Getting started](./docs/getting-started.md)
- [Architecture overview](./docs/architecture.md)
- [API overview](./docs/api.md)
- [Configuration](./docs/configuration.md)
- [Building on macOS Apple Silicon](./docs/macos-build.md)

## Quick start

### 1) Install dependencies

Use the helper script for supported Linux families and macOS Apple Silicon:

```bash
./scripts/install-deps.sh
```

The script currently supports Arch-family Linux, Fedora-family Linux, and macOS with Homebrew.

### 2) Configure and build

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

### 3) Run tests

```bash
ctest --test-dir build --output-on-failure
```

## Running

### Service

```bash
./build/openrcc
```

The service reads config from:
- `/etc/openrcc/service.toml`

If missing, built-in defaults are used.

### Tutorial mode

```bash
./build/openrcc --tutorial
```

For CI/non-interactive output:

```bash
./build/openrcc --tutorial --non-interactive
```

### SOAP shim demo executable

```bash
./build/openrcc-soap
```

## Repository layout

- `src/service/` — gRPC server startup and RPC implementation
- `src/job_manager/` — job lifecycle FSM and manager
- `src/luau_runtime/` — Luau sandbox skeleton
- `src/soap/` — SOAP-to-gRPC bridge skeleton
- `src/tutorial/` — scripted tutorial mode
- `proto/` — protobuf service contracts
- `test/` — unit tests
- `systemd/` — service unit file
- `docs/` — project documentation

## Notes

OpenRCC is intentionally educational and scaffold-oriented; several components are placeholders designed to explain architecture and safe-by-default control-plane patterns.
