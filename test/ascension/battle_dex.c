#include "global.h"
#include "test/test.h"
#include "ascension_battle_dex.h"
#include "battle.h"
#include "pokedex.h"
#include "string_util.h"
#include "text.h"
#include "constants/characters.h"
#include "constants/moves.h"

// Pokémon Ascension F016: in-battle Pokédex panel content.
static bool32 TextHas(const u8 *text, const u8 *part)
{
    u32 i, n = StringLength(part);
    for (i = 0; text[i] != EOS; i++)
        if (StringCompareN(&text[i], part, n) == 0)
            return TRUE;
    return FALSE;
}

TEST("Battle Pokédex: uncaught shows types only, caught adds abilities, stats and weaknesses")
{
    u8 text[200];
    u32 natDex = SpeciesToNationalPokedexNum(SPECIES_GARDEVOIR);
    gBattlerPositions[1] = B_POSITION_OPPONENT_LEFT;
    gBattlerPartyIndexes[1] = 0;
    gBattleMons[1].species = SPECIES_GARDEVOIR;
    gBattleMons[1].level = 50;
    Ascension_DexBattleStart();

    if (!GetSetPokedexFlag(natDex, FLAG_GET_CAUGHT))
    {
        EXPECT_EQ(Ascension_DexPageCount(1), 2);
        Ascension_DexPageText(1, 0, text);
        EXPECT(TextHas(text, COMPOUND_STRING("Psychic / Fairy")));
        EXPECT(TextHas(text, COMPOUND_STRING("Catch one")));
        Ascension_DexPageText(1, 1, text);
        EXPECT(!TextHas(text, COMPOUND_STRING("Weak")));     // page 2 is moves seen
    }
    GetSetPokedexFlag(natDex, FLAG_SET_CAUGHT);
    EXPECT_EQ(Ascension_DexPageCount(1), 5);
    Ascension_DexPageText(1, 1, text);
    EXPECT(TextHas(text, COMPOUND_STRING("Weak to: Poison, Ghost, Steel")));
    Ascension_DexPageText(1, 2, text);
    EXPECT(TextHas(text, COMPOUND_STRING("Trace")));
    Ascension_DexPageText(1, 3, text);
    EXPECT(TextHas(text, COMPOUND_STRING("SpA 125")));
}

TEST("Battle Pokédex: moves seen are recorded per opposing Pokémon, once each, and reset per battle")
{
    u8 text[200];
    gBattlerPositions[0] = B_POSITION_PLAYER_LEFT;
    gBattlerPositions[1] = B_POSITION_OPPONENT_LEFT;
    gBattlerPartyIndexes[1] = 2;
    gBattleMons[1].species = SPECIES_GARDEVOIR;
    Ascension_DexBattleStart();
    Ascension_DexPageText(1, Ascension_DexPageCount(1) - 1, text);
    EXPECT(TextHas(text, COMPOUND_STRING("none yet")));
    Ascension_DexRecordMove(1, MOVE_MOONBLAST);
    Ascension_DexRecordMove(1, MOVE_MOONBLAST);
    Ascension_DexRecordMove(0, MOVE_TACKLE);            // the player's own moves are not recorded
    Ascension_DexPageText(1, Ascension_DexPageCount(1) - 1, text);
    EXPECT(TextHas(text, COMPOUND_STRING("Moonblast")));
    EXPECT(!TextHas(text, COMPOUND_STRING("Moonblast, Moonblast")));
    gBattlerPartyIndexes[1] = 0;                        // another party member: nothing seen
    Ascension_DexPageText(1, Ascension_DexPageCount(1) - 1, text);
    EXPECT(TextHas(text, COMPOUND_STRING("none yet")));
    Ascension_DexBattleStart();
}

TEST("Battle Pokédex: long text is wrapped to the panel width, at most three lines")
{
    u8 text[200];
    u32 lines = 1, i;
    u8 *line = text;
    gBattlerPositions[1] = B_POSITION_OPPONENT_LEFT;
    gBattlerPartyIndexes[1] = 0;
    gBattleMons[1].species = SPECIES_EXEGGUTOR;   // many weaknesses, Bug x4
    GetSetPokedexFlag(SpeciesToNationalPokedexNum(SPECIES_EXEGGUTOR), FLAG_SET_CAUGHT);
    Ascension_DexPageText(1, 1, text);
    EXPECT(TextHas(text, COMPOUND_STRING("Bug x4")));
    Ascension_WrapText(text, FONT_NARROW, 142);
    for (i = 0; ; i++)
    {
        if (text[i] == CHAR_NEWLINE || text[i] == EOS)
        {
            u8 saved = text[i];
            text[i] = EOS;
            EXPECT_LE(GetStringWidth(FONT_NARROW, line, 0), 142);
            text[i] = saved;
            if (saved == EOS)
                break;
            line = &text[i + 1];
            lines++;
        }
    }
    EXPECT_LE(lines, 3);
}
