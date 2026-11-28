#include "global.h"
#include "test/test.h"
#include "ascension_moves.h"
#include "move.h"
#include "pokemon.h"
#include "string_util.h"
#include "constants/moves.h"
#include "constants/species.h"

// Pokémon Ascension F014: battle move list slots 4+ (until F015's move library).

TEST("Move list: the battle list holds more than four distinct moves")
{
    struct Pokemon mon;
    enum Move moves[MAX_BATTLE_MOVES] = {MOVE_THUNDERBOLT, MOVE_QUICK_ATTACK, MOVE_NONE, MOVE_NONE};
    u8 pp[MAX_BATTLE_MOVES] = {0};
    u32 count, i, j;

    CreateMon(&mon, SPECIES_PIKACHU, 50, 0, OTID_STRUCT_PRESET(0));
    gAscensionMoveListTestOverride = TRUE;
    moves[2] = MOVE_NONE; moves[3] = MOVE_NONE;
    count = Ascension_FillExtraBattleMoves(&mon, moves, pp);
    gAscensionMoveListTestOverride = FALSE;

    EXPECT_GT(count, MAX_MON_MOVES);
    EXPECT_LE(count, MAX_BATTLE_MOVES);
    for (i = MAX_MON_MOVES; i < count; i++)
    {
        EXPECT_NE(moves[i], MOVE_NONE);
        EXPECT_EQ(pp[i], GetMovePP(moves[i]));
        for (j = 0; j < i; j++)
            EXPECT_NE(moves[i], moves[j]);   // no duplicates
    }
}

TEST("Move list: off means slots 5-8 stay empty")
{
    struct Pokemon mon;
    enum Move moves[MAX_BATTLE_MOVES] = {MOVE_TACKLE};
    u8 pp[MAX_BATTLE_MOVES] = {0};
    u32 i;

    CreateMon(&mon, SPECIES_PIKACHU, 50, 0, OTID_STRUCT_PRESET(0));
    for (i = MAX_MON_MOVES; i < MAX_BATTLE_MOVES; i++)
        moves[i] = MOVE_SURF;   // garbage that must be cleared
    EXPECT_EQ(Ascension_FillExtraBattleMoves(&mon, moves, pp), MAX_MON_MOVES);
    for (i = MAX_MON_MOVES; i < MAX_BATTLE_MOVES; i++)
        EXPECT_EQ(moves[i], MOVE_NONE);
}

TEST("Talent tree: a branch's move grows with level (Pikachu's electric special chain)")
{
    u32 b, found = FALSE;
    for (b = 0; b < Ascension_TalentBranchCount(SPECIES_PIKACHU); b++)
    {
        if (Ascension_TalentBranchMove(SPECIES_PIKACHU, b, 36, 0) == MOVE_THUNDERBOLT)
        {
            found = TRUE;
            EXPECT_EQ(Ascension_TalentBranchMove(SPECIES_PIKACHU, b, 35, 0), MOVE_DISCHARGE);
            EXPECT_EQ(Ascension_TalentBranchMove(SPECIES_PIKACHU, b, 1, 0), MOVE_THUNDER_SHOCK);
            EXPECT_EQ(Ascension_TalentBranchMove(SPECIES_PIKACHU, b, 0, 0), MOVE_NONE);
        }
    }
    EXPECT(found);
    EXPECT_LE(Ascension_TalentBranchCount(SPECIES_PIKACHU), MAX_BATTLE_MOVES);
}

TEST("Talent tree: the battle list comes from the tree and refreshes the saved four as a cache")
{
    struct Pokemon mon;
    enum Move moves[MAX_BATTLE_MOVES] = {MOVE_SPLASH, MOVE_SPLASH, MOVE_SPLASH, MOVE_SPLASH};
    u8 pp[MAX_BATTLE_MOVES] = {0};
    u32 count, i;

    CreateMon(&mon, SPECIES_PIKACHU, 50, 0, OTID_STRUCT_PRESET(0));
    gAscensionMoveListTestOverride = TRUE;
    count = Ascension_FillExtraBattleMoves(&mon, moves, pp);
    gAscensionMoveListTestOverride = FALSE;

    EXPECT_GT(count, MAX_MON_MOVES);
    for (i = 0; i < count; i++)
        EXPECT_NE(moves[i], MOVE_SPLASH);
    for (i = 0; i < MAX_MON_MOVES; i++)
        EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE1 + i), moves[i]);
}

