// Pokémon Ascension Battle Lab (F007).
// A developer-only screen, opened with L + SELECT on the title screen, that starts
// real battles from scenario data (src/data/battle_lab_trainers.party) and returns
// to itself afterwards. It never writes the save: Exit soft-resets, which reloads
// the campaign save from flash. Compiled out of release builds (ASCENSION_BATTLE_LAB).
#include "global.h"
#include "constants/battle_end_turn.h"
#include "util.h"
#include "config/ascension.h"
#include "battle_lab.h"
#include "graphics.h"

#if ASCENSION_BATTLE_LAB

#include "battle.h"
#include "battle_controllers.h"
#include "battle_interface.h"
#include "battle_message.h"
#include "battle_main.h"
#include "battle_setup.h"
#include "bg.h"
#include "data.h"
#include "debug.h"
#include "gpu_regs.h"
#include "main.h"
#include "menu.h"
#include "new_game.h"
#include "palette.h"
#include "pokemon.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "random.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "battle_z_move.h"
#include "item.h"
#include "move.h"
#include "constants/battle_ai.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/rgb.h"
#include "constants/songs.h"

enum LabTrainerIds
{
    LAB_TRAINER_001_PLAYER,
    LAB_TRAINER_001_OPPONENT,
    LAB_TRAINERS_COUNT
};

static const struct Trainer sLabTrainers[DIFFICULTY_COUNT][LAB_TRAINERS_COUNT] =
{
#include "data/battle_lab_trainers.h"
};

enum
{
    WIN_TITLE,
    WIN_MENU,
    WIN_INFO,
};

enum
{
    LAB_MENU_PLAYER_TEAM,
    LAB_MENU_OPPONENT_TEAM,
    LAB_MENU_RULES,
    LAB_MENU_SCENARIO,
    LAB_MENU_START,
    LAB_MENU_EXIT,
    LAB_MENU_COUNT
};

#define FRAME_TILE   500
#define FRAME_PAL    14
#define TEXT_PAL     15

static const struct BgTemplate sLabBgTemplates[] =
{
    {.bg = 0, .charBaseIndex = 0, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0},
};

static const struct WindowTemplate sLabWindowTemplates[] =
{
    [WIN_TITLE] = {.bg = 0, .tilemapLeft = 1,  .tilemapTop = 1, .width = 28, .height = 2,  .paletteNum = TEXT_PAL, .baseBlock = 1},
    [WIN_MENU]  = {.bg = 0, .tilemapLeft = 1,  .tilemapTop = 4, .width = 15, .height = 15, .paletteNum = TEXT_PAL, .baseBlock = 57},
    [WIN_INFO]  = {.bg = 0, .tilemapLeft = 19, .tilemapTop = 5, .width = 10, .height = 12, .paletteNum = TEXT_PAL, .baseBlock = 282},
    DUMMY_WIN_TEMPLATE,
};

static const struct MenuAction sLabMenuItems[LAB_MENU_COUNT] =
{
    [LAB_MENU_PLAYER_TEAM]   = {COMPOUND_STRING("Player team"),   {NULL}},
    [LAB_MENU_OPPONENT_TEAM] = {COMPOUND_STRING("Opponent team"), {NULL}},
    [LAB_MENU_RULES]         = {COMPOUND_STRING("Rules"),         {NULL}},
    [LAB_MENU_SCENARIO]      = {COMPOUND_STRING("Scenario"),      {NULL}},
    [LAB_MENU_START]         = {COMPOUND_STRING("Start battle"),  {NULL}},
    [LAB_MENU_EXIT]          = {COMPOUND_STRING("Exit"),          {NULL}},
};

static const u8 sText_TeamHelp[] = _("A: edit\nB: back\nSTART:\nreset all\n! = edited");
static const u8 sText_ConfirmReset[] = _("Reset both\nteams to the\nscenario?\n\nA: yes\nB: no");

static const u16 sLabBackdrop = RGB(3, 5, 9);

static EWRAM_DATA u8 sLabCursor = 0;
static EWRAM_DATA bool8 sLabNeedsFreshGame = FALSE;
EWRAM_DATA struct LabSlot gBattleLabTeams[LAB_SIDE_COUNT][PARTY_SIZE] = {0};
EWRAM_DATA struct LabRules gBattleLabRules = {0};
static EWRAM_DATA bool8 sLabInBattleFlag = FALSE;   // a Lab battle is running

// ---------------------------------------------------------------- scenarios (F011, D022)
// Scenarios come from repository files (lab/scenarios/*.json). tools/ascension/lab_scenario.py
// writes them into this reserved block of a Lab ROM copy; the block is all 0xFF in the built
// ROM, so a plain dev ROM has only the built-in LAB-001. Dev builds only; never in the save.
#define LAB_BLOCK_MARKER_SIZE 16
#define LAB_BLOCK_VERSION     1
__attribute__((aligned(4))) const u8 gBattleLabScenarioBlock[LAB_SCENARIO_BLOCK_SIZE] =
{
    'A', 'S', 'C', 'E', 'N', 'S', 'I', 'O', 'N', '-', 'L', 'A', 'B', '-', '0', '1',
    [LAB_BLOCK_MARKER_SIZE ... LAB_SCENARIO_BLOCK_SIZE - 1] = 0xFF,
};
STATIC_ASSERT(sizeof(struct LabScenarioRecord) == 280, LabScenarioRecordSize);

static EWRAM_DATA struct LabScenarioRecord sLabCurrent = {0};   // what Reset returns to
static EWRAM_DATA u8 sLabScenarioIndex = 0;                     // 0 = built-in LAB-001
static EWRAM_DATA u8 sLabScenarioCursor = 0;
EWRAM_DATA struct LabScenarioRecord gBattleLabScratchRecord = {0};

// Team screen state (task-independent so it survives returning from a battle)
enum { LAB_MODE_MAIN, LAB_MODE_TEAM, LAB_MODE_EDIT, LAB_MODE_LETTER, LAB_MODE_CONFIRM_RESET, LAB_MODE_RULES, LAB_MODE_SCENARIO };
enum
{
    EDIT_ROW_SPECIES, EDIT_ROW_LEVEL, EDIT_ROW_ACTIVE,
    EDIT_ROW_MOVE1, EDIT_ROW_MOVE2, EDIT_ROW_MOVE3, EDIT_ROW_MOVE4,
    EDIT_ROW_ABILITY, EDIT_ROW_ITEM, EDIT_ROW_NATURE, EDIT_ROW_IVS, EDIT_ROW_HP, EDIT_ROW_STATUS,
    EDIT_ROW_RESET, EDIT_ROW_CLEAR, EDIT_ROW_COUNT
};
enum { RULES_ROW_FORMAT, RULES_ROW_COMBAT, RULES_ROW_ENERGY, RULES_ROW_DAMAGE, RULES_ROW_COST, RULES_ROW_MOVES, RULES_ROW_WEATHER, RULES_ROW_TERRAIN, RULES_ROW_AI, RULES_ROW_SEED_MODE, RULES_ROW_SEED, RULES_ROW_COUNT };
#define RULES_VISIBLE_ROWS 10
#define LAB_VISIBLE_ROWS 6
static EWRAM_DATA u8 sLabMode = 0;
static EWRAM_DATA u8 sLabSide = 0;
static EWRAM_DATA u8 sLabSlot = 0;
static EWRAM_DATA u8 sLabEditRow = 0;
static EWRAM_DATA u8 sLabLetter = 0;
static EWRAM_DATA u8 sLabEditScroll = 0;
static EWRAM_DATA u8 sLabRulesRow = 0;
static EWRAM_DATA u8 sLabLetterRow = 0;   // editor row the letter picker fills
// Battle loop (F010)
enum { LAB_END_NONE, LAB_END_QUIT, LAB_END_RESTART };
static EWRAM_DATA u8 sLabEndRequest = LAB_END_NONE;
static EWRAM_DATA u8 sLabLastResult = LAB_RESULT_NONE;
static EWRAM_DATA u8 sLabLastTurns = 0;

static void CB2_BattleLab_Init(void);
static void CB2_BattleLab_Main(void);
static void CB2_BattleLab_ReturnFromBattle(void);
static void VBlankCB_BattleLab(void);
static void Task_BattleLab_Input(u8 taskId);
static void Task_BattleLab_StartBattle(u8 taskId);
static void Task_BattleLab_Exit(u8 taskId);
static void BattleLab_PrintInfo(const u8 *text);
static void BattleLab_DrawTitle(void);
static void BattleLab_DrawMainInfo(void);
static void BattleLab_DrawTeam(void);
static void BattleLab_DrawEditor(void);
static void BattleLab_DrawLetters(void);
static void BattleLab_ShowMainMenu(void);
static void BattleLab_DrawScenarios(void);
static void BattleLab_HandleScenarioInput(void);
static void LabEnsureScenario(void);
static void BattleLab_HandleTeamInput(void);
static void BattleLab_HandleEditInput(void);
static void BattleLab_HandleLetterInput(void);
static void BattleLab_DrawRules(void);
static void BattleLab_HandleRulesInput(void);

void BattleLab_Open(void)
{
    sLabCursor = LAB_MENU_START;
    sLabNeedsFreshGame = TRUE;
    sLabMode = LAB_MODE_MAIN;
    {
        s32 start = BattleLab_AutoOpenScenario();
        BattleLab_SelectScenario(start > 0 ? start : 0);
    }
    FadeOutBGM(4);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    SetMainCallback2(CB2_BattleLab_Init);
    gMain.state = 0;
}

// ---------------------------------------------------------------- team model (F008)

static const struct Trainer *BattleLab_ScenarioTrainer(u32 side)
{
    return &sLabTrainers[DIFFICULTY_NORMAL][side == LAB_SIDE_PLAYER ? LAB_TRAINER_001_PLAYER : LAB_TRAINER_001_OPPONENT];
}

static void LabFillLevelUpMoves(struct LabSlot *s)
{
    struct Pokemon *mon = &gParties[B_TRAINER_OPPONENT_B][0];   // scratch: unused outside multi battles
    u32 i;
    CreateMon(mon, s->species, s->level, 0, OTID_STRUCT_PLAYER_ID);
    GiveMonInitialMoveset(mon);
    for (i = 0; i < MAX_MON_MOVES; i++)
        s->moves[i] = GetMonData(mon, MON_DATA_MOVE1 + i);
    ZeroMonData(mon);
}

static u32 LabAbilitySlotFor(u32 species, u32 ability)
{
    u32 i;
    for (i = 0; i < NUM_ABILITY_SLOTS; i++)
        if (ability != ABILITY_NONE && gSpeciesInfo[species].abilities[i] == ability)
            return i;
    return 0;
}

static void LabSlotFromTrainer(u32 side, u32 slot, struct LabSlot *s)
{
    const struct Trainer *trainer = BattleLab_ScenarioTrainer(side);
    u32 i;

    memset(s, 0, sizeof(*s));
    s->level = LAB_DEFAULT_LEVEL;
    s->ivs = MAX_PER_STAT_IVS;
    s->hpPercent = 100;
    if (slot < trainer->partySize)
    {
        const struct TrainerMon *tm = &trainer->party[slot];
        s->species = tm->species;
        s->level = tm->lvl;
        s->flags = LAB_SLOT_ACTIVE;
        for (i = 0; i < MAX_MON_MOVES; i++)
            s->moves[i] = tm->moves[i];
        if (s->moves[0] == MOVE_NONE)
            LabFillLevelUpMoves(s);
        s->item = tm->heldItem;
        s->abilitySlot = LabAbilitySlotFor(tm->species, tm->ability);
        s->nature = tm->nature < NUM_NATURES ? tm->nature : 0;
        s->ivs = tm->iv & 0x1F;
    }
}

void BattleLab_ResetSlot(u32 side, u32 slot)
{
    LabEnsureScenario();
    gBattleLabTeams[side][slot] = sLabCurrent.teams[side][slot];
}

void BattleLab_ResetTeams(void)
{
    u32 side, slot;
    for (side = 0; side < LAB_SIDE_COUNT; side++)
        for (slot = 0; slot < PARTY_SIZE; slot++)
            BattleLab_ResetSlot(side, slot);
}

void BattleLab_ClearSlot(u32 side, u32 slot)
{
    struct LabSlot *s = &gBattleLabTeams[side][slot];
    memset(s, 0, sizeof(*s));
    s->level = LAB_DEFAULT_LEVEL;
    s->ivs = MAX_PER_STAT_IVS;
    s->hpPercent = 100;
    s->flags = LAB_SLOT_MODIFIED;
}

// Species the Lab offers: enabled, not an Egg, and not a form that only exists mid-battle.
bool32 BattleLab_IsSpeciesSelectable(u32 species)
{
    const struct SpeciesInfo *info;
    if (species == SPECIES_NONE || species >= NUM_SPECIES || !IsSpeciesEnabled(species))
        return FALSE;
    info = &gSpeciesInfo[species];
    return !(info->isMegaEvolution || info->isPrimalReversion || info->isUltraBurst
          || info->isGigantamax || info->isTeraForm);
}

// Order: National Dex number, then species id (so forms follow their base species).
static bool32 LabSpeciesBefore(u32 a, u32 b)
{
    u32 da = gSpeciesInfo[a].natDexNum, db = gSpeciesInfo[b].natDexNum;
    return da < db || (da == db && a < b);
}

static u32 LabNextSpecies(u32 species, bool32 forward)
{
    u32 i, best = SPECIES_NONE, wrap = SPECIES_NONE;
    for (i = 1; i < NUM_SPECIES; i++)
    {
        if (!BattleLab_IsSpeciesSelectable(i) || i == species)
            continue;
        if (forward)
        {
            if (species == SPECIES_NONE || LabSpeciesBefore(species, i))
            {
                if (best == SPECIES_NONE || LabSpeciesBefore(i, best))
                    best = i;
            }
            if (wrap == SPECIES_NONE || LabSpeciesBefore(i, wrap))
                wrap = i;   // first overall
        }
        else
        {
            if (species == SPECIES_NONE || LabSpeciesBefore(i, species))
            {
                if (best == SPECIES_NONE || LabSpeciesBefore(best, i))
                    best = i;
            }
            if (wrap == SPECIES_NONE || LabSpeciesBefore(wrap, i))
                wrap = i;   // last overall
        }
    }
    return best != SPECIES_NONE ? best : wrap;
}

u32 BattleLab_StepSpecies(u32 species, s32 steps)
{
    bool32 forward = steps > 0;
    if (steps < 0)
        steps = -steps;
    while (steps-- > 0)
        species = LabNextSpecies(species, forward);
    return species;
}

// First selectable species (in Lab order) whose name starts with letter 0..25 (A..Z).
u32 BattleLab_FirstSpeciesForLetter(u32 letter)
{
    u32 i, best = SPECIES_NONE;
    for (i = 1; i < NUM_SPECIES; i++)
    {
        if (!BattleLab_IsSpeciesSelectable(i) || GetSpeciesName(i)[0] != CHAR_A + letter)
            continue;
        if (best == SPECIES_NONE || LabSpeciesBefore(i, best))
            best = i;
    }
    return best;
}

// 1 for a base species, 2+ for the Nth selectable entry sharing its dex number.
static u32 LabFormNumber(u32 species)
{
    u32 i, n = 1;
    for (i = 1; i < species; i++)
        if (BattleLab_IsSpeciesSelectable(i) && gSpeciesInfo[i].natDexNum == gSpeciesInfo[species].natDexNum)
            n++;
    return n;
}

void BattleLab_SetSlotSpecies(u32 side, u32 slot, u32 species)
{
    struct LabSlot *s = &gBattleLabTeams[side][slot];
    if (s->species == SPECIES_NONE)
        s->flags |= LAB_SLOT_ACTIVE;
    s->species = species;
    s->flags |= LAB_SLOT_MODIFIED;
    s->abilitySlot = 0;
    if (species != SPECIES_NONE)
        LabFillLevelUpMoves(s);
}

void BattleLab_SetSlotLevel(u32 side, u32 slot, s32 level)
{
    struct LabSlot *s = &gBattleLabTeams[side][slot];
    if (level < 1)
        level = 1;
    if (level > MAX_LEVEL)
        level = MAX_LEVEL;
    if (s->level != level)
        s->flags |= LAB_SLOT_MODIFIED;
    s->level = level;
}

void BattleLab_SetSlotActive(u32 side, u32 slot, bool32 active)
{
    struct LabSlot *s = &gBattleLabTeams[side][slot];
    if (active)
        s->flags |= LAB_SLOT_ACTIVE;
    else
        s->flags &= ~LAB_SLOT_ACTIVE;
}

static bool32 LabSlotInBattle(const struct LabSlot *s)
{
    return s->species != SPECIES_NONE && (s->flags & LAB_SLOT_ACTIVE);
}

u32 BattleLab_ActiveCount(u32 side)
{
    u32 slot, n = 0;
    for (slot = 0; slot < PARTY_SIZE; slot++)
        n += LabSlotInBattle(&gBattleLabTeams[side][slot]);
    return n;
}


// ---------------------------------------------------------------- Pokémon details (F009)

// Moves: any real move (Z-Moves and Max Moves sit at or above MOVES_COUNT, so they are excluded).
bool32 BattleLab_IsMoveSelectable(u32 move)
{
    return move > MOVE_NONE && move < MOVES_COUNT && GetMoveName(move)[0] != EOS;
}

// Steps through MOVE_NONE, then every selectable move in id order, wrapping.
u32 BattleLab_StepMove(u32 move, s32 steps)
{
    s32 dir = steps > 0 ? 1 : -1;
    if (steps < 0)
        steps = -steps;
    while (steps-- > 0)
    {
        do
        {
            move = (move + MOVES_COUNT + dir) % MOVES_COUNT;
        } while (move != MOVE_NONE && !BattleLab_IsMoveSelectable(move));
    }
    return move;
}

u32 BattleLab_FirstMoveForLetter(u32 letter)
{
    u32 move;
    for (move = 1; move < MOVES_COUNT; move++)
        if (BattleLab_IsMoveSelectable(move) && GetMoveName(move)[0] == CHAR_A + letter)
            return move;
    return MOVE_NONE;
}

// Items a Pokémon can hold: not key items or TMs/HMs, and with a real name.
bool32 BattleLab_IsItemSelectable(u32 item)
{
    const u8 *name;
    enum Pocket pocket;
    if (item == ITEM_NONE || item >= ITEMS_COUNT)
        return FALSE;
    pocket = GetItemPocket(item);
    if (pocket == POCKET_KEY_ITEMS || pocket == POCKET_TM_HM)
        return FALSE;
    name = GetItemName(item);
    return name[0] != EOS && name[0] != CHAR_QUESTION_MARK;
}

u32 BattleLab_StepItem(u32 item, s32 steps)
{
    s32 dir = steps > 0 ? 1 : -1;
    if (steps < 0)
        steps = -steps;
    while (steps-- > 0)
    {
        do
        {
            item = (item + ITEMS_COUNT + dir) % ITEMS_COUNT;
        } while (item != ITEM_NONE && !BattleLab_IsItemSelectable(item));
    }
    return item;
}

u32 BattleLab_FirstItemForLetter(u32 letter)
{
    u32 item;
    for (item = 1; item < ITEMS_COUNT; item++)
        if (BattleLab_IsItemSelectable(item) && GetItemName(item)[0] == CHAR_A + letter)
            return item;
    return ITEM_NONE;
}

// Level-up, teachable or egg move for this species (informational only: the Lab allows any move).
bool32 BattleLab_SpeciesCanLearn(u32 species, u32 move)
{
    const struct LevelUpMove *levelUp = GetSpeciesLevelUpLearnset(species);
    const u16 *list;
    u32 i;

    if (move == MOVE_NONE)
        return TRUE;
    for (i = 0; levelUp != NULL && levelUp[i].move != LEVEL_UP_MOVE_END; i++)
        if (levelUp[i].move == move)
            return TRUE;
    list = GetSpeciesTeachableLearnset(species);
    for (i = 0; list != NULL && list[i] != MOVE_UNAVAILABLE; i++)
        if (list[i] == move)
            return TRUE;
    list = GetSpeciesEggMoves(species);
    for (i = 0; list != NULL && list[i] != MOVE_UNAVAILABLE; i++)
        if (list[i] == move)
            return TRUE;
    return FALSE;
}

