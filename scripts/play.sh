#!/usr/bin/env bash
# Build the ROM, then open it in mGBA.
# Usage: ./scripts/play.sh [--release] [--no-build]
#        ./scripts/play.sh --lab | --scenario ID    (Battle Lab: available from F007 / F011)
# Game saves are written by mGBA next to the ROM: build/pokemon-ascension.sav
source "$(dirname "$0")/lib/common.sh"
build_args=(); do_build=1; lab=0
for a in "$@"; do
  case "$a" in
    --release)  build_args+=(--release) ;;
    --no-build) do_build=0 ;;
    --lab) lab=1 ;;
    --scenario|--scenario=*) die "Saved scenarios arrive in F011. For now: ./scripts/play.sh --lab, then L + SELECT on the title screen." ;;
    -h|--help) sed -n '2,5p' "$0"; exit 0 ;;
    *) die "Unknown option: $a" ;;
  esac
done
if [ "$do_build" = 1 ]; then "$ROOT/scripts/build.sh" ${build_args[@]+"${build_args[@]}"}; fi
[ -f "$ROM_COPY" ] || die "No ROM at build/pokemon-ascension.gba. Run without --no-build."
if is_macos; then
  if [ -d /Applications/mGBA.app ] || [ -d "$HOME/Applications/mGBA.app" ]; then
    ok "Opening in mGBA"
    open -a mGBA "$ROM_COPY"
  else
    die "mGBA is not installed. Install with: brew install --cask mgba"
  fi
elif have mgba-qt; then
  mgba-qt "$ROM_COPY" &
elif have mgba; then
  mgba "$ROM_COPY" &
else
  die "mGBA not found. Install it from https://mgba.io/downloads.html"
fi
if [ "$lab" = 1 ]; then
  [ "${build_args[*]:-}" = "--release" ] && die "The Battle Lab is not in release builds; drop --release."
  ok "Battle Lab: on the title screen hold L (A key) and press SELECT (Backspace)."
fi
info "Controls: arrows = D-pad, X = A, Z = B, A = L, S = R, Enter = Start, Backspace = Select (mGBA defaults)."
info "Save in-game; the save file is build/pokemon-ascension.sav."
