# Battle Lab scenario registry

Each scenario is reproducible data in `src/data/battle_lab_trainers.party` (dev builds only). Open the Lab with `./scripts/play.sh --lab`, then **L + SELECT** on the title screen.

| ID | Purpose | Player | Opponent | Format | Feature | Expected result | Status |
|---|---|---|---|---|---|---|---|
| LAB-001 | Baseline 1v1 with the unmodified battle engine | Swampert Lv50 (Waterfall, Earthquake, Ice Beam, Protect), 31 IVs | Blaziken Lv50 (Blaze Kick, Sky Uppercut, Rock Slide, Bulk Up), 31 IVs, Basic Trainer AI | Single, trainer battle | F007 | Battle plays as vanilla; win, loss or run returns to the Lab; repeatable | Active |
| LAB-002 | Dry/wet Onix conductivity | — | — | — | F025 | — | Planned |
| LAB-003 | Defensive Iron Tail | — | — | — | F022 | — | Planned |
| LAB-004 | Reactive switch | — | — | — | F024 | — | Planned |
| LAB-005 | Trap trigger | — | — | — | F023 | — | Planned |
| LAB-006 | Branching move mastery | — | — | — | F027–F028 | — | Planned |
| LAB-007 | Mid-battle evolution | — | — | — | F029 | — | Planned |
| LAB-008 | Permanent Mega | — | — | — | F031 | — | Planned |
| LAB-009 | Six-member trainer battle | — | — | — | F008–F010 | — | Planned |
| LAB-010 | Over-levelled wild encounter | — | — | — | F009 | — | Planned |

Automated check: `test/ascension/battle_lab.c` (LAB-001 parties). Scenarios are never marked passed before their mechanics exist.