TEST("Talent tree: a branch choice is saved with the Pokémon and picks that option")
{
    struct Pokemon mon;
    u32 b, branch = MAX_BATTLE_MOVES;
    CreateMon(&mon, SPECIES_PIKACHU, 50, 0, OTID_STRUCT_PRESET(0));
    // Pikachu's first status branch starts with a 3-way choice (Play Nice / Tail Whip / Growl).
    for (b = 0; b < Ascension_TalentBranchCount(SPECIES_PIKACHU); b++)
        if (Ascension_TalentBranchMove(SPECIES_PIKACHU, b, 1, 0) != Ascension_TalentBranchMove(SPECIES_PIKACHU, b, 1, 1))
            branch = b;
    EXPECT_LT(branch, MAX_BATTLE_MOVES);
    EXPECT_EQ(Ascension_GetTalentChoice(&mon, branch), 0);
    Ascension_SetTalentChoice(&mon, branch, 2);
    EXPECT_EQ(Ascension_GetTalentChoice(&mon, branch), 2);
    for (b = 0; b < MAX_BATTLE_MOVES; b++)   // other branches untouched
        if (b != branch)
            EXPECT_EQ(Ascension_GetTalentChoice(&mon, b), 0);
    Ascension_SetTalentChoice(&mon, 7, 3);   // the top bits survive the split across substructures
    EXPECT_EQ(Ascension_GetTalentChoice(&mon, 7), 3);
    EXPECT_EQ(Ascension_GetTalentChoice(&mon, branch), 2);
}

static bool32 ListHas(struct Pokemon *mon, u16 move)
{
    u16 list[MAX_BATTLE_MOVES];
    u32 i, n = Ascension_BuildTalentList(mon, list, NULL);
    for (i = 0; i < n; i++)
        if (list[i] == move)
            return TRUE;
    return FALSE;
}

TEST("Teaching (D027): a move ahead in its own tree jumps the branch there")
{
    struct Pokemon mon;
    CreateMon(&mon, SPECIES_PIKACHU, 20, 0, OTID_STRUCT_PRESET(0));
    EXPECT(!ListHas(&mon, MOVE_THUNDERBOLT));
    EXPECT_EQ(Ascension_TeachMove(&mon, MOVE_THUNDERBOLT, 0), TALENT_TEACH_JUMPED);
    EXPECT(ListHas(&mon, MOVE_THUNDERBOLT));
    EXPECT(!ListHas(&mon, MOVE_ELECTRO_BALL));            // the branch moved on
    EXPECT_EQ(Ascension_TeachMove(&mon, MOVE_THUNDERBOLT, 0), TALENT_TEACH_ALREADY_KNOWN);
}

TEST("Teaching (D027): a move it has grown past is refused")
{
    struct Pokemon mon;
    CreateMon(&mon, SPECIES_PIKACHU, 40, 0, OTID_STRUCT_PRESET(0));
    EXPECT_EQ(Ascension_TeachMove(&mon, MOVE_DISCHARGE, 0), TALENT_TEACH_GROWN_PAST);
    EXPECT_EQ(Ascension_TeachMove(&mon, MOVE_THUNDERBOLT, 0), TALENT_TEACH_ALREADY_KNOWN);
}

TEST("Teaching (D027): a move outside its tree joins that move's global chain, then grows with level")
{
    struct Pokemon mon;
    u32 level = 10;
    CreateMon(&mon, SPECIES_PIKACHU, 10, 0, OTID_STRUCT_PRESET(0));
    EXPECT(!ListHas(&mon, MOVE_EMBER));
    EXPECT_EQ(Ascension_TeachMove(&mon, MOVE_EMBER, 0), TALENT_TEACH_JOINED);
    EXPECT(ListHas(&mon, MOVE_EMBER));
    level = 100;
    SetMonData(&mon, MON_DATA_LEVEL, &level);
    EXPECT(!ListHas(&mon, MOVE_EMBER));                   // grew along the fire chain
    EXPECT_EQ(Ascension_TeachMove(&mon, MOVE_SURF, 0), TALENT_TEACH_JOINED);
    EXPECT_EQ(Ascension_TeachMove(&mon, MOVE_ICE_BEAM, 0), TALENT_TEACH_NO_ROOM);   // two joins at most
}

