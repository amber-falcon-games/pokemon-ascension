#ifndef GUARD_ASCENSION_BATTLE_DEX_H
#define GUARD_ASCENSION_BATTLE_DEX_H

// Pokémon Ascension F016: the in-battle Pokédex panel (L in the action menu).
bool32 Ascension_DexKnowsAll(u32 species);          // caught (or a Lab battle): everything is shown
u32 Ascension_DexPageCount(u32 battler);
void Ascension_DexPageText(u32 battler, u32 page, u8 *dest);   // at most 3 lines
void Ascension_DexBattleStart(void);
void Ascension_WrapText(u8 *text, u32 fontId, u32 width);     // breaks lines at spaces to fit width
void Ascension_DexRecordMove(u32 battler, u32 move);           // an opposing Pokémon used a move

#endif // GUARD_ASCENSION_BATTLE_DEX_H
