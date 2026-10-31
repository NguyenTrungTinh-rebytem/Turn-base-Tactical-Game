# The First Step – Ember & Rune UI upgrade

## Scope / installation

Merge the files from `Ember_Rune_UI_PATCH.zip` into the **original** `Turn-base-Tactical-Game-improve_font_and_UI` source tree, preserving relative paths. The patch is NOT a standalone game; original `assets/images`, `assets/audio` and `data` are still required.

This update changes UI drawing, screen input, render scaling and small BOM handling only. The algorithms and unit tests of core game combat, entities and DSA remain unchanged. `GameEngine.cpp/.h` has new UI input/render hooks.

### Required font downloads (SIL Open Font License 1.1)

Place these two exact files inside `assets/fonts/`:

- `NotoSerifDisplay-SemiBold.ttf` (display titles, large commands), from Noto Serif Display: https://fonts.google.com/noto/specimen/Noto+Serif+Display
- `NotoSans-Medium.ttf` (readable Vietnamese UI/body), from Noto Sans: https://fonts.google.com/noto/specimen/Noto+Sans

Both are redistributed by their respective upstream font providers under SIL OFL 1.1; license notice is in `assets/fonts/LICENSE-Noto.txt`. For Vietnamese support these exact locally inspected font versions cover all U+1EA0–U+1EF9 glyphs, ASCII and precomposed Latin accents. The project intentionally uses simple procedural geometry rather than text arrows (some Unicode arrows lack glyph coverage). The font binary files are **not included in this patch**. Without the font downloads, Raylib falls back to its own default font and Vietnamese glyphs may not render correctly.

**Why this pairing?** Noto Serif Display Semibold has a sharp, substantial serif shape that works with etched plaques. Noto Sans Medium is more readable at 14–24 px and has clear numbers for HUD stats. Both cover the complete Vietnamese precomposed diacritic range.

### Build (MSYS2 UCRT64 / VS2022)

```
cmake -S . -B build-ucrt64 -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-ucrt64 --parallel 4
ctest --test-dir build-ucrt64 --output-on-failure
```

For MSVC Visual Studio 2022 the CMake project now sets `/utf-8 /W4`. CMake already globs `src/*.cpp` recursively and thus includes `src/graphics/ui/*.cpp` automatically. Your existing Raylib 4.x+ setup is required.

### Controls

Start Menu: click Start; Exit now opens a confirm dialog. Hero Select: select portrait then Confirm. Wave Select: click wave card then Start. Prep: click tabs, squad slots, then Enter Battle. During battle use the Attack / Skill / Item buttons; click enemy or ally as target, then End Turn. Right-click or Cancel dismisses targeting. Menu opens Pause (with Resume and return-to-menu confirmation). At victory/defeat click Menu or Retry. The tiny top-left dot toggles a Vietnamese font-test panel. Window resizing keeps a 1280×720 logical canvas in letterbox.

### UI tuning

- `src/graphics/ui/UiTheme.h` – palette and base resolution
- `src/graphics/ui/FontManager.cpp` – font filenames and fixed sizes, Unicode codepoints
- `src/graphics/ui/UiFrame.cpp` – iron/stone notched edges, rivets, gauges
- `src/graphics/ui/UiWidgets.cpp` – buttons, tooltips and consumable item icons
- `src/graphics/ui/UiInput.cpp` – pointer, mouse-to-logical transform, cursor ripple
- `src/graphics/Battleui.cpp` – battle HUD and skill popup
- `src/core/GameEngine.cpp` – mouse battle actions, pause, victory/retry screens, collision avoidance

### Verification matrix

- PASS locally: `test_entities`, `test_core_integration`, `test_gamedata` (compiled independently with g++ C++17).
- PASS syntax-only: production `.cpp` files with a temporary Raylib API declaration shim; no new-code compiler warnings detected. This does **not** prove an actual Raylib link or runtime behavior.
- NOT VERIFIED: VS2022/MSVC executable, 60 FPS under gameplay, full mouse-only walkthrough on Windows, pixel-perfect label placement, font atlas rendered visually, and no visual clipping.
- PARTIAL: no user-facing key-hint labels remain in production screen drawing, but optional keyboard control code is retained. Intro, selection, prep, battle commands and pause now have mouse controls. Tooltip behavior and style are improved; not every user-requested animation, settings subpanel, scrollbar, disabled reason, language switch or accessibility detail is finished.

This is an incremental source patch, not a claim of full acceptance against every requirement in the provided prompt.
