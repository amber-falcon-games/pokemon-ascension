# F006 — Baseline release 0.1.0: smoke test, resource baseline, release script
Status: ACTIVE (tooling done; awaiting owner Mac run, smoke test and first release)
Dependencies: F004b, F005b (F002–F005 owner Mac verification happens in the same session)
Owner decision IDs: D017 (deliberate releases, patch made on the owner's Mac)
Branch / commit: `claude/sharp-euler-yfhti2`

## Objective and scope
Produce the first playable release: a `.bps` patch of the current game (upstream 1.17.2 plus the F005b cuts), made and published from the owner's Mac, with a recorded resource baseline that later milestones are compared against.
- `tools/ascension/bps.py`: dependency-free BPS patch tool (`create`, `apply`, `verify`), with unit tests in `test/ascension/test_bps.py` (run by `scripts/test.sh` and the CI docs job).
- `scripts/release.sh X.Y.Z [--base PATH] [--no-publish]`:
  1. Checks the base ROM's SHA-1 against `rom.sha1`. Default base location: `.local/base/emerald.gba`, which is git-ignored.
  2. On a real release, requires a clean, up-to-date `main` and a new tag.
  3. Builds the release ROM and writes `dist/pokemon-ascension-vX.Y.Z.bps`, verifying it reproduces the ROM byte for byte.
  4. Asks before tagging `vX.Y.Z` and publishing a GitHub Release, using `gh` if installed or printing the manual upload steps.
- Smoke test checklist (owner, below) and resource baseline (below).

## Explicit exclusions
No automatic releases on merge (D017). No built ROMs, saves or base ROMs in the repository, CI or releases; only the patch is published.

## Acceptance criteria
- [x] Patch tool unit tests: 6/6 (identical files, in-place edits, moved blocks, runs, growth, shrink, empty, wrong source rejected, corrupt patch rejected, varints).
- [x] Real-size round trip: upstream ROM → F005b ROM (32 MB each), patch 1.96 MB, created in 17 s, verified exact. 16 MB source (Emerald-sized) → 32 MB target: patch 11.2 MB, verified exact.
- [x] Guided mode (owner request): with no version argument the script proposes one (0.1.0 when there are no tags, otherwise the next minor), offers to switch to and pull `main`, asks for the base ROM path, opens the release build with a smoke-test checklist and waits, opens Rom Patcher JS and Finder at the patch and waits, then asks before publishing. Tested in a pseudo-terminal with scripted answers (version accepted, smoke test yes, patch check yes): all steps ran in order and stopped before publishing with `--no-publish`.
- [x] `release.sh --no-publish` end to end in the container, with a stand-in base ROM (the checksum check pointed at the stand-in for the test only): release build, 13.1 MB patch, verified exact.
- [ ] Owner: smoke test passes on the Mac (checklist below).
- [ ] Owner: `./scripts/release.sh 0.1.0` with their own Emerald dump publishes `v0.1.0`; the patch applies in Rom Patcher JS (independent implementation) and the result boots.

## Smoke test (owner, Mac)
1. `./scripts/play.sh --release`, then **New Game** (or SELECT on the title screen for quickstart in a non-release build).
2. Walk to Route 101, battle a wild Pokémon, win or run.
3. At a Pokémon Center PC, open "Someone's PC", cycle boxes with L/R: there are **10** boxes and box 10 wraps to box 1.
4. Save in-game, quit mGBA, relaunch with `./scripts/play.sh --no-build`, Continue: the position and party are restored.

## Resource baseline (0.1.0 candidate, Linux build, normal target)
EWRAM 214,028 B (81.65%), IWRAM 28,364 B (86.56%), ROM 26,744,176 B (79.70%). Save: SaveBlock1 14,160 / 15,872 B; SaveBlock2 2,544 / 3,968 B; PokemonStorage 24,504 / 35,712 B.

## Technical approach and risks
- The patch is created against the owner's own dump, so the base never leaves their machine.
- Risk: the patch tool differs subtly from other BPS implementations. Mitigation: the format follows the published BPS1 layout (varints, the three CRC32s, signed relative offsets for copies). The owner checks with Rom Patcher JS (an independent implementation) before announcing a release. A PyPI BPS library for cross-checking in CI could not be installed in this environment.
- Patch size is about 11–13 MB because the ROM grows from 16 MB to 32 MB. That is fine for Rom Patcher JS.

## Rollback
Delete the GitHub Release and tag (`git push --delete origin v0.1.0`); nothing else depends on them.

## Evidence and acceptance
See acceptance criteria. Owner steps pending.
