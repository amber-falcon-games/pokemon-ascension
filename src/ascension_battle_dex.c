#include "global.h"
#include "ascension_battle_dex.h"
#include "battle.h"
#include "battle_lab.h"
#include "battle_util.h"
#include "fpmath.h"
#include "move.h"
#include "pokedex.h"
#include "pokemon.h"
#include "string_util.h"
#include "constants/characters.h"

// Pokémon Ascension F016: the in-battle Pokédex panel (L in the action menu). What it shows follows
// what the player's Pokédex knows: types for every Pokémon met (the field Pokédex shows them once
// seen); abilities, base stats and weaknesses once caught. Lab battles show everything.

enum { DEX_PAGE_BASICS, DEX_PAGE_ABILITIES, DEX_PAGE_STATS, DEX_PAGE_MOVES };

bool32 Ascension_DexKnowsAll(u32 species)
{
    if (BattleLab_IsLabBattle())
        return TRUE;
    return GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_CAUGHT);
}

// Moves the player has seen each opposing party member use in this battle (survives switching).
static EWRAM_DATA u16 sSeenMoves[PARTY_SIZE][MAX_BATTLE_MOVES] = {0};

void Ascension_DexBattleStart(void)
{
    memset(sSeenMoves, 0, sizeof(sSeenMoves));
}

void Ascension_DexRecordMove(u32 battler, u32 move)
{
    u32 i, slot = gBattlerPartyIndexes[battler];
    if (GetBattlerSide(battler) != B_SIDE_OPPONENT || slot >= PARTY_SIZE || move == MOVE_NONE)
        return;
    for (i = 0; i < MAX_BATTLE_MOVES; i++)
    {
        if (sSeenMoves[slot][i] == move)
            return;
        if (sSeenMoves[slot][i] == MOVE_NONE)
        {
            sSeenMoves[slot][i] = move;
            return;
        }
    }
}

static enum Move SeenMove(u32 battler, u32 i)
{
    u32 slot = gBattlerPartyIndexes[battler];
    return slot < PARTY_SIZE ? sSeenMoves[slot][i] : MOVE_NONE;
}

static u32 SeenMoveCount(u32 battler)
{
    u32 i, n = 0;
    for (i = 0; i < MAX_BATTLE_MOVES; i++)
        n += (SeenMove(battler, i) != MOVE_NONE);
    return n;
}

// Basics, (caught: abilities, base stats,) then moves seen, four to a page.
u32 Ascension_DexPageCount(u32 battler)
{
    u32 movePages = SeenMoveCount(battler) > 4 ? 2 : 1;
    return (Ascension_DexKnowsAll(gBattleMons[battler].species) ? 3 : 1) + movePages;
}

static u8 *AppendNum(u8 *dest, u32 n)
{
    return ConvertIntToDecimalStringN(dest, n, STR_CONV_MODE_LEFT_ALIGN, 3);
}

static u8 *AppendTypes(u8 *dest, u32 species)
{
    enum Type t1 = GetSpeciesType(species, 0), t2 = GetSpeciesType(species, 1);
    dest = StringAppend(dest, gTypesInfo[t1].name);
    if (t2 != t1)
    {
        dest = StringAppend(dest, COMPOUND_STRING(" / "));
        dest = StringAppend(dest, gTypesInfo[t2].name);
    }
    return dest;
}

static u8 *AppendWeaknesses(u8 *dest, u32 species)
{
    enum Type t1 = GetSpeciesType(species, 0), t2 = GetSpeciesType(species, 1), atk;
    u32 count = 0;
    dest = StringAppend(dest, COMPOUND_STRING("Weak: "));
    for (atk = TYPE_NONE + 1; atk < NUMBER_OF_MON_TYPES; atk++)
    {
        uq4_12_t mod;
        if (atk == TYPE_MYSTERY || atk == TYPE_STELLAR)
            continue;
        mod = uq4_12_multiply(GetTypeModifier(atk, t1), t2 != t1 ? GetTypeModifier(atk, t2) : UQ_4_12(1.0));
        if (mod <= UQ_4_12(1.0))
            continue;
        if (count++ > 0)
            dest = StringAppend(dest, COMPOUND_STRING(", "));
        dest = StringAppend(dest, gTypesInfo[atk].name);
        if (mod >= UQ_4_12(4.0))
            dest = StringAppend(dest, COMPOUND_STRING(" x4"));
    }
    if (count == 0)
        dest = StringAppend(dest, COMPOUND_STRING("none"));
    return dest;
}