// Next ability slot (0, 1, hidden) that the species actually has.
u32 BattleLab_StepAbilitySlot(u32 species, u32 abilitySlot, s32 dir)
{
    u32 i;
    for (i = 0; i < NUM_ABILITY_SLOTS; i++)
    {
        abilitySlot = (abilitySlot + NUM_ABILITY_SLOTS + (dir > 0 ? 1 : -1)) % NUM_ABILITY_SLOTS;
        if (gSpeciesInfo[species].abilities[abilitySlot] != ABILITY_NONE)
            return abilitySlot;
    }
    return 0;
}

static struct LabSlot *LabEditSlot(u32 side, u32 slot)
{
    struct LabSlot *s = &gBattleLabTeams[side][slot];
    s->flags |= LAB_SLOT_MODIFIED;
    return s;
}

void BattleLab_SetSlotMove(u32 side, u32 slot, u32 index, u32 move) { LabEditSlot(side, slot)->moves[index] = move; }
void BattleLab_SetSlotItem(u32 side, u32 slot, u32 item)           { LabEditSlot(side, slot)->item = item; }
void BattleLab_SetSlotAbility(u32 side, u32 slot, u32 abilitySlot) { LabEditSlot(side, slot)->abilitySlot = abilitySlot; }
void BattleLab_SetSlotNature(u32 side, u32 slot, u32 nature)       { LabEditSlot(side, slot)->nature = nature % NUM_NATURES; }
void BattleLab_SetSlotStatus(u32 side, u32 slot, u32 status)       { LabEditSlot(side, slot)->status = status % LAB_STATUS_COUNT; }

void BattleLab_SetSlotIvs(u32 side, u32 slot, s32 ivs)
{
    LabEditSlot(side, slot)->ivs = ivs < 0 ? 0 : (ivs > MAX_PER_STAT_IVS ? MAX_PER_STAT_IVS : ivs);
}

void BattleLab_SetSlotHpPercent(u32 side, u32 slot, s32 percent)
{
    LabEditSlot(side, slot)->hpPercent = percent < 1 ? 1 : (percent > 100 ? 100 : percent);
}

static u32 LabStatusToStatus1(u32 status)
{
    switch (status)
    {
    case LAB_STATUS_POISON:    return STATUS1_POISON;
    case LAB_STATUS_TOXIC:     return STATUS1_TOXIC_POISON;
    case LAB_STATUS_BURN:      return STATUS1_BURN;
    case LAB_STATUS_PARALYSIS: return STATUS1_PARALYSIS;
    case LAB_STATUS_SLEEP:     return STATUS1_SLEEP_TURN(3);
    case LAB_STATUS_FREEZE:    return STATUS1_FREEZE;
    default:                   return 0;
    }
}

// Builds one Pokémon from every slot field.
static void LabBuildMonFromSlot(struct Pokemon *mon, const struct LabSlot *s)
{
    u32 i, value;

    CreateMonWithIVs(mon, s->species, s->level, Random32(), OTID_STRUCT_PLAYER_ID, s->ivs);
    for (i = 0; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(mon, s->moves[i], i);
    value = s->item;
    SetMonData(mon, MON_DATA_HELD_ITEM, &value);
    value = s->abilitySlot;
    SetMonData(mon, MON_DATA_ABILITY_NUM, &value);
    value = s->nature;
    SetMonData(mon, MON_DATA_HIDDEN_NATURE, &value);
    CalculateMonStats(mon);
    value = GetMonData(mon, MON_DATA_MAX_HP) * s->hpPercent / 100;
    if (value == 0)
        value = 1;
    SetMonData(mon, MON_DATA_HP, &value);
    value = LabStatusToStatus1(s->status);
    SetMonData(mon, MON_DATA_STATUS, &value);
}

// ---------------------------------------------------------------- battle rules (F009)

void BattleLab_ResetRules(void)
{
    LabEnsureScenario();
    gBattleLabRules = sLabCurrent.rules;
}

bool32 BattleLab_RulesEdited(void)
{
    LabEnsureScenario();
    return memcmp(&gBattleLabRules, &sLabCurrent.rules, sizeof(gBattleLabRules)) != 0;
}

u64 BattleLab_OpponentAiFlags(void)
{
    switch (gBattleLabRules.ai)
    {
    case LAB_AI_SMART:  return AI_FLAG_SMART_TRAINER;
    case LAB_AI_RANDOM: return 0;   // no scoring flags: any usable move
    default:            return BattleLab_ScenarioTrainer(LAB_SIDE_OPPONENT)->aiFlags;
    }
}

// Weather and terrain for the whole battle, via upstream's starting statuses.
void BattleLab_ApplyStartingStatuses(void)
{
    memset(&gStartingStatuses, 0, sizeof(gStartingStatuses));
    switch (gBattleLabRules.weather)
    {
    case LAB_WEATHER_RAIN: gStartingStatuses.weatherRain = TRUE; break;
    case LAB_WEATHER_SUN:  gStartingStatuses.weatherSun = TRUE; break;
    case LAB_WEATHER_SAND: gStartingStatuses.weatherSandstorm = TRUE; break;
    case LAB_WEATHER_HAIL: gStartingStatuses.weatherHail = TRUE; break;
    case LAB_WEATHER_SNOW: gStartingStatuses.weatherSnow = TRUE; break;
    }
    switch (gBattleLabRules.terrain)
    {
    case LAB_TERRAIN_ELECTRIC: gStartingStatuses.electricTerrain = TRUE; break;
    case LAB_TERRAIN_GRASSY:   gStartingStatuses.grassyTerrain = TRUE; break;
    case LAB_TERRAIN_MISTY:    gStartingStatuses.mistyTerrain = TRUE; break;
    case LAB_TERRAIN_PSYCHIC:  gStartingStatuses.psychicTerrain = TRUE; break;
    }
}

// Wild 1v1 uses only the opponent's first active slot (the party is already compacted).
void BattleLab_TrimToWildOpponent(void)
{
    u32 i;
    for (i = 1; i < PARTY_SIZE; i++)
        ZeroMonData(&gParties[B_TRAINER_OPPONENT_A][i]);
    CalculateEnemyPartyCount();
}

bool32 BattleLab_IsLabBattle(void)
{
    return sLabInBattleFlag;
}

bool32 BattleLab_TalentMoves(void)
{
    return sLabInBattleFlag && gBattleLabRules.talentMoves;
}

const u8 *BattleLab_ValidateStart(void)
{
    u32 need = (gBattleLabRules.format == LAB_FORMAT_TRAINER_DOUBLE) ? 2 : 1;
    if (BattleLab_ActiveCount(LAB_SIDE_PLAYER) < need)
        return need == 2 ? COMPOUND_STRING("Doubles:\nyou need 2\nin battle.") : COMPOUND_STRING("Your team\nhas nobody\nin battle.");
    if (BattleLab_ActiveCount(LAB_SIDE_OPPONENT) < need)
        return need == 2 ? COMPOUND_STRING("Doubles:\nopponent\nneeds 2.") : COMPOUND_STRING("Opponent\nhas nobody\nin battle.");
    return NULL;
}

// TRUE if any slot differs from the scenario (species, level, or in-battle setting).
// TRUE if this slot differs from the loaded scenario (the "modified" build flag is ignored).
bool32 BattleLab_SlotEdited(u32 side, u32 slot)
{
    struct LabSlot a = gBattleLabTeams[side][slot];
    struct LabSlot b;
    LabEnsureScenario();
    b = sLabCurrent.teams[side][slot];
    a.flags &= ~LAB_SLOT_MODIFIED;
    b.flags &= ~LAB_SLOT_MODIFIED;
    if (a.species == SPECIES_NONE && b.species == SPECIES_NONE)
        return FALSE;
    return memcmp(&a, &b, sizeof(a)) != 0;
}

// TRUE if anything differs from the loaded scenario.
bool32 BattleLab_IsEdited(void)
{
    u32 side, slot;
    if (BattleLab_RulesEdited())
        return TRUE;
    for (side = 0; side < LAB_SIDE_COUNT; side++)
        for (slot = 0; slot < PARTY_SIZE; slot++)
            if (BattleLab_SlotEdited(side, slot))
                return TRUE;
    return FALSE;
}

// ---------------------------------------------------------------- scenario loading (F011)

static const volatile u8 *LabBlock(void)
{
    // volatile: the block is patched after the build, so its contents must never be constant-folded.
    return (const volatile u8 *)gBattleLabScenarioBlock;
}

static bool32 LabBlockValid(void)
{
    const volatile u8 *b = LabBlock();
    u32 count = b[LAB_BLOCK_MARKER_SIZE + 1];
    return b[LAB_BLOCK_MARKER_SIZE] == LAB_BLOCK_VERSION
        && count <= LAB_MAX_BLOCK_SCENARIOS;
}

u32 BattleLab_BlockScenarioCount(void)
{
    return LabBlockValid() ? LabBlock()[LAB_BLOCK_MARKER_SIZE + 1] : 0;
}

u32 BattleLab_ScenarioCount(void)
{
    return 1 + BattleLab_BlockScenarioCount();
}

// Scenario to open with (0 = built-in), or -1 if the ROM doesn't ask to open the Lab.
s32 BattleLab_AutoOpenScenario(void)
{
    u32 value;
    if (!LabBlockValid())
        return -1;
    value = LabBlock()[LAB_BLOCK_MARKER_SIZE + 2];
    if (value == 0 || value > BattleLab_ScenarioCount())
        return -1;
    return value - 1;
}

// Decided once per boot (the first title-screen frame) and latched, so holding B at boot keeps the
// normal title screen even after B is released. A soft reset (e.g. Lab Exit) clears EWRAM and decides again.
enum { LAB_AUTO_UNDECIDED, LAB_AUTO_OPEN, LAB_AUTO_SKIP };
static EWRAM_DATA u8 sLabAutoOpenDecision = LAB_AUTO_UNDECIDED;
static EWRAM_DATA u16 sLabLastSeed = 0;
static EWRAM_DATA bool8 sLabHasLastSeed = FALSE;

// ---------------------------------------------------------------- stamina combat (F017 spike, D023)
// One energy bar: the battler's HP. Moves spend it; damage drains it as usual; 0 = fainted.
// Spike numbers, tuned in the Lab: damaging move ~ power/6 % of max, status 5 %, Protect-type
// and recovery moves free. A Pokémon's own move never takes it below 1.

// F019: campaign battles use the overhaul's rules when ASCENSION_COMBAT is on (Dual bars; damage
// 50% in trainer battles, 75% in wild ones; energy x1; costs 100%). Proposed defaults, tuned later.
#if TESTING
EWRAM_DATA bool8 gAscensionCombatTestOverride = FALSE;
#endif
static EWRAM_DATA bool8 sAscCampaignBattle = FALSE;
static EWRAM_DATA u8 sActiveCombat = LAB_COMBAT_CLASSIC;   // enum LabCombat of the running battle
static void LabSaveCampaignEnergy(void);
static EWRAM_DATA struct LabRules sCampaignRules = {0};

// The campaign record only in a campaign battle; otherwise the Lab's rules (which the Lab also reads
// while setting a battle up, before its battle flag is set).
static const struct LabRules *LabActiveRules(void)
{
    return sAscCampaignBattle ? &sCampaignRules : &gBattleLabRules;
}

bool32 BattleLab_CampaignBattle(void)
{
    return sAscCampaignBattle;
}

#if TESTING
static void Task_LabExhaustedTint(u8 taskId);
// Test teardown: a test battle can stop before the battle-end hook that ends campaign mode.
void BattleLab_ResetForTest(void)
{
    u8 taskId = FindTaskIdByFunc(Task_LabExhaustedTint);
    sAscCampaignBattle = FALSE;
    sActiveCombat = LAB_COMBAT_CLASSIC;
    if (taskId != TASK_NONE)
        DestroyTask(taskId);
}
#endif

// Hot path (damage calculation, AI scoring): the battle's combat mode is cached in one byte, set
// when a Lab or campaign battle starts and cleared when it ends.
bool32 BattleLab_StaminaMode(void)
{
    return sActiveCombat != LAB_COMBAT_CLASSIC;
}

// Dual (owner idea, 2026-11-27): health and energy are separate bars. Damage takes health
// (0 = fainted). Moves spend energy, which regenerates; Exhausted is low energy. Lost health
// caps energy (a hard hit saps stamina, like a boxer's): the cap is 50% + 50% x health share.
// Healing costs energy and restores health. Energy is kept per party Pokémon for the battle.
bool32 BattleLab_DualMode(void)
{
    return sActiveCombat == LAB_COMBAT_DUAL;
}

// Dual bars in a singles battle: healthboxes 8 px taller with the energy bar inside them.
bool32 BattleLab_TallHealthboxes(void)
{
    return BattleLab_DualMode() && !IsDoubleBattle() && !(gBattleTypeFlags & BATTLE_TYPE_SAFARI);
}

static EWRAM_DATA u16 sLabEnergy[2][PARTY_SIZE] = {0};
static EWRAM_DATA u8 sLabSapped = 0;         // battler mask: health capped energy this round (message)
static EWRAM_DATA u16 sLabEnergyMax[2][PARTY_SIZE] = {0};

static u16 *LabEnergyPtr(u32 battler)
{
    if (BattleLab_DualMode())
        return &sLabEnergy[IsOnPlayerSide(battler) ? 0 : 1][gBattlerPartyIndexes[battler]];
    return &gBattleMons[battler].hp;
}

static u32 LabEnergyMax(u32 battler)
{
    if (BattleLab_DualMode())
        return sLabEnergyMax[IsOnPlayerSide(battler) ? 0 : 1][gBattlerPartyIndexes[battler]];
    return gBattleMons[battler].maxHP;
}

static u32 LabEnergyCap(u32 battler)
{
    u32 max = LabEnergyMax(battler);
    if (!BattleLab_DualMode() || gBattleMons[battler].maxHP == 0)
        return max;
    return max / 2 + max * gBattleMons[battler].hp / gBattleMons[battler].maxHP / 2;
}

u32 BattleLab_EnergyCost(u32 move, u32 maxEnergy)
{
    u32 effect = GetMoveEffect(move), percent, cost;
    if (move == MOVE_NONE || IsHealingMove(move)
     || effect == EFFECT_PROTECT || effect == EFFECT_ENDURE || effect == EFFECT_MAT_BLOCK)
        return 0;
    if (GetMoveCategory(move) == DAMAGE_CATEGORY_STATUS)
        percent = 5;
    else
        percent = GetMovePower(move) / 6;
    if (percent == 0)
        percent = 3;   // variable-power moves (power 0/1): a light cost
    cost = maxEnergy * percent / 100;
    return cost > 0 ? cost : 1;
}

// Exhausted (D023): at or below 25% energy, whatever the cause.
bool32 BattleLab_IsExhausted(u32 battler)
{
    return BattleLab_StaminaMode() && gBattleMons[battler].hp > 0
        && *LabEnergyPtr(battler) * 4 <= LabEnergyMax(battler);
}

// Speed x0.25 while Exhausted (used by GetBattlerTotalSpeedStat).
u32 BattleLab_ExhaustedSpeed(u32 battler, u32 speed)
{
    return BattleLab_IsExhausted(battler) ? speed / 4 : speed;   // owner: x0.25, a slowdown you notice (2026-11-27)
}

// Exhausted Pokémon can't use their heaviest moves (power 100+).
// Dual: a move costing more energy than is left can't be used either (nothing affordable
// leaves Struggle, as when every move is out of PP).
u32 BattleLab_BoostedCost(u32 battler, u32 move);
bool32 BattleLab_ExhaustionLocksMove(u32 battler, u32 move)
{
    if (BattleLab_DualMode() && move != MOVE_STRUGGLE && BattleLab_BoostedCost(battler, move) > *LabEnergyPtr(battler))
        return TRUE;
    return BattleLab_IsExhausted(battler) && GetMoveCategory(move) != DAMAGE_CATEGORY_STATUS
        && GetMovePower(move) >= 100;
}

static EWRAM_DATA u8 sLabProtectedThisTurn = 0;   // battler bit mask; no regen on a turn you protected

// Timeline (D023, spike step 3). Each battler has a clock time for its next action. A round
// lets only the battler(s) whose time is earliest act; everyone else sits the round out.
// Acting pushes the actor's time on by 1000 x weight / Speed, so a faster Pokémon (or one
// whose Speed rises) acts more often, and a heavy move or Exhaustion delays the next action.
#define LAB_TIMELINE_SCALE 1000
static EWRAM_DATA u32 sLabNextAct[MAX_BATTLERS_COUNT] = {0};
static EWRAM_DATA u8 sLabPartyIndex[MAX_BATTLERS_COUNT] = {0};
static EWRAM_DATA u32 sLabNow = 0;
static EWRAM_DATA u8 sLabActors = 0;        // battlers acting this round
static EWRAM_DATA u8 sLabPrevActors = 0;    // ... and last round ("acts again" message)
static EWRAM_DATA u8 sLabCharged = 0;       // actors whose next time is already set this round
static EWRAM_DATA bool8 sLabRoundPlanned = FALSE;
static EWRAM_DATA bool8 sLabTimelineStarted = FALSE;
// F018: end-of-turn effects (weather, Leftovers, poison, Speed Boost, timers...) run once per
// "beat" of clock time (LAB_TIMELINE_SCALE, about one plain action at Speed 100), not after every
// one-action round. A round whose time crosses into a new beat runs them.
static EWRAM_DATA s32 sLabLastBeat = 0;   // set to -1 when a Lab battle starts
static EWRAM_DATA bool8 sLabBeat = FALSE;
static EWRAM_DATA u8 sLabBoost[MAX_BATTLERS_COUNT] = {0};   // enum LabBoost, chosen with R on the move menu

static u32 LabMoveWeight(u32 move)
{
    if (move == MOVE_NONE || GetMoveCategory(move) == DAMAGE_CATEGORY_STATUS)
        return 100;
    return 100 + GetMovePower(move) / 3;    // Tackle 113, Flamethrower 130, Hyper Beam 150
}

static u32 LabActionDelay(u32 battler, u32 weight)
{
    u32 speed = GetBattlerTotalSpeedStat(battler, GetBattlerAbility(battler), GetBattlerHoldEffect(battler));
    if (speed == 0)
        speed = 1;
    return LAB_TIMELINE_SCALE * weight / speed;
}

static bool32 LabBattlerOnField(u32 battler)
{
    return !(gAbsentBattlerFlags & (1u << battler)) && gBattleMons[battler].hp > 0;
}

static void LabEnsureExhaustedTint(void);
static void LabEnsureEnergyBars(void);

static void LabPlanRound(void)
{
    u32 battler, earliest = 0xFFFFFFFF;
    sLabRoundPlanned = TRUE;
    LabEnsureExhaustedTint();
    LabEnsureEnergyBars();
    sLabPrevActors = sLabActors;
    sLabActors = 0;
    sLabCharged = 0;
    for (battler = 0; battler < gBattlersCount; battler++)
    {
        if (!LabBattlerOnField(battler))
            continue;
        if (!sLabTimelineStarted)
        {
            // Opening round: everyone picks a move before anything happens (owner, 2026-11-27),
            // and the round runs in normal priority/Speed order, so e.g. Protect can block.
            sLabNextAct[battler] = 0;
            sLabPartyIndex[battler] = gBattlerPartyIndexes[battler];
        }
        else if (sLabPartyIndex[battler] != gBattlerPartyIndexes[battler])
        {
            // A replacement for a fainted Pokémon may act straight away; a Pokémon switched in
            // by choice joins half a step after "now" (switching is not free).
            struct Pokemon *party = IsOnPlayerSide(battler) ? gParties[B_TRAINER_PLAYER] : gParties[B_TRAINER_OPPONENT_A];
            bool32 replacedFainted = GetMonData(&party[sLabPartyIndex[battler]], MON_DATA_HP) == 0;
            sLabNextAct[battler] = sLabNow + (replacedFainted ? 0 : LabActionDelay(battler, 100) / 2);
            sLabPartyIndex[battler] = gBattlerPartyIndexes[battler];
        }
        if (sLabNextAct[battler] < earliest)
            earliest = sLabNextAct[battler];
    }
    sLabTimelineStarted = TRUE;
    if (earliest == 0xFFFFFFFF)
        return;
    sLabNow = earliest;
    sLabBeat = (s32)(sLabNow / LAB_TIMELINE_SCALE) > sLabLastBeat;
    if (sLabBeat)
        sLabLastBeat = sLabNow / LAB_TIMELINE_SCALE;
    for (battler = 0; battler < gBattlersCount; battler++)
        if (LabBattlerOnField(battler) && sLabNextAct[battler] == earliest)
            sLabActors |= 1u << battler;
    for (battler = 0; battler < gBattlersCount; battler++)
        if (LabBattlerOnField(battler))
            DebugPrintf("LAB clock: %S at %d%s", GetSpeciesName(gBattleMons[battler].species),
                        sLabNextAct[battler], (sLabActors & (1u << battler)) ? " ACTS" : "");
}

// F018: TRUE = skip this end-of-turn step this round (no new beat). Steps that keep the battle
// itself going (turn order, replacements, fainting, form changes, Eject Pack, trainer slides)
// always run.
bool32 BattleLab_EndTurnSkips(u32 state)
{
    if (!BattleLab_StaminaMode() || !sLabTimelineStarted || sLabBeat)
        return FALSE;
    switch (state)
    {
    case ENDTURN_ORDER:
    case ENDTURN_SEND_OUT_REPLACEMENTS_1:
    case ENDTURN_SEND_OUT_REPLACEMENTS_2:
    case ENDTURN_SEND_OUT_REPLACEMENTS_3:
    case ENDTURN_SEND_OUT_REPLACEMENTS_4:
    case ENDTURN_SEND_OUT_REPLACEMENTS_5:
    case ENDTURN_ROOST:   // Roost's Flying-type loss lasts one action, not until the next beat
    case ENDTURN_FORM_CHANGE:
    case ENDTURN_EJECT_PACK:
    case ENDTURN_ARENA_TURN_END:
    case ENDTURN_FAINTED_MON_ACTIONS:
    case ENDTURN_TRAINER_A_SLIDES:
    case ENDTURN_TRAINER_B_SLIDES:
    case ENDTURN_TRAINER_PARTNER_SLIDES:
        return FALSE;
    default:
        return state < ENDTURN_COUNT;
    }
}

// F018: TRUE for a battler on the field that sat out the last timeline round.
bool32 BattleLab_SatOutLastRound(u32 battler)
{
    return BattleLab_StaminaMode() && sLabTimelineStarted && sLabActors != 0 && !(sLabActors & (1u << battler));
}

bool32 BattleLab_TimelineBeat(void)
{
    return sLabBeat;
}

static u32 LabSpeed(u32 battler)
{
    return GetBattlerTotalSpeedStat(battler, GetBattlerAbility(battler), GetBattlerHoldEffect(battler));
}

// F018: predicted order of the next actions, assuming each takes a typical step (an ordinary
// attack). Fills battlers[] (up to count) and returns how many were predicted.
#define LAB_TYPICAL_WEIGHT 115
u32 BattleLab_PredictOrder(u8 *battlers, u32 count)
{
    u32 times[MAX_BATTLERS_COUNT], n = 0, b;
    if (!BattleLab_StaminaMode() || !sLabTimelineStarted)
        return 0;
    for (b = 0; b < gBattlersCount; b++)
        times[b] = LabBattlerOnField(b) ? sLabNextAct[b] : 0xFFFFFFFF;
    while (n < count)
    {
        u32 best = MAX_BATTLERS_COUNT;
        for (b = 0; b < gBattlersCount; b++)
        {
            if (times[b] == 0xFFFFFFFF)
                continue;
            // Ties (e.g. the opening round) go as the battle orders them: the faster first.
            if (best == MAX_BATTLERS_COUNT || times[b] < times[best]
             || (times[b] == times[best] && LabSpeed(b) > LabSpeed(best)))
                best = b;
        }
        if (best == MAX_BATTLERS_COUNT)
            break;
        battlers[n++] = best;
        times[best] += LabActionDelay(best, LAB_TYPICAL_WEIGHT);
    }
    return n;
}

void BattleLab_DrawTimelineStrip(void)
{
    u8 order[4];
    u32 n = BattleLab_PredictOrder(order, ARRAY_COUNT(order)), i, b;
    u8 *end = gDisplayedStringBattle;
    bool32 mirror = FALSE;
    if (n == 0 || gBattleScripting.windowsType != B_WIN_TYPE_NORMAL)
        return;
    for (b = 0; b < gBattlersCount; b++)   // same species on both sides: mark the foe
        for (i = 0; i < gBattlersCount; i++)
            if (LabBattlerOnField(b) && LabBattlerOnField(i) && GetBattlerSide(b) != GetBattlerSide(i)
             && gBattleMons[b].species == gBattleMons[i].species)
                mirror = TRUE;
    end = StringCopy(end, COMPOUND_STRING("Next: "));
    for (i = 0; i < n; i++)
    {
        if (i > 0)
            end = StringAppend(end, COMPOUND_STRING(" {RIGHT_ARROW} "));
        if (mirror && GetBattlerSide(order[i]) == B_SIDE_OPPONENT)
            end = StringAppend(end, COMPOUND_STRING("Foe "));
        end = StringAppend(end, GetSpeciesName(gBattleMons[order[i]].species));
    }
    PrependFontIdToFit(gDisplayedStringBattle, end, FONT_SMALL_NARROW, WindowWidthPx(B_WIN_ASC_TIMELINE) - 4);
    BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_ASC_TIMELINE);
    PutWindowTilemap(B_WIN_ASC_TIMELINE);
    CopyWindowToVram(B_WIN_ASC_TIMELINE, COPYWIN_FULL);
}

