# F004a — CI speed, selective runs and status badges
Status: TODO (next; pulled ahead of F002 by D014)
Dependencies: F001. Repository made public (D013) before measuring, since public runners differ (more CPU, no minute quota).
Owner decision IDs: D012, D013, D014
Branch / commit: —

## Objective and scope
Make CI fast and proportional to the change, without weakening what it checks.
- Pull requests only: build and test on `pull_request`, not on push to `main`.
- Skip work a change cannot affect: docs-only changes skip build and tests (docs check only); changes only under `test/` rebuild and run just the changed test files (the runner supports filtering by file); any change to `src/`, `include/`, `data/`, `graphics/`, `sound/`, `tools/` or build files runs the full suite.
- Caching: build tools (already done upstream) plus compiled objects. Restore file modification times from git so `make` only recompiles changed sources against a cached `build/` directory; cache keyed by toolchain and Makefile hash.
- Drop jobs we don't need: FireRed and LeafGreen builds, and the duplicate release build (release built nightly instead).
- Nightly status: scheduled workflow that checks whether `main` changed in the last 24 h and, only if so, runs the full build plus tests asynchronously. README badges point at it.
- Optional, if the measured gain justifies it: split the test suite across parallel jobs (needs a small change to upstream's test-runner tool).

## Explicit exclusions
No change to game code. `scripts/test.sh` is F004b.

## Acceptance criteria
- [ ] Docs-only PR: no build or test jobs run; docs check passes.
- [ ] Source PR with a warm cache: measured wall time recorded; target is a large reduction from the current ~32 min (tests) and ~9 min (build). Actual numbers, not estimates.
- [ ] Test-only PR runs only the changed test files.
- [ ] Full suite still runs whenever engine code changes; no test skipped or disabled to save time.
- [ ] Nightly workflow runs only when `main` changed; badges show its status in README.
- [ ] No ROMs uploaded as artifacts.

## Technical approach and risks
GitHub Actions cache limit is 10 GB per repository; `build/` is a few hundred MB. Restoring mtimes must be correct or `make` may skip a needed rebuild; mitigate by keying the cache on toolchain and build-file hashes and by the nightly clean build. Path filters can miss indirect dependencies (e.g. generated headers); keep the full-suite trigger broad.

## Automated tests and regression
Exercise each path with throwaway PRs (docs-only, test-only, source change) and record timings.

## Resource budgets and save compatibility
None (CI only).

## Rollback
Revert the workflow commit; upstream's original `build.yml` is in history.

## Evidence and acceptance
Pending.