void Ascension_DexPageText(u32 battler, u32 page, u8 *dest)
{
    u32 species = gBattleMons[battler].species, i, count = 0;
    bool32 all = Ascension_DexKnowsAll(species);

    *dest = EOS;
    u32 movePage = 0;
    if (!all && page > DEX_PAGE_BASICS)
        page += DEX_PAGE_MOVES - 1;   // uncaught: basics, then moves seen
    if (page > DEX_PAGE_MOVES)
    {
        movePage = page - DEX_PAGE_MOVES;
        page = DEX_PAGE_MOVES;
    }
    switch (page)
    {
    case DEX_PAGE_BASICS:
        dest = StringAppend(dest, GetSpeciesName(species));
        dest = StringAppend(dest, COMPOUND_STRING(" Lv. "));
        dest = AppendNum(dest, gBattleMons[battler].level);
        dest = StringAppend(dest, COMPOUND_STRING("\n"));
        dest = AppendTypes(dest, species);
        dest = StringAppend(dest, COMPOUND_STRING("\n"));
        if (all)
            AppendWeaknesses(dest, species);
        else
            StringAppend(dest, COMPOUND_STRING("Catch one to learn more."));
        break;
    case DEX_PAGE_ABILITIES:
        dest = StringAppend(dest, COMPOUND_STRING("Abilities:"));
        for (i = 0; i < NUM_ABILITY_SLOTS; i++)
        {
            enum Ability a = GetSpeciesAbility(species, i);
            u32 j;
            bool32 dup = (a == ABILITY_NONE);
            for (j = 0; j < i && !dup; j++)
                dup = (GetSpeciesAbility(species, j) == a);
            if (dup)
                continue;
            dest = StringAppend(dest, count++ == 0 ? COMPOUND_STRING("\n") : COMPOUND_STRING(", "));
            dest = StringAppend(dest, gAbilitiesInfo[a].name);
        }
        break;
    case DEX_PAGE_STATS:
        dest = StringAppend(dest, COMPOUND_STRING("Base stats\nHP "));
        dest = AppendNum(dest, GetSpeciesBaseStat(species, STAT_HP));
        dest = StringAppend(dest, COMPOUND_STRING("  Atk "));
        dest = AppendNum(dest, GetSpeciesBaseStat(species, STAT_ATK));
        dest = StringAppend(dest, COMPOUND_STRING("  Def "));
        dest = AppendNum(dest, GetSpeciesBaseStat(species, STAT_DEF));
        dest = StringAppend(dest, COMPOUND_STRING("\nSpA "));
        dest = AppendNum(dest, GetSpeciesBaseStat(species, STAT_SPATK));
        dest = StringAppend(dest, COMPOUND_STRING("  SpD "));
        dest = AppendNum(dest, GetSpeciesBaseStat(species, STAT_SPDEF));
        dest = StringAppend(dest, COMPOUND_STRING("  Spe "));
        AppendNum(dest, GetSpeciesBaseStat(species, STAT_SPEED));
        break;
    default:   // moves this Pokémon has used in this battle
        dest = StringAppend(dest, COMPOUND_STRING("Moves seen:"));
        for (i = 0; i < MAX_BATTLE_MOVES; i++)
        {
            enum Move m = SeenMove(battler, i);
            if (m == MOVE_NONE)
                continue;
            if (count >= movePage * 4 && count < movePage * 4 + 4)
            {
                dest = StringAppend(dest, count % 2 == 0 ? COMPOUND_STRING("\n") : COMPOUND_STRING(", "));
                dest = StringAppend(dest, GetMoveName(m));
            }
            count++;
        }
        if (count == 0)
            StringAppend(dest, COMPOUND_STRING("\nnone yet"));
        break;
    }
}
