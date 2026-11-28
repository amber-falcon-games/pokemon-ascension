#ifndef GUARD_BATTLE_LAB_H
#define GUARD_BATTLE_LAB_H

#include "global.h"
#include "config/ascension.h"

// Pokémon Ascension Battle Lab (F007–F009): dev-build sandbox for real battles.
// Opened with L + SELECT on the title screen; compiled out of release builds.

enum { LAB_SIDE_PLAYER, LAB_SIDE_OPPONENT, LAB_SIDE_COUNT };

#define LAB_SLOT_ACTIVE    (1 << 0)   // sent into battle
#define LAB_SLOT_MODIFIED  (1 << 1)   // differs from the scenario: built from the slot fields
#define LAB_DEFAULT_LEVEL  50

enum LabStatus
{
    LAB_STATUS_NONE,
    LAB_STATUS_POISON,
    LAB_STATUS_TOXIC,
    LAB_STATUS_BURN,
    LAB_STATUS_PARALYSIS,
    LAB_STATUS_SLEEP,
    LAB_STATUS_FREEZE,
    LAB_STATUS_COUNT
};

struct LabSlot
{
    u16 species;
    u8 level;
    u8 flags;
    u16 moves[MAX_MON_MOVES];
    u16 item;
    u8 abilitySlot;     // 0, 1 or 2 (hidden)
    u8 nature;
    u8 ivs;             // applied to all six stats
    u8 hpPercent;       // 1–100
    u8 status;          // enum LabStatus
    u8 padding;
};

enum LabFormat { LAB_FORMAT_TRAINER_SINGLE, LAB_FORMAT_TRAINER_DOUBLE, LAB_FORMAT_WILD_SINGLE, LAB_FORMAT_COUNT };
enum LabWeather { LAB_WEATHER_NONE, LAB_WEATHER_RAIN, LAB_WEATHER_SUN, LAB_WEATHER_SAND, LAB_WEATHER_HAIL, LAB_WEATHER_SNOW, LAB_WEATHER_COUNT };
enum LabTerrain { LAB_TERRAIN_NONE, LAB_TERRAIN_ELECTRIC, LAB_TERRAIN_GRASSY, LAB_TERRAIN_MISTY, LAB_TERRAIN_PSYCHIC, LAB_TERRAIN_COUNT };
enum LabAi { LAB_AI_SCENARIO, LAB_AI_SMART, LAB_AI_RANDOM, LAB_AI_COUNT };
enum LabCombat { LAB_COMBAT_CLASSIC, LAB_COMBAT_STAMINA, LAB_COMBAT_DUAL, LAB_COMBAT_COUNT };   // F017 spike: D023 one bar; Dual = separate health and energy bars
enum LabEnergyScale { LAB_ENERGY_X1, LAB_ENERGY_X1_5, LAB_ENERGY_X2, LAB_ENERGY_X3, LAB_ENERGY_COUNT };
enum LabDamageScale { LAB_DAMAGE_100, LAB_DAMAGE_75, LAB_DAMAGE_50, LAB_DAMAGE_33, LAB_DAMAGE_COUNT };
enum LabCostScale { LAB_COST_100, LAB_COST_75, LAB_COST_50, LAB_COST_25, LAB_COST_COUNT };
enum LabBoost { LAB_BOOST_NONE, LAB_BOOST_SPEED, LAB_BOOST_POWER, LAB_BOOST_BOTH, LAB_BOOST_COUNT };   // bit 0 speed, bit 1 power

struct LabRules
{
    u8 format;
    u8 weather;
    u8 terrain;
    u8 ai;
    u8 fixedSeed:1;
    u8 talentMoves:1;   // F015: moves come from the talent tree (this byte was bool8 fixedSeed, so old records read as hand-set moves)
    u8 unusedRules:6;
    // F017 spike (D023). This byte was padding, so old scenario records read as Classic x1/100%/100%.
    u8 combat:2;        // enum LabCombat
    u8 energyScale:2;   // enum LabEnergyScale: max energy x1, x1.5, x2, x3
    u8 damageScale:2;   // enum LabDamageScale: damage 100%, 75%, 50%, 33%
    u8 costScale:2;     // enum LabCostScale: energy costs 100%, 75%, 50%, 25%
    u16 seed;
};

extern struct LabSlot gBattleLabTeams[LAB_SIDE_COUNT][PARTY_SIZE];
extern struct LabRules gBattleLabRules;

void BattleLab_Open(void);
void BattleLab_SetupScenarioParties(void);   // builds both battle parties from the Lab teams

