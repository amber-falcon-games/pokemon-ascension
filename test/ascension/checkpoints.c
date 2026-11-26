#include "global.h"
#include "event_data.h"
#include "item.h"
#include "money.h"
#include "script.h"
#include "test/test.h"
#include "constants/items.h"

// Pokémon Ascension F007b: dev checkpoints (data/scripts/ascension_checkpoints.inc).
// Each test applies one checkpoint's state script (no warp) from a cleared game and checks
// the documented badges, team, key items and main-path story state.

extern const u8 Ascension_CPState_Start[];
extern const u8 Ascension_CPState_PokeBalls[];
extern const u8 Ascension_CPState_Gym1[];
extern const u8 Ascension_CPState_Gym2[];
extern const u8 Ascension_CPState_Gym3[];
extern const u8 Ascension_CPState_Gym4[];
extern const u8 Ascension_CPState_Gym5[];
extern const u8 Ascension_CPState_Gym6[];
extern const u8 Ascension_CPState_Gym7[];
extern const u8 Ascension_CPState_Gym8[];

static void ApplyCheckpoint(const u8 *state)
{
    ZeroPlayerPartyMons();
    ClearBag();
    InitEventData();
    SetMoney(&gSaveBlock1Ptr->money, 3000);
    RunScriptImmediately(state);
}

static u32 CountBadges(void)
{
    u32 i, n = 0;
    for (i = FLAG_BADGE01_GET; i <= FLAG_BADGE08_GET; i++)
        n += FlagGet(i);
    return n;
}

static u32 PartySize(void)
{
    u32 i, n = 0;
    for (i = 0; i < PARTY_SIZE; i++)
        n += (GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES) != SPECIES_NONE);
    return n;
}

#define EXPECT_ACE(species, level) \
    do { \
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_SPECIES), species); \
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_LEVEL), level); \
    } while (0)

TEST("Checkpoint Start: intro done, no Pokémon")
{
    ApplyCheckpoint(Ascension_CPState_Start);
    EXPECT_EQ(PartySize(), 0);
    EXPECT_EQ(CountBadges(), 0);
    EXPECT_EQ(VarGet(VAR_LITTLEROOT_INTRO_STATE), 7);
    EXPECT(!FlagGet(FLAG_SYS_POKEMON_GET));
}

TEST("Checkpoint Poké Balls: Mudkip, Pokédex, 5 Poké Balls")
{
    ApplyCheckpoint(Ascension_CPState_PokeBalls);
    EXPECT_EQ(PartySize(), 1);
    EXPECT_ACE(SPECIES_MUDKIP, 7);
    EXPECT(FlagGet(FLAG_SYS_POKEDEX_GET));
    EXPECT(CheckBagHasItem(ITEM_POKE_BALL, 5));
    EXPECT(!FlagGet(FLAG_SYS_B_DASH)); // Mom's Running Shoes scene still to come
}

TEST("Checkpoint Gym 1: before Roxanne")
{
    ApplyCheckpoint(Ascension_CPState_Gym1);
    EXPECT_EQ(PartySize(), 2);
    EXPECT_ACE(SPECIES_MUDKIP, 15);
    EXPECT_EQ(CountBadges(), 0);
    EXPECT(FlagGet(FLAG_SYS_B_DASH));
    EXPECT_EQ(VarGet(VAR_PETALBURG_GYM_STATE), 2);
}

TEST("Checkpoint Gym 2: before Brawly")
{
    ApplyCheckpoint(Ascension_CPState_Gym2);
    EXPECT_EQ(PartySize(), 3);
    EXPECT_ACE(SPECIES_MARSHTOMP, 19);
    EXPECT_EQ(CountBadges(), 1);
    EXPECT(FlagGet(FLAG_SYS_POKENAV_GET));
    EXPECT(CheckBagHasItem(ITEM_LETTER, 1));
    EXPECT(CheckBagHasItem(ITEM_DEVON_PARTS, 1));
    EXPECT(!FlagGet(FLAG_DEFEATED_DEWFORD_GYM));
}

