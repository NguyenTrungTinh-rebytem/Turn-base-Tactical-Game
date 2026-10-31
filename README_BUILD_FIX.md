# Ember & Rune — Raylib GOLD / VIOLET macro collision fix

## Root cause
Raylib's `raylib.h` defines `GOLD` and `VIOLET` as preprocessor macros, so `UiTheme::GOLD` and declarations `constexpr Color GOLD{...}` are expanded before C++ compilation. This caused the errors in `UiTheme.h`, `Battleui.cpp`, `GameEngine.cpp`, etc.

## Fix
- Rename `UiTheme::GOLD` to `UiTheme::AGED_GOLD` (same RGB color).
- Rename `UiTheme::VIOLET` to `UiTheme::VIOLET_SHADOW` (same RGB color).
- Update all affected usages in 6 `.cpp` files.
- No modifications to combat rules, core algorithms, entity logic, or data content.

## Apply
If you already installed `Ember_Rune_UI_PATCH.zip`, extract `Ember_Rune_UI_BUILD_FIX_ONLY.zip` into your project root, replacing files when prompted.

Or, instead, extract `Ember_Rune_UI_PATCH_FIXED.zip` into the original game project folder to install the full revised UI patch (not a standalone game).

Windows PowerShell:
```powershell
$project = 'D:\CODE\Turn-base-Tactical-Game'
Expand-Archive -LiteralPath "$env:USERPROFILE\Downloads\Ember_Rune_UI_BUILD_FIX_ONLY.zip" -DestinationPath $project -Force
```

Build in MSYS2 UCRT64:
```bash
cd /d/CODE/Turn-base-Tactical-Game
cmake -S . -B build-ucrt64 -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-ucrt64 --parallel 4
```

If the ZIP was extracted into a subfolder, the resulting files must end up at `D:\CODE\Turn-base-Tactical-Game\src\...`, **not** `...\Ember_Rune_UI_BUILD_FIX_ONLY\src\...`.

## Verification status
- Verified syntax of all project `.cpp` translation units with G++ C++17 and Raylib-compatible mock declarations including genuine `GOLD` and `VIOLET`-style macro expansions. PASS.
- `test_entities` and `test_core_integration`: PASS in a Linux Debug build; `test_gamedata`: PASS when compiled manually.
- In Release mode `test_core_integration` segfaults on Linux because the original test uses `assert()` calls with side effects; `NDEBUG` removes the operations, leaving the party unpopulated. This is an existing **test-design** issue and is not caused by this UI macro fix.
- Actual Windows/MSYS2 build and game runtime not available in this environment; these were not verified.

## Changed files
- `src/graphics/ui/UiTheme.h`
- `src/core/GameEngine.cpp`
- `src/graphics/Battleui.cpp`
- `src/graphics/StartMenu.cpp`
- `src/graphics/ui/UiFrame.cpp`
- `src/graphics/ui/UiText.cpp`
- `src/graphics/ui/UiWidgets.cpp`
