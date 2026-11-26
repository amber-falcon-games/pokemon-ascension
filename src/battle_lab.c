// Pokémon Ascension Battle Lab (F007).
// A developer-only screen, opened with L + SELECT on the title screen, that starts
// real battles from scenario data (src/data/battle_lab_trainers.party) and returns
// to itself afterwards. It never writes the save: Exit soft-resets, which reloads
// the campaign save from flash. Compiled out of release builds (ASCENSION_BATTLE_LAB).
#include "global.h"
#include "config/ascension.h"
#include "battle_lab.h"

#if ASCENSION_BATTLE_LAB

#include "battle.h"
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
#include "constants/battle_ai.h"
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
    [WIN_MENU]  = {.bg = 0, .tilemapLeft = 1,  .tilemapTop = 5, .width = 15, .height = 12, .paletteNum = TEXT_PAL, .baseBlock = 57},
    [WIN_INFO]  = {.bg = 0, .tilemapLeft = 19, .tilemapTop = 5, .width = 10, .height = 12, .paletteNum = TEXT_PAL, .baseBlock = 237},
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

static const u8 sText_Rules[] = _("Conditions,\nAI, items:\nF009.");
static const u8 sText_ScenarioSoon[] = _("Saved\nscenarios:\nF011.\nNow: LAB-001");
static const u8 sText_TeamHelp[] = _("A: edit\nB: back\nSTART:\nreset all\n! = edited");
static const u8 sText_ConfirmReset[] = _("Reset both\nteams to\nLAB-001?\n\nA: yes\nB: no");
static const u8 sText_PlayerEmpty[] = _("Your team\nhas nobody\nin battle.");
static const u8 sText_OpponentEmpty[] = _("Opponent\nhas nobody\nin battle.");

static const u16 sLabBackdrop = RGB(3, 5, 9);

static EWRAM_DATA u8 sLabCursor = 0;
static EWRAM_DATA bool8 sLabNeedsFreshGame = FALSE;
EWRAM_DATA struct LabSlot gBattleLabTeams[LAB_SIDE_COUNT][PARTY_SIZE] = {0};

// Team screen state (task-independent so it survives returning from a battle)
enum { LAB_MODE_MAIN, LAB_MODE_TEAM, LAB_MODE_EDIT, LAB_MODE_LETTER, LAB_MODE_CONFIRM_RESET };
enum { EDIT_ROW_SPECIES, EDIT_ROW_LEVEL, EDIT_ROW_ACTIVE, EDIT_ROW_RESET, EDIT_ROW_CLEAR, EDIT_ROW_COUNT };
static EWRAM_DATA u8 sLabMode = 0;
static EWRAM_DATA u8 sLabSide = 0;
static EWRAM_DATA u8 sLabSlot = 0;
static EWRAM_DATA u8 sLabEditRow = 0;
static EWRAM_DATA u8 sLabLetter = 0;

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
static void BattleLab_HandleTeamInput(void);
static void BattleLab_HandleEditInput(void);
static void BattleLab_HandleLetterInput(void);

