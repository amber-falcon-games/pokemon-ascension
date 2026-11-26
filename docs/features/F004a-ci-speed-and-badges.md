# F004a — CI speed, selective runs and status badges
Status: DONE (merged in PR #2, merge commit `520ffeb`, 2026-11-26)
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
- Local (4 cores): full suite with everything built 697 s; shard 0/4 175 s. Shards 0–3 totals sum to 5,876 total, 5,429 passed, 419 TO_DO, 19 KNOWN_FAILING, 9 EXPECT_FAILING, identical to the unsharded run, so no test is lost.
- Cache simulation on a fresh clone: 0 compiles when nothing changed; exactly 1 (`battle_util.o`) after editing `src/battle_util.c`. Exact-hit and deletion-only restore bug found by Codex review, reproduced (exit 1) and fixed (exit 0).
- GitHub, PR #2 head `365b9ea`, cold cache, public runners: build 3 m 36 s; test shards 9 m 08 s / 9 m 19 s / 10 m 23 s / 14 m 25 s (each includes compiling the test build); whole run ≈ 15 min, versus ≈ 32 min before.
- Warm-cache timings and the docs-only and test-only paths are measured on the next PRs, after the first nightly runs seed `main`. Results to be appended here.