// Team model (F008/F009); exposed for tests.
void BattleLab_ResetTeams(void);
void BattleLab_ResetSlot(u32 side, u32 slot);
void BattleLab_ClearSlot(u32 side, u32 slot);
bool32 BattleLab_IsSpeciesSelectable(u32 species);
u32 BattleLab_StepSpecies(u32 species, s32 steps);
u32 BattleLab_FirstSpeciesForLetter(u32 letter);
void BattleLab_SetSlotSpecies(u32 side, u32 slot, u32 species);
void BattleLab_SetSlotLevel(u32 side, u32 slot, s32 level);
void BattleLab_SetSlotActive(u32 side, u32 slot, bool32 active);
u32 BattleLab_ActiveCount(u32 side);
bool32 BattleLab_IsEdited(void);
bool32 BattleLab_SlotEdited(u32 side, u32 slot);

// Pokémon details (F009)
bool32 BattleLab_IsMoveSelectable(u32 move);
u32 BattleLab_StepMove(u32 move, s32 steps);
u32 BattleLab_FirstMoveForLetter(u32 letter);
bool32 BattleLab_IsItemSelectable(u32 item);
u32 BattleLab_StepItem(u32 item, s32 steps);
u32 BattleLab_FirstItemForLetter(u32 letter);
bool32 BattleLab_SpeciesCanLearn(u32 species, u32 move);
u32 BattleLab_StepAbilitySlot(u32 species, u32 abilitySlot, s32 dir);
void BattleLab_SetSlotMove(u32 side, u32 slot, u32 index, u32 move);
void BattleLab_SetSlotItem(u32 side, u32 slot, u32 item);
void BattleLab_SetSlotAbility(u32 side, u32 slot, u32 abilitySlot);
void BattleLab_SetSlotNature(u32 side, u32 slot, u32 nature);
void BattleLab_SetSlotIvs(u32 side, u32 slot, s32 ivs);
void BattleLab_SetSlotHpPercent(u32 side, u32 slot, s32 percent);
void BattleLab_SetSlotStatus(u32 side, u32 slot, u32 status);

// Battle rules (F009)
void BattleLab_ResetRules(void);
bool32 BattleLab_RulesEdited(void);
u64 BattleLab_OpponentAiFlags(void);
void BattleLab_ApplyStartingStatuses(void);
const u8 *BattleLab_ValidateStart(void);   // NULL if the battle can start, else a message
void BattleLab_TrimToWildOpponent(void);

