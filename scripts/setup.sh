#!/usr/bin/env bash
# Install project prerequisites on Apple Silicon macOS. Safe to re-run.
# Never uses sudo. Installs Homebrew formulae (after asking) and the pinned ARM toolchain into .local/.
# Usage: ./scripts/setup.sh [--yes]     --yes answers yes to the install prompts.
source "$(dirname "$0")/lib/common.sh"
[ "${1:-}" = "--yes" ] && ASSUME_YES=1

is_macos || die "setup.sh supports macOS only. On Linux see INSTALL.md."
is_arm64 || warn "Not a native arm64 shell; continuing, but the toolchain is the Apple Silicon build."

# 1. Xcode Command Line Tools (Apple installer dialog; cannot be scripted silently).
if ! xcode-select -p >/dev/null 2>&1; then
  fail "Xcode Command Line Tools are required."
  info "Run: xcode-select --install   (a dialog opens; then re-run this script)"
  exit 1
fi
ok "Xcode Command Line Tools"

# 2. Homebrew (needs your password, so we never install it for you).
BREW="$(brew_bin)"
if [ -z "$BREW" ]; then
  fail "Homebrew is required."
  info "Install it from https://brew.sh (it asks for your password), then re-run this script."
  exit 1
fi
ok "Homebrew"
to_install=()
for f in "${BREW_FORMULAE[@]}"; do "$BREW" list --formula "$f" >/dev/null 2>&1 || to_install+=("$f"); done
if [ "${#to_install[@]}" -gt 0 ]; then
  if ask_yes "Install with Homebrew: ${to_install[*]}?"; then
    "$BREW" install "${to_install[@]}"
  else
    die "Skipped. Install manually: brew install ${to_install[*]}"
  fi
fi
ok "Homebrew formulae: ${BREW_FORMULAE[*]}"

# 3. ARM GNU Toolchain, pinned, into .local/ (no system changes).
if toolchain_ok; then
  ok "ARM toolchain $ARM_TC_VERSION already installed"
else
  mkdir -p "$LOCAL_DIR/downloads"
  tarball="$LOCAL_DIR/downloads/${ARM_TC_NAME}.tar.xz"
  if [ ! -f "$tarball" ]; then
    info "Downloading ${ARM_TC_NAME} (~150 MB) from developer.arm.com"
    curl -fL --retry 3 -o "$tarball.part" "$ARM_TC_URL" && mv "$tarball.part" "$tarball"
  fi
  info "Verifying checksum against ARM's published value"
  curl -fsSL --retry 3 -o "$tarball.sha256asc" "${ARM_TC_URL}.sha256asc"
  expected="$(awk '{print $1; exit}' "$tarball.sha256asc")"
  actual="$(shasum -a 256 "$tarball" | awk '{print $1}')"
  if [ -z "$expected" ] || [ "$expected" != "$actual" ]; then
    rm -f "$tarball"
    die "Checksum mismatch for the toolchain download (expected ${expected:-none}, got $actual). Re-run to retry."
  fi
  ok "Checksum $actual"
  rm -rf "$TOOLCHAIN_DIR" "$LOCAL_DIR/extract"
  mkdir -p "$LOCAL_DIR/extract"
  tar -xJf "$tarball" -C "$LOCAL_DIR/extract"
  top="$(find "$LOCAL_DIR/extract" -mindepth 1 -maxdepth 1 -type d | head -1)"
  [ -x "$top/bin/arm-none-eabi-gcc" ] || die "Unexpected toolchain layout in $tarball"
  mv "$top" "$TOOLCHAIN_DIR" && rm -rf "$LOCAL_DIR/extract"
  # Downloaded binaries carry a quarantine flag; clear it for this folder only.
  xattr -dr com.apple.quarantine "$TOOLCHAIN_DIR" 2>/dev/null || true
  toolchain_ok || die "Toolchain installed but did not run: $("$(toolchain_gcc)" --version 2>&1 | head -1)"
  ok "ARM toolchain $ARM_TC_VERSION installed in .local/arm-gnu-toolchain"
fi

echo
"$ROOT/scripts/doctor.sh"
