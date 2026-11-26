#!/usr/bin/env bash
# Run the automated test suite (upstream's tests plus project tests). Non-zero exit on any failure.
# Usage: ./scripts/test.sh [test/path/file.c | "Test name prefix"]
source "$(dirname "$0")/lib/common.sh"
cd "$ROOT"
tc="$(make_toolchain_args)"
if is_macos; then
  have gtimeout || warn "coreutils not found; run ./scripts/setup.sh if tests fail to start"
  xattr -d com.apple.quarantine tools/mgba/mgba-rom-test-mac 2>/dev/null || true
fi
filter_args=()
if [ -n "${1:-}" ]; then
  rm -f pokeemerald-test.elf   # the filter is written into the test image when it is linked
  filter_args=("TESTS=$1")
fi
start=$(date +%s)
if [ -z "${1:-}" ]; then
  info "Project tool tests (Python)"
  python3 -I test/ascension/test_bps.py || die "Project tool tests failed"
fi
if make -j"$(cpu_count)" $tc check ${filter_args[@]+"${filter_args[@]}"}; then
  ok "Tests passed in $(( $(date +%s) - start ))s"
else
  die "Tests failed after $(( $(date +%s) - start ))s"
fi
