#!/usr/bin/env bash
# Shared helpers for project scripts. Source, don't execute.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
LOCAL_DIR="$ROOT/.local"                       # git-ignored; per-checkout tools live here

# Pinned toolchain (D-F002): ARM GNU Toolchain, same GCC 13.2.1 as CI (Ubuntu 24.04 gcc-arm-none-eabi 13.2.rel1).
ARM_TC_VERSION="13.2.rel1"
ARM_TC_NAME="arm-gnu-toolchain-${ARM_TC_VERSION}-darwin-arm64-arm-none-eabi"
ARM_TC_URL="https://developer.arm.com/-/media/Files/downloads/gnu/${ARM_TC_VERSION}/binrel/${ARM_TC_NAME}.tar.xz"
TOOLCHAIN_DIR="$LOCAL_DIR/arm-gnu-toolchain"   # passed to make as TOOLCHAIN=

BREW_FORMULAE=(libpng pkg-config coreutils)
MIN_FREE_GB=3

if [ -t 1 ]; then
  C_OK=$'\033[32m'; C_WARN=$'\033[33m'; C_ERR=$'\033[31m'; C_DIM=$'\033[2m'; C_OFF=$'\033[0m'
else
  C_OK=""; C_WARN=""; C_ERR=""; C_DIM=""; C_OFF=""
fi
ok()   { printf '%s✔%s %s\n' "$C_OK" "$C_OFF" "$*"; }
warn() { printf '%s!%s %s\n' "$C_WARN" "$C_OFF" "$*"; }
fail() { printf '%s✘%s %s\n' "$C_ERR" "$C_OFF" "$*"; }
info() { printf '%s  %s%s\n' "$C_DIM" "$*" "$C_OFF"; }
die()  { fail "$*"; exit 1; }

is_macos()  { [ "$(uname -s)" = "Darwin" ]; }
is_arm64()  { [ "$(uname -m)" = "arm64" ] && ! { is_macos && [ "$(sysctl -in sysctl.proc_translated 2>/dev/null || echo 0)" = "1" ]; }; }
have()      { command -v "$1" >/dev/null 2>&1; }
brew_bin()  { if have brew; then command -v brew; elif [ -x /opt/homebrew/bin/brew ]; then echo /opt/homebrew/bin/brew; fi; }

toolchain_gcc() { echo "$TOOLCHAIN_DIR/bin/arm-none-eabi-gcc"; }
toolchain_ok()  { [ -x "$(toolchain_gcc)" ] && "$(toolchain_gcc)" --version 2>/dev/null | head -1 | grep -q '13\.2\.1'; }

free_gb() { df -Pk "$ROOT" | awk 'NR==2 {printf "%d", $4/1024/1024}'; }

ask_yes() {  # ask_yes "question" -> 0 if yes. Non-interactive (CI or --yes) answers yes only when ASSUME_YES=1.
  if [ "${ASSUME_YES:-0}" = "1" ]; then return 0; fi
  if [ ! -t 0 ]; then return 1; fi
  local reply; read -r -p "$1 [y/N] " reply; [[ "$reply" =~ ^[Yy]$ ]]
}

ROM_OUT="$ROOT/pokeemerald.gba"                # upstream's output name
ROM_COPY="$ROOT/build/pokemon-ascension.gba"   # friendly copy (build/ is git-ignored)

cpu_count() { if is_macos; then sysctl -n hw.ncpu; else nproc; fi; }

# Arguments to pass to make so it uses the right toolchain.
make_toolchain_args() {
  if toolchain_ok; then
    echo "TOOLCHAIN=$TOOLCHAIN_DIR"
  elif have arm-none-eabi-gcc; then
    echo ""                                    # system toolchain on PATH (Linux/CI)
  else
    die "No ARM toolchain found. Run ./scripts/setup.sh first."
  fi
}
