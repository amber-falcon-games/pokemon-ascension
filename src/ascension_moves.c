#include "global.h"
#include "ascension_moves.h"
#include "battle_lab.h"
#include "move.h"
#include "pokemon.h"
#include "string_util.h"
#include "constants/characters.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "data/ascension/talent_trees.h"

#if TESTING
EWRAM_DATA bool8 gAscensionMoveListTestOverride = FALSE;
#endif

u32 Ascension_GetTalentChoice(struct Pokemon *mon, u32 branch)
{
    return (GetMonData(mon, MON_DATA_TALENT_CHOICES) >> (2 * branch)) & 3;
}

void Ascension_SetTalentChoice(struct Pokemon *mon, u32 branch, u32 choice)
{
    u32 choices = GetMonData(mon, MON_DATA_TALENT_CHOICES);
    choices = (choices & ~(3u << (2 * branch))) | ((choice & 3) << (2 * branch));
    SetMonData(mon, MON_DATA_TALENT_CHOICES, &choices);
}

u32 Ascension_TalentBranchCount(u32 species)
{
    return species < NUM_SPECIES ? gTalentTrees[species].branchCount : 0;
}

// The node a branch is on at a level, never before minNode (a TM jump); -1 before its first node.
static s32 TalentBranchNode(u32 species, u32 branch, u32 level, s32 minNode)
{
    const struct TalentBranch *b = &gTalentTrees[species].branches[branch];
    s32 i;
    for (i = b->nodeCount - 1; i >= 0; i--)
        if (b->nodes[i].level <= level)
            break;
    if (minNode > i)
        i = min(minNode, b->nodeCount - 1);
    return i;
}

u16 Ascension_TalentBranchMove(u32 species, u32 branch, u32 level, u32 choice)
{
    s32 node;
    if (branch >= Ascension_TalentBranchCount(species))
        return MOVE_NONE;
    node = TalentBranchNode(species, branch, level, -1);
    if (node < 0)
        return MOVE_NONE;
    return gTalentTrees[species].branches[branch].nodes[node].moves[choice % gTalentTrees[species].branches[branch].nodes[node].count];
}

// Saved TM progress lives in the contest data (contests are cut, D011/D027; layout unchanged):
//  jumps (3 bits x 8 branches: node index + 1)   -> Cool, Beauty, Cute
//  join 0 (chain + 1:6, position:7, slot:3)      -> Smart, Tough
//  join 1                                        -> Sheen, Cool/Beauty/Cute ribbon tiers (3+3+2 bits)
static u32 GetJumps(struct Pokemon *mon)
{
    return GetMonData(mon, MON_DATA_COOL) | (GetMonData(mon, MON_DATA_BEAUTY) << 8) | (GetMonData(mon, MON_DATA_CUTE) << 16);
}

static void SetJumps(struct Pokemon *mon, u32 jumps)
{
    u32 b0 = jumps & 0xFF, b1 = (jumps >> 8) & 0xFF, b2 = (jumps >> 16) & 0xFF;
    SetMonData(mon, MON_DATA_COOL, &b0);
    SetMonData(mon, MON_DATA_BEAUTY, &b1);
    SetMonData(mon, MON_DATA_CUTE, &b2);
}

static u32 GetJoin(struct Pokemon *mon, u32 k)
{
    if (k == 0)
        return GetMonData(mon, MON_DATA_SMART) | (GetMonData(mon, MON_DATA_TOUGH) << 8);
    return GetMonData(mon, MON_DATA_SHEEN) | ((GetMonData(mon, MON_DATA_COOL_RIBBON) & 7) << 8)
         | ((GetMonData(mon, MON_DATA_BEAUTY_RIBBON) & 7) << 11) | ((GetMonData(mon, MON_DATA_CUTE_RIBBON) & 3) << 14);
}

