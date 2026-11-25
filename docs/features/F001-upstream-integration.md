# F001 — Upstream audit, pinned baseline and integration plan
Status: BLOCKED (awaiting owner decision D010)
Dependencies: F000
Owner decision IDs: D010
Branch / commit: investigation on `claude/sharp-euler-yfhti2`; import will happen on `feature/F001-upstream-import` after approval

## Objective and scope
Audit RHH's pokeemerald-expansion, recommend a pinned baseline and a Git integration method that preserves upstream history and attribution and keeps this repository's documents. Import only after the owner approves D010.

## Explicit exclusions
No gameplay changes, no project scripts (F002+), no CI redesign (F004), no ROM or save committed.

## Audit findings (measured 2026-11-25)
Source: `https://github.com/rh-hideout/pokeemerald-expansion`, cloned with full history (blobless) into a scratch directory outside this repository.

**Releases and branches.** Tags `expansion/X.Y.Z`; latest stable is **`expansion/1.17.2`**, commit `79dd436920845ebc145577649de624f65298c209`, released 2026-10-31 (prior: 1.17.1 on 2026-09-29, 1.17.0 on 2026-08-31; roughly monthly patch cadence). Branches: `master` = latest release plus bugfixes; `upcoming` = new features (unstable). 20,954 commits of history reachable from 1.17.2; packed history ≈ 97 MiB; working tree ≈ 253 MB including build outputs.

**Attribution and licence.** README asks hacks to credit RHH: `Based off RHH's pokeemerald-expansion 1.17.2 https://github.com/rh-hideout/pokeemerald-expansion/` and to consider crediting all contributors (`CREDITS.md`). There is **no top-level licence file**: the codebase derives from pret's pokeemerald decompilation, which has none. Only bundled tools carry licences (`tools/{bin2c,gbagfx,mid2agb,preproc,ramscrgen,rsfont,scaninc,wav2agb}/LICENSE`, `tools/gbafix/COPYING`). Practical meaning: no grant of rights beyond normal fan-project norms; this reinforces the existing private, non-commercial, no-public-ROM boundary.

**Build prerequisites (upstream `INSTALL.md`, `docs/install/mac/MAC_OS.md`).** Xcode CLT, Homebrew `libpng` and `pkg-config`, devkitARM via devkitPro pacman (`sudo dkp-pacman -S gba-dev`; installer `.pkg` — privileged), Python 3; `coreutils` for tests. The Makefile also accepts any `arm-none-eabi` toolchain on `PATH` (`TOOLCHAIN` override) — the Linux CI uses Ubuntu's `gcc-arm-none-eabi` + `libnewlib-arm-none-eabi`. The Mac toolchain choice belongs to F002 and needs owner permission for privileged installs.

