#ifndef GUARD_BATTLE_LAB_H
#define GUARD_BATTLE_LAB_H

#include "global.h"

// Pokémon Ascension Battle Lab (F007, F008): dev-build sandbox for real battles.
// Opened with L + SELECT on the title screen; compiled out of release builds.

enum { LAB_SIDE_PLAYER, LAB_SIDE_OPPONENT, LAB_SIDE_COUNT };

#define LAB_SLOT_ACTIVE    (1 << 0)   // sent into battle
#define LAB_SLOT_MODIFIED  (1 << 1)   // differs from the scenario: built fresh at battle start
#define LAB_DEFAULT_LEVEL  50

struct LabSlot
{
    u16 species;
    u8 level;
    u8 flags;
};

extern struct LabSlot gBattleLabTeams[LAB_SIDE_COUNT][PARTY_SIZE];

void BattleLab_Open(void);
void BattleLab_SetupScenarioParties(void);   // builds both battle parties from the Lab teams

// Team model (F008); exposed for tests.
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

#endif // GUARD_BATTLE_LAB_H