static void SetJoin(struct Pokemon *mon, u32 k, u32 join)
{
    u32 a = join & 0xFF, b = (join >> 8) & 0xFF, r0 = (join >> 8) & 7, r1 = (join >> 11) & 7, r2 = (join >> 14) & 3;
    if (k == 0)
    {
        SetMonData(mon, MON_DATA_SMART, &a);
        SetMonData(mon, MON_DATA_TOUGH, &b);
        return;
    }
    SetMonData(mon, MON_DATA_SHEEN, &a);
    SetMonData(mon, MON_DATA_COOL_RIBBON, &r0);
    SetMonData(mon, MON_DATA_BEAUTY_RIBBON, &r1);
    SetMonData(mon, MON_DATA_CUTE_RIBBON, &r2);
}

#define JOIN_CHAIN(j) ((s32)((j) & 0x3F) - 1)   // -1: none
#define JOIN_POS(j)   (((j) >> 6) & 0x7F)
#define JOIN_SLOT(j)  (((j) >> 13) & 7)
#define MAKE_JOIN(chain, pos, slot) (((chain) + 1) | ((pos) << 6) | ((slot) << 13))

// On a joined global chain: the furthest move whose typical level it has reached, never before pos.
static u16 GlobalChainMove(u32 chain, u32 pos, u32 level)
{
    const struct TalentGlobalMove *c = gTalentGlobalChains[chain];
    u32 i = pos;
    while (c[i + 1].move != MOVE_NONE && c[i + 1].level <= level)
        i++;
    return c[i].move;
}

u32 Ascension_BuildTalentList(struct Pokemon *mon, u16 moves[MAX_BATTLE_MOVES], u8 *sources)
{
    u32 species = GetMonData(mon, MON_DATA_SPECIES), level = GetMonData(mon, MON_DATA_LEVEL);
    u32 jumps = GetJumps(mon), n = Ascension_TalentBranchCount(species), i, k, count = 0;
    u16 list[MAX_BATTLE_MOVES + TALENT_MAX_JOINS] = {0};
    for (i = 0; i < n && i < MAX_BATTLE_MOVES; i++)
    {
        s32 node = TalentBranchNode(species, i, level, (s32)((jumps >> (3 * i)) & 7) - 1);
        if (node >= 0)
        {
            const struct TalentNode *nd = &gTalentTrees[species].branches[i].nodes[node];
            list[i] = nd->moves[Ascension_GetTalentChoice(mon, i) % nd->count];
        }
    }
    for (k = 0; k < TALENT_MAX_JOINS; k++)
    {
        u32 join = GetJoin(mon, k);
        if (JOIN_CHAIN(join) < 0)
            continue;
        if (JOIN_SLOT(join) < n)
            list[JOIN_SLOT(join)] = GlobalChainMove(JOIN_CHAIN(join), JOIN_POS(join), level);
        else
            list[MAX_BATTLE_MOVES + k] = GlobalChainMove(JOIN_CHAIN(join), JOIN_POS(join), level);
    }
    for (i = 0; i < MAX_BATTLE_MOVES + TALENT_MAX_JOINS && count < MAX_BATTLE_MOVES; i++)
    {
        u32 j;
        bool32 dup = (list[i] == MOVE_NONE);
        for (j = 0; j < count && !dup; j++)
            dup = (moves[j] == list[i]);
        if (!dup)
        {
            if (sources != NULL)
                sources[count] = i;
            moves[count++] = list[i];
        }
    }
    for (i = count; i < MAX_BATTLE_MOVES; i++)
    {
        if (sources != NULL)
            sources[i] = TALENT_SLOT_ASK;
        moves[i] = MOVE_NONE;
    }
    return count;
}

enum TalentTeachResult Ascension_TeachMove(struct Pokemon *mon, u16 move, u32 replaceSlot)
{
    u32 species = GetMonData(mon, MON_DATA_SPECIES), level = GetMonData(mon, MON_DATA_LEVEL);
    u32 jumps = GetJumps(mon), n = Ascension_TalentBranchCount(species), b, k, c;
    s32 node, cur;

