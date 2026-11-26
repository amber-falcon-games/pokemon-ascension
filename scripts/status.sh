#!/usr/bin/env bash
# Show project state: current feature, next action, blockers and last known-good version.
source "$(dirname "$0")/lib/common.sh"
cd "$ROOT"
echo "Branch: $(git rev-parse --abbrev-ref HEAD) @ $(git rev-parse --short HEAD)$( [ -n "$(git status --porcelain)" ] && echo ' (uncommitted changes)')"
echo "Latest release tag: $(git describe --tags --abbrev=0 --match 'v*' 2>/dev/null || echo none)"
echo
grep -E '^(Current phase|Current feature|Next eligible feature|Last known-good playable build|Pending owner decisions):' PROJECT_STATUS.md
echo
echo "Exact next action:"
grep -E '^- Exact next action' PROJECT_STATUS.md | sed 's/^- Exact next action: */  /'
