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
    [WIN_MENU]  = {.bg = 0, .tilemapLeft = 1,  .tilemapTop = 5, .width = 13, .height = 12, .paletteNum = TEXT_PAL, .baseBlock = 57},
    [WIN_INFO]  = {.bg = 0, .tilemapLeft = 17, .tilemapTop = 5, .width = 12, .height = 12, .paletteNum = TEXT_PAL, .baseBlock = 213},
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

static const u8 sText_Title[] = _("BATTLE LAB · LAB-001 baseline 1v1");
static const u8 sText_Scenario[] = _("LAB-001\nSwampert Lv50\nvs Blaziken Lv50\n\nA: choose\nB: exit");
static const u8 sText_Team[] = _("Team builder\narrives in\nF008.");
static const u8 sText_Rules[] = _("Conditions, AI\nand items\narrive in F009.");
static const u8 sText_ScenarioSoon[] = _("Saved scenarios\narrive in F011.\nNow: LAB-001.");

static const u16 sLabBackdrop = RGB(3, 5, 9);

static EWRAM_DATA u8 sLabCursor = 0;
static EWRAM_DATA bool8 sLabNeedsFreshGame = FALSE;

static void CB2_BattleLab_Init(void);
static void CB2_BattleLab_Main(void);
static void CB2_BattleLab_ReturnFromBattle(void);
static void VBlankCB_BattleLab(void);
static void Task_BattleLab_Input(u8 taskId);
static void Task_BattleLab_StartBattle(u8 taskId);
static void Task_BattleLab_Exit(u8 taskId);
static void BattleLab_PrintInfo(const u8 *text);

void BattleLab_Open(void)
{
    sLabCursor = LAB_MENU_START;
    sLabNeedsFreshGame = TRUE;
    FadeOutBGM(4);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    SetMainCallback2(CB2_BattleLab_Init);
    gMain.state = 0;
}

void BattleLab_SetupScenarioParties(void)
{
    ZeroPlayerPartyMons();
    ZeroEnemyPartyMons();
    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_PLAYER], &sLabTrainers[DIFFICULTY_NORMAL][LAB_TRAINER_001_PLAYER]);
    CreateNPCTrainerPartyFromTrainer(gParties[B_TRAINER_OPPONENT_A], &sLabTrainers[DIFFICULTY_NORMAL][LAB_TRAINER_001_OPPONENT]);
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
        AddTextPrinterParameterized(WIN_TITLE, FONT_NORMAL, sText_Title, 0, 1, TEXT_SKIP_DRAW, NULL);
        PrintMenuTable(WIN_MENU, LAB_MENU_COUNT, sLabMenuItems);
        InitMenuInUpperLeftCornerNormal(WIN_MENU, LAB_MENU_COUNT, sLabCursor);
        BattleLab_PrintInfo(sText_Scenario);
        for (i = 0; i <= WIN_INFO; i++)
            CopyWindowToVram(i, COPYWIN_FULL);
        gMain.state++;
        break;
    }
    case 4:
        ShowBg(0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
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

static void Task_BattleLab_Input(u8 taskId)
{
    s8 choice;

    if (gPaletteFade.active)
        return;

    choice = Menu_ProcessInput();
    if (choice == MENU_NOTHING_CHOSEN)
        return;
    if (choice == MENU_B_PRESSED)
        choice = LAB_MENU_EXIT;
    sLabCursor = choice;

    switch (choice)
    {
    case LAB_MENU_PLAYER_TEAM:
    case LAB_MENU_OPPONENT_TEAM:
        BattleLab_PrintInfo(sText_Team);
        break;
    case LAB_MENU_RULES:
        BattleLab_PrintInfo(sText_Rules);
        break;
    case LAB_MENU_SCENARIO:
        BattleLab_PrintInfo(sText_ScenarioSoon);
        break;
    case LAB_MENU_START:
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