// TRUE while a Battle Lab battle runs (used by small upstream hooks); always FALSE in release.
// BattleLab_ShouldAutoOpen: the ROM carries injected scenarios asking to open the Lab (F011).
#if ASCENSION_BATTLE_LAB
bool32 BattleLab_IsLabBattle(void);
// F019: campaign battles in the overhaul's combat mode (ASCENSION_COMBAT).
void BattleLab_CampaignBattleInit(void);    // battle init, after gBattleTypeFlags are set: decides the mode
void BattleLab_CampaignBattleStart(void);   // battle intro, once both parties exist: fills energy
bool32 BattleLab_CampaignBattle(void);
void BattleLab_OnBattleScreenReshown(void);           // after the battle screen is rebuilt
void BattleLab_OnPlayerStep(void);                   // walking recovers energy
void BattleLab_RestoreAllEnergy(struct Pokemon *mon);  // full heals
// PP items restore energy (F019). UseEnergyItem: apply = FALSE only checks it would do something.
u32 BattleLab_EnergyItemPercent(u32 item);
u32 BattleLab_CampaignMoveCost(u32 move, u32 maxEnergy);   // energy cost shown outside battle
bool32 BattleLab_EnergyItemsActive(void);
bool32 BattleLab_UseEnergyItem(struct Pokemon *mon, u32 item, bool32 apply);
bool32 BattleLab_TalentMoves(void);
bool32 BattleLab_TallHealthboxes(void);               // Dual bars, singles: energy bar inside a taller healthbox
bool32 BattleLab_EndTurnSkips(u32 state);              // F018: no new timeline beat, skip this end-of-turn step
bool32 BattleLab_TimelineBeat(void);
bool32 BattleLab_SatOutLastRound(u32 battler);       // F018: timeline round where this battler did not act
u32 BattleLab_PredictOrder(u8 *battlers, u32 count);   // F018: who acts next (timeline battles)
void BattleLab_DrawTimelineStrip(void);                 // F018: "Next: A > B > ..." at the top of the action menu                   // F015: a Lab battle whose rules use talent-tree moves
bool32 BattleLab_StaminaMode(void);                   // F017: a Lab battle with energy (Stamina or Dual)
bool32 BattleLab_DualMode(void);                      // ... with separate health and energy bars
bool32 BattleLab_DrawEnergyBar(u32 battler, u32 healthboxSpriteId);   // Dual: the player's EXP bar shows energy
u32 BattleLab_EnergyCost(u32 move, u32 maxEnergy);     // energy a move costs, as HP
void BattleLab_PayEnergy(u32 battler, u32 move);       // called where PP would be deducted
bool32 BattleLab_IsExhausted(u32 battler);
u32 BattleLab_ExhaustedSpeed(u32 battler, u32 speed);
bool32 BattleLab_ExhaustionLocksMove(u32 battler, u32 move);
bool32 BattleLab_TimelineSkips(u32 battler);           // timeline: this battler sits the round out
// F020 queue (D028): SELECT in the move menu queues a follow-up move.
struct AscQueueCheck
{
    bool8 samePokemon, moveUsable, exhausted, selfTarget, targetAlive, sameTarget, confusedBefore, confusedNow;
    u16 hpBefore, hpNow, maxHp;
    u32 statusBefore, statusNow;
};
bool32 BattleLab_QueueStillValid(const struct AscQueueCheck *c);
bool32 BattleLab_QueueToggle(u32 battler, u32 slot);
bool32 BattleLab_QueuedSlot(u32 battler, u32 slot);
void BattleLab_QueueArm(u32 battler);
bool32 BattleLab_TakeQueuedMove(u32 battler, u32 *slot, u32 *target);
void BattleLab_ClearQueue(u32 battler);
s32 BattleLab_ScaleDamage(s32 damage);                 // Rules "Damage" scale in stamina battles
void BattleLab_ScaleStartingEnergy(void);              // Rules "Energy" scale, applied to both parties
u32 BattleLab_BoostedCost(u32 battler, u32 move);      // Boost (R): energy cost with the chosen boost
u32 BattleLab_BoostPower(u32 battler, u32 power);
void BattleLab_CycleBoost(u32 battler);
void BattleLab_ClearBoost(u32 battler);
void BattleLab_ShowMoveCost(u32 battler, u32 move);    // move menu: cost in the PP box, preview on the bar
void BattleLab_EndMoveCostPreview(u32 battler);
void BattleLab_MoveMenuFrame(u32 battler);             // blinks the cost preview on the bar
const void *BattleLab_HealthbarLabelGfx(u32 battler, const void *hpLabel);   // "EN" (or red "EX" when Exhausted) in stamina battles
const u8 *BattleLab_UsedMoveText(u32 attacker, const u8 *defaultText);   // "X used Y!" plus why (boost, acts again, exhausted)
void BattleLab_AiTimerStart(void);                     // F017 measurement: AI decision time
bool32 BattleLab_TakePendingMessage(void);             // end of round: a Lab message to print?
const u8 *BattleLab_PendingMessageText(void);
void BattleLab_AiTimerStop(u32 battler);
bool32 BattleLab_ShouldAutoOpen(void);
void BattleLab_LogTurnEnd(void);   // F012: per-turn battle log (mGBA log window); no-op outside Lab battles
void BattleLab_LogBattleEnd(void); // F012: final state before the battle clears its data
#else
static inline bool32 BattleLab_StaminaMode(void) { return FALSE; }
static inline bool32 BattleLab_DualMode(void) { return FALSE; }
static inline bool32 BattleLab_DrawEnergyBar(u32 battler, u32 healthboxSpriteId) { return FALSE; }
static inline void BattleLab_PayEnergy(u32 battler, u32 move) {}
static inline bool32 BattleLab_IsExhausted(u32 battler) { return FALSE; }
static inline u32 BattleLab_ExhaustedSpeed(u32 battler, u32 speed) { return speed; }
static inline bool32 BattleLab_ExhaustionLocksMove(u32 battler, u32 move) { return FALSE; }
static inline bool32 BattleLab_TimelineSkips(u32 battler) { return FALSE; }
static inline bool32 BattleLab_QueueToggle(u32 battler, u32 slot) { return FALSE; }
static inline bool32 BattleLab_QueuedSlot(u32 battler, u32 slot) { return FALSE; }
static inline void BattleLab_QueueArm(u32 battler) {}
static inline bool32 BattleLab_TakeQueuedMove(u32 battler, u32 *slot, u32 *target) { return FALSE; }
static inline void BattleLab_ClearQueue(u32 battler) {}
static inline s32 BattleLab_ScaleDamage(s32 damage) { return damage; }
static inline u32 BattleLab_BoostPower(u32 battler, u32 power) { return power; }
static inline void BattleLab_CycleBoost(u32 battler) {}
static inline void BattleLab_ClearBoost(u32 battler) {}
static inline void BattleLab_ShowMoveCost(u32 battler, u32 move) {}
static inline void BattleLab_EndMoveCostPreview(u32 battler) {}
static inline void BattleLab_MoveMenuFrame(u32 battler) {}
static inline const void *BattleLab_HealthbarLabelGfx(u32 battler, const void *hpLabel) { return hpLabel; }
static inline const u8 *BattleLab_UsedMoveText(u32 attacker, const u8 *defaultText) { return defaultText; }
static inline void BattleLab_AiTimerStart(void) {}
static inline void BattleLab_AiTimerStop(u32 battler) {}
static inline bool32 BattleLab_IsLabBattle(void) { return FALSE; }
static inline void BattleLab_CampaignBattleInit(void) {}
static inline void BattleLab_CampaignBattleStart(void) {}
static inline bool32 BattleLab_CampaignBattle(void) { return FALSE; }
static inline void BattleLab_OnBattleScreenReshown(void) {}
static inline void BattleLab_OnPlayerStep(void) {}
static inline void BattleLab_RestoreAllEnergy(struct Pokemon *mon) {}
static inline u32 BattleLab_EnergyItemPercent(u32 item) { return 0; }
static inline u32 BattleLab_CampaignMoveCost(u32 move, u32 maxEnergy) { return 0; }
static inline bool32 BattleLab_EnergyItemsActive(void) { return FALSE; }
static inline bool32 BattleLab_UseEnergyItem(struct Pokemon *mon, u32 item, bool32 apply) { return FALSE; }
static inline bool32 BattleLab_TalentMoves(void) { return FALSE; }
static inline bool32 BattleLab_TallHealthboxes(void) { return FALSE; }
static inline bool32 BattleLab_EndTurnSkips(u32 state) { return FALSE; }
static inline bool32 BattleLab_TimelineBeat(void) { return FALSE; }
static inline bool32 BattleLab_SatOutLastRound(u32 battler) { return FALSE; }
static inline u32 BattleLab_PredictOrder(u8 *battlers, u32 count) { return 0; }
static inline void BattleLab_DrawTimelineStrip(void) {}
static inline bool32 BattleLab_ShouldAutoOpen(void) { return FALSE; }
static inline void BattleLab_LogTurnEnd(void) {}
static inline void BattleLab_LogBattleEnd(void) {}
#endif