TEST("Checkpoint Gym 3: before Wattson")
{
    ApplyCheckpoint(Ascension_CPState_Gym3);
    EXPECT_EQ(PartySize(), 4);
    EXPECT_ACE(SPECIES_MARSHTOMP, 24);
    EXPECT_EQ(CountBadges(), 2);
    EXPECT(FlagGet(FLAG_DELIVERED_DEVON_GOODS));
    EXPECT(!CheckBagHasItem(ITEM_DEVON_PARTS, 1));
    EXPECT(!CheckBagHasItem(ITEM_LETTER, 1));
    EXPECT(CheckBagHasItem(ITEM_MACH_BIKE, 1));
    EXPECT(FlagGet(FLAG_DEFEATED_WALLY_MAUVILLE));
}

TEST("Checkpoint Gym 4: before Flannery")
{
    ApplyCheckpoint(Ascension_CPState_Gym4);
    EXPECT_EQ(PartySize(), 5);
    EXPECT_ACE(SPECIES_MARSHTOMP, 29);
    EXPECT_EQ(CountBadges(), 3);
    EXPECT(FlagGet(FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY));
    EXPECT(CheckBagHasItem(ITEM_METEORITE, 1));
}

TEST("Checkpoint Gym 5: before Norman")
{
    ApplyCheckpoint(Ascension_CPState_Gym5);
    EXPECT_EQ(PartySize(), 6);
    EXPECT_ACE(SPECIES_MARSHTOMP, 31);
    EXPECT_EQ(CountBadges(), 4);
    EXPECT_EQ(VarGet(VAR_PETALBURG_GYM_STATE), 6); // Norman accepts the challenge
    EXPECT(CheckBagHasItem(ITEM_GO_GOGGLES, 1));
}

TEST("Checkpoint Gym 6: before Winona")
{
    ApplyCheckpoint(Ascension_CPState_Gym6);
    EXPECT_EQ(PartySize(), 6);
    EXPECT_ACE(SPECIES_MARSHTOMP, 33);
    EXPECT_EQ(CountBadges(), 5);
    EXPECT(CheckBagHasItem(ITEM_HM_SURF, 1));
    EXPECT(CheckBagHasItem(ITEM_HM_FLY, 1));
    EXPECT(CheckBagHasItem(ITEM_DEVON_SCOPE, 1));
    EXPECT(FlagGet(FLAG_HIDE_FORTREE_CITY_KECLEON));
}

TEST("Checkpoint Gym 7: before Tate & Liza")
{
    ApplyCheckpoint(Ascension_CPState_Gym7);
    EXPECT_EQ(PartySize(), 6);
    EXPECT_ACE(SPECIES_SWAMPERT, 42);
    EXPECT_EQ(CountBadges(), 6);
    EXPECT(FlagGet(FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE));
    EXPECT(FlagGet(FLAG_HIDE_LILYCOVE_CITY_AQUA_GRUNTS));
    EXPECT(CheckBagHasItem(ITEM_MAGMA_EMBLEM, 1));
}

TEST("Checkpoint Gym 8: before Juan")
{
    ApplyCheckpoint(Ascension_CPState_Gym8);
    EXPECT_EQ(PartySize(), 6);
    EXPECT_ACE(SPECIES_SWAMPERT, 46);
    EXPECT_EQ(CountBadges(), 7);
    EXPECT(FlagGet(FLAG_SOOTOPOLIS_ARCHIE_MAXIE_LEAVE)); // Gym door unlocked
    EXPECT(!FlagGet(FLAG_SYS_WEATHER_CTRL));
    EXPECT(CheckBagHasItem(ITEM_HM_DIVE, 1));
    EXPECT(CheckBagHasItem(ITEM_HM_WATERFALL, 1));
    EXPECT(!FlagGet(FLAG_DEFEATED_SOOTOPOLIS_GYM));
}
