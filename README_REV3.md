# The First Step — Revision 3 UI Patch

## Base / installation

Apply only to a project that **already has Obsidian & Ember Revision 2.1** installed. Make a backup. This archive contains complete new/modified files only; it is not a standalone game executable.

Extract with folders intact to `D:\CODE\Turn-base-Tactical-Game` (overwrite matching source files). Do NOT extract the package inside an extra top-level folder.

In MSYS2 UCRT64:

```bash
cd /d/CODE/Turn-base-Tactical-Game
cmake -S . -B build-ucrt64 -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-ucrt64 --parallel 4
./build-ucrt64/game.exe
```

CMakeLists.txt already includes `file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")` and `/utf-8` for MSVC, so **no new CMake change is needed**. Running configure is still recommended so Ninja discovers new `UiLayout.cpp`.

## New layout and behavior

- Solid dark shard plates (opaque alpha 255), no decorative cross-lines in the widget renderer.
- One small turn-order strip at upper-left with portrait head crops and element badges (no names inside).
- Compact command cluster bottom-right with three visible buttons. Gear button sits in the header.
- Skill and item panels open directly *above* the command cluster, never in the middle of the field. Each list shows seven rows, supports mouse wheel and dragging its scrollbar, and uses icon/name/cost columns with UTF-8-safe ellipsis and scissor clipping.
- The battle HUD draws a maximum of one unit tooltip at the end of its pass. Hovered skill/item help is shown in the panel footer, not over units.
- Eight item types use different procedural Raylib icons; Prep Menu accepts English item display names as well as canonical `ITM_...` ids.
- Three compact party cards appear bottom-left; enemy field bars remain small.
- Hero Select radar removed in favor of a larger pixel-art preview with the original animation source; buttons now use solid widgets.
- VI/EN toggle hidden until all fixed battle-log messages have translations; UI defaults to Vietnamese.

Geometry and footprint: see `REV3_DESIGN_SPEC.md`. HUD base occupancy **11.84%**, including all three party cards, log button, turn strip, wave chip, and command. With one popup open: **20.01%** (both measured at 1280×720 and identical at 1920×1080). Only a popup *or* expanded log should be visible at once.

## Fonts

Revision 3 continues to use the **existing fonts from Revision 2** (no new downloads required):

- `assets/fonts/NotoSans-Medium.ttf`
- `assets/fonts/NotoSans-CondensedSemiBold.ttf`
- `assets/fonts/NotoSerifDisplay-SemiBold.ttf`
- `assets/fonts/LICENSE-Noto.txt` (SIL Open Font License)

If Vietnamese shows tofu, verify these files and the FontTest debug corner. Runtime uppercase conversion is not used by the new widgets.

## Tests done in this environment

- `test_entities`: PASS (Debug / Linux)
- `test_core_integration`: PASS (Debug / Linux)
- `test_gamedata`: PASS (Debug / Linux)
- Changed UI `.cpp` files, including `GameEngine.cpp`, passed `g++ -std=c++17 -fsyntax-only` against a Raylib API **mock header**. This checks C++ syntax and identifiers, but **does not** establish that the real Raylib build links/runs correctly.
- File integrity and layout overlap arithmetic: checked at ZIP creation.

## Not yet confirmed

- Native Raylib Windows/MSYS2 UCRT64 build and manual click-through (requires your Windows machine).
- 1280×720 vs. 1920×1080 vs. fullscreen actual screenshot comparison, FPS profiling and guaranteed 60 FPS.
- Full button-state easing/animation, HP damage trail tweening, timed ticker, and localized dynamic combat log templates.
- Battle scene enemy placement, especially for atypically large bosses; field-versus-popup collision requires real screenshots.

## Where to tune

- Colors: `src/graphics/ui/UiTheme.h`
- Solid panel bevel and selected marker: `src/graphics/ui/UiFrame.cpp`
- Battle rectangles/hitboxes: `src/graphics/ui/UiBattleLayout.h`
- Popup text, icon shape, and tooltips: `src/graphics/ui/UiWidgets.cpp`
- Font atlas: `src/graphics/ui/FontManager.cpp`; strings: `src/graphics/ui/Strings.cpp`
- Tiny UI texture generator: `tools/gen_ui_assets.py` (optional; runtime icons are procedural).

When testing in MSYS2, send **the first compiler error**, or a real Start / Hero / Prep / Battle screenshot. The syntax check with a mock header cannot guarantee Windows rendering correctness.
