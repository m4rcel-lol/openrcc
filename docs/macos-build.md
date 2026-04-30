# Building on macOS Apple Silicon

OpenRCC can be built natively on Apple Silicon (`arm64`) with the same CMake
project used for Linux and Windows.

## Prerequisites

- Apple Silicon Mac (`uname -m` should print `arm64`)
- Xcode Command Line Tools:
  ```bash
  xcode-select --install
  ```
- Homebrew:
  ```bash
  /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
  ```

Install build tools:

```bash
./scripts/install-deps.sh
```

The CMake build fetches pinned third-party dependencies through
`FetchContent`, so Homebrew is only required for the local build tools.

## Configure, Build, and Test

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES=arm64

cmake --build build --config Release
ctest --test-dir build --output-on-failure -C Release
```

## Install Staging Tree

To create a relocatable staging tree without writing into `/usr/local`:

```bash
rm -rf package/root
mkdir -p package/root
DESTDIR="$PWD/package/root" cmake --install build --prefix /usr/local --config Release --strip --component openrcc_runtime
```

The installed binaries are:

- `package/root/usr/local/bin/openrcc`
- `package/root/usr/local/bin/openrcc-soap`

## GitHub Actions

The `Build Installers` workflow includes a native macOS Apple Silicon job using
the `macos-15` runner, verifies `uname -m` is `arm64`, builds with
`-DCMAKE_OSX_ARCHITECTURES=arm64`, runs tests, and uploads a
`openrcc-<version>-macos-arm64.tar.gz` artifact.
