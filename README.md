# Pokémon Ascension

An experimental, anime-inspired Pokémon Emerald overhaul: dynamic tactical battles, creative elemental interactions and expanded evolution.

**Status:** upstream engine imported (pokeemerald-expansion 1.17.2, F001); project scripts not yet implemented (F002+). Until then, build with upstream's `make` (see upstream [INSTALL.md](INSTALL.md)). See [PROJECT_STATUS.md](PROJECT_STATUS.md) for the current task and [roadmap](docs/ROADMAP.md) for the ordered delivery plan.

## How to play

> **Status:** there is no playable release yet. The first one is planned at roadmap milestone F006. Until then the only way to play is to build the ROM yourself (see "Build it yourself" below). These steps will be updated as each milestone lands.

Pokémon Ascension will be released as a **patch file** (`.bps`), never as a ROM. You apply it to your own copy of Pokémon Emerald. This is standard practice for fan projects: the patch contains our changes, not Nintendo's game.

### 1. Get a base ROM (your own cartridge)
- You need a ROM dumped from **your own** *Pokémon Emerald Version* cartridge (USA/Europe). We do not link to ROM downloads.
- Cartridges can be dumped with a GBA cartridge reader (e.g. GBxCart RW or GB Operator) following that device's instructions.
- Check that your dump is the correct version. Its SHA-1 must be `f3ae088181bf583e55daf962a92bb46f4f1d07b7` (also in [`rom.sha1`](rom.sha1)). On macOS: `shasum -a 1 "your-emerald.gba"`.

### 2. Apply the patch
1. Download `pokemon-ascension-<version>.bps` from this repository's Releases page (available from F006).
2. Open [Rom Patcher JS](https://www.marcrobledo.com/RomPatcher.js/). It runs in the browser on desktop and phone, and your ROM is not uploaded anywhere.
3. Choose your Emerald ROM, then the `.bps` patch, then **Apply patch**. Save the result as `pokemon-ascension.gba`.

### 3a. Play on desktop (macOS, Windows, Linux): mGBA
1. Install mGBA from [mgba.io/downloads](https://mgba.io/downloads.html). On macOS: `brew install --cask mgba`, or download the `.dmg`.
2. Open mGBA, choose **File → Load ROM…** and select `pokemon-ascension.gba`.
3. Default keys: arrow keys = D-pad, **X** = A, **Z** = B, **A** = L, **S** = R, **Enter** = Start, **Backspace** = Select. Rebind under **Settings → Controllers**.
4. Saving: use the in-game **Save** option. mGBA writes `pokemon-ascension.sav` next to the ROM. Keep that file; save states (`.ss0`–`.ss9`) are emulator snapshots and are not the same as game saves.

### 3b. Play on iPhone or iPad: Delta
1. Install **Delta** from the App Store (see [faq.deltaemulator.com](https://faq.deltaemulator.com/)).
2. Patch the ROM as in step 2 (Rom Patcher JS works in Safari) and save `pokemon-ascension.gba` to the Files app.
3. In Delta, tap **+**, choose **Files**, and select the ROM.
4. Save in-game as usual. Delta keeps the save with the game.

On Android, any GBA emulator that loads `.gba` files works the same way.

### Build it yourself (developers, until releases exist)
Follow upstream's [INSTALL.md](INSTALL.md) for your OS, then run `make` in this folder. The ROM is written to `pokeemerald.gba` (never commit it). One-command scripts (`./scripts/setup.sh`, `./scripts/play.sh`) are planned in F002–F005.

## Start development on Apple Silicon macOS

1. Clone this repository:
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

Based off RHH's pokeemerald-expansion 1.17.2 https://github.com/rh-hideout/pokeemerald-expansion/ — please also see upstream [CREDITS.md](CREDITS.md). Upstream history is preserved in this repository (decision D010). Upstream has no top-level licence (it derives from pret's pokeemerald decompilation); only bundled tools carry their own licences. Preserve attribution and existing upstream files during integration. This is a noncommercial fan project, not an official Pokémon product. Do not commit or publicly distribute ROM binaries, copyrighted asset bundles, saves, or secrets. Review rights and upstream terms before any distribution; patches do not remove all IP risk.
