# Pokémon Ascension

An experimental, anime-inspired Pokémon Emerald overhaul: dynamic tactical battles, creative elemental interactions and expanded evolution.

**Status:** documentation bootstrap only. No engine, ROM or build tooling has been installed yet. See [PROJECT_STATUS.md](PROJECT_STATUS.md) for the current task and [roadmap](docs/ROADMAP.md) for the ordered delivery plan.

## Start development on Apple Silicon macOS

1. Clone this private repository:
   `git clone https://github.com/amber-falcon-games/pokemon-ascension.git`
2. Enter it: `cd pokemon-ascension`.
3. Start Claude Code locally: `claude` (install/authenticate Claude Code first if needed).
4. Paste: **Read CLAUDE.md and PROJECT_STATUS.md. Begin the next eligible feature and follow the project gates.**
5. Review the proposed upstream integration and environment setup. Claude will request any necessary permissions.

Do not attempt the following commands until Claude has implemented them during Phase 0:

| Planned command | Purpose |
|---|---|
| `./scripts/setup.sh` | Configure pinned prerequisites |
| `./scripts/doctor.sh` | Diagnose the toolchain |
| `./scripts/build.sh` | Compile a ROM |
| `./scripts/test.sh` | Run automated tests |
| `./scripts/play.sh` | Build and launch mGBA |
| `./scripts/play.sh --lab` | Build and launch directly into the Battle Lab |
| `./scripts/play.sh --scenario LAB-001` | Launch a reproducible scenario |
| `./scripts/status.sh` | Show project state |

See [testing](docs/TESTING.md), [Battle Lab](docs/BATTLE_LAB.md), [vision](docs/VISION.md), and [workflow](docs/DEVELOPMENT_WORKFLOW.md).

## Boundaries

Proposed upstream: https://github.com/rh-hideout/pokeemerald-expansion. Recommended baseline `expansion/1.17.2`, pending owner decision D010. Upstream has no top-level licence (it derives from pret's pokeemerald decompilation); only bundled tools carry their own licences. Preserve attribution and existing upstream files during integration. This is a private noncommercial fan project, not an official Pokémon product. Do not commit or publicly distribute ROM binaries, copyrighted asset bundles, saves, or secrets. Review rights and upstream terms before any distribution; patches do not remove all IP risk.
