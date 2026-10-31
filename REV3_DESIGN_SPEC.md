# The First Step — Revision 3: Compact · Solid · Readable

Design coordinate system: **1280 × 720** logical units; all positions below multiplied by **1.5** for the requested 1920 × 1080 reference frame. Existing native-resolution Raylib camera and matching mouse transform are retained.

| Component | Logical (x,y,w,h) | At 1920×1080 | Area (logical px²) |
|---|---|---|---:|
| Timeline | (18,18,405,60) | (27,27,608,90) | 24,300 |
| Wave/turn chip | (1092,20,164,36) | (1638,30,246,54) | 5,904 |
| Party cards, 3 | (18/193/368,630,168,70) | ~252×105 each | 35,280 |
| Command | (1014,536,242,166) | (1521,804,363,249) | 40,172 |
| Log button | (18,591,108,32) | (27,887,162,48) | 3,456 |
| Expandable log | (18,394,250,191) | (27,591,375,287) | 47,750 |
| Skill/item popup | (1014,219,242,311) | (1521,329,363,467) | 75,262 |

**Always-on HUD:** 109,112 / 921,600 = **11.84%**, including the round chip and log button. **With skill or item list:** 184,374 / 921,600 = **20.01%**. The log and list are mutually exclusive, so expanded HUD remains below 22% (log-only: 17.0%). The center x=430–990 remains free from panels, except unit-specific hover tooltips.

Palette: solid obsidian `#15121F` or muted plum `#1E1830`, alpha 255; title ivory `#F1E9DA`; secondary muted bone `#B7AEA0`; restrained gold `#DCC296`; green HP `#71BE87`; blue MP `#6BAEC5`. Surfaces have one top edge and one accent strip only on selection; no crossing decoration. 8-unit rhythm, 10–14-unit inner padding, clipped label/cost columns, one tooltip drawn once, last.

Battle: compact command at bottom-right; popup aligned to the command's left edge immediately above it; enemy turn collapses to an unobtrusive chip. Party cards occupy bottom-left. Heads-up field gauges remain under feet. Timeline shows only medallions/sigils; no names inside. Buttons use solid hit areas; list and log scrolling are mouse-driven.

Other scenes: real filled tabs/buttons, simplified hero cards, remove radar in favor of an enlarged hero preview. Vietnamese is the only UI language until the log and all other labels are translated consistently.

Verification: mathematical HUD footprint above is exact at logical coordinates but unit placement, FPS and real Windows MSYS2 rendering require running the game; those are not implied by this calculation.

Enemy spacing tweak: `BattleMap.cpp` uses a narrower `STEP_X_ENEMY=0.062` (vs the original 0.085) to move the rightmost standard enemy left of the popup; this changes presentation positions only, not turn order, targeting or combat stats. Large bosses still require visual verification.
