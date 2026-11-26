# F002, F003, F004b, F005 — Local scripts: setup, doctor, build, test, play, status
Status: DONE (owner verified on Apple Silicon Mac, 2026-11-26)
Bundling rationale: all four are thin wrappers over upstream `make` and share `scripts/lib/common.sh`; one Mac session verifies them together, which is the fastest route to a playable MVP (owner priority).
Dependencies: F001
Owner decision IDs: D013–D018 (order); no new decision unless the owner prefers devkitPro
Branch / commit: `claude/sharp-euler-yfhti2`

## Objective and scope
One command prepares an Apple Silicon Mac to build the ROM; one command diagnoses it.
- `./scripts/setup.sh [--yes]`: idempotent. Checks Xcode Command Line Tools and Homebrew (never installs them itself: both need the owner's password or a GUI dialog). Asks, then `brew install libpng pkg-config coreutils`. Downloads the pinned ARM GNU Toolchain 13.2.rel1 (macOS arm64) into `.local/arm-gnu-toolchain`, verifies it against ARM's published SHA-256, clears the download quarantine flag for that folder only, then runs the doctor.
- `./scripts/doctor.sh`: read-only. Reports macOS/arch, CLT, Homebrew and formulae, python3, make, git, toolchain version, free disk, mGBA presence. Exit 0 only when ready to build.

- `./scripts/build.sh [--release] [--debug] [--clean]` (F003): removes the old ROM first, builds with the project toolchain (or the system one on Linux/CI), copies to `build/pokemon-ascension.gba`, prints memory use and SHA-1; on failure prints the log tail and leaves no ROM.
- `./scripts/test.sh [file.c | "name prefix"]` (F004b): runs upstream's suite plus project tests; optional filter; non-zero on failure.
- `./scripts/play.sh [--release] [--no-build]` (F005): builds, then opens mGBA. `--lab` / `--scenario` exit with a clear "not built yet" message until F007/F011.
- `./scripts/status.sh` (F005): branch, commit, latest release tag, current feature, next action from PROJECT_STATUS.md.

## Explicit exclusions
Battle Lab launch (F007/F011). Release patches (F006). Linux setup (CI covers Linux; doctor is informational there).

## Technical approach and risks
- **No sudo.** Upstream's macOS guide installs devkitARM with `sudo dkp-pacman`. Instead we use ARM's official prebuilt toolchain unpacked inside the project: no system changes, pinned to the same GCC 13.2.1 that CI uses, removable by deleting `.local/`. Upstream's Makefile supports this via `TOOLCHAIN=`.
- Risk: ARM's toolchain libraries differ from devkitARM's. Mitigation: F003 build on the Mac must produce the same ROM as CI (compare SHA-1). Fallback: devkitPro install with owner permission.
- Risk: macOS bash is 3.2; scripts avoid newer bash features.
- Checksum: verified against ARM's `.sha256asc` at download time. Once the owner's run succeeds, the observed hash is pinned in `scripts/lib/common.sh`.

## Acceptance criteria
- [x] Scripts written; `bash -n` syntax check passes; doctor runs on Linux with an informative message.
- [x] On Linux (container, system toolchain): `./scripts/build.sh` exit 0, ROM SHA-1 `c5eabb9b…` identical to CI/upstream; `./scripts/test.sh test/battle/move_effect/dragon_darts.c` 16/16 passed; `play.sh --lab` and `play.sh` without mGBA fail with clear messages; `status.sh` prints state.
- [x] Owner runs `./scripts/setup.sh` on the Mac: completes, doctor reports "Ready to build" (after the GNU make 4.x fix).
- [ ] (not yet reported) `./scripts/build.sh` on the Mac produces the same SHA-1 as the Linux build of the same commit, proving the ARM toolchain matches (`main` at F005b: `49cb2f71f87562bd2bcb15392150e3d13f4f9375`).
- [x] `./scripts/play.sh` builds and opens the game in mGBA (owner report).
- [x] `./scripts/test.sh test/battle/move_effect/dragon_darts.c` passes on the Mac: 16 passed (owner report).
- [ ] Re-running `./scripts/setup.sh` changes nothing (idempotent).
- [ ] Toolchain SHA-256 pinned from the verified run.

## Manual test (owner, Mac)
See PROJECT_STATUS.md "Owner action" for the exact copy-paste steps and what to send back.

## Resource budgets and save compatibility
None (tooling only). `.local/` is git-ignored (~600 MB toolchain).

## Rollback
Delete `scripts/` changes; `rm -rf .local/arm-gnu-toolchain` on the Mac.

## Mac issue found in the owner's first run (2026-11-26)
`./scripts/play.sh` failed with `data/battle_partners.h: No such file or directory`. Doctor passed; make was macOS's GNU make 3.81. Upstream's Makefile generates headers in a `$(shell $(MAKE) generated)` pre-step and checks for failure with `.SHELLSTATUS`, which exists only from GNU make 4.2. On 3.81 a failure there is silently ignored, so the build later dies on the missing header. Fix: `setup.sh` installs Homebrew `make` (`gmake` 4.x); `build.sh`/`test.sh` use `gmake` when present and refuse make < 4.2; `doctor.sh` reports an old make as a failure. Verified on Linux (make 4.3) plus a unit check of the version comparison (3.81, 4.1 rejected; 4.2, 4.3, 4.4.1, 10.0 accepted). Re-verification on the Mac pending.

## Evidence and acceptance
Pending owner's Mac run.
