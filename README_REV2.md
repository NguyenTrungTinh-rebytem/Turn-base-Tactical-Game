# The First Step — Obsidian & Ember / UI Revision 2

## Installation on the existing working Windows project

This is a **source patch, not an executable**, made against the earlier Ember & Rune UI patch (the one that already builds on your PC). Keep your game repository; back it up before applying.

1. Extract the patch ZIP into `D:\CODE\Turn-base-Tactical-Game`, preserving the `src/`, `assets/`, and `tools/` folder structure. Allow overwrite of matching files.
2. Three complete OFL font files are INCLUDED in `assets/fonts/` (you do not need to find or rename fonts).
3. Open MSYS2 UCRT64 and run:

   ```bash
   cd /d/CODE/Turn-base-Tactical-Game
   cmake -S . -B build-ucrt64 -G Ninja -DCMAKE_BUILD_TYPE=Release
   cmake --build build-ucrt64 --parallel 4
   ./build-ucrt64/game.exe
   ```

4. To verify font glyphs in-game, click the tiny colored dot at top-left to show/hide the FontTest view. An offline font sheet is also supplied as `assets/ui/font_test_vietnamese.png`.

If the compilation fails: **send the entire first error and the full compiler invocation**. This patch passed Linux C++17 syntax checks with a Raylib API mock, not a real Windows MSYS2 build.

## What is implemented

- New Obsidian & Ember palette and chamfered glass/shard frames and buttons.
- Vietnamese labels restored for major menu headings, including `Đội hình`, `Thông tin công`, `Chọn thủ lĩnh`, `Chỉ số`; no runtime uppercase conversion.
- `Strings.h/.cpp` with Vietnamese and English lookup tables for most top-level UI commands; start-menu VI/EN toggle (not all legacy descriptive text is translated).
- Noto Serif Display SemiBold / Noto Sans Condensed SemiBold / Noto Sans Medium files and included Noto OFL license.
- Explicit ASCII, Latin Extended and Vietnamese codepoint lists, startup `GetGlyphIndex` checks; rare black triangle icons are rendered with Raylib geometry instead of relying on absent font glyphs.
- Pixel-size-aware font reloading when window scale changes, native window drawing through a Camera2D (no 1280x720 render-target upscaling), logical mouse coordinates retained.
- Horizontal hex turn timeline, compact six-pixel HP field bars, party card portraits, right command list with icons, skill/item popup on the left, collapsible battle log ticker and hover tooltips.
- Hero-select heptagon radar visualization and improved font and panel colors; Prep menu tabs and item panels restyled; pentagon elemental reference reads the **original** `ELEMENT_MATRIX` without changing it.
- Reusable procedural action icons, UI layout constants, and optional generated decorative PNG assets with a reproducible Pillow script.

## Scope / limitations

- Existing gameplay mechanics, entity and DSA behavior remain unchanged. `src/core/GameEngine.cpp` has **rendering, UI interaction coordinates, and Vietnamese label** changes only, plus a log expand/collapse UI flag. No new combat commands were introduced.
- A few pieces are deliberately not claimed complete: visually tested 60 FPS, drag-scrollbars, fully translated English, portrait head cropping on the timeline (the timeline currently uses element sigils), sprite-clipped hexagonal portraits, and animated bar damage trails. Assets `assets/ui/*.png` are supplied as optional art resources; current frames are procedural.
- Native visual verification on 1280×720, 1920×1080 and fullscreen is still needed on a Raylib/Windows machine. Our static FontTest checks glyph availability in bundled TTFs; they do not replace on-screen Raylib inspection.
- The checked version uses 1280×720 UI layout coordinates, scaled to native window pixels. This yields a 1920×1080 design-grid appearance at 1.5×, but is not a complete coordinate rewrite to a new 1920 logical space.

## Where to tune

- `src/graphics/ui/UiTheme.h`: theme palette, time constants.
- `src/graphics/ui/UiFrame.cpp`: shard shapes, highlights, notches, gradients.
- `src/graphics/ui/UiBattleLayout.h`: all battle clickable button geometry.
- `src/graphics/ui/UiIcons.cpp`: procedural Five Elements and command icons.
- `src/graphics/ui/FontManager.cpp`: font sources, sizes, glyph audits.
- `src/graphics/ui/Strings.cpp`: selected VI/EN labels.
- `src/graphics/Battleui.cpp`, `src/graphics/Turnorderbar.cpp`, `src/core/GameEngine.cpp`: battle appearance.
- `tools/gen_ui_assets.py`: regenerate optional PNG textures and FontTest sheet (requires `Pillow`, optionally `fonttools`).

## Static validation performed in this environment

| Validation | Result |
| --- | --- |
| `test_entities` compiled and executed (Debug equivalent, Linux C++17) | PASS |
| `test_core_integration` compiled and executed (Debug equivalent) | PASS |
| `test_gamedata` compiled and executed (Debug equivalent) | PASS |
| UTF-8 coverage check of selected Vietnamese alphabet/pangram across bundled Noto TTFs with `fontTools` | PASS |
| Source syntax for changed UI and GameEngine C++17 with Raylib stub | PASS |
| Real MSVC / UCRT64 Raylib build and running gameplay | NOT VERIFIED |
| Frame rate, mouse placement and typography in live Windows window | NOT VERIFIED |

Original Noto fonts are licensed under SIL Open Font License 1.1; see `assets/fonts/LICENSE-Noto.txt`.
