# UI Revision 2 — Obsidian & Ember

## Scope and scaling

A 1920×1080 art direction grid (8 px spacing), mapped to the project's existing 1280×720 UI coordinates at a design factor of 1.5. The original gameplay/world-space coordinates and mouse input remain in their proven 1280×720 space. Native-window rendering uses camera scaling rather than upscaling a 1280×720 render-texture; fonts use rasterization appropriate to the device scale. This preserves existing hit boxes and core logic while sharpening the image at 1080p.

## Palette / visual language

Obsidian `#0B0A10`, translucent plum glass `#15121F` to `#221B33`, bone `#F1E9DA`, ash `#B7AEA0`, disabled `#6C6577`, gold `#FFD27A`, ember `#E8832B`, verdigris `#3FD6C0`, HP `#6BE07C → #2FA85A`, MP `#38C8E0 → #2B7FD1`, danger `#EF4B4B`. Five elements: Kim `#E9ECF5`, Mộc `#62D26F`, Thủy `#4C9DFF`, Hỏa `#FF5A4A`, Thổ `#D6A45C`.

Shape: asymmetrical cut-corner polygon or shard with 8–12 logical px corner cuts, offset upper highlight, ember seam, dark gradient glass. No thin-outline-only controls: all buttons have solid fill and >48×48 px hit areas at 1080p. Mouse cursor should remain visible.

Type (all SIL OFL): Noto Serif Display SemiBold for display 32+ logical px; Noto Sans Condensed SemiBold for HUD/buttons; Noto Sans Medium for descriptions and Vietnamese. Sizes: game title 76, screen title 38–48, sections 28, buttons 22–28, body 18–20, caption ≥16 at 1080p. UTF-8 precomposed labels (no uppercase conversion at runtime), explicit Vietnamese codepoints, shadow at 1 logical px. FontTest asset also includes an offline visual glyph sample.

## Wireframes (schematic)

Start menu: `╱ THE FIRST STEP ╲` at top over art + dark scrim; `⟪ Bắt đầu ⟫` / `⟪ Thoát ⟫` as broad glass shards in central column; `VI | EN` toggle in a corner.

Hero select: `╱ CHỌN ANH HÙNG ╲` | portrait grid across left 2/3 | right glass detail column with stat radar & skill summaries | `Quay lại` / `Xác nhận` base bar.

Prep menu: horizontal shard tabs `Đội hình | Kỹ năng | Túi đồ | Thông tin công`; 3 sizable hero/slot panels or item tiles; anchored CTA `Quay lại` / `Vào trận`.

Battle: top translucent timeline `(active) ◇—◇—◇—◇ [Màn • Lượt]`; field middle deliberately free of hovering plates; bottom-left log ticker/expand; 3 party cards in bottom middle; right vertical `Tấn công / Kỹ năng / Vật phẩm / Hủy` command panel.

Pause & results: center large shard dialog with strong title and 2 sizable buttons, surroundings dimmed.

## Acceptance evidence

Font coverage is validated with fontTools against a strict Vietnamese character test list (ASCII, tonal characters, punctuation); build and runtime status will be reported distinctly. Sprite art remains unchanged and cannot be judged from the missing referenced screenshots.