**Build verified (Linux container, not the owner's Mac).**
```
git -C pe checkout expansion/1.17.2
make -j4          # exit 0, 218 s, Ubuntu 24.04, arm-none-eabi-gcc 13.2.1, ld 2.42
```
| Region | Used | Size | Used % | Free |
|---|---|---|---|---|
| EWRAM | 226,416 B | 256 KB | 86.37 % | 35,728 B |
| IWRAM | 28,376 B | 32 KB | 86.60 % | 4,392 B |
| ROM | 26,751,104 B | 32 MB | 79.72 % | 6,803,328 B |

Output `pokeemerald.gba` (33,554,432 B, SHA-1 `c5eabb9bca66e218063a623e3dcd0d357f4e68d5` for this toolchain; not committed). EWRAM includes a 115,968 B heap (`HEAP_SIZE 0x1C500`) from which battle structures are allocated; peak in-battle heap use needs runtime measurement (F006/F017).

**Automated tests.** `make check` builds a test ELF and runs it headless in the bundled `tools/mgba/mgba-rom-test` (Mac: `mgba-rom-test-mac`) via `mgba-rom-test-hydra`. 968 test files; macros counted: 3,506 `SINGLE_BATTLE_TEST`, 956 `DOUBLE_BATTLE_TEST`, 543 AI tests, 59 wild, 21 multi, 920 generic `TEST`. Tests are a declarative battle DSL (`GIVEN { PLAYER(...) } WHEN { TURN { MOVE(...) } } SCENE { ... }`), an excellent regression net and a model for Battle Lab scenario presets. Test-run result: see Evidence.

**Save layout (`include/save.h`, measured sizes from the ELF).** Flash 128 KB = 32 sectors × 4 KB; 3,968 data bytes + 116-byte SaveBlock3 chunk per sector; 14-sector slots ×2, plus Hall of Fame (28–29), Trainer Hill (30) and Recorded Battle (31).
| Block | Used | Capacity | Free |
|---|---|---|---|
| SaveBlock2 | 3,884 B | 3,968 B | 84 B |
| SaveBlock1 | 15,568 B | 15,872 B | 304 B |
| PokemonStorage | 34,144 B | 35,712 B | 1,568 B |
| SaveBlock3 (spread over 14 sectors) | ≈4 B with default config | 1,624 B | ≈1,620 B |

**Move slots.** `MAX_MON_MOVES 4` (`include/constants/global.h:136`) is referenced 484 times across 59 source/header files; party and box structures store four moves per Pokémon.

**Battle engine size.** `battle_script_commands.c` 12,415 lines, `battle_util.c` 11,259, AI files ≈ 17,400, `data/battle_scripts_1.s` 6,456. Turn-based structure; turn order is computed per turn.

**Reusable debug tooling.** Debug builds (disabled on `make release`): overworld debug menu (R+Start) including start wild/single/double/multi battles and AI-flag toggles (`src/debug.c`); in-battle debug menu (Select) inspecting/editing moves, ability, item, PP, types, stats, stages, status, volatiles, hazards, side status, AI scores, instant win (`src/battle_debug.c`); text-format trainer definitions (`src/data/debug_trainers.party`); title-screen quickstart (`include/config/quickstart.h`). These cover much of F009/F012 and give F007/F011 a starting point.

**Upstream files that collide or need handling.** `README.md`, `.gitignore`, `.github/pull_request_template.md`. Upstream `docs/` has no filename collisions with ours, but `.github/workflows/build.yml` job `docs_validate` requires every `docs/**/*.md` to be listed in `docs/SUMMARY.md` and would fail on our docs; `docs.yml` deploys GitHub Pages on `master`; `labels.yml` is RHH-specific; `.github/ISSUE_TEMPLATE/config.yml` points to RHH's Discord.

## Feasibility assessment (for escalation per ROADMAP)
- **Memory:** ~35 KB EWRAM and ~4 KB IWRAM free. Enough for a Battle Lab UI and modest scheduler state if allocated from the heap during battle; IWRAM is the scarce resource. Low–medium risk.
- **Save capacity:** very tight. Per-Pokémon persistent move libraries are **not feasible** in box storage (≈1.5 KB free across up to 420 stored Pokémon). A library derived from data (e.g., level-up learnset up to current level) plus small flag sets is feasible. This shapes F015 and must be decided by the owner then; nothing is decided now.
- **Lab scenario persistence (F011):** no free sectors; debug-only reuse of the Trainer Hill or Recorded Battle sector (3,968 B each) is a candidate to evaluate in F011.
- **Four-move assumption:** raising `MAX_MON_MOVES` is high-risk (484 references, save format). Preferable direction for F014–F015: keep four battle-loaded slots internally, or a layered menu choosing from a derived library. Owner decision at F014/F015.
- **Scheduler:** layering initiative/time costs on the existing turn order is plausible; a full real-time scheduler would touch the core engine and the test DSL (which is turn-based). Keep the F017 spike gate. Medium–high risk, not blocking the foundation.

No finding undermines the foundation phase (F002–F013).

## Integration options (D010)
| Option | Description | History & attribution | Future upstream updates | Risk |
|---|---|---|---|---|
| **A (recommended)** | Pin `expansion/1.17.2`; on `feature/F001-upstream-import`, `git merge --allow-unrelated-histories` the tag into our history. Resolve: keep our `README.md` (add RHH credit line), union `.gitignore`, keep our PR template and issue templates, drop RHH `config.yml`, disable `docs.yml` and `labels.yml`, list our docs in `docs/SUMMARY.md` so `docs_validate` passes. Add `rhh` remote locally only. | Both histories fully preserved; RHH credit in README. | `git merge expansion/X.Y.Z` on a feature branch, exactly as upstream's own update guide. | Low. Repo grows by ≈97 MiB of history. |
| B | Same as A but track `master` instead of a tag. | Same. | Picks up bugfixes but is a moving, unversioned target. | Medium. |
| C | Copy a 1.17.2 snapshot as one commit, no upstream history. | Lost (credit text only). | Painful manual diffs. | High; violates "preserve history". |
| D | Rebuild our repo on top of upstream history (rebase our two commits). | Preserved but rewrites our `main`, needs force-push. | Same as A. | Violates "no force-push". |

Not recommended: `upcoming` (unstable), submodule/subtree (upstream build expects repository root).

## Acceptance criteria
- [x] Upstream releases, branches, prerequisites, tests and debug tools inspected.
- [x] Memory, save, move-slot and scheduler constraints measured and recorded.
- [x] Baseline and integration recommendation with options.
- [ ] Owner approves D010.
- [ ] After approval: merge performed on `feature/F001-upstream-import`; `make` and `make check` pass on the merged tree; all scaffold docs intact; PR opened with evidence.

## Automated tests and regression
After import: `make -j$(nproc)` and `make check -j$(nproc)` must reproduce the figures above (allowing for our documentation-only additions, which do not affect the ROM). `git log` must show both root commits.

## Manual playtest
None for F001. First playable check is F006 (owner runs on Mac).

## Resource budgets and save compatibility
Baseline is upstream 1.17.2; no prior saves exist. Measured budgets above become the F006 baseline.

## Rollback
Before merge to `main`: delete the feature branch. After merge: revert the merge commit (`git revert -m 1 <merge>`); no force-push needed.

## Evidence and acceptance
- Investigation commit: this document on `claude/sharp-euler-yfhti2`.
- Build: see table above (Linux container, 2026-11-25).
- Tests: `make check` exit 0; 5,429 passed, 0 unexpected failures (details below).
- Owner approval: pending (D010).

### Upstream test run
Command: `make check -j4` on `expansion/1.17.2` (Linux container, bundled `tools/mgba/mgba-rom-test`). Exit code 0, 874 s.
Result: TOTAL 5,876 — PASSED 5,429, TO_DO 419 (upstream placeholders), KNOWN_FAILING 19 (upstream-acknowledged), EXPECT_FAILING 9 (includes the two test-runner self-tests "fatalf counts as CRASH" and "Tests resume after CRASH", which crash by design). No unexpected failures.
Not run here: Mac test runner (`mgba-rom-test-mac`), to be verified in F002/F004.
