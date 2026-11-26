#!/usr/bin/env bash
# Nightly gate: run only if main changed in the last 26 hours (or on manual dispatch).
# Otherwise carry the previous nightly result forward, so the badge keeps showing the real state of main.
# Env: GH_TOKEN, GITHUB_REPOSITORY, GITHUB_RUN_ID, GITHUB_EVENT_NAME, WORKFLOW_FILE, GITHUB_OUTPUT
set -euo pipefail
if [ "$GITHUB_EVENT_NAME" = "workflow_dispatch" ]; then
  echo "run=true" >> "$GITHUB_OUTPUT"; echo "Manual run."; exit 0
fi
last_commit=$(git log -1 --format=%ct)
age=$(( $(date +%s) - last_commit ))
if [ "$age" -lt $((26 * 3600)) ]; then
  echo "run=true" >> "$GITHUB_OUTPUT"; echo "main changed ${age}s ago; running."; exit 0
fi
echo "run=false" >> "$GITHUB_OUTPUT"
previous=$(gh api "repos/${GITHUB_REPOSITORY}/actions/workflows/${WORKFLOW_FILE}/runs?branch=main&status=completed&per_page=10" \
  --jq "[.workflow_runs[] | select(.id != ${GITHUB_RUN_ID})][0].conclusion // \"success\"")
echo "main unchanged; carrying forward previous result: ${previous}."
[ "$previous" = "success" ]
