#!/usr/bin/env bash
# Build the ROM. Prints its path on success; never leaves a stale ROM after a failure.
# Usage: ./scripts/build.sh [--release] [--debug] [--clean]
#   --release  optimised build with debug menus disabled (what players get)
#   --debug    build with debug symbols
#   --clean    remove previous build outputs first (slow full rebuild)
source "$(dirname "$0")/lib/common.sh"
target=all; clean=0
for a in "$@"; do
  case "$a" in
    --release) target=release ;;
    --debug)   target=debug ;;
    --clean)   clean=1 ;;
    -h|--help) sed -n '2,7p' "$0"; exit 0 ;;
    *) die "Unknown option: $a" ;;
  esac
done
cd "$ROOT"
tc="$(make_toolchain_args)"
MAKE_BIN="$(make_bin)"
make_ok || die "GNU make $MIN_MAKE_VERSION or newer is required (found $(make_version) at $MAKE_BIN). Run ./scripts/setup.sh."
rom_src="$ROM_OUT"
[ "$target" = release ] && rom_src="$ROOT/pokeemerald-release.gba"   # upstream names release builds differently
rm -f "$rom_src" "$ROM_COPY"
[ "$clean" = 1 ] && { info "Cleaning previous build outputs"; "$MAKE_BIN" tidy >/dev/null; }
jobs="$(cpu_count)"
# Talent trees (F015): your edits to data/ascension/talent_trees.json are checked and compiled into the game.
python3 "$ROOT/tools/ascension/talent_trees.py" check >/dev/null || die "data/ascension/talent_trees.json has errors: run python3 tools/ascension/talent_trees.py check"
python3 "$ROOT/tools/ascension/talent_trees.py" compile >/dev/null
info "make -j$jobs $target ${tc:+(project toolchain)}"
start=$(date +%s)
log="$ROOT/build/last-build.log"; mkdir -p "$ROOT/build"
if ! "$MAKE_BIN" -j"$jobs" $tc "$target" >"$log" 2>&1; then
  rm -f "$rom_src" "$ROM_COPY"
  tail -n 30 "$log"
  die "Build failed after $(( $(date +%s) - start ))s. Full log: build/last-build.log"
fi
[ -f "$rom_src" ] || die "Build reported success but $rom_src is missing. Log: build/last-build.log"
cp "$rom_src" "$ROM_COPY"
grep -A3 'Memory region' "$log" || true
ok "Built ($target) in $(( $(date +%s) - start ))s: build/pokemon-ascension.gba"
info "SHA-1 $(shasum -a 1 "$ROM_COPY" 2>/dev/null | awk '{print $1}' || sha1sum "$ROM_COPY" | awk '{print $1}')"