// Action selection asks this for each battler; TRUE = this battler sits the round out.
bool32 BattleLab_TimelineSkips(u32 battler)
{
    if (!BattleLab_StaminaMode())
        return FALSE;
    if (!sLabRoundPlanned)
        LabPlanRound();
    return sLabActors != 0 && !(sLabActors & (1u << battler));
}

static void LabChargeAction(u32 battler, u32 move)
{
    if (!(sLabActors & (1u << battler)) || (sLabCharged & (1u << battler)))
        return;
    sLabCharged |= 1u << battler;
    sLabNextAct[battler] = sLabNow + LabActionDelay(battler, LabMoveWeight(move))
                         * ((sLabBoost[battler] & LAB_BOOST_SPEED) ? 6 : 10) / 10;
}

// End of round: actors that did not use a move (switch, item, full paralysis...) take a plain
// step; the next round is planned afresh.
static void LabTimelineRoundEnd(void)
{
    u32 battler;
    for (battler = 0; battler < gBattlersCount; battler++)
        if (LabBattlerOnField(battler))
            LabChargeAction(battler, MOVE_NONE);   // party index is left for LabPlanRound to spot replacements
    sLabRoundPlanned = FALSE;
    memset(sLabBoost, 0, sizeof(sLabBoost));
}

// Energy changed outside the damage path (a move's cost, regen): copy it to the party
// Pokémon and redraw that battler's bar and number at once. (The controller's HP update only
// changes the stored data; the bar is otherwise redrawn only by damage animations.)
static void LabRefreshEnergyLabel(u32 battler);
static EWRAM_DATA bool8 sLabBadgeBlinkOff = FALSE;   // the EX badge blinks: off = show the normal label

// A short colour pulse on one 16-colour sprite palette (about a quarter of a second):
// green on the energy bar when it regenerates, the Boost colour on the attacker.
#define tPalOffset data[0]
#define tColorLo   data[1]
#define tColorHi   data[2]
#define tFrame     data[3]
static void Task_LabPalettePulse(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u32 color = (u16)tColorLo | ((u32)(u16)tColorHi << 16);
    u32 coeff = (tFrame < 8) ? tFrame : 16 - tFrame;
    BlendPalette(tPalOffset, 16, coeff, color);
    if (++tFrame > 16)
    {
        BlendPalette(tPalOffset, 16, 0, color);
        DestroyTask(taskId);
    }
}

static void LabPulseSpritePalette(u32 spriteId, u32 color)
{
    u8 taskId = CreateTask(Task_LabPalettePulse, 5);
    gTasks[taskId].tPalOffset = OBJ_PLTT_ID(gSprites[spriteId].oam.paletteNum);
    gTasks[taskId].tColorLo = color & 0xFFFF;
    gTasks[taskId].tColorHi = color >> 16;
    gTasks[taskId].tFrame = 1;
}
#undef tPalOffset
#undef tColorLo
#undef tColorHi
#undef tFrame

// Exhausted Pokémon look it: their sprite is tinted a dull purple that slowly "pants" (fades
// in and out), reapplied every frame. Runs while the battle does; started on the first round.
static EWRAM_DATA u8 sLabTintedMask = 0;
static void Task_LabExhaustedTint(u8 taskId)
{
    u32 battler, coeff;
    s16 *frame = &gTasks[taskId].data[0];
    if (!BattleLab_StaminaMode())
    {
        DestroyTask(taskId);
        return;
    }
    (*frame)++;
    if ((*frame & 31) == 0)   // the EX badge blinks: about half a second on, half off
    {
        sLabBadgeBlinkOff = (*frame >> 5) & 1;
        for (battler = 0; battler < gBattlersCount; battler++)
            if (BattleLab_IsExhausted(battler))
                LabRefreshEnergyLabel(battler);
    }
    coeff = 4 + ((*frame >> 3) & 3);   // 4..7, stepping every 8 frames
    if ((*frame >> 5) & 1)
        coeff = 11 - coeff;
    for (battler = 0; battler < gBattlersCount; battler++)
    {
        u32 spriteId = gBattlerSpriteIds[battler], offset;
        if (spriteId >= MAX_SPRITES || !gSprites[spriteId].inUse)
            continue;
        offset = OBJ_PLTT_ID(gSprites[spriteId].oam.paletteNum);
        if (BattleLab_IsExhausted(battler) && gBattleMons[battler].hp > 0)
        {
            BlendPalette(offset, 16, coeff, RGB(12, 4, 18));
            sLabTintedMask |= 1u << battler;
        }
        else if (sLabTintedMask & (1u << battler))
        {
            BlendPalette(offset, 16, 0, RGB(12, 4, 18));
            sLabTintedMask &= ~(1u << battler);
        }
    }
}

static void LabEnsureExhaustedTint(void)
{
    if (!FuncIsActiveTask(Task_LabExhaustedTint))
        CreateTask(Task_LabExhaustedTint, 4);
}

// Dual: every battler gets its own energy bar, a separate 64x8 strip (two 32x8 sprites) attached
// to its healthbox: under the foe's box, above yours. The EXP bar keeps showing experience.
// Colours come from the healthbox palette: blue fill (red when Exhausted); the empty part that
// energy can still refill is dark, and the part above the health cap is darker still.
#define LAB_ENBAR_TAG 0xA5D0
// Up to four sprites per bar: 32+32 px (foe, doubles), or 32+32+32+8 px across your wide box,
// where the energy bar fills the whole row (owner, 2026-11-28: there is no number beside it).
#define LAB_ENBAR_PARTS 4
#define LAB_ENBAR_MAX_W 104
static EWRAM_DATA u8 sLabEnBarSprite[MAX_BATTLERS_COUNT][LAB_ENBAR_PARTS] = {0};

static u32 LabEnBarWidth(u32 battler)
{
    return (BattleLab_TallHealthboxes() && IsOnPlayerSide(battler)) ? LAB_ENBAR_MAX_W : 64;
}

static u32 LabEnBarParts(u32 battler)
{
    return LabEnBarWidth(battler) == LAB_ENBAR_MAX_W ? 4 : 2;
}

static void LabEnBarPixel(u8 *tiles, s32 x, s32 y, u32 color)   // tiles left to right, 8 px each
{
    u8 *b;
    if (x < 0 || x >= LAB_ENBAR_MAX_W || y < 0 || y >= 8)
        return;
    b = &tiles[(x / 8) * 32 + y * 4 + (x % 8) / 2];
    if (x & 1)
        *b = (*b & 0x0F) | (color << 4);
    else
        *b = (*b & 0xF0) | color;
}

static void SpriteCB_LabEnBar(struct Sprite *sprite)
{
    u32 battler = sprite->data[0], healthbox = gHealthboxSpriteIds[battler], bar;
    if (!BattleLab_DualMode() || healthbox >= MAX_SPRITES || !gSprites[healthbox].inUse)
    {
        sprite->invisible = TRUE;
        return;
    }
    bar = gSprites[healthbox].data[5];   // hMain_HealthBarSpriteId (battle_interface.c)
    if (BattleLab_TallHealthboxes() && bar < MAX_SPRITES && gSprites[bar].inUse)
    {
        // Same column as the HP bar, on the row under it (the last, 8 px part is centred 12 px earlier).
        sprite->x = gSprites[bar].x + sprite->data[3] * 32 - (sprite->data[3] == 3 ? 12 : 0);
        sprite->y = gSprites[healthbox].y + 12;
        sprite->x2 = gSprites[bar].x2;
        sprite->y2 = gSprites[healthbox].y2;
    }
    else
    {
        sprite->x = gSprites[healthbox].x + sprite->data[1];
        sprite->y = gSprites[healthbox].y + sprite->data[2];
        sprite->x2 = gSprites[healthbox].x2;
        sprite->y2 = gSprites[healthbox].y2;
    }
    sprite->invisible = gSprites[healthbox].invisible || gBattleMons[battler].hp == 0;
}

// The energy bar is drawn like the HP bar: the "EN" label (EX, blinking, when Exhausted), then
// the HP bar's own fill tiles, in blue (red when Exhausted) from a copy of its palette. The empty
// part above the health cap is darker: energy can't refill there until health returns.
#define LAB_ENBAR_PAL_TAG 0xA5DF
extern const u32 gBattleLabEnergyLabelGfx[];      // HP-style "EN" / "EX" label tiles (below)
extern const u32 gBattleLabExhaustedLabelGfx[];
static EWRAM_DATA u16 sLabEnBarPal[16] = {0};

static void LabLoadEnBarPalette(void)
{
    struct SpritePalette pal = {.data = sLabEnBarPal, .tag = LAB_ENBAR_PAL_TAG};
    if (IndexOfSpritePaletteTag(LAB_ENBAR_PAL_TAG) != 0xFF)
        return;
    CpuCopy16(gBattleInterface_BallDisplayPal, sLabEnBarPal, sizeof(sLabEnBarPal));
    sLabEnBarPal[10] = RGB(12, 21, 31);   // fill, lower row (the HP bar's light green)
    sLabEnBarPal[11] = RGB(7, 15, 28);    // fill, upper row
    sLabEnBarPal[12] = RGB(5, 5, 7);      // empty beyond the health cap
    sLabEnBarPal[13] = RGB(3, 3, 5);
    LoadSpritePalette(&pal);
}

static void LabDrawEnergyBarValue(u32 battler, u32 value)
{
    u8 tiles[(LAB_ENBAR_MAX_W / 8) * TILE_SIZE_4BPP] ALIGNED(4);
    u32 i, x, pixels, capPixels, max = LabEnergyMax(battler), cap = LabEnergyCap(battler);
    u32 width = LabEnBarWidth(battler), fillWidth = width - 16, parts = LabEnBarParts(battler);
    bool32 exhausted = BattleLab_IsExhausted(battler);
    const u8 *fill = Ascension_HealthbarFillGfx(exhausted);
    const void *label = (exhausted && !sLabBadgeBlinkOff) ? (const void *)gBattleLabExhaustedLabelGfx : (const void *)gBattleLabEnergyLabelGfx;
    if (sLabEnBarSprite[battler][0] >= MAX_SPRITES || max == 0)
        return;
    pixels = value * fillWidth / max;
    if (value > 0 && pixels == 0)
        pixels = 1;
    capPixels = cap * fillWidth / max;
    CpuCopy32(label, tiles, 2 * TILE_SIZE_4BPP);
    for (i = 0; i < fillWidth / 8; i++)
    {
        u32 level = pixels > i * 8 ? min(pixels - i * 8, 8) : 0;
        CpuCopy32(fill + level * TILE_SIZE_4BPP, &tiles[(2 + i) * TILE_SIZE_4BPP], TILE_SIZE_4BPP);
    }
    // beyond the health cap: the empty fill rows (3-4) in the darker colours
    for (x = capPixels; x < fillWidth; x++)
    {
        if (x < pixels)
            continue;
        LabEnBarPixel(tiles, 16 + x, 3, 12);
        LabEnBarPixel(tiles, 16 + x, 4, 13);
    }
    // close the bar's right end, like the HP bar's frame end
    for (x = 2; x <= 5; x++)
        LabEnBarPixel(tiles, width - 1, x, 6);
    LabEnBarPixel(tiles, width - 1, 1, 0);
    LabEnBarPixel(tiles, width - 1, 6, 0);
    for (i = 0; i < parts; i++)
    {
        u32 n = (i == 3) ? 1 : 4;   // tiles in this part
        if (sLabEnBarSprite[battler][i] < MAX_SPRITES)
            CpuCopy32(tiles + i * 4 * TILE_SIZE_4BPP, (void *)(OBJ_VRAM0 + gSprites[sLabEnBarSprite[battler][i]].oam.tileNum * TILE_SIZE_4BPP), n * TILE_SIZE_4BPP);
    }
}

static void LabEnsureEnergyBars(void)
{
    static const struct OamData sOam = {.shape = SPRITE_SHAPE(32x8), .size = SPRITE_SIZE(32x8), .priority = 1};
    static const struct OamData sOamEnd = {.shape = SPRITE_SHAPE(8x8), .size = SPRITE_SIZE(8x8), .priority = 1};
    u32 battler, half;
    if (!BattleLab_DualMode())
        return;
    LabLoadEnBarPalette();
    for (battler = 0; battler < gBattlersCount; battler++)
    {
        u32 healthbox = gHealthboxSpriteIds[battler];
        if (sLabEnBarSprite[battler][0] < MAX_SPRITES && gSprites[sLabEnBarSprite[battler][0]].inUse
         && gSprites[sLabEnBarSprite[battler][0]].callback == SpriteCB_LabEnBar)
            continue;
        if (healthbox >= MAX_SPRITES)
            continue;
        for (half = 0; half < LabEnBarParts(battler); half++)
        {
            u8 blank[128] ALIGNED(4) = {0};
            struct SpriteSheet sheet = {.data = blank, .size = half == 3 ? 32 : 128, .tag = LAB_ENBAR_TAG + battler * LAB_ENBAR_PARTS + half};
            struct SpriteTemplate tmpl = {.tileTag = LAB_ENBAR_TAG + battler * LAB_ENBAR_PARTS + half, .paletteTag = LAB_ENBAR_PAL_TAG,
                                          .oam = half == 3 ? &sOamEnd : &sOam,
                                          .anims = gDummySpriteAnimTable, .images = NULL, .affineAnims = gDummySpriteAffineAnimTable,
                                          .callback = SpriteCB_LabEnBar};
            u32 spriteId;
            if (LoadSpriteSheet(&sheet) == 0)
                return;
            spriteId = CreateSprite(&tmpl, 0, 0, 0);
            if (spriteId >= MAX_SPRITES)
                return;
            gSprites[spriteId].data[0] = battler;
            gSprites[spriteId].data[3] = half;
            // Doubles: just under the foe's healthbox, just above yours.
            gSprites[spriteId].data[1] = (IsOnPlayerSide(battler) ? 16 : 8) + half * 32;
            gSprites[spriteId].data[2] = IsOnPlayerSide(battler) ? -18 : 15;
            sLabEnBarSprite[battler][half] = spriteId;
            SpriteCB_LabEnBar(&gSprites[spriteId]);
        }
        LabDrawEnergyBarValue(battler, *LabEnergyPtr(battler));
    }
}

