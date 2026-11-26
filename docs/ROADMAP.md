# Dependency-ordered roadmap

Status key: TODO, ACTIVE, BLOCKED, AWAITING_ACCEPTANCE, DONE. All entries TODO initially except F000 ready for review. Dependencies are minimum prerequisites; Claude may split work into stable subfeatures after source analysis, recording rationale. Do not reorder foundation without approval. Each feature must be independently runnable after F006; use disabled flags for incomplete mechanics. Do not mark DONE without evidence.

## Approved execution order (owner, 2026-11-26)
Work proceeds strictly in this order; the table below keeps dependencies and descriptions. Owner priority: reach a lean playable MVP quickly, then visible changes.
1. F004a CI speed-up · 2. F002 Mac setup/doctor · 3. F003 build script · 4. F004b test script · 5. F005 mGBA play/status scripts · 6. F005b remove cut features (D011) and reduce to 10 boxes · 7. F006 baseline release 0.1.0 + release script · 8–14. F007–F013 Battle Lab (0.2.0) · 15. F017 feasibility spike (timeline, energy, >4 moves; platform checkpoint) · 16. F014 categorised move menu · 17. F015 talent-tree move data and persistence · 18. F016 move info and Pokédex (0.3.0) · 19. F018 initiative timeline · 20. F019 energy replaces PP · 21. F020 queue/cancel/intervene · 22. F021 AI parity and regression (0.4.0) · 23–27. F022–F026 reactive combat (0.5.0) · 28–32. F027–F031 progression and evolution (0.6.0) · 33–35. F032–F034 campaign (1.0) · 36. F035 quests and mythology.

| ID | Deliverable | Depends on |
|---|---|---|
| F000 | Documentation scaffold review — DONE ([spec](features/F000-scaffold-review.md)) | — |
| F001 | Upstream audit, pinned baseline, approved integration preserving history — DONE ([spec](features/F001-upstream-integration.md)) | F000 |
| F004a | CI speed, selective runs, nightly status and badges — DONE ([spec](features/F004a-ci-speed-and-badges.md)) | F001 |
| F002 | Mac setup and doctor scripts, pinned dependencies — AWAITING_ACCEPTANCE, bundled with F003/F004b/F005 ([spec](features/F002-F005-local-scripts.md)) | F001 |
| F003 | Reproducible ROM compilation, build script | F002 |
| F004b | Test script (`scripts/test.sh`) on top of F004a CI | F003 |
| F005 | mGBA launch, local play and status scripts | F003 |
| F005b | DONE ([spec](features/F005b-lean-cuts.md)) — Lean cuts: features with upstream switches (Mystery Gift/Event, e-Reader, record-mixing records, Union Room chat, link battle records, Match Call → VS Seeker, Pokémon Jump, Trainer Hill, extra Pokédex flags) and PC reduced to 10 boxes | F005 |
| F005c | Deep cuts needing map/script edits (Secret Bases, TV, Contests and ribbons, Mail, flavour NPCs, link minigames, Recorded Battle sector). Split from F005b for a faster MVP; must land before F015, which needs the space | F005b |
| F006 | Baseline playable release 0.1.0, smoke test, resource measurements, release script (D017) — ACTIVE ([spec](features/F006-baseline-release.md)) | F004b,F005b |
| F007 | Battle Lab entry and debug UI | F006 |
| F008 | Both team builders | F007 |
| F009 | Battle conditions, moves, abilities, items and AI setup | F008 |
| F010 | Play, edit, restart and exit custom battles | F009 |
| F011 | Save/load named scenarios and preset registry | F010 |
| F012 | State inspection, deterministic controls and flags | F011 |
| F013 | Lab acceptance and stable foundation release | F012 |
| F014 | Categorised Final Fantasy-style move menu (D015) | F017 |
| F015 | Move library data/persistence and learning | F014 |
| F016 | Quick move info and in-battle Pokédex | F015 |
| F017 | Feasibility spike: timeline, energy (D016), >4 moves, AI time; platform checkpoint | F013 |
| F018 | Initiative timeline: order from energy, recovery and Speed | F016 |
| F019 | Energy replaces PP: costs, regeneration, recovery | F018 |
| F020 | Queue, cancellation and manual intervention | F019 |
| F021 | Scheduler-aware enemy AI and full battle regression | F020 |
| F022 | Defensive move variants and conditional reactions | F021 |
| F023 | Delayed actions and traps | F022 |
| F024 | Tactical switch/intercept/pivot | F023 |
| F025 | Elemental rule engine and conductivity slice | F024 |
| F026 | Battle pacing and anti-sweep balance | F025 |
| F027 | Move mastery progression | F026 |
| F028 | Branching move evolution interface and data | F027 |
| F029 | Mid-battle level evolution | F028 |
| F030 | Contextual evolution triggers | F029 |
| F031 | Permanent Mega and alternate evolution branches | F030 |
| F032 | Campaign trainer/gym AI and wild balance | F031 |
| F033 | Boss scenarios, tutorials and progression | F032 |
| F034 | Full campaign regression, economy and polish | F033 |
| F035 | Original quests, exploration and mythology (decompose later) | F034 |

Milestone releases: F006 baseline; F013 foundation; F016 expanded commands; F021 dynamic combat; F026 reactive combat; F031 evolution/mastery; F034 integrated campaign. F035 open-ended. Assess scheduler and move-library feasibility during F001, before investing; escalate if GBA constraints undermine vision.

