# F000 — Documentation scaffold review
Status: DONE
Dependencies: —
Owner decision IDs: none required (amendments below are non-gameplay)
Branch / commit: `claude/sharp-euler-yfhti2` (review base `39bf610`)

## Objective and scope
Review the bootstrap documentation for internal consistency, gaps and feasibility, record amendments, and plan F001. Documentation only; no engine, scripts or gameplay.

## Explicit exclusions
No upstream import, no scripts, no gameplay decisions.

## Acceptance criteria
- [x] Every file in the scaffold read (19 files: CLAUDE.md, README, PROJECT_STATUS, CHANGELOG, 10 docs, feature template, 3 GitHub templates, .gitignore).
- [x] Consistency check across CLAUDE.md, roadmap, decisions, Battle Lab, testing and workflow docs.
- [x] Amendments recorded below and applied where purely factual/procedural.
- [x] F001 spec written (`docs/features/F001-upstream-integration.md`).
- [x] Owner accepts the review and amendments (2026-11-26).

## Findings
Consistent: dependency order (F000→F013 foundation before combat), Lab-first rule (D003, BATTLE_LAB.md, CLAUDE.md), save-isolation rules (SAVE_COMPATIBILITY.md, BATTLE_LAB F010), "no fabricated results" across workflow, testing and PR template, and ROM/save exclusion in README, .gitignore and PR checklist.

Gaps and amendments:
| # | Finding | Action |
|---|---|---|
| A1 | No feature specs existed; template had no naming convention. | Convention adopted: `docs/features/FXXX-short-slug.md`. F000 and F001 specs created. |
| A2 | D001–D009 lack the date/options/owner-response fields that DECISIONS.md requires. | Annotated as recorded at bootstrap 2026-11-25 by the owner; new decisions use the full format (D010 onward). |
| A3 | Development environment unstated. Claude may run in a Linux cloud container or locally on the owner's Mac. | TESTING.md now states: Linux container verifies build and automated tests; Apple Silicon setup/play scripts (F002, F005) need verification on the owner's Mac (owner runs them, or a local Claude Code session). |
| A4 | Upstream has no top-level licence (inherited from pret's decompilation); only bundled tools carry licences. | Recorded in F001 and README boundaries. Consistent with the existing private, non-commercial stance; reinforces "no public ROM distribution". |
| A5 | Upstream ships GitHub workflows that would misbehave here (GitHub Pages deploy on `master`, docs check that fails on our docs, five CI jobs including FireRed/LeafGreen builds). | Handled in the F001 integration plan; CI trimmed in F004. |
| A6 | Upstream measurements show tight EWRAM/IWRAM and save headroom; the move-library and scheduler visions depend on them. | Measured figures recorded in ARCHITECTURE.md and SAVE_COMPATIBILITY.md; risks flagged for F015/F017. No gameplay decision taken. |
| A7 | `.gitignore` will need upstream's build-output patterns. | Union of both files at integration (F001). |

No contradictions requiring owner decisions were found.

## Rollback
Revert the documentation commit.

## Evidence and acceptance
Review performed 2026-11-25 against commit `39bf610`. Owner accepted 2026-11-26.