bool32 BattleLab_DrawEnergyBar(u32 battler, u32 healthboxSpriteId)
{
    return FALSE;   // the EXP bar shows experience again; energy has its own bar
}

static void LabSyncEnergy(u32 battler)
{
    struct Pokemon *mon = GetBattlerMon(battler);
    u32 hp = gBattleMons[battler].hp;
    if (BattleLab_DualMode())
    {
        LabDrawEnergyBarValue(battler, *LabEnergyPtr(battler));
        LabRefreshEnergyLabel(battler);
        return;
    }
    SetMonData(mon, MON_DATA_HP, &hp);
    UpdateHealthboxAttribute(gHealthboxSpriteIds[battler], mon, HEALTHBOX_HEALTH_BAR);
    UpdateHealthboxAttribute(gHealthboxSpriteIds[battler], mon, HEALTHBOX_CURRENT_HP);
    LabRefreshEnergyLabel(battler);
}

// Boost (D023, spike step 4): R on the move menu cycles None > Speed > Power > Both.
// Speed: +50% cost, the next action comes sooner (delay x0.6). Power: +50% cost, power x1.3.
// Both: +100% cost. The choice lasts until the end of the round. The AI does not boost yet.
static const char *const sLabBoostLog[LAB_BOOST_COUNT] = {"", " +SPEED", " +POWER", " +BOTH"};
static const u8 sLabBoostTag[LAB_BOOST_COUNT][6] = {_("Cost"), _("Spd+"), _("Pow+"), _("All+")};   // PP box label in stamina battles
static const u32 sLabBoostFlashColor[LAB_BOOST_COUNT] = {0, RGB(8, 28, 31), RGB(31, 8, 4), RGB(31, 26, 4)};

static const u8 sLabEnergyScaleTenths[LAB_ENERGY_COUNT] = {10, 15, 20, 30};
static const u8 sLabDamageScalePercent[LAB_DAMAGE_COUNT] = {100, 75, 50, 33};
static const u8 sLabCostScalePercent[LAB_COST_COUNT] = {100, 75, 50, 25};

// Rules "Damage": scales every damaging hit in stamina battles (never below 1).
s32 BattleLab_ScaleDamage(s32 damage)
{
    if (!BattleLab_StaminaMode() || damage <= 0)
        return damage;
    damage = damage * sLabDamageScalePercent[LabActiveRules()->damageScale] / 100;
    return damage > 0 ? damage : 1;
}

// Rules "Energy": multiplies both parties' max (and current) energy before the battle starts.
void BattleLab_ScaleStartingEnergy(void)
{
    u32 i, tenths = sLabEnergyScaleTenths[LabActiveRules()->energyScale];
    if (LabActiveRules()->combat == LAB_COMBAT_DUAL)
    {
        // Max energy = max HP (species, level) x the Energy rule; everyone starts full.
        for (i = 0; i < PARTY_SIZE * 2; i++)
        {
            struct Pokemon *mon = (i < PARTY_SIZE) ? &gParties[B_TRAINER_PLAYER][i] : &gParties[B_TRAINER_OPPONENT_A][i - PARTY_SIZE];
            u32 max = GetMonData(mon, MON_DATA_MAX_HP) * tenths / 10;
            sLabEnergyMax[i / PARTY_SIZE][i % PARTY_SIZE] = max > 0xFFFF ? 0xFFFF : max;
            sLabEnergy[i / PARTY_SIZE][i % PARTY_SIZE] = sLabEnergyMax[i / PARTY_SIZE][i % PARTY_SIZE];
        }
        return;
    }
    if (LabActiveRules()->combat != LAB_COMBAT_STAMINA || tenths == 10)
        return;
    for (i = 0; i < PARTY_SIZE * 2; i++)
    {
        struct Pokemon *mon = (i < PARTY_SIZE) ? &gParties[B_TRAINER_PLAYER][i] : &gParties[B_TRAINER_OPPONENT_A][i - PARTY_SIZE];
        u32 maxHp = GetMonData(mon, MON_DATA_MAX_HP), hp = GetMonData(mon, MON_DATA_HP);
        if (maxHp == 0)
            continue;
        maxHp = maxHp * tenths / 10;
        hp = hp * tenths / 10;
        if (maxHp > 0xFFFF)
            maxHp = 0xFFFF;
        if (hp > maxHp)
            hp = maxHp;
        SetMonData(mon, MON_DATA_MAX_HP, &maxHp);
        SetMonData(mon, MON_DATA_HP, &hp);
    }
}

// F019: a move's energy cost in campaign battles (Dual bars, costs 100%), for menus outside battle.
u32 BattleLab_CampaignMoveCost(u32 move, u32 maxEnergy)
{
    if (move != MOVE_NONE && IsHealingMove(move))
        return maxEnergy / 5;
    return BattleLab_EnergyCost(move, maxEnergy);
}

u32 BattleLab_BoostedCost(u32 battler, u32 move)
{
    u32 cost = BattleLab_EnergyCost(move, LabEnergyMax(battler));
    if (BattleLab_DualMode() && IsHealingMove(move))
        cost = LabEnergyMax(battler) / 5;   // Dual: healing costs 20% of max energy (restores health)
    if (cost != 0)
    {
        cost = cost * sLabCostScalePercent[LabActiveRules()->costScale] / 100;
        if (cost == 0)
            cost = 1;
    }
    switch (sLabBoost[battler])
    {
    case LAB_BOOST_SPEED:
    case LAB_BOOST_POWER:
        return cost * 3 / 2;
    case LAB_BOOST_BOTH:
        return cost * 2;
    default:
        return cost;
    }
}

u32 BattleLab_BoostPower(u32 battler, u32 power)
{
    if (BattleLab_StaminaMode() && (sLabBoost[battler] & LAB_BOOST_POWER))
        return power * 13 / 10;
    return power;
}

void BattleLab_CycleBoost(u32 battler)
{
    sLabBoost[battler] = (sLabBoost[battler] + 1) % LAB_BOOST_COUNT;
}

void BattleLab_ClearBoost(u32 battler)
{
    sLabBoost[battler] = LAB_BOOST_NONE;
}

// "X used Y!" in stamina battles, with a second line saying why (owner, 2026-11-27):
// the Boost used, a faster Pokémon acting again before its foe, or an Exhausted, sluggish move.
static const u8 sText_LabUsedBoostSpeed[] = _("{B_ATK_NAME_WITH_PREFIX} used {B_BUFF3}\nwith boosted Speed!");
static const u8 sText_LabUsedBoostPower[] = _("{B_ATK_NAME_WITH_PREFIX} used {B_BUFF3}\nwith boosted Power!");
static const u8 sText_LabUsedBoostBoth[] = _("{B_ATK_NAME_WITH_PREFIX} used {B_BUFF3}\nwith boosted Speed and Power!");
static const u8 sText_LabAgainBoostSpeed[] = _("{B_ATK_NAME_WITH_PREFIX} acts again: {B_BUFF3}\nwith boosted Speed!");
static const u8 sText_LabAgainBoostPower[] = _("{B_ATK_NAME_WITH_PREFIX} acts again: {B_BUFF3}\nwith boosted Power!");
static const u8 sText_LabAgainBoostBoth[] = _("{B_ATK_NAME_WITH_PREFIX} acts again: {B_BUFF3}\nwith Speed and Power boosted!");
static const u8 sText_LabAgain[] = _("{B_ATK_NAME_WITH_PREFIX} is faster and\nacts again: {B_BUFF3}!");
static const u8 sText_LabExhaustedUsed[] = _("The exhausted {B_ATK_NAME_WITH_PREFIX}\nslowly used {B_BUFF3}...");
static const u8 *const sLabUsedBoostTexts[LAB_BOOST_COUNT] = {NULL, sText_LabUsedBoostSpeed, sText_LabUsedBoostPower, sText_LabUsedBoostBoth};
static const u8 *const sLabAgainBoostTexts[LAB_BOOST_COUNT] = {sText_LabAgain, sText_LabAgainBoostSpeed, sText_LabAgainBoostPower, sText_LabAgainBoostBoth};

static bool32 LabActsAgain(u32 battler)
{
    u32 bit = 1u << battler, others = 0, b;
    for (b = 0; b < gBattlersCount; b++)
        if (b != battler && LabBattlerOnField(b))
            others |= 1u << b;
    // Acted last round and this round, while every other battler sat both rounds out.
    return (sLabPrevActors & bit) && (sLabActors & bit) && others != 0
        && !((sLabPrevActors | sLabActors) & others);
}

const u8 *BattleLab_UsedMoveText(u32 attacker, const u8 *defaultText)
{
    u32 boost;
    if (!BattleLab_StaminaMode() || attacker >= MAX_BATTLERS_COUNT)
        return defaultText;
    boost = sLabBoost[attacker];
    if (LabActsAgain(attacker))
        return sLabAgainBoostTexts[boost];
    if (boost != LAB_BOOST_NONE)
        return sLabUsedBoostTexts[boost];
    if (BattleLab_IsExhausted(attacker))
        return sText_LabExhaustedUsed;
    return defaultText;
}

// Move menu in stamina battles: the PP box shows the boost and the energy cost, and the
// player's energy bar previews what is left after paying it.
// Stamina battles relabel the bar "EN" (energy): two 8x8 tiles in place of upstream's "HP".
// Exhausted Pokémon show a red "EX" there instead: a badge that never hides a status condition
// (while a status icon is shown, upstream draws it over this label, and the label comes back after).
const u32 gBattleLabEnergyLabelGfx[] = INCBIN_U32("graphics/battle_interface/lab_energy_label.4bpp");
const u32 gBattleLabExhaustedLabelGfx[] = INCBIN_U32("graphics/battle_interface/lab_exhausted_label.4bpp");

const void *BattleLab_HealthbarLabelGfx(u32 battler, const void *hpLabel)
{
    if (!BattleLab_StaminaMode())
        return hpLabel;
    if (BattleLab_IsExhausted(battler) && !BattleLab_DualMode())
        return gBattleLabExhaustedLabelGfx;
    if (BattleLab_DualMode())   // the health bar keeps its "HP" label; EX shows on the energy bar
        return hpLabel != NULL ? hpLabel : (const void *)gHealthboxElementsGfxTable[1];   // upstream "HP" tiles (HEALTHBOX_GFX_1)
    return gBattleLabEnergyLabelGfx;
}

// Redraw the EN/EX label after energy changes (no-op while a status icon occupies it).
static void LabRefreshEnergyLabel(u32 battler)
{
    u32 barSpriteId;
    const void *gfx;
    if (BattleLab_DualMode())   // the EN/EX label is part of the energy bar
    {
        if (gBattleMons[battler].hp != 0)
            LabDrawEnergyBarValue(battler, *LabEnergyPtr(battler));
        return;
    }
    if (!BattleLab_StaminaMode() || gBattleMons[battler].status1 != 0 || gBattleMons[battler].hp == 0)
        return;
    barSpriteId = gSprites[gHealthboxSpriteIds[battler]].data[5];   // hMain_HealthBarSpriteId (battle_interface.c)
    gfx = BattleLab_HealthbarLabelGfx(battler, NULL);
    if (gfx == (const void *)gBattleLabExhaustedLabelGfx && sLabBadgeBlinkOff)
        gfx = BattleLab_DualMode() ? (const void *)gHealthboxElementsGfxTable[1] : (const void *)gBattleLabEnergyLabelGfx;
    CpuCopy32(gfx, (void *)(OBJ_VRAM0 + gSprites[barSpriteId].oam.tileNum * TILE_SIZE_4BPP), 64);
}

// The preview blinks: the bar alternates between now and after paying, so the part that
// would be spent flashes (8 frames each).
static EWRAM_DATA u16 sLabPreviewAfter[MAX_BATTLERS_COUNT] = {0};
static EWRAM_DATA u8 sLabPreviewTimer = 0;

static void LabDrawBar(u32 battler, u32 value)
{
    SetBattleBarStruct(battler, gHealthboxSpriteIds[battler], gBattleMons[battler].maxHP, value, 0);
    MoveBattleBar(battler, gHealthboxSpriteIds[battler], HEALTH_BAR, 0);
}

static void LabDrawPreview(u32 battler, u32 value)
{
    if (BattleLab_DualMode())
        LabDrawEnergyBarValue(battler, value);
    else
        LabDrawBar(battler, value);
}

void BattleLab_MoveMenuFrame(u32 battler)
{
    if (!BattleLab_StaminaMode() || sLabPreviewAfter[battler] == *LabEnergyPtr(battler))
        return;
    sLabPreviewTimer++;
    if ((sLabPreviewTimer & 7) == 0)
        LabDrawPreview(battler, (sLabPreviewTimer & 8) ? sLabPreviewAfter[battler] : *LabEnergyPtr(battler));
}

void BattleLab_ShowMoveCost(u32 battler, u32 move)
{
    u32 hp = *LabEnergyPtr(battler), cost = BattleLab_BoostedCost(battler, move), after;
    u32 floor = BattleLab_DualMode() ? 0 : 1;
    after = (cost == 0 || hp <= floor) ? hp : (hp > cost + floor ? hp - cost : floor);
    sLabPreviewAfter[battler] = after;
    sLabPreviewTimer = 8;
    BattlePutTextOnWindow(sLabBoostTag[sLabBoost[battler]], B_WIN_PP);
    ConvertIntToDecimalStringN(gDisplayedStringBattle, cost, STR_CONV_MODE_RIGHT_ALIGN, 3);
    BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_PP_REMAINING);
    LabDrawPreview(battler, after);
}

void BattleLab_EndMoveCostPreview(u32 battler)
{
    sLabPreviewAfter[battler] = *LabEnergyPtr(battler);
    LabDrawPreview(battler, *LabEnergyPtr(battler));
}

void BattleLab_PayEnergy(u32 battler, u32 move)
{
    u16 *energy = LabEnergyPtr(battler);
    u32 hp = *energy, floor = BattleLab_DualMode() ? 0 : 1;   // Stamina: own moves never faint you
    u32 cost = BattleLab_BoostedCost(battler, move), effect = GetMoveEffect(move);
    if (effect == EFFECT_PROTECT || effect == EFFECT_ENDURE || effect == EFFECT_MAT_BLOCK)
        sLabProtectedThisTurn |= 1u << battler;
    LabChargeAction(battler, move);
    if (cost == 0 || hp <= floor)
        return;
    hp = (hp > cost + floor) ? hp - cost : floor;
    if (sLabBoost[battler] != LAB_BOOST_NONE)   // Boost: flash the attacker (Speed cyan, Power red, Both gold)
        LabPulseSpritePalette(gBattlerSpriteIds[battler], sLabBoostFlashColor[sLabBoost[battler]]);
    DebugPrintf("LAB energy: %S pays %d for %S%s (%d -> %d)", GetSpeciesName(gBattleMons[battler].species),
                cost, GetMoveName(move), sLabBoostLog[sLabBoost[battler]], *energy, hp);
    *energy = hp;
    LabSyncEnergy(battler);
}

// ---------------------------------------------------------------- diagnostics (F012)

// F017 measurement: how long an AI decision takes, timed with hardware timer 3 at 1/64 of the
// CPU clock (timer 3 is only used by the link cable, never in Lab battles). 4389 ticks = 1 frame.
static EWRAM_DATA u32 sLabAiTicksTotal = 0;
static EWRAM_DATA u16 sLabAiTicksMax = 0;
static EWRAM_DATA u16 sLabAiDecisions = 0;

void BattleLab_AiTimerStart(void)
{
    if (!sLabInBattleFlag)
        return;
    REG_TM3CNT_H = 0;
    REG_TM3CNT_L = 0;
    REG_TM3CNT_H = TIMER_ENABLE | TIMER_64CLK;
}

void BattleLab_AiTimerStop(u32 battler)
{
    u32 ticks;
    if (!sLabInBattleFlag)
        return;
    ticks = REG_TM3CNT_L;
    REG_TM3CNT_H = 0;
    sLabAiTicksTotal += ticks;
    sLabAiDecisions++;
    if (ticks > sLabAiTicksMax)
        sLabAiTicksMax = ticks;
    DebugPrintf("LAB ai: %S decided in %d ticks (%d.%02d frames)", GetSpeciesName(gBattleMons[battler].species),
                ticks, ticks / 4389, (ticks % 4389) * 100 / 4389);
}

u32 BattleLab_LastSeed(void)
{
    return sLabHasLastSeed ? sLabLastSeed : 0;
}

bool32 BattleLab_HasLastSeed(void)
{
    return sLabHasLastSeed;
}

// Rules "Seed no." + A: replay the last battle's seed.
bool32 BattleLab_UseLastSeed(void)
{
    if (!sLabHasLastSeed)
        return FALSE;
    gBattleLabRules.fixedSeed = TRUE;
    gBattleLabRules.seed = sLabLastSeed;
    return TRUE;
}

static const char *const sLabFormatLog[LAB_FORMAT_COUNT] = {"trainer 1v1", "trainer 2v2", "wild 1v1"};
static const char *const sLabWeatherLog[LAB_WEATHER_COUNT] = {"none", "rain", "sun", "sand", "hail", "snow"};
static const char *const sLabTerrainLog[LAB_TERRAIN_COUNT] = {"none", "electric", "grassy", "misty", "psychic"};
static const char *const sLabAiLog[LAB_AI_COUNT] = {"scenario", "smart", "random"};

// Battle log for mGBA's log window (Tools > View Logs). DebugPrintf is compiled out in release.
void BattleLab_LogBattleStart(void)
{
    const struct LabRules *r = &gBattleLabRules;
    LabEnsureScenario();
    DebugPrintf("LAB start %S: %s, weather %s, terrain %s, AI %s, seed %d%s",
                sLabCurrent.id, sLabFormatLog[r->format], sLabWeatherLog[r->weather],
                sLabTerrainLog[r->terrain], sLabAiLog[r->ai], sLabLastSeed,
                r->fixedSeed ? " (fixed)" : "");
}

static void LabLogBattlers(void)
{
    u32 battler;
    for (battler = 0; battler < gBattlersCount; battler++)
    {
        const struct BattlePokemon *mon = &gBattleMons[battler];
        DebugPrintf("  %s%d %S Lv%d HP %d/%d EN %d/%d status 0x%x%s",
                    IsOnPlayerSide(battler) ? "you" : "foe", battler,
                    GetSpeciesName(mon->species), mon->level, mon->hp, mon->maxHP,
                    BattleLab_DualMode() ? *LabEnergyPtr(battler) : mon->hp, LabEnergyMax(battler), mon->status1,
                    BattleLab_IsExhausted(battler) ? " EXHAUSTED" : "");
    }
}

// End of round in stamina combat: +2% energy per 1000 clock units (about one action at Speed
// 100) for every active battler that did not protect this round. Timeline rounds are single
// actions, so regen follows clock time, not rounds; fractions carry over.
#define LAB_REGEN_PERCENT 2
static EWRAM_DATA u32 sLabRegenAt = 0;
static EWRAM_DATA u32 sLabRegenCarry[MAX_BATTLERS_COUNT] = {0};

// Regen "+N" (owner, 2026-11-27): a small green number rises from the battler's healthbox and
// fades, Final Fantasy style. It is a 32x16 sprite drawn with a built-in 3x5 pixel font in the
// health bar's own palette (green with a dark outline), so it needs no extra palette or window.
#define LAB_POPUP_TAG 0xA5C0
static EWRAM_DATA u8 sLabPopupSprite[MAX_BATTLERS_COUNT] = {0};
static const u8 sLabPopupGlyphs[11][5] =   // 3x5 digits 0-9 and '+', bit 2 = left column
{
    {7,5,5,5,7}, {2,6,2,2,7}, {7,1,7,4,7}, {7,1,3,1,7}, {5,5,7,1,1},
    {7,4,7,1,7}, {7,4,7,5,7}, {7,1,1,2,2}, {7,5,7,5,7}, {7,5,7,1,7}, {0,2,7,2,0},
};