    // 1. In its own tree?
    for (b = 0; b < n; b++)
    {
        const struct TalentBranch *br = &gTalentTrees[species].branches[b];
        for (node = 0; node < br->nodeCount; node++)
        {
            for (c = 0; c < br->nodes[node].count; c++)
            {
                if (br->nodes[node].moves[c] != move)
                    continue;
                cur = TalentBranchNode(species, b, level, (s32)((jumps >> (3 * b)) & 7) - 1);
                if (node > cur)
                {
                    jumps = (jumps & ~(7u << (3 * b))) | ((u32)(node + 1) << (3 * b));
                    SetJumps(mon, jumps);
                    Ascension_SetTalentChoice(mon, b, c);
                    return TALENT_TEACH_JUMPED;
                }
                if (node == cur && Ascension_GetTalentChoice(mon, b) % br->nodes[node].count == c)
                    return TALENT_TEACH_ALREADY_KNOWN;
                return TALENT_TEACH_GROWN_PAST;
            }
        }
    }
    // 2. Join (or advance on) the global chain that holds it.
    for (c = 0; gTalentGlobalChains[c] != NULL; c++)
    {
        u32 pos;
        for (pos = 0; gTalentGlobalChains[c][pos].move != MOVE_NONE; pos++)
        {
            u32 freeSlot = TALENT_MAX_JOINS;
            if (gTalentGlobalChains[c][pos].move != move)
                continue;
            for (k = 0; k < TALENT_MAX_JOINS; k++)
            {
                u32 join = GetJoin(mon, k);
                if (JOIN_CHAIN(join) == (s32)c)
                {
                    u16 current = GlobalChainMove(c, JOIN_POS(join), level);
                    if (current == move)
                        return TALENT_TEACH_ALREADY_KNOWN;
                    if (pos <= JOIN_POS(join) || gTalentGlobalChains[c][pos].level <= level)
                        return TALENT_TEACH_GROWN_PAST;
                    SetJoin(mon, k, MAKE_JOIN(c, pos, JOIN_SLOT(join)));
                    return TALENT_TEACH_JUMPED;
                }
                if (JOIN_CHAIN(join) < 0 && freeSlot == TALENT_MAX_JOINS)
                    freeSlot = k;
            }
            if (freeSlot == TALENT_MAX_JOINS)
                return TALENT_TEACH_NO_ROOM;
            // A free battle-list place goes after the tree's branches.
            if (n + freeSlot < MAX_BATTLE_MOVES)
            {
                SetJoin(mon, freeSlot, MAKE_JOIN(c, pos, MAX_BATTLE_MOVES - 1));
                return TALENT_TEACH_JOINED;
            }
            // A full list: the player picks the move it replaces (a source from BuildTalentList).
            if (replaceSlot == TALENT_SLOT_ASK)
                return TALENT_TEACH_NEEDS_SLOT;
            if (replaceSlot >= MAX_BATTLE_MOVES)   // replacing a joined chain: take over that join
            {
                k = (replaceSlot - MAX_BATTLE_MOVES) % TALENT_MAX_JOINS;
                SetJoin(mon, k, MAKE_JOIN(c, pos, JOIN_SLOT(GetJoin(mon, k))));
            }
            else
            {
                SetJoin(mon, freeSlot, MAKE_JOIN(c, pos, replaceSlot));
            }
            return TALENT_TEACH_JOINED;
        }
    }
    return TALENT_TEACH_CANNOT;
}

bool32 Ascension_UsesTalentTree(struct Pokemon *mon)
{
    return ASCENSION_MOVE_LIST && Ascension_TalentBranchCount(GetMonData(mon, MON_DATA_SPECIES)) > 0
        && (!BattleLab_IsLabBattle() || BattleLab_TalentMoves());
}

