# T08 - Hero, Party, Item, Inventory (danh cho nguoi tong hop)

## Pham vi thay doi (de merge)
Chi them/sua trong `src/entities/`, them `data/items.csv`, `tests/test_entities.cpp`, va 1 khoi test o cuoi `CMakeLists.txt`.
**Khong sua** `src/core/*`, `src/data/*`. Engine cu chay y nhu cu; muon dung he thong moi thi noi them (xem cuoi file).

File CU duoc sua (chi them, hanh vi cu giu nguyen khi chua co vat pham):
| File | Sua gi |
|---|---|
| `Character.h/.cpp` | + `restoreMp`, `consumeMp`, getter Chi tiet, `modifiers()`, `onTurnEnd()`, `resetBattleState()`, `dealSkillDamage()`; 1 dong goi hook trong `takeDamage` |
| `Player.h/.cpp` | + race/class/skills; `executeSkill` goi `dealSkillDamage` |
| `Enemy.cpp`, `Boss.cpp` | `executeSkill` goi `dealSkillDamage` (1 dong) |
| `Inventory.h` | + `getItem`, `removeAt`, `size`, `findIndex`, `countIf`... (API cu giu nguyen) |

File MOI: `CharacterFactory`, `Party`, `Item` (+Potion/ManaPotion/ReviveItem/BoosterItem/HeldGear), `ItemFactory`,
`ItemData.h`, `ItemDatabase`, `CombatModifiers`.

## Dung nhu the nao
```cpp
GameData data;  data.loadAll("data");                       // phan Data cua nguoi khac (khong sua)
ItemDatabase items;  items.load("data");                    // hoac tryLoad(...) khong nem exception

auto hero = CharacterFactory::createHero(data, "Anh", "Human", "Swordman", Element::KIM);
Party party;  party.addMember(std::move(hero));             // toi da 3
party.addItem(ItemFactory::createItem(items, "ITM_HP_MINOR"));   // ton trong maxCarry
party.equipGear(0, someHeldGear);                           // Held Gear: tui -> hero

ItemPtr potion = party.getInventory().getItem(0);
if (potion->canUse(*hero)) potion->use(*hero);              // Undo: potion->revert(*hero, amountTraVe)
```
Getter cho man hinh khac: `Character::getMaxHp/Mp/Attack/...`, `Player::getRaceName/getClassName/getSkills`,
`Item::getName/getNameVi/getDescription/getCategory/...`, `Party::getMembers/getItemsByCategory`.

## Du lieu doc truc tiep tu CSV
- `data/items.csv`: sua so lieu hoac **them vat pham moi = them 1 dong**, khong can bien dich lai
  (`ItemDatabase::reload()` nap lai ngay trong luc chay). Doc theo TEN cot nen them/doi cot khong hong.
- File sai -> `ItemLoadError` ghi ro `file:dong:nguyen nhan`; dung `tryLoad` de game van chay (khong co vat pham).
- Hero doc tu `races/classes/skills.csv` qua `GameData` co san.
- **Con nam trong code**: tap hop `effect` (them loai hieu ung moi moi can code), `Party::MAX_SIZE = 3`,
  `CombatModifiers::BASE_CRIT_DAMAGE_PERCENT = 150`.

## Diem phu thuoc vao phan cua nguoi khac (neu ho doi se anh huong)
`CharacterFactory` goi `GameData::getBaseStats / getHeroSkillIds / getSkill / isLoaded` va doc `SkillData`.
Neu phan Data doi ten cac ham nay, chi sua `CharacterFactory.cpp`.

## Noi vao engine (tuy chon - nhanh `feature/T08-engine-integration`)
Da co 1 commit mau: hero tu Factory, `Party` giu hero + tui do, vat pham tu CSV, phim `[M]`.
Luu y khi noi: `m_combatants` doi `unique_ptr` -> `shared_ptr` de Party va engine cung giu hero.
Cac hook ma TurnManager/engine can goi khi tich hop day du:
`character.onTurnEnd()` (cuoi luot), `character.resetBattleState()` (dau tran),
`character.modifiers().consumeExtraTurn()` (Haste Tonic: neu true thi cho hanh dong them).
