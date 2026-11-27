#include "global.h"
#include "test/test.h"
#include "ascension_moves.h"
#include "move.h"
#include "pokemon.h"
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