// Determinism and diagnostics (F012)
u32 BattleLab_LastSeed(void);
bool32 BattleLab_HasLastSeed(void);
bool32 BattleLab_UseLastSeed(void);
void BattleLab_LogBattleStart(void);

// Scenarios (F011, D022): repository files injected into a reserved ROM block of a Lab ROM copy.
// Block layout (tools/ascension/lab_scenario.py): 16-byte ASCII marker "ASCENSION-LAB-01",
// then version (1), count, auto-open (0 = no, n = open scenario n-1), reserved, then `count`
// records. Little-endian, same layout as these structs.
#define LAB_SCENARIO_BLOCK_SIZE  4096
#define LAB_MAX_BLOCK_SCENARIOS  14   // (4096 - 20) / 280

struct LabScenarioRecord
{
    u8 id[8];           // game charset, EOS-terminated, e.g. "LAB-009"
    u8 name[24];
    struct LabRules rules;
    struct LabSlot teams[LAB_SIDE_COUNT][PARTY_SIZE];
};

extern const u8 gBattleLabScenarioBlock[LAB_SCENARIO_BLOCK_SIZE];
extern struct LabScenarioRecord gBattleLabScratchRecord;
u32 BattleLab_BlockScenarioCount(void);
u32 BattleLab_ScenarioCount(void);          // built-in LAB-001 + block scenarios
s32 BattleLab_AutoOpenScenario(void);       // -1 if none
bool32 BattleLab_SelectScenario(u32 index); // 0 = built-in

// Battle loop (F010)
enum LabResult
{
    LAB_RESULT_NONE, LAB_RESULT_WON, LAB_RESULT_LOST, LAB_RESULT_DREW, LAB_RESULT_RAN,
    LAB_RESULT_CAUGHT, LAB_RESULT_QUIT, LAB_RESULT_RESTARTED, LAB_RESULT_COUNT
};
bool32 BattleLab_RequestEnd(bool32 restart);   // from the in-battle debug menu; FALSE outside Lab battles
u32 BattleLab_ResultFromOutcome(u32 outcome, bool32 quit, bool32 restart);
const u8 *BattleLab_ResultName(u32 result);
u32 BattleLab_TurnsPlayed(u32 battleTurnCounter);

#if TESTING
extern bool8 gAscensionCombatTestOverride;
void BattleLab_ResetForTest(void);
#endif

#endif // GUARD_BATTLE_LAB_H
