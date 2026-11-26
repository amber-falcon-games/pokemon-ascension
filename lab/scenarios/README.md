# Battle Lab scenario files

One JSON file per scenario (F011, decision D022). `./scripts/play.sh --lab` loads all of them into
a separate Lab ROM (`build/pokemon-ascension-lab.gba`, with its own save file) and opens the
Battle Lab; `./scripts/play.sh --scenario LAB-009` does the same and opens with that scenario.
In the Lab, **Scenario** lists the built-in LAB-001 and every file here. Nothing is stored in a
save. Check files with `python3 tools/ascension/lab_scenario.py check lab/scenarios/*.json`.
Up to 14 files fit.

```json
{
  "id": "LAB-009",                       // up to 7 characters, unique
  "name": "Six-member trainer",          // up to 23 characters, shown in the Lab
  "rules": {
    "format": "trainer_single",          // trainer_single | trainer_double | wild_single
    "weather": "none",                   // none | rain | sun | sand | hail | snow
    "terrain": "none",                   // none | electric | grassy | misty | psychic
    "ai": "scenario",                    // scenario (basic trainer) | smart | random
    "seed": null                         // null = random, or 0-65535 for a repeatable run
  },
  "player":   [ { ... up to 6 Pokémon ... } ],
  "opponent": [ { ... up to 6 Pokémon ... } ]
}
```

Each Pokémon (only `species` is required):

| Field | Default | Example |
|---|---|---|
| `species` | — | `"SWAMPERT"` (constant name without `SPECIES_`; forms as in the game, e.g. `"ROTOM_WASH"`) |
| `level` | 50 | 1–100 |
| `moves` | level-up moves | up to 4, e.g. `["WATERFALL", "ICE_BEAM"]` |
| `item` | none | `"LEFTOVERS"` |
| `ability` | 1 | `1`, `2` or `"hidden"` |
| `nature` | Hardy | `"ADAMANT"` |
| `ivs` | 31 | 0–31 (all six stats) |
| `hp` | 100 | starting HP %, 1–100 |
| `status` | none | none, poison, toxic, burn, paralysis, sleep, freeze |
| `battle` | true | false = listed but not sent into battle |

JSON has no comments: the `//` notes above are for this page only.
