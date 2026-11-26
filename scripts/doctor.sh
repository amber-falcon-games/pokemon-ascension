#!/usr/bin/env bash
# Diagnose the local toolchain. Read-only: changes nothing.
# Usage: ./scripts/doctor.sh      Exit code: 0 if ready to build, 1 if something required is missing.
source "$(dirname "$0")/lib/common.sh"
missing=0
need() { fail "$1"; info "fix: $2"; missing=1; }

echo "Pokémon Ascension — environment check"
if is_macos; then
  ok "macOS $(sw_vers -productVersion)"
  if is_arm64; then ok "Apple Silicon (native arm64)"; else warn "Not native arm64 (Intel Mac or Rosetta shell); scripts target Apple Silicon"; fi
  if xcode-select -p >/dev/null 2>&1; then ok "Xcode Command Line Tools"; else need "Xcode Command Line Tools missing" "xcode-select --install"; fi
  BREW="$(brew_bin)"
  if [ -n "$BREW" ]; then
    ok "Homebrew $("$BREW" --version | head -1 | awk '{print $2}')"
    for f in "${BREW_FORMULAE[@]}"; do
      if "$BREW" list --formula "$f" >/dev/null 2>&1; then ok "brew: $f"; else need "brew: $f not installed" "./scripts/setup.sh (or: brew install $f)"; fi
    done
  else
    need "Homebrew missing" "install from https://brew.sh (asks for your password), then ./scripts/setup.sh"
  fi
else
  warn "$(uname -s) detected: these scripts target macOS. On Linux, follow INSTALL.md (CI uses Ubuntu packages)."
fi
if have python3; then ok "python3 $(python3 --version | awk '{print $2}')"; else need "python3 missing" "comes with Xcode Command Line Tools"; fi
if have make || have gmake; then
  if make_ok; then ok "GNU make $(make_version) ($(make_bin))"; else need "GNU make $(make_version) is too old (need $MIN_MAKE_VERSION+; macOS ships 3.81)" "./scripts/setup.sh (or: brew install make)"; fi
else
  need "make missing" "comes with Xcode Command Line Tools"
fi
if have git; then ok "git $(git --version | awk '{print $3}')"; else need "git missing" "comes with Xcode Command Line Tools"; fi
if toolchain_ok; then
  ok "ARM toolchain $ARM_TC_VERSION ($("$(toolchain_gcc)" --version | head -1 | awk '{print $NF}')) in .local/"
else
  need "ARM toolchain $ARM_TC_VERSION not found in .local/arm-gnu-toolchain" "./scripts/setup.sh"
fi
gb="$(free_gb)"
if [ "$gb" -ge "$MIN_FREE_GB" ]; then ok "Free disk: ${gb} GB"; else need "Free disk: ${gb} GB (need ${MIN_FREE_GB} GB)" "free up space"; fi
if is_macos; then
  if [ -d "/Applications/mGBA.app" ] || [ -d "$HOME/Applications/mGBA.app" ]; then ok "mGBA (for play)"; else warn "mGBA not installed (needed to play; not to build): brew install --cask mgba"; fi
fi
echo
if [ "$missing" = 0 ]; then ok "Ready to build."; else fail "Not ready: fix the items above, then re-run ./scripts/doctor.sh"; fi
exit "$missing"