void BattleLab_Open(void)
{
    sLabCursor = LAB_MENU_START;
    sLabNeedsFreshGame = TRUE;
    sLabMode = LAB_MODE_MAIN;
    BattleLab_ResetTeams();
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

void BattleLab_ResetSlot(u32 side, u32 slot)
{
    const struct Trainer *trainer = BattleLab_ScenarioTrainer(side);
    struct LabSlot *s = &gBattleLabTeams[side][slot];

    if (slot < trainer->partySize)
    {
        s->species = trainer->party[slot].species;
        s->level = trainer->party[slot].lvl;
        s->flags = LAB_SLOT_ACTIVE;
    }
    else
    {
        s->species = SPECIES_NONE;
        s->level = LAB_DEFAULT_LEVEL;
        s->flags = 0;
    }
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
    s->species = SPECIES_NONE;
    s->level = LAB_DEFAULT_LEVEL;
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

// TRUE if any slot differs from the scenario (species, level, or in-battle setting).
bool32 BattleLab_IsEdited(void)
{
    u32 side, slot;
    for (side = 0; side < LAB_SIDE_COUNT; side++)
    {
        const struct Trainer *trainer = BattleLab_ScenarioTrainer(side);
        for (slot = 0; slot < PARTY_SIZE; slot++)
        {
            const struct LabSlot *s = &gBattleLabTeams[side][slot];
            bool32 scenarioHasMon = slot < trainer->partySize;
            if (s->flags & LAB_SLOT_MODIFIED)
                return TRUE;
            if (scenarioHasMon != LabSlotInBattle(s))
                return TRUE;
        }
    }
    return FALSE;
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
            CreateRandomMonWithIVs(&party[out], s->species, s->level, MAX_PER_STAT_IVS);
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
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB_BattleLab);
        CreateTask(Task_BattleLab_Input, 0);
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
    FillWindowPixelBuffer(WIN_TITLE, PIXEL_FILL(1));
    AddTextPrinterParameterized(WIN_TITLE, FONT_NORMAL,
        BattleLab_IsEdited() ? COMPOUND_STRING("BATTLE LAB · LAB-001 (edited)")
                             : COMPOUND_STRING("BATTLE LAB · LAB-001 baseline 1v1"),
        0, 1, TEXT_SKIP_DRAW, NULL);
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
            if (s->flags & LAB_SLOT_MODIFIED)
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
static void BattleLab_DrawEditor(void)
{
    static const u8 rowY[EDIT_ROW_COUNT] = {1, 33, 49, 65, 81};
    const struct LabSlot *s = &gBattleLabTeams[sLabSide][sLabSlot];
    u8 *buf = gStringVar4;
    u32 row;

    FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(1));
    for (row = 0; row < EDIT_ROW_COUNT; row++)
    {
        buf[0] = (row == sLabEditRow) ? CHAR_RIGHT_ARROW : CHAR_SPACE;
        buf[1] = EOS;
        switch (row)
        {
        case EDIT_ROW_SPECIES:
            StringAppend(buf, s->species == SPECIES_NONE ? COMPOUND_STRING("- - -") : GetSpeciesName(s->species));
            break;
        case EDIT_ROW_LEVEL:
            StringAppend(buf, COMPOUND_STRING("Lv "));
            LabAppendNumber(buf, s->level, 3);
            break;
        case EDIT_ROW_ACTIVE:
            StringAppend(buf, (s->flags & LAB_SLOT_ACTIVE) ? COMPOUND_STRING("Battle: Yes") : COMPOUND_STRING("Battle: No"));
            break;
        case EDIT_ROW_RESET:
            StringAppend(buf, COMPOUND_STRING("Reset slot"));
            break;
        case EDIT_ROW_CLEAR:
            StringAppend(buf, COMPOUND_STRING("Clear slot"));
            break;
        }
        AddTextPrinterParameterized3(WIN_INFO, FONT_NARROW, 0, rowY[row], sTextColorNormal, TEXT_SKIP_DRAW, buf);
    }
    // Dex number (and form) under the species name.
    if (s->species != SPECIES_NONE)
    {
        u32 form = LabFormNumber(s->species);
        buf[0] = EOS;
        StringAppend(buf, COMPOUND_STRING(" No."));
        ConvertIntToDecimalStringN(gStringVar1, gSpeciesInfo[s->species].natDexNum, STR_CONV_MODE_LEADING_ZEROS, 4);
        StringAppend(buf, gStringVar1);
        if (form > 1)
        {
            StringAppend(buf, COMPOUND_STRING(" form "));
            LabAppendNumber(buf, form, 2);
        }
        AddTextPrinterParameterized3(WIN_INFO, FONT_SMALL, 0, 17, sTextColorDim, TEXT_SKIP_DRAW, buf);
    }
    CopyWindowToVram(WIN_INFO, COPYWIN_GFX);
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
    BattleLab_DrawTeam();
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

static void BattleLab_HandleEditInput(void)
{
    struct LabSlot *s = &gBattleLabTeams[sLabSide][sLabSlot];
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
        delta = (sLabEditRow == EDIT_ROW_SPECIES) ? 25 : 10;
    else if (JOY_REPEAT(L_BUTTON))
        delta = (sLabEditRow == EDIT_ROW_SPECIES) ? -25 : -10;

    switch (sLabEditRow)
    {
    case EDIT_ROW_SPECIES:
        if (delta != 0)
        {
            BattleLab_SetSlotSpecies(sLabSide, sLabSlot, BattleLab_StepSpecies(s->species, delta));
            BattleLab_RefreshTeamAndEditor();
        }
        else if (JOY_NEW(SELECT_BUTTON) || JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            sLabMode = LAB_MODE_LETTER;
            sLabLetter = (s->species != SPECIES_NONE) ? GetSpeciesName(s->species)[0] - CHAR_A : 0;
            if (sLabLetter >= 26)
                sLabLetter = 0;
            BattleLab_DrawLetters();
        }
        break;
    case EDIT_ROW_LEVEL:
        if (delta != 0 && s->species != SPECIES_NONE)
        {
            BattleLab_SetSlotLevel(sLabSide, sLabSlot, (s32)s->level + delta);
            BattleLab_RefreshTeamAndEditor();
        }
        break;
    case EDIT_ROW_ACTIVE:
        if ((delta != 0 || JOY_NEW(A_BUTTON)) && s->species != SPECIES_NONE)
        {
            PlaySE(SE_SELECT);
            BattleLab_SetSlotActive(sLabSide, sLabSlot, !(s->flags & LAB_SLOT_ACTIVE));
            BattleLab_RefreshTeamAndEditor();
        }
        break;
    case EDIT_ROW_RESET:
        if (JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            BattleLab_ResetSlot(sLabSide, sLabSlot);
            BattleLab_RefreshTeamAndEditor();
        }
        break;
    case EDIT_ROW_CLEAR:
        if (JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            BattleLab_ClearSlot(sLabSide, sLabSlot);
            BattleLab_RefreshTeamAndEditor();
        }
        break;
    }
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
        u32 species = BattleLab_FirstSpeciesForLetter(sLabLetter);
        PlaySE(SE_SELECT);
        if (species != SPECIES_NONE)
            BattleLab_SetSlotSpecies(sLabSide, sLabSlot, species);
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
        BattleLab_PrintInfo(sText_Rules);
        break;
    case LAB_MENU_SCENARIO:
        BattleLab_PrintInfo(sText_ScenarioSoon);
        break;
    case LAB_MENU_START:
        if (BattleLab_ActiveCount(LAB_SIDE_PLAYER) == 0)
        {
            PlaySE(SE_FAILURE);
            BattleLab_PrintInfo(sText_PlayerEmpty);
            break;
        }
        if (BattleLab_ActiveCount(LAB_SIDE_OPPONENT) == 0)
        {
            PlaySE(SE_FAILURE);
            BattleLab_PrintInfo(sText_OpponentEmpty);
            break;
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

static void Task_BattleLab_StartBattle(u8 taskId)
{
    const struct Trainer *opponent = &sLabTrainers[DIFFICULTY_NORMAL][LAB_TRAINER_001_OPPONENT];

    if (gPaletteFade.active)
        return;

    BattleLab_SetupScenarioParties();
    gDebugAiTrainerOverride = opponent;   // GetTrainerStructFromId() returns this during debug battles
    gIsDebugBattle = TRUE;
    gDebugAIFlags = opponent->aiFlags;
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    TRAINER_BATTLE_PARAM.opponentA = 1;   // any real trainer id; the debug override supplies the data
    TRAINER_BATTLE_PARAM.opponentB = 0xFFFF;
    gBattleEnvironment = BATTLE_ENVIRONMENT_BUILDING;
    CalculateEnemyPartyCount();
    DestroyTask(taskId);
    FreeAllWindowBuffers();
    PlayBGM(MUS_VS_TRAINER);
    gMain.savedCallback = CB2_BattleLab_ReturnFromBattle;
    SetMainCallback2(CB2_InitBattle);
}

static void CB2_BattleLab_ReturnFromBattle(void)
{
    gIsDebugBattle = FALSE;
    gDebugAiTrainerOverride = NULL;
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