// The popup is 32x16: four 8x8 tiles per row, two rows (1D mapping). Glyphs are drawn 2x.
static void LabPopupPixel(u8 *tiles, s32 x, s32 y, u32 color)
{
    u8 *b;
    if (x < 0 || x >= 32 || y < 0 || y >= 16)
        return;
    b = &tiles[((y / 8) * 4 + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];
    if (x & 1)
        *b = (*b & 0x0F) | (color << 4);
    else
        *b = (*b & 0xF0) | color;
}

static void SpriteCB_LabRegenPopup(struct Sprite *sprite)
{
    sprite->data[0]++;
    if ((sprite->data[0] & 3) == 0 && sprite->data[0] <= 24)
        sprite->y2--;                                   // rises 6 px
    if (sprite->data[0] > 30)
        sprite->invisible = (sprite->data[0] & 2) != 0; // flickers out
    if (sprite->data[0] > 46)
    {
        u32 tag = sprite->data[1];
        DestroySprite(sprite);
        FreeSpriteTilesByTag(tag);
    }
}

static void LabShowRegenPopup(u32 battler, u32 amount)
{
    static const struct OamData sOam = {.shape = SPRITE_SHAPE(32x16), .size = SPRITE_SIZE(32x16), .priority = 0};
    struct SpriteTemplate tmpl = {.tileTag = LAB_POPUP_TAG + battler, .paletteTag = TAG_NONE, .oam = &sOam,
                                  .anims = gDummySpriteAnimTable, .images = NULL, .affineAnims = gDummySpriteAffineAnimTable,
                                  .callback = SpriteCB_LabRegenPopup};
    struct SpriteSheet sheet = {.data = NULL, .size = 32 * 8, .tag = LAB_POPUP_TAG + battler};
    u8 digits[4] ALIGNED(4), tiles[32 * 8] ALIGNED(4), healthbox = gHealthboxSpriteIds[battler];
    u32 n = 0, i, gx, x, y, spriteId, barSprite;
    s32 dx, dy;
    if (amount == 0 || healthbox >= MAX_SPRITES || gSprites[healthbox].invisible)
        return;
    // Replace any popup still showing for this battler.
    spriteId = sLabPopupSprite[battler];
    if (spriteId < MAX_SPRITES && gSprites[spriteId].inUse && gSprites[spriteId].callback == SpriteCB_LabRegenPopup)
    {
        DestroySprite(&gSprites[spriteId]);
        FreeSpriteTilesByTag(LAB_POPUP_TAG + battler);
    }
    if (amount > 999)
        amount = 999;
    digits[n++] = 10;   // '+'
    if (amount >= 100) digits[n++] = amount / 100;
    if (amount >= 10)  digits[n++] = amount / 10 % 10;
    digits[n++] = amount % 10;
    // Outline (dark) first, then the green glyph on top; each font pixel is 2x2.
    memset(tiles, 0, sizeof(tiles));
    for (i = 0; i < n; i++)
        for (y = 0; y < 5; y++)
            for (gx = 0; gx < 3; gx++)
                if (sLabPopupGlyphs[digits[i]][y] & (4 >> gx))
                    for (dy = -1; dy <= 2; dy++)
                        for (dx = -1; dx <= 2; dx++)
                            LabPopupPixel(tiles, 1 + i * 7 + gx * 2 + dx, 2 + y * 2 + dy, 1);
    for (i = 0; i < n; i++)
        for (y = 0; y < 5; y++)
            for (gx = 0; gx < 3; gx++)
                if (sLabPopupGlyphs[digits[i]][y] & (4 >> gx))
                    for (dy = 0; dy <= 1; dy++)
                        for (dx = 0; dx <= 1; dx++)
                            LabPopupPixel(tiles, 1 + i * 7 + gx * 2 + dx, 2 + y * 2 + dy, 10);
    sheet.data = tiles;   // LoadSpriteSheet copies the drawn number into sprite VRAM
    if (LoadSpriteSheet(&sheet) == 0xFFFF)
        return;
    x = gSprites[healthbox].x + gSprites[healthbox].x2 + 32;   // just above the healthbox's right side
    y = gSprites[healthbox].y + gSprites[healthbox].y2 - (IsOnPlayerSide(battler) && BattleLab_DualMode() ? 30 : 16);   // above your energy bar
    spriteId = CreateSprite(&tmpl, x, y, 0);
    if (spriteId >= MAX_SPRITES)
    {
        FreeSpriteTilesByTag(LAB_POPUP_TAG + battler);
        return;
    }
    barSprite = gSprites[healthbox].data[5];   // the health bar sprite: its palette has the bar green
    gSprites[spriteId].oam.paletteNum = gSprites[barSprite].oam.paletteNum;
    gSprites[spriteId].data[1] = LAB_POPUP_TAG + battler;
    sLabPopupSprite[battler] = spriteId;
}

static void LabRegenEnergy(void)
{
    u32 battler, elapsed = sLabNow - sLabRegenAt;
    sLabRegenAt = sLabNow;
    if (BattleLab_DualMode())   // lost health caps energy
    {
        for (battler = 0; battler < gBattlersCount; battler++)
        {
            u16 *energy = LabEnergyPtr(battler);
            if (gBattleMons[battler].hp > 0 && *energy > LabEnergyCap(battler))
            {
                DebugPrintf("LAB energy: %S capped by health (%d -> %d)", GetSpeciesName(gBattleMons[battler].species), *energy, LabEnergyCap(battler));
                sLabSapped |= 1u << battler;
                *energy = LabEnergyCap(battler);
                LabSyncEnergy(battler);
            }
        }
    }
    for (battler = 0; battler < gBattlersCount; battler++)
    {
        u16 *energy = LabEnergyPtr(battler);
        u32 hp = *energy, maxHp = LabEnergyCap(battler), gain;
        if (gBattleMons[battler].hp == 0 || hp >= maxHp || (sLabProtectedThisTurn & (1u << battler)))
        {
            sLabRegenCarry[battler] = 0;
            continue;
        }
        sLabRegenCarry[battler] += LabEnergyMax(battler) * (BattleLab_DualMode() ? 3 : LAB_REGEN_PERCENT) * elapsed;   // units of 1/(100*SCALE) HP
        gain = sLabRegenCarry[battler] / (100 * LAB_TIMELINE_SCALE);
        sLabRegenCarry[battler] %= 100 * LAB_TIMELINE_SCALE;
        if (gain == 0)
            continue;
        hp = (hp + gain < maxHp) ? hp + gain : maxHp;
        LabShowRegenPopup(battler, hp - *energy);
        *energy = hp;
        LabSyncEnergy(battler);
    }
    sLabProtectedThisTurn = 0;
}

// End-of-round messages (owner, 2026-11-27): "X is exhausted!", "X got its second wind!" and
// "X's stamina was sapped by the blow!". Built here, printed by the end-of-turn script.
static EWRAM_DATA u8 sLabMessage[200] = {0};
static EWRAM_DATA u8 sLabWasExhausted = 0;   // battler mask, for the exhausted / second-wind messages

static void LabQueueMessage(u32 battler, const u8 *what)
{
    u8 nick[POKEMON_NAME_LENGTH + 1];
    if (StringLength(sLabMessage) + 40 >= sizeof(sLabMessage))
        return;
    if (sLabMessage[0] != EOS)
        StringAppend(sLabMessage, COMPOUND_STRING("\p"));
    if (!IsOnPlayerSide(battler))
        StringAppend(sLabMessage, COMPOUND_STRING("The opposing "));
    GetMonData(GetBattlerMon(battler), MON_DATA_NICKNAME, nick);
    StringGet_Nickname(nick);
    StringAppend(sLabMessage, nick);
    StringAppend(sLabMessage, what);
}

bool32 BattleLab_TakePendingMessage(void)
{
    return BattleLab_StaminaMode() && sLabMessage[0] != EOS;
}

const u8 *BattleLab_PendingMessageText(void)
{
    // Copied out so the next round can queue new messages while this one prints.
    StringCopy(gStringVar4, sLabMessage);
    sLabMessage[0] = EOS;
    return gStringVar4;
}

static void LabQueueRoundMessages(void)
{
    u32 battler;
    for (battler = 0; battler < gBattlersCount; battler++)
    {
        u32 bit = 1u << battler;
        bool32 exhausted = gBattleMons[battler].hp > 0 && BattleLab_IsExhausted(battler);
        if (sLabSapped & bit)
            LabQueueMessage(battler, COMPOUND_STRING("'s stamina\nwas sapped by the blow!"));
        if (exhausted && !(sLabWasExhausted & bit))
            LabQueueMessage(battler, COMPOUND_STRING(" is exhausted!\nIt slows right down..."));
        else if (!exhausted && (sLabWasExhausted & bit) && gBattleMons[battler].hp > 0)
            LabQueueMessage(battler, COMPOUND_STRING(" got its\nsecond wind!"));
        if (exhausted)
            sLabWasExhausted |= bit;
        else
            sLabWasExhausted &= ~bit;
    }
    sLabSapped = 0;
}

// Dual: Pokémon on the bench catch their breath (+8% of max energy per 1000 clock units),
// but only up to the cap their health allows, so a battered Pokémon can't fully recover.
#define LAB_BENCH_REGEN_PERCENT 8
static EWRAM_DATA u32 sLabBenchRegenAt = 0;
static void LabBenchRegen(void)
{
    u32 side, i, battler, elapsed = sLabNow - sLabBenchRegenAt;
    sLabBenchRegenAt = sLabNow;
    for (side = 0; side < 2; side++)
    {
        struct Pokemon *party = side == 0 ? gParties[B_TRAINER_PLAYER] : gParties[B_TRAINER_OPPONENT_A];
        for (i = 0; i < PARTY_SIZE; i++)
        {
            u32 hp = GetMonData(&party[i], MON_DATA_HP), maxHp = GetMonData(&party[i], MON_DATA_MAX_HP);
            u32 max = sLabEnergyMax[side][i], cap, gain;
            bool32 active = FALSE;
            if (hp == 0 || maxHp == 0 || max == 0)
                continue;
            for (battler = 0; battler < gBattlersCount; battler++)
                if ((IsOnPlayerSide(battler) ? 0 : 1) == side && gBattlerPartyIndexes[battler] == i)
                    active = TRUE;
            if (active)
                continue;
            cap = max / 2 + max * hp / maxHp / 2;
            gain = max * LAB_BENCH_REGEN_PERCENT * elapsed / (100 * LAB_TIMELINE_SCALE);
            if (sLabEnergy[side][i] < cap)
                sLabEnergy[side][i] = (sLabEnergy[side][i] + gain < cap) ? sLabEnergy[side][i] + gain : cap;
        }
    }
}

void BattleLab_LogTurnEnd(void)
{
    if (!sLabInBattleFlag && !sAscCampaignBattle)
        return;
    if (BattleLab_StaminaMode())
    {
        u32 battler;
        if (BattleLab_DualMode())
            LabBenchRegen();
        LabRegenEnergy();
        LabTimelineRoundEnd();
        LabQueueRoundMessages();
        for (battler = 0; battler < gBattlersCount; battler++)
        {
            LabRefreshEnergyLabel(battler);   // damage may have tipped someone into Exhausted
            if (BattleLab_DualMode())
                LabDrawEnergyBarValue(battler, *LabEnergyPtr(battler));   // and moved the health cap
        }
    }
    if (sAscCampaignBattle)
        LabSaveCampaignEnergy();   // F019: kept up to date every round, and again at the end
    if (!sLabInBattleFlag)
        return;
    DebugPrintf("LAB turn %d", gBattleResults.battleTurnCounter + 1);
    LabLogBattlers();
}

// Final state, logged by the battle just before it clears its data (battle_main.c hook), so a
// decisive turn (a knockout, end-of-turn damage) is always in the log even though the battle
// leaves before the turn-end hook.
void BattleLab_LogBattleEnd(void)
{
    if (sAscCampaignBattle)
    {
        LabSaveCampaignEnergy();
        sAscCampaignBattle = FALSE;   // F019: the campaign battle is over
        sActiveCombat = LAB_COMBAT_CLASSIC;
        return;
    }
    if (!sLabInBattleFlag)
        return;
    if (sLabAiDecisions)
        DebugPrintf("LAB ai summary: %d decisions, mean %d ticks, max %d ticks (4389 ticks = 1 frame)",
                    sLabAiDecisions, sLabAiTicksTotal / sLabAiDecisions, sLabAiTicksMax);
    DebugPrintf("LAB end: %S after %d turn(s)",
                BattleLab_ResultName(BattleLab_ResultFromOutcome(gBattleOutcome, sLabEndRequest == LAB_END_QUIT, sLabEndRequest == LAB_END_RESTART)),
                BattleLab_TurnsPlayed(gBattleResults.battleTurnCounter));
    LabLogBattlers();
}

bool32 BattleLab_ShouldAutoOpen(void)
{
    if (sLabAutoOpenDecision == LAB_AUTO_UNDECIDED)
        sLabAutoOpenDecision = (BattleLab_AutoOpenScenario() >= 0 && !JOY_HELD(B_BUTTON)) ? LAB_AUTO_OPEN : LAB_AUTO_SKIP;
    if (sLabAutoOpenDecision != LAB_AUTO_OPEN)
        return FALSE;
    sLabAutoOpenDecision = LAB_AUTO_SKIP;   // open once; the Lab's Exit soft-resets anyway
    return TRUE;
}

static void LabCopyFromBlock(u32 blockIndex, struct LabScenarioRecord *out)
{
    const volatile u8 *src = LabBlock() + LAB_BLOCK_MARKER_SIZE + 4 + blockIndex * sizeof(struct LabScenarioRecord);
    u8 *dst = (u8 *)out;
    u32 i;
    for (i = 0; i < sizeof(*out); i++)
        dst[i] = src[i];
}

// Scenario files are checked by the tool, but the game never trusts the block: anything out of
// range becomes empty or a safe default.
static void LabSanitiseSlot(struct LabSlot *s)
{
    u32 i;
    if (!BattleLab_IsSpeciesSelectable(s->species))
    {
        memset(s, 0, sizeof(*s));
        s->level = LAB_DEFAULT_LEVEL;
        s->ivs = MAX_PER_STAT_IVS;
        s->hpPercent = 100;
        return;
    }
    s->flags = (s->flags & LAB_SLOT_ACTIVE) | LAB_SLOT_MODIFIED;   // built from the fields
    if (s->level < 1) s->level = 1;
    if (s->level > MAX_LEVEL) s->level = MAX_LEVEL;
    if (s->moves[0] == MOVE_NONE && s->moves[1] == MOVE_NONE && s->moves[2] == MOVE_NONE && s->moves[3] == MOVE_NONE)
        LabFillLevelUpMoves(s);   // no moves in the file: the species' level-up moves
    for (i = 0; i < MAX_MON_MOVES; i++)
        if (s->moves[i] != MOVE_NONE && !BattleLab_IsMoveSelectable(s->moves[i]))
            s->moves[i] = MOVE_NONE;
    if (s->item != ITEM_NONE && !BattleLab_IsItemSelectable(s->item))
        s->item = ITEM_NONE;
    if (s->abilitySlot >= NUM_ABILITY_SLOTS || gSpeciesInfo[s->species].abilities[s->abilitySlot] == ABILITY_NONE)
        s->abilitySlot = 0;
    if (s->nature >= NUM_NATURES) s->nature = 0;
    if (s->ivs > MAX_PER_STAT_IVS) s->ivs = MAX_PER_STAT_IVS;
    if (s->hpPercent < 1 || s->hpPercent > 100) s->hpPercent = 100;
    if (s->status >= LAB_STATUS_COUNT) s->status = LAB_STATUS_NONE;
}

static void LabSanitiseRules(struct LabRules *r)
{
    if (r->format >= LAB_FORMAT_COUNT) r->format = 0;
    if (r->weather >= LAB_WEATHER_COUNT) r->weather = 0;
    if (r->terrain >= LAB_TERRAIN_COUNT) r->terrain = 0;
    if (r->ai >= LAB_AI_COUNT) r->ai = 0;
    r->fixedSeed = r->fixedSeed ? TRUE : FALSE;
    if (r->combat >= LAB_COMBAT_COUNT) r->combat = LAB_COMBAT_CLASSIC;
}

static void LabLoadBuiltIn(struct LabScenarioRecord *rec)
{
    u32 side, slot;
    memset(rec, 0, sizeof(*rec));
    StringCopy(rec->id, COMPOUND_STRING("LAB-001"));
    StringCopy(rec->name, COMPOUND_STRING("Baseline 1v1"));
    for (side = 0; side < LAB_SIDE_COUNT; side++)
        for (slot = 0; slot < PARTY_SIZE; slot++)
            LabSlotFromTrainer(side, slot, &rec->teams[side][slot]);
}

// Loads scenario `index` (0 = built-in LAB-001, 1.. = the ROM block) as the current one and
// resets the teams and rules to it. Returns FALSE (and loads nothing) for a bad index.
bool32 BattleLab_SelectScenario(u32 index)
{
    u32 side, slot;
    if (index >= BattleLab_ScenarioCount())
        return FALSE;
    if (index == 0)
    {
        LabLoadBuiltIn(&sLabCurrent);
    }
    else
    {
        LabCopyFromBlock(index - 1, &sLabCurrent);
        sLabCurrent.id[sizeof(sLabCurrent.id) - 1] = EOS;
        sLabCurrent.name[sizeof(sLabCurrent.name) - 1] = EOS;
        LabSanitiseRules(&sLabCurrent.rules);
        for (side = 0; side < LAB_SIDE_COUNT; side++)
            for (slot = 0; slot < PARTY_SIZE; slot++)
                LabSanitiseSlot(&sLabCurrent.teams[side][slot]);
    }
    sLabScenarioIndex = index;
    BattleLab_ResetTeams();
    BattleLab_ResetRules();
    return TRUE;
}

static void LabEnsureScenario(void)
{
    if (sLabCurrent.id[0] == 0 || sLabCurrent.id[0] == EOS)
    {
        LabLoadBuiltIn(&sLabCurrent);
        sLabScenarioIndex = 0;
    }
}

// Scenario id / name for the menu (index 0 = built-in).
static void LabScenarioLabel(u32 index, u8 *id, u8 *name)
{
    struct LabScenarioRecord *tmp;
    if (index == 0)
    {
        StringCopy(id, COMPOUND_STRING("LAB-001"));
        StringCopy(name, COMPOUND_STRING("Baseline 1v1"));
        return;
    }
    tmp = &gBattleLabScratchRecord;
    LabCopyFromBlock(index - 1, tmp);
    tmp->id[sizeof(tmp->id) - 1] = EOS;
    tmp->name[sizeof(tmp->name) - 1] = EOS;
    StringCopy(id, tmp->id);
    StringCopy(name, tmp->name);
}

// Builds one side's battle party from the Lab slots, in place: the scenario party is created
// first so unchanged slots keep their scenario moves, IVs and items; changed slots get a fresh
// Pokémon (31 IVs, level-up moves, no item). Inactive and empty slots are skipped.
static void BattleLab_BuildSide(u32 side, struct Pokemon *party)
{
    u32 slot, out = 0;
    CreateNPCTrainerPartyFromTrainer(party, BattleLab_ScenarioTrainer(side));
    for (slot = 0; slot < PARTY_SIZE; slot++)
    {
        const struct LabSlot *s = &gBattleLabTeams[side][slot];
        if (!LabSlotInBattle(s))
            continue;
        if (s->flags & LAB_SLOT_MODIFIED)
            LabBuildMonFromSlot(&party[out], s);
        else if (out != slot)
            party[out] = party[slot];
        out++;
    }
    for (; out < PARTY_SIZE; out++)
        ZeroMonData(&party[out]);
}

void BattleLab_SetupScenarioParties(void)
{
    ZeroPlayerPartyMons();
    ZeroEnemyPartyMons();
    BattleLab_BuildSide(LAB_SIDE_PLAYER, gParties[B_TRAINER_PLAYER]);
    BattleLab_BuildSide(LAB_SIDE_OPPONENT, gParties[B_TRAINER_OPPONENT_A]);
    CalculatePlayerPartyCount();
    CalculateEnemyPartyCount();
}

static void VBlankCB_BattleLab(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_BattleLab_Main(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void CB2_BattleLab_Init(void)
{
    switch (gMain.state)
    {
    case 0:
        // Let the title screen's fade to black finish first.
        UpdatePaletteFade();
        if (!gPaletteFade.active)
            gMain.state++;
        break;
    case 1:
        SetVBlankCallback(NULL);
        DmaClearLarge16(3, (void *)(VRAM), VRAM_SIZE, 0x1000);
        DmaClear32(3, OAM, OAM_SIZE);
        DmaClear16(3, PLTT, PLTT_SIZE);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sLabBgTemplates, ARRAY_COUNT(sLabBgTemplates));
        ChangeBgX(0, 0, BG_COORD_SET);
        ChangeBgY(0, 0, BG_COORD_SET);
        InitWindows(sLabWindowTemplates);
        DeactivateAllTextPrinters();
        ResetPaletteFade();
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        FreeAllSpritePalettes();
        gMain.state++;
        break;
    case 2:
        // The Lab is a fresh, unsaved game in memory; the campaign save on the
        // cartridge is never written and is reloaded by the soft reset on Exit.
        if (sLabNeedsFreshGame)
        {
            // Replace the campaign data loaded at the title screen with a blank game, so
            // Lab battles never see the campaign party or bag. Nothing is saved.
            sLabNeedsFreshGame = FALSE;
            NewGameInitData();
            StringCopy(gSaveBlock2Ptr->playerName, COMPOUND_STRING("LAB"));
            gSaveBlock2Ptr->playerGender = MALE;
        }
        LoadPalette(&sLabBackdrop, BG_PLTT_ID(0), PLTT_SIZEOF(1));
        LoadUserWindowBorderGfx(WIN_TITLE, FRAME_TILE, BG_PLTT_ID(FRAME_PAL));
        LoadPalette(gStandardMenuPalette, BG_PLTT_ID(TEXT_PAL), PLTT_SIZE_4BPP);
        gMain.state++;
        break;
    case 3:
    {
        u32 i;
        for (i = 0; i <= WIN_INFO; i++)
        {
            FillWindowPixelBuffer(i, PIXEL_FILL(1));
            PutWindowTilemap(i);
            DrawStdFrameWithCustomTileAndPalette(i, FALSE, FRAME_TILE, FRAME_PAL);
        }
        sLabMode = LAB_MODE_MAIN;   // after a battle, always come back to the Lab menu
        BattleLab_DrawTitle();
        PrintMenuTable(WIN_MENU, LAB_MENU_COUNT, sLabMenuItems);
        InitMenuInUpperLeftCornerNormal(WIN_MENU, LAB_MENU_COUNT, sLabCursor);
        BattleLab_DrawMainInfo();
        for (i = 0; i <= WIN_INFO; i++)
            CopyWindowToVram(i, COPYWIN_FULL);
        gMain.state++;
        break;
    }
    case 4:
        // Set the display mode first: ShowBg() then adds the BG0 enable bit to it.
        // (The other order clears BG0 again and hides every Lab window.)
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        SetVBlankCallback(VBlankCB_BattleLab);
        if (sLabEndRequest == LAB_END_RESTART)
        {
            // Lab: Restart (F010): stay black and start the same setup again.
            sLabEndRequest = LAB_END_NONE;
            BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 16, RGB_BLACK);
            CreateTask(Task_BattleLab_StartBattle, 0);
        }
        else
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
            CreateTask(Task_BattleLab_Input, 0);
        }
        SetMainCallback2(CB2_BattleLab_Main);
        break;
    }
}

