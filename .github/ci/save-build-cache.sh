#!/usr/bin/env bash
# Pack generated files and objects (everything git ignores, except built ROM/ELF images) for the cache.
# Usage: save-build-cache.sh <tarball>
set -euo pipefail
tarball="$1"
git rev-parse HEAD > .ci-cache-sha
git ls-files --others --ignored --exclude-standard -z \
  | grep -zvE '^(pokeemerald|pokefirered|pokeleafgreen)[^/]*\.(gba|elf)$' \
  | tar --null -T - -cf "$tarball"
ls -lh "$tarball"
