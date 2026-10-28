# CORE INTEGRATION V2 - HUONG DAN VA TRANG THAI

Ban goc: Turn-base-Tactical-Game.zip, ngay 28/10/2026.

## Da ket noi vao Core

- GameData: Race, Class, Skill, Monster va ngu hanh tu CSV.
- CharacterFactory: tao Player, nap chi so Race/Class va 4 skill.
- Party: 1-3 Hero, chung Inventory; Core quan ly luot cua ca ba (khong chi leader).
- ItemDatabase/ItemFactory: doc data/items.csv; mau, mana, hoi sinh, 4 loai Booster, Held Gear.
- Party.equipGear(): chon mot trong Held Gear qua [G] truoc tran cho leader.
- CombatModifiers: don danh thuong qua Player::executeSkill nen tinh buff, trang bi, crit, lifesteal, reflect.
- onTurnEnd / resetBattleState / consumeExtraTurn: da goi o Core.
- Undo: snapshot HP, MP, buffs va trang bi cho don danh/skill; dung Item::revert() cho vat pham.
- StatusEffectManager: theo doi burn, regen, stun, slow va buff; speed SLOW dua tren toc do goc de tranh cong don.
- DSA: CombatAI target selection, TurnManager, WaveManager, CombatObserver.
- Graphics: menu, tao Hero, chon Wave, prep, battle map, sprite, skill list va UI.

## Cach su dung

- Trong Prep Menu: [T] thay doi so thanh vien 1 -> 2 -> 3 -> 1. Dong doi duoc tao tu Race/Class CSV, ten "Dong doi 2" va "Dong doi 3".
- Trong Prep Menu: [G] chuyen Held Gear leader: Khong -> lan luot gear trong items.csv -> Khong.
- Trong battle: [A] danh thuong; [S] skill, [1-4] chon skill, [Enter] dung.
- Trong battle: [I] mo tui do (HP/MP/Revive/Booster), [1-9] chon loai, [F1-F3] chon thanh vien, [Enter] dung.
- [Tab] doi muc tieu tan cong, [Enter] xac nhan, [Z] hoan tac khi chua xac nhan.

## Luu y ve pham vi

- AI hien tai van chi danh thuong. CombatAI do nhom cung cap chi co API selectTarget/filterAliveTargets; khong co danh sach skill hay quyet dinh heal/buff cho Monster/Boss trong data/monsters.csv. Khong tu y tao skill quai.
- PrepMenuScreen cua nhom chi hien thi hero chinh; phan Core moi cho phep chon 1-3 thanh vien bang [T] va hien so luong, nhung chua co giao dien chon danh tinh/ky nang/gear tung dong doi. Dong doi duoc tao tu Race/Class mac dinh.
- Chua co src/audio/SoundManager trong du an. File nhac trong assets/audio chua duoc gan vao game qua module Audio.
- Bao gom kiem thu Entities va Core bridge, khong co Raylib trong moi truong Linux kiem thu nen chua xac nhan game.exe chay Windows. Can build va test gameplay tren MSYS2 UCRT64.
- Khong thay doi Game-Windows/ da chay; script build tao Game-Windows-v2/.

## Build Windows MSYS2 UCRT64

```bash
cd /d/CODE/Turn-base-Tactical-Game
cmake -S . -B build-ucrt64 -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-ucrt64 --parallel 4
cmake --build build-ucrt64 --target test_entities test_core_integration --parallel 4
ctest --test-dir build-ucrt64 --output-on-failure
./build-ucrt64/game.exe
```

Chay `build_game_windows.bat` neu muon tao thu muc Game-Windows-v2 cung data, assets va DLL; ban Game-Windows cu khong bi sua.
