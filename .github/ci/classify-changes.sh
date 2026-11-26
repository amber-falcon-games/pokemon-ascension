#!/usr/bin/env bash
# Decide how much CI a pull request needs, from its list of changed files.
# Outputs (to $GITHUB_OUTPUT): mode=docs|tests|full and tests=<space-separated test files>.
# Usage: classify-changes.sh <file-with-one-path-per-line>
set -euo pipefail
list="$1"
mode=docs
tests=()
while IFS= read -r f; do
  [ -z "$f" ] && continue
  case "$f" in
    docs/*|*.md|.github/ISSUE_TEMPLATE/*|.github/pull_request_template.md) ;;      # documentation
    test/test_runner*.c|test/*.h|test/**/*.h) mode=full ;;                          # test framework
    test/*.c|test/*/*.c|test/*/*/*.c|test/*/*/*/*.c)
      [ "$mode" = docs ] && mode=tests
      [ -f "$f" ] && tests+=("$f") ;;                                                # changed or added test (skip deleted)
    *) mode=full ;;                                                                  # engine, data, tools, build files, CI
  esac
done < "$list"
echo "mode=$mode"
echo "tests=${tests[*]:-}"
if [ -n "${GITHUB_OUTPUT:-}" ]; then
  echo "mode=$mode" >> "$GITHUB_OUTPUT"
  echo "tests=${tests[*]:-}" >> "$GITHUB_OUTPUT"
fi
