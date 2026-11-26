# Battle Lab scenario registry

LAB-001 is built in (`src/data/battle_lab_trainers.party`). Every other scenario is a file in `lab/scenarios/<ID>.json` (F011, D022; format in `lab/scenarios/README.md`). `./scripts/play.sh --lab` opens the Lab with all of them; `--scenario ID` opens on one. Nothing is stored in a save.

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
| LAB-009 | Six-member trainer battle | Swampert, Gardevoir, Salamence, Metagross, Breloom, Manectric (Lv50) | Blaziken, Sceptile, Aggron, Milotic, Flygon, Altaria (Lv50), Smart AI | Single, trainer battle | F008–F011 | Full 6v6 plays to the end; switching and the six-ball display work | Active (`lab/scenarios/LAB-009.json`) |
| LAB-010 | Over-levelled wild encounter | Marshtomp Lv25, Swellow Lv24 | Wild Tyranitar Lv70 | Wild single | F009–F011 | Wild battle against one far stronger Pokémon; running works | Active (`lab/scenarios/LAB-010.json`) |

Automated check: `test/ascension/battle_lab.c` (LAB-001 parties). Scenarios are never marked passed before their mechanics exist.