TEST("Teaching (D027): with a full list, joining asks which move to replace, and replaces only that one")
{
    struct Pokemon mon;
    u16 before[MAX_BATTLE_MOVES], after[MAX_BATTLE_MOVES];
    u8 sources[MAX_BATTLE_MOVES];
    u32 i, n, replaced = MAX_BATTLE_MOVES;
    CreateMon(&mon, SPECIES_SWAMPERT, 50, 0, OTID_STRUCT_PRESET(0));
    EXPECT_EQ(Ascension_TalentBranchCount(SPECIES_SWAMPERT), MAX_BATTLE_MOVES);
    n = Ascension_BuildTalentList(&mon, before, sources);
    EXPECT_EQ(Ascension_TeachMove(&mon, MOVE_EMBER, TALENT_SLOT_ASK), TALENT_TEACH_NEEDS_SLOT);
    EXPECT(!ListHas(&mon, MOVE_EMBER));                   // asking changes nothing
    EXPECT_EQ(Ascension_TeachMove(&mon, MOVE_EMBER, sources[n - 1]), TALENT_TEACH_JOINED);
    EXPECT(ListHas(&mon, MOVE_EMBER) || !ListHas(&mon, before[n - 1]));
    Ascension_BuildTalentList(&mon, after, NULL);
    for (i = 0; i < n; i++)
    {
        u32 j;
        bool32 found = FALSE;
        for (j = 0; j < MAX_BATTLE_MOVES; j++)
            found |= (after[j] == before[i]);
        if (!found)
            replaced = i;
    }
    EXPECT_EQ(replaced, n - 1);                           // only the picked move is gone
}

TEST("Teaching: the saved 4 move slots follow the tree after a TM")
{
    struct Pokemon mon;
    u16 list[MAX_BATTLE_MOVES];
    u32 i;
    CreateMon(&mon, SPECIES_PIKACHU, 20, 0, OTID_STRUCT_PRESET(0));
    EXPECT_EQ(Ascension_TeachMove(&mon, MOVE_THUNDERBOLT, TALENT_SLOT_ASK), TALENT_TEACH_JUMPED);
    gAscensionMoveListTestOverride = TRUE;
    Ascension_RefreshMoveCache(&mon);
    gAscensionMoveListTestOverride = FALSE;
    Ascension_BuildTalentList(&mon, list, NULL);
    for (i = 0; i < MAX_MON_MOVES; i++)
        EXPECT_EQ(GetMonData(&mon, MON_DATA_MOVE1 + i), list[i]);
}

static bool32 FindGrowth(struct Pokemon *mon, u32 level, u16 to, struct TalentGrowth *out)
{
    u32 iter = 0;
    while (Ascension_NextTalentGrowth(mon, level, &iter, out))
    {
        u32 i;
        for (i = 0; i < out->count; i++)
            if (out->options[i] == to)
                return TRUE;
    }
    return FALSE;
}

TEST("Level-up growth: a branch reports the move it grows from and into, only at that level")
{
    struct Pokemon mon;
    struct TalentGrowth g;
    CreateMon(&mon, SPECIES_PIKACHU, 12, 0, OTID_STRUCT_PRESET(0));
    EXPECT(FindGrowth(&mon, 12, MOVE_ELECTRO_BALL, &g));
    EXPECT_EQ(g.from, MOVE_THUNDER_SHOCK);
    EXPECT_EQ(g.count, 1);
    EXPECT(!FindGrowth(&mon, 13, MOVE_ELECTRO_BALL, &g));
}

TEST("Level-up growth: a TM jump suppresses growth into nodes it already passed")
{
    struct Pokemon mon;
    struct TalentGrowth g;
    CreateMon(&mon, SPECIES_PIKACHU, 20, 0, OTID_STRUCT_PRESET(0));
    EXPECT_EQ(Ascension_TeachMove(&mon, MOVE_THUNDERBOLT, TALENT_SLOT_ASK), TALENT_TEACH_JUMPED);
    EXPECT(!FindGrowth(&mon, 32, MOVE_DISCHARGE, &g));
    EXPECT(FindGrowth(&mon, 44, MOVE_THUNDER, &g));
    EXPECT_EQ(g.from, MOVE_THUNDERBOLT);
}

