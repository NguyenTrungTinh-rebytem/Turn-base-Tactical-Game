# The First Step — UI Comfort + Performance Patch (Revision 2.1)

Applies **on top of Obsidian & Ember UI Revision 2**, not on the original
unmodified game. Only the files listed in this ZIP are replaced.

## Visual changes (less eye strain)

- Desaturated gold/ember, verdigris, HP/MP and Five Elements accents.
- Charcoal/slate panels replace the high-saturation purple cast.
- Body text and secondary text have higher contrast against the panels.
- Removed continuously pulsing button seams and reduced timeline glow.
- Menu title no longer bobs; selected buttons no longer expand aggressively.
- Reduced menu ember particles from 18 to 7, dimmed their light.
- Battle log message is placed on a dedicated dark ticker on the left;
  it no longer sits on top of the three character cards.
- Small active-card lift; command pane and tooltips use subtle borders.
- The same toned-down palette is applied to hero, prep, wave and battle UI.

## Rendering/performance changes

- Each chamfered panel now uses one `DrawTriangleFan` rather than eight
  individual `DrawTriangle` calls; less per-widget draw overhead.
- Flat HP/MP gauge fill avoids the per-frame gradient path.
- `UiText::drawEllipsis()` avoids a temporary string allocation in the
  common case where text already fits. Truncation preserves UTF-8 codepoints.
- `GameEngine::renderCombatants()` reuses `m_renderOrder` rather than
  allocating its index vector every frame.
- Existing input geometry, game mechanics, sprites and font files are kept.

## Install (Windows / MSYS2 UCRT64)

1. Back up `D:\CODE\Turn-base-Tactical-Game`.
2. Extract this ZIP into `D:\CODE\Turn-base-Tactical-Game` while
   preserving paths and replacing existing files.
3. From MSYS2 UCRT64:

```sh
cd /d/CODE/Turn-base-Tactical-Game
cmake -S . -B build-ucrt64 -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-ucrt64 --parallel 4
./build-ucrt64/game.exe
```

## Quick checks

- Start Menu: button states and language toggle respond to the mouse.
- Hero/Prep/Wave screens retain their previous navigation and content.
- Battle: Attack/Skill/Item/Cancel/Menu still use the same hitboxes.
- With three heroes, text remains in the left log ticker, not over hero cards.
- Unicode font remains Noto from Revision 2.

## Validation

- All three headless core/data tests PASS (`test_entities`,
  `test_core_integration`, `test_gamedata`) with Linux g++ C++17.
- `UiFrame.cpp`, `UiTheme.cpp`, `UiWidgets.cpp`, `UiText.cpp` pass a C++17
  syntax check against a Raylib API mock header.
- **A real Raylib/MSYS2 Windows build, visual test and FPS measurement are
  not available in this environment.** Please test the patch on your machine.
- No gameplay logic calculations or keyboard/mouse command binding changed.

## Adjust theme

`src/graphics/ui/UiTheme.h` — all core color constants.
`src/graphics/ui/UiTheme.cpp` — Five Element colors.
`src/graphics/ui/UiFrame.cpp` — border intensity, plaque fills.
`src/graphics/StartMenu.cpp` — particle count and brightness.

Rollback by restoring the Revision 2 copies of the same 13 files.
