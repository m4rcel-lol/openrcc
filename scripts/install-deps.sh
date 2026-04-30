#!/usr/bin/env bash
set -euo pipefail

log() { printf '[INFO] %s\n' "$*"; }
warn() { printf '[WARN] %s\n' "$*"; }
err() { printf '[ERR ] %s\n' "$*"; }

print_status_table() {
  printf '\n%-20s | %-6s\n' 'Binary' 'Status'
  printf '%s\n' '---------------------+--------'
  for bin in "$@"; do
    if command -v "${bin}" >/dev/null 2>&1; then
      printf '%-20s | PASS\n' "${bin}"
    else
      printf '%-20s | FAIL\n' "${bin}"
    fi
  done
}

install_macos() {
  log "Detected macOS ($(uname -m))"

  if ! xcode-select -p >/dev/null 2>&1; then
    err "Xcode Command Line Tools are required. Run: xcode-select --install"
    exit 1
  fi

  if ! command -v brew >/dev/null 2>&1 && [[ -x /opt/homebrew/bin/brew ]]; then
    # shellcheck disable=SC2046
    eval "$(/opt/homebrew/bin/brew shellenv)"
  fi

  if ! command -v brew >/dev/null 2>&1; then
    err "Homebrew is required on macOS. Install it from https://brew.sh/ and rerun this script."
    exit 1
  fi

  brew install cmake ninja git
}

if [[ "$(uname -s)" == "Darwin" ]]; then
  install_macos
  print_status_table cmake ninja git clang++
  exit 0
fi

if [[ ! -f /etc/os-release ]]; then
  err "/etc/os-release not found"
  exit 1
fi

# shellcheck disable=SC1091
source /etc/os-release

OS_ID="${ID:-unknown}"
OS_LIKE="${ID_LIKE:-}"
COMBINED="${OS_ID} ${OS_LIKE}"

is_arch_family=false
is_fedora_family=false
if [[ "${COMBINED}" =~ (arch|cachyos|manjaro|endeavouros) ]]; then
  is_arch_family=true
fi
if [[ "${COMBINED}" =~ (fedora|bazzite|nobara|silverblue|kinoite) ]]; then
  is_fedora_family=true
fi

install_arch() {
  log "Detected Arch-family distribution (${OS_ID})"
  sudo pacman -Sy --needed --noconfirm \
    base-devel cmake ninja git grpc protobuf spdlog systemd
}

install_fedora() {
  log "Detected Fedora-family distribution (${OS_ID})"
  local pkgs=(
    gcc-c++ cmake ninja-build git grpc-devel protobuf-compiler protobuf-devel
    spdlog-devel systemd-devel make
  )

  if command -v rpm-ostree >/dev/null 2>&1; then
    warn "Immutable OS detected. Packages will be layered via rpm-ostree. A reboot will be required."
    sudo rpm-ostree install "${pkgs[@]}"
  else
    sudo dnf install -y "${pkgs[@]}"
  fi
}

if [[ "${is_arch_family}" == true ]]; then
  install_arch
elif [[ "${is_fedora_family}" == true ]]; then
  install_fedora
else
  err "Unsupported distribution: ID=${OS_ID}, ID_LIKE=${OS_LIKE}"
  exit 1
fi

print_status_table cmake ninja grpc_cpp_plugin protoc
