# Ban tich hop Core (28-10-2026)

## Chinh sua
- GameEngine dung CombatAI::selectTarget() cua DSA cho viec chon muc tieu.
- GameEngine dung WaveManager de quan ly wave dang chay va khoa/mo wave; UI van dung danh sach wave cleared cua Core.
- Skill duoc xu ly theo Command Pattern: tru MP, HP, ap dung StatusEffect va phuc hoi snapshot khi Undo.
- Skill HEAL/BUFF su dung Character::consumeMp() truc tiep thay vi gia lap mot lan attack.
- Sau khi dung skill, co the Enter de xac nhan hoac ESC/Z de Undo truoc khi chuyen luot.
- CMake loai tru graphics/main_*_test.cpp (cac chuong trinh demo co main rieng) ra khoi game.
- CMake test_entities lien ket SkillElementConfig.cpp.

## Build (Windows MSYS2 UCRT64)
1. Cai raylib va cmake, ninja, gcc trong MSYS2 UCRT64.
2. Tu thu muc du an: `cmake -S . -B build -G Ninja`
3. `cmake --build build --target game`
4. `./build/game.exe`
5. `cmake --build build --target test_entities` va `ctest --test-dir build --output-on-failure`.

## Ket qua kiem chung
- `tests/test_entities.cpp`: ALL TESTS PASSED khi bien dich voi GCC Linux va tat ca src/entities/*.cpp + src/data/*.cpp.
- Test CommandStack/UseSkillCommand thuc thi + Undo: PASSED.
- Chua xac minh build va UI game tren may kiem tra nay do thieu Raylib dev package. Can build tren Windows MSYS2 UCRT64 cua nhom.

## Nguyen tac
- Khong can sua code cua data/entities/dsa/graphics trong lan tich hop nay.
- Kiem tra game thuc te tren Windows: menu -> tao hero -> wave -> tan cong/skill/item -> undo -> AI -> win/lose.
