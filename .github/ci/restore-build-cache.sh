#!/usr/bin/env bash
# Make `make` incremental on a fresh checkout.
# 1. Give every tracked file an old timestamp.
# 2. Unpack cached generated files and objects (they keep their newer timestamps).
# 3. Give files changed since the cached commit the current time, so only they rebuild.
# Without a usable cache, everything stays "old" with no outputs, i.e. a normal full build.
# Usage: restore-build-cache.sh <tarball>
set -euo pipefail
tarball="$1"
git ls-files -z | xargs -0 touch -h -d '2000-01-01 00:00:00 UTC'
if [ ! -f "$tarball" ]; then
  echo "No build cache; full build."
  exit 0
fi
tar -xf "$tarball"
cached_sha="$(cat .ci-cache-sha 2>/dev/null || true)"
if [ -z "$cached_sha" ] || ! git fetch --quiet --depth=1 origin "$cached_sha" 2>/dev/null; then
  echo "Cached commit unavailable; discarding cache for a safe full build."
  git ls-files --others --ignored --exclude-standard -z | xargs -0 rm -f
  exit 0
fi
changed="$(git diff --name-only "$cached_sha" HEAD || true)"
echo "Cache from ${cached_sha}; $(printf '%s\n' "$changed" | grep -c . || true) tracked files changed since."
printf '%s\n' "$changed" | while IFS= read -r f; do
  if [ -n "$f" ] && [ -e "$f" ]; then touch "$f"; fi   # skip blank lines and deleted files
done
exit 0