static void BattleLab_PrintInfo(const u8 *text)
{
    FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(1));
    AddTextPrinterParameterized(WIN_INFO, FONT_NORMAL, text, 0, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(WIN_INFO, COPYWIN_GFX);
}

// ---------------------------------------------------------------- screens (F008)

static const u8 sTextColorNormal[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 sTextColorDim[]    = {TEXT_COLOR_WHITE, TEXT_COLOR_LIGHT_GRAY, TEXT_COLOR_WHITE};
static const u8 sTextColorPick[]   = {TEXT_COLOR_WHITE, TEXT_COLOR_RED, TEXT_COLOR_LIGHT_RED};

static void LabAppendNumber(u8 *dst, u32 value, u32 digits)
{
    u8 num[8];
    ConvertIntToDecimalStringN(num, value, STR_CONV_MODE_LEFT_ALIGN, digits);
    StringAppend(dst, num);
}

static void BattleLab_DrawTitle(void)
{
    u8 *buf = gStringVar3;

    buf[0] = EOS;
    LabEnsureScenario();
    StringAppend(buf, COMPOUND_STRING("BATTLE LAB · "));
    StringAppend(buf, sLabCurrent.id);
    if (BattleLab_IsEdited())
        StringAppend(buf, COMPOUND_STRING(" (edited)"));
    // Last battle's result (F010), e.g. "· Won 4t".
    if (sLabLastResult != LAB_RESULT_NONE)
    {
        StringAppend(buf, COMPOUND_STRING(" · "));
        StringAppend(buf, BattleLab_ResultName(sLabLastResult));
        if (sLabLastResult == LAB_RESULT_WON || sLabLastResult == LAB_RESULT_LOST)
        {
            StringAppend(buf, COMPOUND_STRING(" "));
            LabAppendNumber(buf, sLabLastTurns, 3);
            StringAppend(buf, COMPOUND_STRING("t"));
        }
    }
    FillWindowPixelBuffer(WIN_TITLE, PIXEL_FILL(1));
    AddTextPrinterParameterized(WIN_TITLE, FONT_NARROW, buf, 0, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(WIN_TITLE, COPYWIN_GFX);
}

// "You 1: SWAMPERT 50" style summary of the lead and team size for each side.
static void LabAppendSideSummary(u8 *dst, u32 side)
{
    u32 slot;
    StringAppend(dst, side == LAB_SIDE_PLAYER ? COMPOUND_STRING("You: ") : COMPOUND_STRING("Foe: "));
    LabAppendNumber(dst, BattleLab_ActiveCount(side), 1);
    StringAppend(dst, gText_NewLine);
    for (slot = 0; slot < PARTY_SIZE; slot++)
    {
        const struct LabSlot *s = &gBattleLabTeams[side][slot];
        if (s->species != SPECIES_NONE && (s->flags & LAB_SLOT_ACTIVE))
        {
            StringAppend(dst, GetSpeciesName(s->species));
            StringAppend(dst, gText_NewLine);
            StringAppend(dst, COMPOUND_STRING(" Lv"));
            LabAppendNumber(dst, s->level, 3);
            return;
        }
    }
    StringAppend(dst, COMPOUND_STRING("(nobody)"));
}

static void BattleLab_DrawMainInfo(void)
{
    u8 *buf = gStringVar4;
    buf[0] = EOS;
    LabAppendSideSummary(buf, LAB_SIDE_PLAYER);
    StringAppend(buf, gText_NewLine);
    LabAppendSideSummary(buf, LAB_SIDE_OPPONENT);
    BattleLab_PrintInfo(buf);
}

static void BattleLab_ShowMainMenu(void)
{
    sLabMode = LAB_MODE_MAIN;
    FillWindowPixelBuffer(WIN_MENU, PIXEL_FILL(1));
    PrintMenuTable(WIN_MENU, LAB_MENU_COUNT, sLabMenuItems);
    InitMenuInUpperLeftCornerNormal(WIN_MENU, LAB_MENU_COUNT, sLabCursor);
    CopyWindowToVram(WIN_MENU, COPYWIN_GFX);
    BattleLab_DrawTitle();
    BattleLab_DrawMainInfo();
}

// Team list: one row per slot, e.g. "▶2 GENGAR! 40" or " 3 -PIKACHU 12" (switched off).
static void BattleLab_DrawTeam(void)
{
    u32 slot;
    u8 *buf = gStringVar4;

    FillWindowPixelBuffer(WIN_MENU, PIXEL_FILL(1));
    for (slot = 0; slot < PARTY_SIZE; slot++)
    {
        const struct LabSlot *s = &gBattleLabTeams[sLabSide][slot];
        const u8 *color = sTextColorNormal;

        buf[0] = (slot == sLabSlot) ? CHAR_RIGHT_ARROW : CHAR_SPACE;
        buf[1] = CHAR_1 + slot;
        buf[2] = CHAR_SPACE;
        buf[3] = EOS;
        if (s->species == SPECIES_NONE)
        {
            StringAppend(buf, COMPOUND_STRING("- - -"));
            color = sTextColorDim;
        }
        else
        {
            if (!(s->flags & LAB_SLOT_ACTIVE))
            {
                StringAppend(buf, COMPOUND_STRING("off "));
                color = sTextColorDim;
            }
            StringAppend(buf, GetSpeciesName(s->species));
            if (BattleLab_SlotEdited(sLabSide, slot))
                StringAppend(buf, COMPOUND_STRING("!"));
        }
        AddTextPrinterParameterized3(WIN_MENU, FONT_NARROW, 0, slot * 16 + 1, color, TEXT_SKIP_DRAW, buf);
        if (s->species != SPECIES_NONE)
        {
            buf[0] = EOS;
            StringAppend(buf, COMPOUND_STRING("Lv"));
            LabAppendNumber(buf, s->level, 3);
            AddTextPrinterParameterized3(WIN_MENU, FONT_NARROW, 92, slot * 16 + 1, color, TEXT_SKIP_DRAW, buf);
        }
    }
    CopyWindowToVram(WIN_MENU, COPYWIN_GFX);
}

static void BattleLab_OpenTeam(u32 side)
{
    sLabMode = LAB_MODE_TEAM;
    sLabSide = side;
    sLabSlot = 0;
    BattleLab_DrawTeam();
    BattleLab_PrintInfo(sText_TeamHelp);
}

// Slot editor in the info window: species, dex number/form, level, in battle, reset, clear.
static const u8 *const sLabStatusNames[LAB_STATUS_COUNT] =
{
    [LAB_STATUS_NONE]      = COMPOUND_STRING("None"),
    [LAB_STATUS_POISON]    = COMPOUND_STRING("Poison"),
    [LAB_STATUS_TOXIC]     = COMPOUND_STRING("Bad poison"),
    [LAB_STATUS_BURN]      = COMPOUND_STRING("Burn"),
    [LAB_STATUS_PARALYSIS] = COMPOUND_STRING("Paralysis"),
    [LAB_STATUS_SLEEP]     = COMPOUND_STRING("Sleep"),
    [LAB_STATUS_FREEZE]    = COMPOUND_STRING("Freeze"),
};

static const u8 *const sLabStatShort[NUM_STATS] =
{
    [STAT_HP] = COMPOUND_STRING("HP"), [STAT_ATK] = COMPOUND_STRING("Atk"), [STAT_DEF] = COMPOUND_STRING("Def"),
    [STAT_SPEED] = COMPOUND_STRING("Spe"), [STAT_SPATK] = COMPOUND_STRING("SpA"), [STAT_SPDEF] = COMPOUND_STRING("SpD"),
};

static const u8 *const sLabEditLabels[EDIT_ROW_COUNT] =
{
    [EDIT_ROW_SPECIES] = COMPOUND_STRING("Species"),
    [EDIT_ROW_LEVEL]   = COMPOUND_STRING("Level"),
    [EDIT_ROW_ACTIVE]  = COMPOUND_STRING("Battle"),
    [EDIT_ROW_MOVE1]   = COMPOUND_STRING("Move 1"),
    [EDIT_ROW_MOVE2]   = COMPOUND_STRING("Move 2"),
    [EDIT_ROW_MOVE3]   = COMPOUND_STRING("Move 3"),
    [EDIT_ROW_MOVE4]   = COMPOUND_STRING("Move 4"),
    [EDIT_ROW_ABILITY] = COMPOUND_STRING("Ability"),
    [EDIT_ROW_ITEM]    = COMPOUND_STRING("Item"),
    [EDIT_ROW_NATURE]  = COMPOUND_STRING("Nature"),
    [EDIT_ROW_IVS]     = COMPOUND_STRING("IVs"),
    [EDIT_ROW_HP]      = COMPOUND_STRING("HP"),
    [EDIT_ROW_STATUS]  = COMPOUND_STRING("Status"),
    [EDIT_ROW_RESET]   = COMPOUND_STRING("Reset slot"),
    [EDIT_ROW_CLEAR]   = COMPOUND_STRING("Clear slot"),
};

static const u8 sText_None[] = _("- - -");

static void LabEditRowValue(u8 *buf, u32 row, const struct LabSlot *s)
{
    buf[0] = EOS;
    if (s->species == SPECIES_NONE && row != EDIT_ROW_SPECIES)
        return;
    switch (row)
    {
    case EDIT_ROW_SPECIES:
        StringAppend(buf, s->species == SPECIES_NONE ? sText_None : GetSpeciesName(s->species));
        break;
    case EDIT_ROW_LEVEL:
        LabAppendNumber(buf, s->level, 3);
        break;
    case EDIT_ROW_ACTIVE:
        StringAppend(buf, (s->flags & LAB_SLOT_ACTIVE) ? COMPOUND_STRING("Yes") : COMPOUND_STRING("No"));
        break;
    case EDIT_ROW_MOVE1 ... EDIT_ROW_MOVE4:
    {
        u32 move = s->moves[row - EDIT_ROW_MOVE1];
        StringAppend(buf, move == MOVE_NONE ? sText_None : GetMoveName(move));
        if (!BattleLab_SpeciesCanLearn(s->species, move))
            StringAppend(buf, COMPOUND_STRING("!"));
        break;
    }
    case EDIT_ROW_ABILITY:
        StringAppend(buf, gAbilitiesInfo[gSpeciesInfo[s->species].abilities[s->abilitySlot]].name);
        break;
    case EDIT_ROW_ITEM:
        StringAppend(buf, s->item == ITEM_NONE ? sText_None : GetItemName(s->item));
        break;
    case EDIT_ROW_NATURE:
        StringAppend(buf, gNaturesInfo[s->nature].name);
        break;
    case EDIT_ROW_IVS:
        LabAppendNumber(buf, s->ivs, 2);
        break;
    case EDIT_ROW_HP:
        LabAppendNumber(buf, s->hpPercent, 3);
        StringAppend(buf, COMPOUND_STRING("%"));
        break;
    case EDIT_ROW_STATUS:
        StringAppend(buf, sLabStatusNames[s->status]);
        break;
    }
}

// Context and controls for the selected editor row, in the info window.
static void BattleLab_DrawEditorInfo(void)
{
    const struct LabSlot *s = &gBattleLabTeams[sLabSide][sLabSlot];
    u8 *buf = gStringVar4;

    buf[0] = EOS;
    switch (sLabEditRow)
    {
    case EDIT_ROW_SPECIES:
        if (s->species != SPECIES_NONE)
        {
            u32 form = LabFormNumber(s->species);
            StringAppend(buf, COMPOUND_STRING("No."));
            ConvertIntToDecimalStringN(gStringVar1, gSpeciesInfo[s->species].natDexNum, STR_CONV_MODE_LEADING_ZEROS, 4);
            StringAppend(buf, gStringVar1);
            if (form > 1)
            {
                StringAppend(buf, COMPOUND_STRING(" f"));
                LabAppendNumber(buf, form, 2);
            }
            StringAppend(buf, gText_NewLine);
        }
        StringAppend(buf, COMPOUND_STRING("{LEFT_ARROW}{RIGHT_ARROW} +-1\nL/R +-25\nSELECT: A-Z"));
        break;
    case EDIT_ROW_MOVE1 ... EDIT_ROW_MOVE4:
        if (!BattleLab_SpeciesCanLearn(s->species, s->moves[sLabEditRow - EDIT_ROW_MOVE1]))
            StringAppend(buf, COMPOUND_STRING("! not in its\nlearnset\n"));
        StringAppend(buf, COMPOUND_STRING("{LEFT_ARROW}{RIGHT_ARROW} +-1\nL/R +-25\nSELECT: A-Z"));
        break;
    case EDIT_ROW_ITEM:
        StringAppend(buf, COMPOUND_STRING("{LEFT_ARROW}{RIGHT_ARROW} +-1\nL/R +-25\nSELECT: A-Z"));
        break;
    case EDIT_ROW_NATURE:
        if (gNaturesInfo[s->nature].statUp != gNaturesInfo[s->nature].statDown)
        {
            StringAppend(buf, COMPOUND_STRING("+"));
            StringAppend(buf, sLabStatShort[gNaturesInfo[s->nature].statUp]);
            StringAppend(buf, COMPOUND_STRING(" -"));
            StringAppend(buf, sLabStatShort[gNaturesInfo[s->nature].statDown]);
        }
        else
        {
            StringAppend(buf, COMPOUND_STRING("neutral"));
        }
        StringAppend(buf, COMPOUND_STRING("\n{LEFT_ARROW}{RIGHT_ARROW} change"));
        break;
    case EDIT_ROW_LEVEL:
    case EDIT_ROW_HP:
        StringAppend(buf, COMPOUND_STRING("{LEFT_ARROW}{RIGHT_ARROW} +-1\nL/R +-10"));
        break;
    case EDIT_ROW_IVS:
        StringAppend(buf, COMPOUND_STRING("All six\nstats\n{LEFT_ARROW}{RIGHT_ARROW} +-1\nL/R +-10"));
        break;
    case EDIT_ROW_RESET:
        StringAppend(buf, COMPOUND_STRING("A: back to\nthe scenario"));
        break;
    case EDIT_ROW_CLEAR:
        StringAppend(buf, COMPOUND_STRING("A: empty\nthis slot"));
        break;
    default:
        StringAppend(buf, COMPOUND_STRING("{LEFT_ARROW}{RIGHT_ARROW} change"));
        break;
    }
    StringAppend(buf, COMPOUND_STRING("\nB: back"));
    FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(1));
    AddTextPrinterParameterized3(WIN_INFO, FONT_SMALL, 0, 1, sTextColorNormal, TEXT_SKIP_DRAW, buf);
    CopyWindowToVram(WIN_INFO, COPYWIN_GFX);
}

// Slot editor: a scrolling list (six rows visible) in the left window.
static void BattleLab_DrawEditor(void)
{
    const struct LabSlot *s = &gBattleLabTeams[sLabSide][sLabSlot];
    u8 *buf = gStringVar4;
    u32 i;

    if (sLabEditRow < sLabEditScroll)
        sLabEditScroll = sLabEditRow;
    else if (sLabEditRow >= sLabEditScroll + LAB_VISIBLE_ROWS)
        sLabEditScroll = sLabEditRow - LAB_VISIBLE_ROWS + 1;

    FillWindowPixelBuffer(WIN_MENU, PIXEL_FILL(1));
    for (i = 0; i < LAB_VISIBLE_ROWS; i++)
    {
        u32 row = sLabEditScroll + i;
        u8 label[16];
        if (row >= EDIT_ROW_COUNT)
            break;
        label[0] = (row == sLabEditRow) ? CHAR_RIGHT_ARROW : CHAR_SPACE;
        label[1] = EOS;
        StringAppend(label, sLabEditLabels[row]);
        AddTextPrinterParameterized3(WIN_MENU, FONT_NARROW, 0, i * 16 + 1, sTextColorDim, TEXT_SKIP_DRAW, label);
        if (row == sLabEditRow)
            AddTextPrinterParameterized3(WIN_MENU, FONT_NARROW, 0, i * 16 + 1, sTextColorNormal, TEXT_SKIP_DRAW, label);
        LabEditRowValue(buf, row, s);
        AddTextPrinterParameterized3(WIN_MENU, FONT_NARROW, 42, i * 16 + 1, sTextColorNormal, TEXT_SKIP_DRAW, buf);
    }
    CopyWindowToVram(WIN_MENU, COPYWIN_GFX);
    BattleLab_DrawEditorInfo();
}

static void BattleLab_DrawLetters(void)
{
    u32 i;
    u8 letter[2];

    FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(1));
    letter[1] = EOS;
    for (i = 0; i < 26; i++)
    {
        letter[0] = CHAR_A + i;
        AddTextPrinterParameterized3(WIN_INFO, FONT_NORMAL, (i % 6) * 13 + 2, (i / 6) * 16 + 1,
                                     i == sLabLetter ? sTextColorPick : sTextColorNormal, TEXT_SKIP_DRAW, letter);
    }
    AddTextPrinterParameterized3(WIN_INFO, FONT_SMALL, 2, 81, sTextColorDim, TEXT_SKIP_DRAW, COMPOUND_STRING("A: jump B: back"));
    CopyWindowToVram(WIN_INFO, COPYWIN_GFX);
}

