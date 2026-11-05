# Battle HUD layout spec

All rectangles below use the 1920×1080 canvas coordinates. The renderer maps them to the 1280×720 design space at 1.5×; hit tests use the same converted rectangles.

| Element | Rectangle `(x, y, w, h)` | Placement |
|---|---:|---|
| Turn strip | `(24, 20, 420, 76)` | Active 64px hex portrait + next six 44px portraits |
| Round chip | `(1688, 20, 208, 76)` | Stage, current actor, and gold |
| Party cards | `(24, 112, 660, 112)` | Three 212×112 cards with 12px gaps |
| Command cluster | `(24, 840, 720, 216)` | Turn banner, actions, cancel/undo/confirm |
| Log button | `(768, 840, 108, 48)` | Opens/closes the battle log |
| Log panel | `(900, 832, 496, 224)` | Latest battle message; mutually exclusive with the list |
| List panel | `(1420, 832, 476, 224)` | Skill/item rows; height shrinks to fit visible content |
| Battlefield safe region | `(24, 236, 1872, 588)` | Units and their shadows; clear of HUD panels |

Panels use their full specified rectangle only at maximum content. The log and list panels are mutually exclusive. Their bottoms align at y=1056; the battlefield ends at y=824, leaving a 16px gap. Other nearest edges have 12px or more clearance: turn strip/party cards 16px; round chip/party cards 16px; party cards/battlefield 12px; command/log button 24px; log button/log panel 24px; log/list panels 24px. The top strip and round chip are independently anchored to opposite corners. No fixed HUD rectangles overlap.

The fixed HUD footprint is `420×76 + 208×76 + 660×112 + 720×216 + 108×48 = 282,352 px²`. The largest optional panel is the log: `496×224 = 111,104 px²`. Worst case is therefore `393,456 / (1920×1080) = 18.98%`, below the 22% limit (`456,192 px²`). The safe region is a placement guide, not an additional HUD footprint.
