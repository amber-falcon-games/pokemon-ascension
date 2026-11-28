#include "global.h"
#include "test/battle.h"
#include "ascension_moves.h"

// Pokémon Ascension F015 (spec item 6): level-up growth along a talent branch, shown in battle.
WILD_BATTLE_TEST("Talent growth: levelling up past a node shows 'grew into' and updates the saved moves")
{
    GIVEN {
        gAscensionMoveListTestOverride = TRUE;
        PLAYER(SPECIES_PIKACHU) { Level(11); }
        OPPONENT(SPECIES_CATERPIE) { Level(30); HP(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDER_SHOCK); }
    } SCENE {
        MESSAGE("Pikachu used Thunder Shock!");
        MESSAGE("The wild Caterpie fainted!");
        MESSAGE("Pikachu's Thunder Shock grew into Electro Ball!{WAIT_SE}");
    } THEN {
        u32 i;
        bool32 found = FALSE;
        for (i = 0; i < MAX_MON_MOVES; i++)
            found |= (GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MOVE1 + i) == MOVE_ELECTRO_BALL);
        EXPECT(found);
    }
}