static void BattleLab_RefreshTeamAndEditor(void)
{
    BattleLab_DrawEditor();
    BattleLab_DrawTitle();
}

static void BattleLab_HandleTeamInput(void)
{
    if (JOY_REPEAT(DPAD_UP))
    {
        sLabSlot = (sLabSlot + PARTY_SIZE - 1) % PARTY_SIZE;
        PlaySE(SE_SELECT);
        BattleLab_DrawTeam();
    }
    else if (JOY_REPEAT(DPAD_DOWN))
    {
        sLabSlot = (sLabSlot + 1) % PARTY_SIZE;
        PlaySE(SE_SELECT);
        BattleLab_DrawTeam();
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        sLabMode = LAB_MODE_EDIT;
        sLabEditRow = EDIT_ROW_SPECIES;
        sLabEditScroll = 0;
        BattleLab_DrawEditor();
    }
    else if (JOY_NEW(START_BUTTON))
    {
        PlaySE(SE_SELECT);
        sLabMode = LAB_MODE_CONFIRM_RESET;
        BattleLab_PrintInfo(sText_ConfirmReset);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        BattleLab_ShowMainMenu();
    }
}

static void LabOpenLetters(u32 row, const u8 *currentName)
{
    PlaySE(SE_SELECT);
    sLabMode = LAB_MODE_LETTER;
    sLabLetterRow = row;
    sLabLetter = (currentName != NULL && currentName[0] >= CHAR_A && currentName[0] < CHAR_A + 26) ? currentName[0] - CHAR_A : 0;
    BattleLab_DrawLetters();
}

static void BattleLab_HandleEditInput(void)
{
    struct LabSlot *s = &gBattleLabTeams[sLabSide][sLabSlot];
    bool32 big = FALSE;
    s32 delta = 0;

    if (JOY_REPEAT(DPAD_UP))
    {
        sLabEditRow = (sLabEditRow + EDIT_ROW_COUNT - 1) % EDIT_ROW_COUNT;
        BattleLab_DrawEditor();
        return;
    }
    if (JOY_REPEAT(DPAD_DOWN))
    {
        sLabEditRow = (sLabEditRow + 1) % EDIT_ROW_COUNT;
        BattleLab_DrawEditor();
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        sLabMode = LAB_MODE_TEAM;
        BattleLab_DrawTeam();
        BattleLab_PrintInfo(sText_TeamHelp);
        BattleLab_DrawTitle();
        return;
    }

    if (JOY_REPEAT(DPAD_RIGHT))
        delta = 1;
    else if (JOY_REPEAT(DPAD_LEFT))
        delta = -1;
    else if (JOY_REPEAT(R_BUTTON))
        delta = 1, big = TRUE;
    else if (JOY_REPEAT(L_BUTTON))
        delta = -1, big = TRUE;

    // Rows other than species need a Pokémon in the slot.
    if (s->species == SPECIES_NONE && sLabEditRow != EDIT_ROW_SPECIES
     && sLabEditRow != EDIT_ROW_RESET && sLabEditRow != EDIT_ROW_CLEAR)
        return;

    switch (sLabEditRow)
    {
    case EDIT_ROW_SPECIES:
        if (delta != 0)
            BattleLab_SetSlotSpecies(sLabSide, sLabSlot, BattleLab_StepSpecies(s->species, delta * (big ? 25 : 1)));
        else if (JOY_NEW(SELECT_BUTTON) || JOY_NEW(A_BUTTON))
            return LabOpenLetters(EDIT_ROW_SPECIES, s->species != SPECIES_NONE ? GetSpeciesName(s->species) : NULL);
        else
            return;
        break;
    case EDIT_ROW_LEVEL:
        if (delta == 0)
            return;
        BattleLab_SetSlotLevel(sLabSide, sLabSlot, (s32)s->level + delta * (big ? 10 : 1));
        break;
    case EDIT_ROW_ACTIVE:
        if (delta == 0 && !JOY_NEW(A_BUTTON))
            return;
        PlaySE(SE_SELECT);
        BattleLab_SetSlotActive(sLabSide, sLabSlot, !(s->flags & LAB_SLOT_ACTIVE));
        break;
    case EDIT_ROW_MOVE1 ... EDIT_ROW_MOVE4:
    {
        u32 index = sLabEditRow - EDIT_ROW_MOVE1;
        if (delta != 0)
            BattleLab_SetSlotMove(sLabSide, sLabSlot, index, BattleLab_StepMove(s->moves[index], delta * (big ? 25 : 1)));
        else if (JOY_NEW(SELECT_BUTTON) || JOY_NEW(A_BUTTON))
            return LabOpenLetters(sLabEditRow, s->moves[index] != MOVE_NONE ? GetMoveName(s->moves[index]) : NULL);
        else
            return;
        break;
    }
    case EDIT_ROW_ABILITY:
        if (delta == 0)
            return;
        BattleLab_SetSlotAbility(sLabSide, sLabSlot, BattleLab_StepAbilitySlot(s->species, s->abilitySlot, delta));
        break;
    case EDIT_ROW_ITEM:
        if (delta != 0)
            BattleLab_SetSlotItem(sLabSide, sLabSlot, BattleLab_StepItem(s->item, delta * (big ? 25 : 1)));
        else if (JOY_NEW(SELECT_BUTTON) || JOY_NEW(A_BUTTON))
            return LabOpenLetters(EDIT_ROW_ITEM, s->item != ITEM_NONE ? GetItemName(s->item) : NULL);
        else
            return;
        break;
    case EDIT_ROW_NATURE:
        if (delta == 0)
            return;
        BattleLab_SetSlotNature(sLabSide, sLabSlot, (s->nature + NUM_NATURES + delta) % NUM_NATURES);
        break;
    case EDIT_ROW_IVS:
        if (delta == 0)
            return;
        BattleLab_SetSlotIvs(sLabSide, sLabSlot, (s32)s->ivs + delta * (big ? 10 : 1));
        break;
    case EDIT_ROW_HP:
        if (delta == 0)
            return;
        BattleLab_SetSlotHpPercent(sLabSide, sLabSlot, (s32)s->hpPercent + delta * (big ? 10 : 1));
        break;
    case EDIT_ROW_STATUS:
        if (delta == 0)
            return;
        BattleLab_SetSlotStatus(sLabSide, sLabSlot, (s->status + LAB_STATUS_COUNT + delta) % LAB_STATUS_COUNT);
        break;
    case EDIT_ROW_RESET:
        if (!JOY_NEW(A_BUTTON))
            return;
        PlaySE(SE_SELECT);
        BattleLab_ResetSlot(sLabSide, sLabSlot);
        break;
    case EDIT_ROW_CLEAR:
        if (!JOY_NEW(A_BUTTON))
            return;
        PlaySE(SE_SELECT);
        BattleLab_ClearSlot(sLabSide, sLabSlot);
        break;
    default:
        return;
    }
    BattleLab_RefreshTeamAndEditor();
}

static void BattleLab_HandleLetterInput(void)
{
    u32 old = sLabLetter;

    if (JOY_REPEAT(DPAD_RIGHT))
        sLabLetter = (sLabLetter + 1) % 26;
    else if (JOY_REPEAT(DPAD_LEFT))
        sLabLetter = (sLabLetter + 25) % 26;
    else if (JOY_REPEAT(DPAD_DOWN))
        sLabLetter = (sLabLetter + 6 < 26) ? sLabLetter + 6 : sLabLetter % 6;
    else if (JOY_REPEAT(DPAD_UP))
        sLabLetter = (sLabLetter >= 6) ? sLabLetter - 6 : ((24 + sLabLetter < 26) ? 24 + sLabLetter : 18 + sLabLetter);
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        if (sLabLetterRow == EDIT_ROW_SPECIES)
        {
            u32 species = BattleLab_FirstSpeciesForLetter(sLabLetter);
            if (species != SPECIES_NONE)
                BattleLab_SetSlotSpecies(sLabSide, sLabSlot, species);
        }
        else if (sLabLetterRow == EDIT_ROW_ITEM)
        {
            u32 item = BattleLab_FirstItemForLetter(sLabLetter);
            if (item != ITEM_NONE)
                BattleLab_SetSlotItem(sLabSide, sLabSlot, item);
        }
        else
        {
            u32 move = BattleLab_FirstMoveForLetter(sLabLetter);
            if (move != MOVE_NONE)
                BattleLab_SetSlotMove(sLabSide, sLabSlot, sLabLetterRow - EDIT_ROW_MOVE1, move);
        }
        sLabMode = LAB_MODE_EDIT;
        BattleLab_RefreshTeamAndEditor();
        return;
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        sLabMode = LAB_MODE_EDIT;
        BattleLab_DrawEditor();
        return;
    }
    if (old != sLabLetter)
        BattleLab_DrawLetters();
}


// ---------------------------------------------------------------- battle loop (F010)

// Called from upstream's in-battle debug menu. TRUE if this is a Lab battle and the request
// was taken (the caller then ends the battle).
bool32 BattleLab_RequestEnd(bool32 restart)
{
    if (!sLabInBattleFlag)
        return FALSE;
    sLabEndRequest = restart ? LAB_END_RESTART : LAB_END_QUIT;
    return TRUE;
}

u32 BattleLab_ResultFromOutcome(u32 outcome, bool32 quit, bool32 restart)
{
    if (restart)
        return LAB_RESULT_RESTARTED;
    if (quit)
        return LAB_RESULT_QUIT;
    switch (outcome & ~B_OUTCOME_LINK_BATTLE_RAN)
    {
    case B_OUTCOME_WON:  return LAB_RESULT_WON;
    case B_OUTCOME_LOST: return LAB_RESULT_LOST;
    case B_OUTCOME_DREW: return LAB_RESULT_DREW;
    case B_OUTCOME_RAN:
    case B_OUTCOME_PLAYER_TELEPORTED:
    case B_OUTCOME_MON_FLED:
    case B_OUTCOME_MON_TELEPORTED:
        return LAB_RESULT_RAN;
    case B_OUTCOME_CAUGHT: return LAB_RESULT_CAUGHT;
    default: return LAB_RESULT_NONE;
    }
}

static const u8 *const sLabResultNames[LAB_RESULT_COUNT] =
{
    [LAB_RESULT_NONE]      = COMPOUND_STRING(""),
    [LAB_RESULT_WON]       = COMPOUND_STRING("Won"),
    [LAB_RESULT_LOST]      = COMPOUND_STRING("Lost"),
    [LAB_RESULT_DREW]      = COMPOUND_STRING("Draw"),
    [LAB_RESULT_RAN]       = COMPOUND_STRING("Ran"),
    [LAB_RESULT_CAUGHT]    = COMPOUND_STRING("Caught"),
    [LAB_RESULT_QUIT]      = COMPOUND_STRING("Quit"),
    [LAB_RESULT_RESTARTED] = COMPOUND_STRING("Restarted"),
};

// battleTurnCounter is the zero-based current turn (saturating u8); this is turns played.
u32 BattleLab_TurnsPlayed(u32 battleTurnCounter)
{
    return battleTurnCounter < 255 ? battleTurnCounter + 1 : 255;
}

const u8 *BattleLab_ResultName(u32 result)
{
    return result < LAB_RESULT_COUNT ? sLabResultNames[result] : sLabResultNames[LAB_RESULT_NONE];
}

// ---------------------------------------------------------------- scenario screen (F011)

static void BattleLab_DrawScenarios(void)
{
    u32 count = BattleLab_ScenarioCount();
    u32 top = 0, i;
    u8 *buf = gStringVar4;
    u8 name[32];

    if (sLabScenarioCursor >= LAB_VISIBLE_ROWS)
        top = sLabScenarioCursor - LAB_VISIBLE_ROWS + 1;
    FillWindowPixelBuffer(WIN_MENU, PIXEL_FILL(1));
    for (i = 0; i < LAB_VISIBLE_ROWS && top + i < count; i++)
    {
        u32 index = top + i;
        buf[0] = (index == sLabScenarioCursor) ? CHAR_RIGHT_ARROW : CHAR_SPACE;
        LabScenarioLabel(index, buf + 1, name);
        if (index == sLabScenarioIndex)
            StringAppend(buf, COMPOUND_STRING(" (now)"));
        AddTextPrinterParameterized3(WIN_MENU, FONT_NARROW, 0, i * 16 + 1, sTextColorNormal, TEXT_SKIP_DRAW, buf);
    }
    CopyWindowToVram(WIN_MENU, COPYWIN_GFX);

    LabScenarioLabel(sLabScenarioCursor, buf, name);
    buf[0] = EOS;
    StringAppend(buf, name);
    StringAppend(buf, count > 1 ? COMPOUND_STRING("\nA: load\nB: back") : COMPOUND_STRING("\nMore: play.sh\n--lab\nB: back"));
    FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(1));
    AddTextPrinterParameterized3(WIN_INFO, FONT_SMALL, 0, 1, sTextColorNormal, TEXT_SKIP_DRAW, buf);
    CopyWindowToVram(WIN_INFO, COPYWIN_GFX);
}

static void BattleLab_HandleScenarioInput(void)
{
    u32 count = BattleLab_ScenarioCount();
    if (JOY_REPEAT(DPAD_UP))
        sLabScenarioCursor = (sLabScenarioCursor + count - 1) % count;
    else if (JOY_REPEAT(DPAD_DOWN))
        sLabScenarioCursor = (sLabScenarioCursor + 1) % count;
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        BattleLab_SelectScenario(sLabScenarioCursor);
        sLabLastResult = LAB_RESULT_NONE;
        BattleLab_ShowMainMenu();
        return;
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        BattleLab_ShowMainMenu();
        return;
    }
    else
        return;
    BattleLab_DrawScenarios();
}

// ---------------------------------------------------------------- rules screen (F009)

static const u8 *const sLabRulesLabels[RULES_ROW_COUNT] =
{
    [RULES_ROW_FORMAT]    = COMPOUND_STRING("Format"),
    [RULES_ROW_COMBAT]    = COMPOUND_STRING("Combat"),
    [RULES_ROW_ENERGY]    = COMPOUND_STRING(" Energy"),
    [RULES_ROW_DAMAGE]    = COMPOUND_STRING(" Damage"),
    [RULES_ROW_COST]      = COMPOUND_STRING(" Cost"),
    [RULES_ROW_MOVES]     = COMPOUND_STRING("Moves"),
    [RULES_ROW_WEATHER]   = COMPOUND_STRING("Weather"),
    [RULES_ROW_TERRAIN]   = COMPOUND_STRING("Terrain"),
    [RULES_ROW_AI]        = COMPOUND_STRING("Foe AI"),
    [RULES_ROW_SEED_MODE] = COMPOUND_STRING("Seed"),
    [RULES_ROW_SEED]      = COMPOUND_STRING("Seed no."),
};
static const u8 *const sLabFormatNames[LAB_FORMAT_COUNT] =
{
    COMPOUND_STRING("Trainer 1v1"), COMPOUND_STRING("Trainer 2v2"), COMPOUND_STRING("Wild 1v1"),
};
static const u8 *const sLabWeatherNames[LAB_WEATHER_COUNT] =
{
    COMPOUND_STRING("None"), COMPOUND_STRING("Rain"), COMPOUND_STRING("Sun"),
    COMPOUND_STRING("Sandstorm"), COMPOUND_STRING("Hail"), COMPOUND_STRING("Snow"),
};
static const u8 *const sLabTerrainNames[LAB_TERRAIN_COUNT] =
{
    COMPOUND_STRING("None"), COMPOUND_STRING("Electric"), COMPOUND_STRING("Grassy"),
    COMPOUND_STRING("Misty"), COMPOUND_STRING("Psychic"),
};
static const u8 *const sLabAiNames[LAB_AI_COUNT] =
{
    COMPOUND_STRING("Scenario"), COMPOUND_STRING("Smart"), COMPOUND_STRING("Random"),
};

static const u8 *const sLabEnergyScaleNames[LAB_ENERGY_COUNT] = {COMPOUND_STRING("x1"), COMPOUND_STRING("x1.5"), COMPOUND_STRING("x2"), COMPOUND_STRING("x3")};
static const u8 *const sLabDamageScaleNames[LAB_DAMAGE_COUNT] = {COMPOUND_STRING("100%"), COMPOUND_STRING("75%"), COMPOUND_STRING("50%"), COMPOUND_STRING("33%")};
static const u8 *const sLabCostScaleNames[LAB_COST_COUNT] = {COMPOUND_STRING("100%"), COMPOUND_STRING("75%"), COMPOUND_STRING("50%"), COMPOUND_STRING("25%")};

static const u8 *const sLabCombatNames[LAB_COMBAT_COUNT] = {COMPOUND_STRING("Classic"), COMPOUND_STRING("Stamina"), COMPOUND_STRING("Dual bars")};