TEST("Level-up growth: a branch point offers its options, and the pick is what it grows into")
{
    struct Pokemon mon;
    struct TalentGrowth g;
    u16 list[MAX_BATTLE_MOVES];
    u32 i;
    bool32 found = FALSE;
    CreateMon(&mon, SPECIES_SWAMPERT, 39, 0, OTID_STRUCT_PRESET(0));
    EXPECT(FindGrowth(&mon, 39, MOVE_MUDDY_WATER, &g));
    EXPECT_EQ(g.count, 2);
    EXPECT_EQ(g.options[0], MOVE_HYDRO_PUMP);
    Ascension_SetTalentChoice(&mon, g.source, 1);
    Ascension_BuildTalentList(&mon, list, NULL);
    for (i = 0; i < MAX_BATTLE_MOVES; i++)
    {
        found |= (list[i] == MOVE_MUDDY_WATER);
        EXPECT_NE(list[i], MOVE_HYDRO_PUMP);
    }
    EXPECT(found);
}

TEST("Evolving keeps talent slots: a TM jump and the current move carry over to the evolved tree")
{
    struct Pokemon mon;
    u32 species = SPECIES_RAICHU;
    CreateMon(&mon, SPECIES_PIKACHU, 20, 0, OTID_STRUCT_PRESET(0));
    EXPECT_EQ(Ascension_TeachMove(&mon, MOVE_THUNDERBOLT, TALENT_SLOT_ASK), TALENT_TEACH_JUMPED);
    EXPECT(ListHas(&mon, MOVE_THUNDERBOLT));
    SetMonData(&mon, MON_DATA_SPECIES, &species);
    Ascension_OnEvolve(&mon, SPECIES_PIKACHU);
    EXPECT(ListHas(&mon, MOVE_THUNDERBOLT));
    EXPECT_EQ(Ascension_TeachMove(&mon, MOVE_THUNDERBOLT, TALENT_SLOT_ASK), TALENT_TEACH_ALREADY_KNOWN);
}

TEST("Talent data survives boxing (as in the PC and the save): choices, TM jumps and joined chains")
{
    struct Pokemon mon, back;
    u16 before[MAX_BATTLE_MOVES], after[MAX_BATTLE_MOVES];
    u32 i;
    CreateMon(&mon, SPECIES_PIKACHU, 30, 0, OTID_STRUCT_PRESET(0));
    Ascension_SetTalentChoice(&mon, 2, 1);
    EXPECT_EQ(Ascension_TeachMove(&mon, MOVE_THUNDERBOLT, TALENT_SLOT_ASK), TALENT_TEACH_JUMPED);
    EXPECT_EQ(Ascension_TeachMove(&mon, MOVE_SURF, TALENT_SLOT_ASK), TALENT_TEACH_JOINED);
    Ascension_BuildTalentList(&mon, before, NULL);
    BoxMonToMon(&mon.box, &back);
    Ascension_BuildTalentList(&back, after, NULL);
    for (i = 0; i < MAX_BATTLE_MOVES; i++)
        EXPECT_EQ(after[i], before[i]);
    EXPECT_EQ(Ascension_GetTalentChoice(&back, 2), 1);
    EXPECT(ListHas(&back, MOVE_SURF));
}

TEST("Move info talent page (F016): next growth, branch point, fully grown and TM path")
{
    struct Pokemon mon;
    u8 text[160];
    u8 expect[160];
    CreateMon(&mon, SPECIES_PIKACHU, 20, 0, OTID_STRUCT_PRESET(0));
    gAscensionMoveListTestOverride = TRUE;
    EXPECT(Ascension_DescribeTalentMove(&mon, MOVE_ELECTRO_BALL, text));
    StringCopy(expect, COMPOUND_STRING("Talent branch 1.\nGrows into Discharge at Lv. 32."));
    EXPECT_EQ(StringCompare(text, expect), 0);
    EXPECT(!Ascension_DescribeTalentMove(&mon, MOVE_SURF, text));          // not in its list
    EXPECT_EQ(Ascension_TeachMove(&mon, MOVE_THUNDER, TALENT_SLOT_ASK), TALENT_TEACH_JUMPED);
    EXPECT(Ascension_DescribeTalentMove(&mon, MOVE_THUNDER, text));
    StringCopy(expect, COMPOUND_STRING("Talent branch 1.\nFully grown."));
    EXPECT_EQ(StringCompare(text, expect), 0);
    EXPECT_EQ(Ascension_TeachMove(&mon, MOVE_EMBER, TALENT_SLOT_ASK), TALENT_TEACH_JOINED);
    EXPECT(Ascension_DescribeTalentMove(&mon, MOVE_EMBER, text));
    gAscensionMoveListTestOverride = FALSE;
}
