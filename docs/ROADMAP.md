# Dependency-ordered roadmap

Status key: TODO, ACTIVE, BLOCKED, AWAITING_ACCEPTANCE, DONE. All entries TODO initially except F000 ready for review. Dependencies are minimum prerequisites; Claude may split work into stable subfeatures after source analysis, recording rationale. Do not reorder foundation without approval. Each feature must be independently runnable after F006; use disabled flags for incomplete mechanics. Do not mark DONE without evidence.

| ID | Deliverable | Depends on |
|---|---|---|
| F000 | Documentation scaffold review | — |
| F001 | Upstream audit, pinned baseline, approved integration preserving history | F000 |
| F002 | Mac setup and doctor scripts, pinned dependencies | F001 |
| F003 | Reproducible ROM compilation, build script | F002 |
| F004 | Upstream test integration, GitHub CI and test script | F003 |
| F005 | mGBA launch, local play and status scripts | F003 |
| F006 | Baseline playable release, smoke test and resource measurements | F004,F005 |
| F007 | Battle Lab entry and debug UI | F006 |
| F008 | Both team builders | F007 |
| F009 | Battle conditions, moves, abilities, items and AI setup | F008 |
| F010 | Play, edit, restart and exit custom battles | F009 |
| F011 | Save/load named scenarios and preset registry | F010 |
| F012 | State inspection, deterministic controls and flags | F011 |
| F013 | Lab acceptance and stable foundation release | F012 |
| F014 | Expanded command menus using baseline moves | F013 |
| F015 | Move library data/persistence and learning | F014 |
| F016 | Quick move info and in-battle Pokédex | F015 |
| F017 | Scheduler feasibility spike, gated and benchmarked | F016 |
| F018 | Initiative and action duration/recovery | F017 |
| F019 | Action resources and cooldowns | F018 |
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