bool32 Ascension_NextTalentGrowth(struct Pokemon *mon, u32 level, u32 *iter, struct TalentGrowth *out)
{
    u32 species = GetMonData(mon, MON_DATA_SPECIES), jumps = GetJumps(mon), n = Ascension_TalentBranchCount(species);
    u32 hidden = 0, k, i;

    if (level <= 1)
        return FALSE;
    for (k = 0; k < TALENT_MAX_JOINS; k++)   // branches a joined chain replaced don't grow on screen
    {
        u32 join = GetJoin(mon, k);
        if (JOIN_CHAIN(join) >= 0 && JOIN_SLOT(join) < n)
            hidden |= 1u << JOIN_SLOT(join);
    }
    for (; *iter < MAX_BATTLE_MOVES + TALENT_MAX_JOINS; (*iter)++)
    {
        u32 src = *iter;
        if (src < MAX_BATTLE_MOVES)
        {
            const struct TalentBranch *br;
            s32 minNode, cur, prev;
            if (src >= n || (hidden & (1u << src)))
                continue;
            br = &gTalentTrees[species].branches[src];
            minNode = (s32)((jumps >> (3 * src)) & 7) - 1;
            cur = TalentBranchNode(species, src, level, minNode);
            prev = TalentBranchNode(species, src, level - 1, minNode);
            if (cur == prev || cur < 0)
                continue;
            out->source = src;
            out->count = br->nodes[cur].count;
            out->from = prev >= 0 ? br->nodes[prev].moves[Ascension_GetTalentChoice(mon, src) % br->nodes[prev].count] : MOVE_NONE;
            for (i = 0; i < TALENT_MAX_CHOICES; i++)
                out->options[i] = i < out->count ? br->nodes[cur].moves[i] : MOVE_NONE;
        }
        else
        {
            u32 join = GetJoin(mon, src - MAX_BATTLE_MOVES);
            u16 before, after;
            if (JOIN_CHAIN(join) < 0)
                continue;
            before = GlobalChainMove(JOIN_CHAIN(join), JOIN_POS(join), level - 1);
            after = GlobalChainMove(JOIN_CHAIN(join), JOIN_POS(join), level);
            if (before == after)
                continue;
            out->source = src;
            out->count = 1;
            out->from = before;
            out->options[0] = after;
            for (i = 1; i < TALENT_MAX_CHOICES; i++)
                out->options[i] = MOVE_NONE;
        }
        (*iter)++;
        return TRUE;
    }
    return FALSE;
}

static s32 NodeWithMove(const struct TalentBranch *br, u16 move)
{
    s32 i, c;
    for (i = 0; i < br->nodeCount; i++)
        for (c = 0; c < br->nodes[i].count; c++)
            if (br->nodes[i].moves[c] == move)
                return i;
    return -1;
}

// Evolving keeps branch slots (the generator puts a pre-evolution's branches first), but a slot's
// nodes can differ between stages: carry each TM jump and branch choice over by move.
void Ascension_OnEvolve(struct Pokemon *mon, u32 oldSpecies)
{
    u32 newSpecies = GetMonData(mon, MON_DATA_SPECIES), level = GetMonData(mon, MON_DATA_LEVEL);
    u32 nOld = Ascension_TalentBranchCount(oldSpecies), nNew = Ascension_TalentBranchCount(newSpecies);
    u32 jumps = GetJumps(mon), newJumps = 0, b;

    if (nNew == 0 || newSpecies == oldSpecies)
        return;
    for (b = 0; b < nNew && b < MAX_BATTLE_MOVES; b++)
    {
        const struct TalentBranch *nb = &gTalentTrees[newSpecies].branches[b];
        s32 jumpNode = -1, oldCur, newCur;
        u16 curMove = MOVE_NONE;
        u32 choice = Ascension_GetTalentChoice(mon, b);
        if (b < nOld)
        {
            const struct TalentBranch *ob = &gTalentTrees[oldSpecies].branches[b];
            s32 oldJump = (s32)((jumps >> (3 * b)) & 7) - 1;
            oldCur = TalentBranchNode(oldSpecies, b, level, oldJump);
            if (oldCur >= 0)
                curMove = ob->nodes[oldCur].moves[choice % ob->nodes[oldCur].count];
            if (oldJump >= 0)
            {
                s32 i;
                jumpNode = NodeWithMove(nb, curMove);
                // Not on the new branch: the furthest node at or below the old jump's level.
                for (i = nb->nodeCount - 1; i >= 0 && jumpNode < 0; i--)
                    if (nb->nodes[i].level <= ob->nodes[oldJump].level)
                        jumpNode = i;
            }
        }
        if (jumpNode >= 0)
            newJumps |= (u32)(jumpNode + 1) << (3 * b);
        newCur = TalentBranchNode(newSpecies, b, level, jumpNode);
        if (b >= nOld)
        {
            Ascension_SetTalentChoice(mon, b, 0);
        }
        else if (newCur >= 0 && curMove != MOVE_NONE)
        {
            u32 c;
            for (c = 0; c < nb->nodes[newCur].count; c++)
                if (nb->nodes[newCur].moves[c] == curMove)
                    Ascension_SetTalentChoice(mon, b, c);
        }
    }
    for (; b < MAX_BATTLE_MOVES; b++)
        Ascension_SetTalentChoice(mon, b, 0);
    SetJumps(mon, newJumps);
    Ascension_RefreshMoveCache(mon);
}

