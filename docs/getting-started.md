# Getting Started

## Prerequisites

- Linux environment
- CMake 3.25+
- C++20 toolchain
- Ninja

For dependency installation on supported distros:

```bash
./scripts/install-deps.sh
```

## Build

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

## Run binaries

- Main service:
  ```bash
  ./build/openrcc
  ```
- SOAP shim:
  ```bash
  ./build/openrcc-soap
  ```
- Tutorial mode:
  ```bash
  ./build/openrcc --tutorial --non-interactive
  ```

## Default service config path

OpenRCC reads:

- `/etc/openrcc/service.toml`

If the file is absent, defaults from `ServiceConfig` are used.
