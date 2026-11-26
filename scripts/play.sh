#!/usr/bin/env bash
# Build the ROM, then open it in mGBA.
# Usage: ./scripts/play.sh [--release] [--no-build]
#        ./scripts/play.sh --lab              Battle Lab with every scenario in lab/scenarios/
#        ./scripts/play.sh --scenario ID      same, opening with that scenario (e.g. LAB-009)
# Game saves are written by mGBA next to the ROM: build/pokemon-ascension.sav.
# The Battle Lab runs from its own copy, build/pokemon-ascension-lab.gba (own .sav; D022).
source "$(dirname "$0")/lib/common.sh"
build_args=(); do_build=1; lab=0; scenario=""
while [ $# -gt 0 ]; do
  case "$1" in
    --release)  build_args+=(--release); shift ;;
    --no-build) do_build=0; shift ;;
    --lab) lab=1; shift ;;
    --scenario) [ $# -ge 2 ] || die "--scenario needs an ID, e.g. --scenario LAB-009"; lab=1; scenario="$2"; shift 2 ;;
    --scenario=*) lab=1; scenario="${1#--scenario=}"; shift ;;
    -h|--help) sed -n '2,7p' "$0"; exit 0 ;;
    *) die "Unknown option: $1" ;;
  esac
done
if [ "$lab" = 1 ] && [ "${build_args[*]:-}" = "--release" ]; then
  die "The Battle Lab is not in release builds; drop --release."
fi
if [ "$do_build" = 1 ]; then "$ROOT/scripts/build.sh" ${build_args[@]+"${build_args[@]}"}; fi
[ -f "$ROM_COPY" ] || die "No ROM at build/pokemon-ascension.gba. Run without --no-build."

rom="$ROM_COPY"
if [ "$lab" = 1 ]; then
  # Scenario files -> reserved block of a Lab-only ROM copy (no save memory used).
  if toolchain_ok; then export ARM_GCC="$(toolchain_gcc)"; fi
  lab_rom="$ROOT/build/pokemon-ascension-lab.gba"
  files=("$ROOT"/lab/scenarios/*.json)
  [ -e "${files[0]}" ] || files=()
  python3 -I "$ROOT/tools/ascension/lab_scenario.py" inject --rom "$ROM_COPY" --out "$lab_rom" \
    --open "${scenario:-builtin}" ${files[@]+"${files[@]}"} || die "Could not prepare the Lab ROM (see above)."
  rom="$lab_rom"
  ok "Battle Lab ROM ready: opens straight into the Lab${scenario:+ with $scenario}. Hold B while it boots for the title screen."
fi

if is_macos; then
  if [ -d /Applications/mGBA.app ] || [ -d "$HOME/Applications/mGBA.app" ]; then
    ok "Opening in mGBA"
    open -a mGBA "$rom"
  else
    die "mGBA is not installed. Install with: brew install --cask mgba"
  fi
elif have mgba-qt; then
  mgba-qt "$rom" &
elif have mgba; then
  mgba "$rom" &
else
  die "mGBA not found. Install it from https://mgba.io/downloads.html"
fi
info "Controls: arrows = D-pad, X = A, Z = B, A = L, S = R, Enter = Start, Backspace = Select (mGBA defaults)."
if [ "$lab" = 1 ]; then
  info "Lab saves nothing to your campaign save; it has its own file, build/pokemon-ascension-lab.sav."
else
  info "Save in-game; the save file is build/pokemon-ascension.sav."
fi