// Keeps the saved 4 move slots a cache of the first four tree moves (PP kept for unchanged moves).
void Ascension_RefreshMoveCache(struct Pokemon *mon)
{
    u16 list[MAX_BATTLE_MOVES];
    u32 i;
    if (!Ascension_UsesTalentTree(mon))
        return;
    Ascension_BuildTalentList(mon, list, NULL);
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        u32 move = list[i], pp;
        if (GetMonData(mon, MON_DATA_MOVE1 + i) == move)
            continue;
        pp = move != MOVE_NONE ? GetMovePP(move) : 0;
        SetMonData(mon, MON_DATA_MOVE1 + i, &move);
        SetMonData(mon, MON_DATA_PP1 + i, &pp);
    }
}

// Builds the battle move list (and PP). With a talent tree, every branch contributes its current
// move (D027: no custom slots), and the saved 4 move slots are refreshed as a cache of the first
// four so menus and upstream code stay in step. Without one, F014's rule: slots 4+ get the most
// recent level-up moves not already known. Returns the number of moves.
u32 Ascension_FillExtraBattleMoves(struct Pokemon *mon, enum Move moves[MAX_BATTLE_MOVES], u8 pp[MAX_BATTLE_MOVES])
{
    u32 count = MAX_MON_MOVES, i, j, level, species;
    const struct LevelUpMove *learnset;
    s32 last;

    for (i = MAX_MON_MOVES; i < MAX_BATTLE_MOVES; i++)
    {
        moves[i] = MOVE_NONE;
        pp[i] = 0;
    }
    if (!ASCENSION_MOVE_LIST)
        return MAX_MON_MOVES;

    species = GetMonData(mon, MON_DATA_SPECIES);
    level = GetMonData(mon, MON_DATA_LEVEL);
    // Lab battles keep the scenario's hand-set moves (plus F014 extras) so scenarios test what they
    // set, unless the Lab's Moves rule is "Tree".
    if (Ascension_UsesTalentTree(mon))
    {
        u16 list[MAX_BATTLE_MOVES];
        count = Ascension_BuildTalentList(mon, list, NULL);
        for (i = 0; i < MAX_BATTLE_MOVES; i++)
            moves[i] = list[i];
        for (i = 0; i < MAX_BATTLE_MOVES; i++)
        {
            if (i >= count)
                moves[i] = MOVE_NONE;
            pp[i] = moves[i] != MOVE_NONE ? GetMovePP(moves[i]) : 0;
        }
        for (i = 0; i < MAX_MON_MOVES; i++)   // keep the saved slots as a cache of the first four
        {
            u32 move = moves[i], movePp = pp[i];
            SetMonData(mon, MON_DATA_MOVE1 + i, &move);
            SetMonData(mon, MON_DATA_PP1 + i, &movePp);
        }
        return count;
    }

    learnset = GetSpeciesLevelUpLearnset(species);
    for (last = 0; learnset[last].move != LEVEL_UP_MOVE_END && learnset[last].level <= level; last++)
        ;
    // Most recent first: walk back from the highest level the Pokémon has reached.
    for (last = last - 1; last >= 0 && count < MAX_BATTLE_MOVES; last--)
    {
        enum Move move = learnset[last].move;
        bool32 known = (move == MOVE_NONE);
        for (j = 0; j < count && !known; j++)
            if (moves[j] == move)
                known = TRUE;
        if (known)
            continue;
        moves[count] = move;
        pp[count] = GetMovePP(move);
        count++;
    }
    return count;
}