static void BattleLab_DrawRules(void)
{
    const struct LabRules *r = &gBattleLabRules;
    u8 *buf = gStringVar4;
    u32 row;

    // 10 rows fit the window: scroll to keep the cursor in view.
    u32 first = sLabRulesRow >= RULES_VISIBLE_ROWS ? sLabRulesRow - RULES_VISIBLE_ROWS + 1 : 0;

    FillWindowPixelBuffer(WIN_MENU, PIXEL_FILL(1));
    for (row = first; row < RULES_ROW_COUNT && row < first + RULES_VISIBLE_ROWS; row++)
    {
        u8 label[16];
        bool32 changed = FALSE;
        label[0] = (row == sLabRulesRow) ? CHAR_RIGHT_ARROW : CHAR_SPACE;
        label[1] = EOS;
        StringAppend(label, sLabRulesLabels[row]);
        AddTextPrinterParameterized3(WIN_MENU, FONT_NARROW, 0, (row - first) * 12, sTextColorNormal, TEXT_SKIP_DRAW, label);
        buf[0] = EOS;
        switch (row)
        {
        case RULES_ROW_COMBAT:    StringAppend(buf, sLabCombatNames[r->combat < LAB_COMBAT_COUNT ? r->combat : 0]); changed = r->combat != sLabCurrent.rules.combat; break;
        case RULES_ROW_ENERGY:    StringAppend(buf, sLabEnergyScaleNames[r->energyScale]); changed = r->energyScale != sLabCurrent.rules.energyScale; break;
        case RULES_ROW_DAMAGE:    StringAppend(buf, sLabDamageScaleNames[r->damageScale]); changed = r->damageScale != sLabCurrent.rules.damageScale; break;
        case RULES_ROW_COST:      StringAppend(buf, sLabCostScaleNames[r->costScale]);     changed = r->costScale != sLabCurrent.rules.costScale; break;
        case RULES_ROW_MOVES:     StringAppend(buf, r->talentMoves ? COMPOUND_STRING("Tree") : COMPOUND_STRING("Set")); changed = r->talentMoves != sLabCurrent.rules.talentMoves; break;
        case RULES_ROW_FORMAT:    StringAppend(buf, sLabFormatNames[r->format]);   changed = r->format != sLabCurrent.rules.format; break;
        case RULES_ROW_WEATHER:   StringAppend(buf, sLabWeatherNames[r->weather]); changed = r->weather != sLabCurrent.rules.weather; break;
        case RULES_ROW_TERRAIN:   StringAppend(buf, sLabTerrainNames[r->terrain]); changed = r->terrain != sLabCurrent.rules.terrain; break;
        case RULES_ROW_AI:        StringAppend(buf, sLabAiNames[r->ai]);           changed = r->ai != sLabCurrent.rules.ai; break;
        case RULES_ROW_SEED_MODE: StringAppend(buf, r->fixedSeed ? COMPOUND_STRING("Fixed") : COMPOUND_STRING("Random")); changed = r->fixedSeed != sLabCurrent.rules.fixedSeed || (r->fixedSeed && r->seed != sLabCurrent.rules.seed); break;
        case RULES_ROW_SEED:
            if (r->fixedSeed)
                LabAppendNumber(buf, r->seed, 5);
            else
                StringAppend(buf, sText_None);
            break;
        }
        if (changed)
            StringAppend(buf, COMPOUND_STRING("!"));
        AddTextPrinterParameterized3(WIN_MENU, FONT_NARROW, 50, (row - first) * 12, sTextColorNormal, TEXT_SKIP_DRAW, buf);
    }
    CopyWindowToVram(WIN_MENU, COPYWIN_GFX);
    buf[0] = EOS;
    if (sLabRulesRow == RULES_ROW_SEED)
    {
        StringAppend(buf, COMPOUND_STRING("{LEFT_ARROW}{RIGHT_ARROW} +-1\nL/R +-100\n"));
        if (sLabHasLastSeed)
        {
            StringAppend(buf, COMPOUND_STRING("Last: "));
            LabAppendNumber(buf, sLabLastSeed, 5);
            StringAppend(buf, COMPOUND_STRING("\nA: use it"));
        }
        StringAppend(buf, COMPOUND_STRING("\nB: back"));
    }
    else
    {
        StringAppend(buf, COMPOUND_STRING("{LEFT_ARROW}{RIGHT_ARROW} change\n! = changed\n"));
        if (sLabHasLastSeed)
        {
            StringAppend(buf, COMPOUND_STRING("Last seed\n"));
            LabAppendNumber(buf, sLabLastSeed, 5);
            StringAppend(buf, gText_NewLine);
        }
        StringAppend(buf, COMPOUND_STRING("B: back"));
    }
    BattleLab_PrintInfo(buf);
    BattleLab_DrawTitle();
}

static void BattleLab_HandleRulesInput(void)
{
    struct LabRules *r = &gBattleLabRules;
    s32 delta = 0;

    if (JOY_REPEAT(DPAD_UP))
        sLabRulesRow = (sLabRulesRow + RULES_ROW_COUNT - 1) % RULES_ROW_COUNT;
    else if (JOY_REPEAT(DPAD_DOWN))
        sLabRulesRow = (sLabRulesRow + 1) % RULES_ROW_COUNT;
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        BattleLab_ShowMainMenu();
        return;
    }
    else if (JOY_REPEAT(DPAD_RIGHT))
        delta = 1;
    else if (JOY_REPEAT(DPAD_LEFT))
        delta = -1;
    else if (JOY_NEW(A_BUTTON) && sLabRulesRow == RULES_ROW_SEED)
    {
        PlaySE(BattleLab_UseLastSeed() ? SE_SELECT : SE_FAILURE);
        BattleLab_DrawRules();
        return;
    }
    else if (JOY_REPEAT(R_BUTTON) && sLabRulesRow == RULES_ROW_SEED)
        delta = 100;
    else if (JOY_REPEAT(L_BUTTON) && sLabRulesRow == RULES_ROW_SEED)
        delta = -100;
    else
        return;

    switch (sLabRulesRow)
    {
    case RULES_ROW_FORMAT:    r->format  = (r->format  + LAB_FORMAT_COUNT  + delta) % LAB_FORMAT_COUNT;  break;
    case RULES_ROW_COMBAT:    r->combat  = (r->combat  + LAB_COMBAT_COUNT  + delta) % LAB_COMBAT_COUNT;  break;
    case RULES_ROW_ENERGY:    r->energyScale = (r->energyScale + LAB_ENERGY_COUNT + delta) % LAB_ENERGY_COUNT; break;
    case RULES_ROW_DAMAGE:    r->damageScale = (r->damageScale + LAB_DAMAGE_COUNT + delta) % LAB_DAMAGE_COUNT; break;
    case RULES_ROW_COST:      r->costScale   = (r->costScale   + LAB_COST_COUNT   + delta) % LAB_COST_COUNT;   break;
    case RULES_ROW_MOVES:     if (delta != 0) r->talentMoves = !r->talentMoves; break;
    case RULES_ROW_WEATHER:   r->weather = (r->weather + LAB_WEATHER_COUNT + delta) % LAB_WEATHER_COUNT; break;
    case RULES_ROW_TERRAIN:   r->terrain = (r->terrain + LAB_TERRAIN_COUNT + delta) % LAB_TERRAIN_COUNT; break;
    case RULES_ROW_AI:        r->ai      = (r->ai      + LAB_AI_COUNT      + delta) % LAB_AI_COUNT;      break;
    case RULES_ROW_SEED_MODE: if (delta != 0) r->fixedSeed = !r->fixedSeed; break;
    case RULES_ROW_SEED:      if (r->fixedSeed) r->seed += delta; break;   // u16 wraps 0..65535
    }
    BattleLab_DrawRules();
}

static void Task_BattleLab_Input(u8 taskId)
{
    s8 choice;

    if (gPaletteFade.active)
        return;

    switch (sLabMode)
    {
    case LAB_MODE_TEAM:
        BattleLab_HandleTeamInput();
        return;
    case LAB_MODE_EDIT:
        BattleLab_HandleEditInput();
        return;
    case LAB_MODE_LETTER:
        BattleLab_HandleLetterInput();
        return;
    case LAB_MODE_RULES:
        BattleLab_HandleRulesInput();
        return;
    case LAB_MODE_SCENARIO:
        BattleLab_HandleScenarioInput();
        return;
    case LAB_MODE_CONFIRM_RESET:
        if (JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            BattleLab_ResetTeams();
            BattleLab_OpenTeam(sLabSide);
            BattleLab_DrawTitle();
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            sLabMode = LAB_MODE_TEAM;
            BattleLab_PrintInfo(sText_TeamHelp);
        }
        return;
    }

    choice = Menu_ProcessInput();
    if (choice == MENU_NOTHING_CHOSEN)
        return;
    if (choice == MENU_B_PRESSED)
        choice = LAB_MENU_EXIT;
    sLabCursor = choice;

    switch (choice)
    {
    case LAB_MENU_PLAYER_TEAM:
        BattleLab_OpenTeam(LAB_SIDE_PLAYER);
        break;
    case LAB_MENU_OPPONENT_TEAM:
        BattleLab_OpenTeam(LAB_SIDE_OPPONENT);
        break;
    case LAB_MENU_RULES:
        sLabMode = LAB_MODE_RULES;
        sLabRulesRow = 0;
        BattleLab_DrawRules();
        break;
    case LAB_MENU_SCENARIO:
        sLabMode = LAB_MODE_SCENARIO;
        sLabScenarioCursor = sLabScenarioIndex;
        BattleLab_DrawScenarios();
        break;
    case LAB_MENU_START:
    {
        const u8 *problem = BattleLab_ValidateStart();
        if (problem != NULL)
        {
            PlaySE(SE_FAILURE);
            BattleLab_PrintInfo(problem);
            break;
        }
    }
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_BattleLab_StartBattle;
        break;
    case LAB_MENU_EXIT:
    default:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_BattleLab_Exit;
        break;
    }
}


// Per-battle state of the stamina rules (timeline, energy bars, messages), for Lab and campaign battles.
static void LabResetBattleState(void)
{
    sLabEndRequest = LAB_END_NONE;
    sLabProtectedThisTurn = 0;
    sLabAiTicksTotal = 0;
    sLabAiTicksMax = 0;
    sLabAiDecisions = 0;
    sLabNow = 0;
    sLabLastBeat = -1;
    sLabBeat = FALSE;
    memset(sLabPopupSprite, 0xFF, sizeof(sLabPopupSprite));
    memset(sLabEnBarSprite, 0xFF, sizeof(sLabEnBarSprite));
    sLabTintedMask = 0;
    sLabMessage[0] = EOS;
    sLabWasExhausted = 0;
    sLabSapped = 0;
    sLabBenchRegenAt = 0;
    sLabRegenAt = 0;
    memset(sLabRegenCarry, 0, sizeof(sLabRegenCarry));
    sLabRoundPlanned = FALSE;
    sLabTimelineStarted = FALSE;
    memset(sLabBoost, 0, sizeof(sLabBoost));
}

// F019: decided at battle init (healthbox art depends on it). Link, Frontier, Safari and other
// special battles stay classic.
void BattleLab_CampaignBattleInit(void)
{
    sAscCampaignBattle = FALSE;
    if (sLabInBattleFlag)
        return;
    sActiveCombat = LAB_COMBAT_CLASSIC;
    if (!ASCENSION_COMBAT)
        return;
    // (Recorded Battle is cut, D011; the test runner's battles carry the recorded flags.)
    if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_SAFARI | BATTLE_TYPE_FRONTIER
                          | BATTLE_TYPE_EREADER_TRAINER | BATTLE_TYPE_CATCH_TUTORIAL | BATTLE_TYPE_POKEDUDE | BATTLE_TYPE_SECRET_BASE))
        return;
    memset(&sCampaignRules, 0, sizeof(sCampaignRules));
    sCampaignRules.combat = LAB_COMBAT_DUAL;
    sCampaignRules.energyScale = 0;                                           // x1
    sCampaignRules.damageScale = (gBattleTypeFlags & BATTLE_TYPE_TRAINER) ? 2 : 1;   // 50% trainers, 75% wild
    sCampaignRules.costScale = 0;                                             // 100%
    LabResetBattleState();
    sAscCampaignBattle = TRUE;
    sActiveCombat = sCampaignRules.combat;
}

// Energy share (percent) a Pokémon can hold at its current health (D024: lost health caps energy).
static u32 LabCapPercent(struct Pokemon *mon)
{
    u32 maxHp = GetMonData(mon, MON_DATA_MAX_HP);
    return maxHp == 0 ? 100 : 50 + 50 * GetMonData(mon, MON_DATA_HP) / maxHp;
}

// F019: your party starts from the energy it kept since its last battle (MON_DATA_ASC_FATIGUE,
// capped by health); opponents start full.
// The battle screen was rebuilt (back from the party menu or bag): every sprite was reset, so the
// energy bars' sprite ids are stale. Forget them and make new bars.
void BattleLab_OnBattleScreenReshown(void)
{
    memset(sLabEnBarSprite, 0xFF, sizeof(sLabEnBarSprite));
    sLabTintedMask = 0;
    LabEnsureEnergyBars();
}

void BattleLab_CampaignBattleStart(void)
{
    u32 i;
    if (!sAscCampaignBattle)
        return;
    BattleLab_ScaleStartingEnergy();
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u32 share, max = sLabEnergyMax[0][i];
        if (max == 0 || GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
            continue;
        share = 100 - min(GetMonData(mon, MON_DATA_ASC_FATIGUE), 100);
        share = min(share, LabCapPercent(mon));
        sLabEnergy[0][i] = max * share / 100;
    }
}

// F019: at the end of a campaign battle your party keeps the energy it has left. A fainted
// Pokémon is saved at 20%, so it wakes still Exhausted when revived (D023).
#define ASC_FAINTED_ENERGY_PERCENT 20
static void LabSaveCampaignEnergy(void)
{
    u32 i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u32 share, fatigue, max = sLabEnergyMax[0][i];
        if (max == 0 || GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        if (GetMonData(mon, MON_DATA_HP) == 0)
            share = ASC_FAINTED_ENERGY_PERCENT;
        else
            share = min((sLabEnergy[0][i] * 100 + max / 2) / max, 100);
        fatigue = 100 - share;
        SetMonData(mon, MON_DATA_ASC_FATIGUE, &fatigue);
    }
}

// F019: outside battle, energy comes back while walking (1% every 16 steps, up to the health cap;
// not for fainted Pokémon) and fully with any full heal (Pokémon Center, bed, HealPlayerParty).
#define ASC_WALK_STEPS_PER_PERCENT 16
void BattleLab_OnPlayerStep(void)
{
    static EWRAM_DATA u8 sSteps = 0;
    u32 i;
    if (!ASCENSION_COMBAT || ++sSteps < ASC_WALK_STEPS_PER_PERCENT)
        return;
    sSteps = 0;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u32 fatigue;
        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG) || GetMonData(mon, MON_DATA_HP) == 0)
            continue;
        fatigue = GetMonData(mon, MON_DATA_ASC_FATIGUE);
        if (fatigue > 0 && 100 - fatigue < LabCapPercent(mon))
        {
            fatigue--;
            SetMonData(mon, MON_DATA_ASC_FATIGUE, &fatigue);
        }
    }
}

// F019: PP items restore energy instead (PP no longer limits moves): Ether 25%, Max Ether 50%,
// Elixir 50%, Max Elixir 100% of max energy, up to the health cap. Proposed values, tuned later.
u32 BattleLab_EnergyItemPercent(u32 item)
{
    switch (item)
    {
    case ITEM_ETHER:      return 25;
    case ITEM_MAX_ETHER:  return 50;
    case ITEM_ELIXIR:     return 50;
    case ITEM_MAX_ELIXIR: return 100;
    default:              return 0;
    }
}

// TRUE while PP items work on energy: in a Lab or campaign battle with energy, or (outside battle)
// with the overhaul on.
bool32 BattleLab_EnergyItemsActive(void)
{
    return gMain.inBattle ? BattleLab_DualMode() : ASCENSION_COMBAT;
}

// The battle's party slot for a player party Pokémon, or PARTY_SIZE.
static u32 LabPlayerPartySlot(struct Pokemon *mon)
{
    u32 i;
    for (i = 0; i < PARTY_SIZE; i++)
        if (mon == &gParties[B_TRAINER_PLAYER][i])
            return i;
    return PARTY_SIZE;
}

// Restores energy to a player party Pokémon; FALSE (and nothing changes) when it would do nothing.
bool32 BattleLab_UseEnergyItem(struct Pokemon *mon, u32 item, bool32 apply)
{
    u32 percent = BattleLab_EnergyItemPercent(item), cap = LabCapPercent(mon);
    if (percent == 0 || GetMonData(mon, MON_DATA_HP) == 0 || GetMonData(mon, MON_DATA_IS_EGG))
        return FALSE;
    if (gMain.inBattle && BattleLab_DualMode())
    {
        u32 slot = LabPlayerPartySlot(mon), max, capEnergy, now, after, battler;
        if (slot >= PARTY_SIZE || sLabEnergyMax[0][slot] == 0)
            return FALSE;
        max = sLabEnergyMax[0][slot];
        capEnergy = max * cap / 100;
        now = sLabEnergy[0][slot];
        if (now >= capEnergy)
            return FALSE;
        after = min(now + max * percent / 100, capEnergy);
        if (!apply)
            return TRUE;
        sLabEnergy[0][slot] = after;
        for (battler = 0; battler < gBattlersCount; battler++)
            if (IsOnPlayerSide(battler) && gBattlerPartyIndexes[battler] == slot)
                LabDrawEnergyBarValue(battler, after);
        return TRUE;
    }
    else
    {
        u32 fatigue = GetMonData(mon, MON_DATA_ASC_FATIGUE), share = 100 - fatigue;
        if (share >= cap)
            return FALSE;
        if (apply)
        {
            share = min(share + percent, cap);
            fatigue = 100 - share;
            SetMonData(mon, MON_DATA_ASC_FATIGUE, &fatigue);
        }
        return TRUE;
    }
}

void BattleLab_RestoreAllEnergy(struct Pokemon *mon)
{
    u32 zero = 0;
    SetMonData(mon, MON_DATA_ASC_FATIGUE, &zero);
}

static void Task_BattleLab_StartBattle(u8 taskId)
{
    const struct Trainer *opponent = &sLabTrainers[DIFFICULTY_NORMAL][LAB_TRAINER_001_OPPONENT];
    bool32 wild = (gBattleLabRules.format == LAB_FORMAT_WILD_SINGLE);

    if (gPaletteFade.active)
        return;

    // A fixed seed covers building the parties too (personality: gender, etc.), so the
    // whole run replays, not just the battle.
    // Every Lab battle runs from a known seed (F012): the fixed one, or a fresh one that is
    // remembered so the battle can be replayed ("Use last seed" in Rules).
    sLabLastSeed = gBattleLabRules.fixedSeed ? gBattleLabRules.seed : (Random() & 0xFFFF);
    sLabHasLastSeed = TRUE;
    SeedRng(sLabLastSeed);
    BattleLab_LogBattleStart();
    BattleLab_SetupScenarioParties();
    BattleLab_ScaleStartingEnergy();
    sLabInBattleFlag = TRUE;
    sActiveCombat = gBattleLabRules.combat;
    LabResetBattleState();
    gIsDebugBattle = TRUE;   // upstream then keeps the Lab's prebuilt enemy party (battle_main.c)
    gDebugAIFlags = BattleLab_OpponentAiFlags();
    if (wild)
    {
        gDebugAiTrainerOverride = NULL;
        gBattleTypeFlags = 0;
        BattleLab_TrimToWildOpponent();
        PlayBGM(MUS_VS_WILD);
    }
    else
    {
        gDebugAiTrainerOverride = opponent;   // GetTrainerStructFromId() returns this during debug battles
        gBattleTypeFlags = BATTLE_TYPE_TRAINER;
        if (gBattleLabRules.format == LAB_FORMAT_TRAINER_DOUBLE)
            gBattleTypeFlags |= BATTLE_TYPE_DOUBLE;
        TRAINER_BATTLE_PARAM.opponentA = 1;   // any real trainer id; the debug override supplies the data
        TRAINER_BATTLE_PARAM.opponentB = 0xFFFF;
        PlayBGM(MUS_VS_TRAINER);
    }
    BattleLab_ApplyStartingStatuses();
    gBattleEnvironment = BATTLE_ENVIRONMENT_BUILDING;
    DestroyTask(taskId);
    FreeAllWindowBuffers();
    gMain.savedCallback = CB2_BattleLab_ReturnFromBattle;
    SetMainCallback2(CB2_InitBattle);
}

static void CB2_BattleLab_ReturnFromBattle(void)
{
    gIsDebugBattle = FALSE;
    gDebugAiTrainerOverride = NULL;
    sLabInBattleFlag = FALSE;
    sActiveCombat = LAB_COMBAT_CLASSIC;
    memset(&gStartingStatuses, 0, sizeof(gStartingStatuses));
    sLabLastResult = BattleLab_ResultFromOutcome(gBattleOutcome, sLabEndRequest == LAB_END_QUIT, sLabEndRequest == LAB_END_RESTART);
    sLabLastTurns = BattleLab_TurnsPlayed(gBattleResults.battleTurnCounter);
    FadeOutBGM(4);
    // Throw away whatever the battle changed (XP, items, money): nothing is saved,
    // and the next battle rebuilds both parties from the scenario data.
    ZeroPlayerPartyMons();
    ZeroEnemyPartyMons();
    gMain.state = 1;
    SetMainCallback2(CB2_BattleLab_Init);
}

static void Task_BattleLab_Exit(u8 taskId)
{
    if (gPaletteFade.active)
        return;
    DestroyTask(taskId);
    // Reload everything, including the campaign save from flash; the Lab never saved.
    DoSoftReset();
}

#else

void BattleLab_Open(void) {}
void BattleLab_SetupScenarioParties(void) {}

#endif // ASCENSION_BATTLE_LAB