// F016: the talent page of the in-battle move info (L, then L again).
static u8 *AppendLevel(u8 *dest, u32 level)
{
    dest = StringAppend(dest, COMPOUND_STRING("Lv. "));
    return ConvertIntToDecimalStringN(dest, level, STR_CONV_MODE_LEFT_ALIGN, 3);
}

static u8 *AppendNode(u8 *dest, const struct TalentNode *node)
{
    u32 i;
    for (i = 0; i < node->count; i++)
    {
        if (i > 0)
            dest = StringAppend(dest, i + 1 == node->count ? COMPOUND_STRING(" or ") : COMPOUND_STRING(", "));
        dest = StringAppend(dest, GetMoveName(node->moves[i]));
    }
    return dest;
}

bool32 Ascension_DescribeTalentMove(struct Pokemon *mon, u16 move, u8 *dest)
{
    u16 list[MAX_BATTLE_MOVES];
    u8 sources[MAX_BATTLE_MOVES];
    u32 species = GetMonData(mon, MON_DATA_SPECIES), level = GetMonData(mon, MON_DATA_LEVEL);
    u32 i, count;

    *dest = EOS;
    if (!Ascension_UsesTalentTree(mon))
        return FALSE;
    count = Ascension_BuildTalentList(mon, list, sources);
    for (i = 0; i < count && list[i] != move; i++)
        ;
    if (i == count)
        return FALSE;
    // A joined chain: the slot's source is the join itself, or a branch slot a join replaced.
    {
        u32 k;
        for (k = 0; k < TALENT_MAX_JOINS; k++)
        {
            u32 join = GetJoin(mon, k), pos;
            const struct TalentGlobalMove *c;
            if (JOIN_CHAIN(join) < 0 || GlobalChainMove(JOIN_CHAIN(join), JOIN_POS(join), level) != move)
                continue;
            c = gTalentGlobalChains[JOIN_CHAIN(join)];
            for (pos = JOIN_POS(join); c[pos].move != move; pos++)
                ;
            dest = StringAppend(dest, COMPOUND_STRING("Talent path from a TM.\n"));
            if (c[pos + 1].move == MOVE_NONE)
                StringAppend(dest, COMPOUND_STRING("Fully grown."));
            else
            {
                dest = StringAppend(dest, COMPOUND_STRING("Grows into "));
                dest = StringAppend(dest, GetMoveName(c[pos + 1].move));
                dest = StringAppend(dest, COMPOUND_STRING(" at "));
                dest = AppendLevel(dest, c[pos + 1].level);
                StringAppend(dest, COMPOUND_STRING("."));
            }
            return TRUE;
        }
    }
    if (sources[i] < Ascension_TalentBranchCount(species))
    {
        u32 b = sources[i];
        const struct TalentBranch *br = &gTalentTrees[species].branches[b];
        s32 cur = TalentBranchNode(species, b, level, (s32)((GetJumps(mon) >> (3 * b)) & 7) - 1);
        dest = StringAppend(dest, COMPOUND_STRING("Talent branch "));
        dest = ConvertIntToDecimalStringN(dest, b + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
        dest = StringAppend(dest, COMPOUND_STRING(".\n"));
        if (cur + 1 >= br->nodeCount)
        {
            StringAppend(dest, COMPOUND_STRING("Fully grown."));
        }
        else if (br->nodes[cur + 1].count > 1)
        {
            dest = AppendLevel(dest, br->nodes[cur + 1].level);
            dest = StringAppend(dest, COMPOUND_STRING(": "));
            dest = AppendNode(dest, &br->nodes[cur + 1]);
            StringAppend(dest, COMPOUND_STRING("."));
        }
        else
        {
            dest = StringAppend(dest, COMPOUND_STRING("Grows into "));
            dest = StringAppend(dest, GetMoveName(br->nodes[cur + 1].moves[0]));
            dest = StringAppend(dest, COMPOUND_STRING(" at "));
            dest = AppendLevel(dest, br->nodes[cur + 1].level);
            StringAppend(dest, COMPOUND_STRING("."));
        }
        return TRUE;
    }
    return FALSE;
}
