#!/usr/bin/env python3
"""Generate the game's small, dependency-free pixel UI icon set."""

import argparse
import hashlib
import struct
import zlib
from pathlib import Path

ICON_KEYS = [
    "item:ITM_HP_MINOR", "item:ITM_HP_MAJOR", "item:ITM_ENERGY", "item:ITM_PHOENIX",
    "item:ITM_POWER_SERUM", "item:ITM_IRON_SHIELD", "item:ITM_HASTE", "item:ITM_PRECISION",
    "item:ITM_LEFTOVER", "item:ITM_VAMPIRIC", "item:ITM_SASH", "item:ITM_GUARDIAN",
    "item:ITM_BERSERK", "item:ITM_EXECUTIONER", "item:ITM_HEAVY_STONE", "item:ITM_COUNTER",
    "skill:R_OGRE_1", "skill:R_OGRE_2", "skill:R_HUMAN_1", "skill:R_HUMAN_2",
    "skill:R_DWARF_1", "skill:R_DWARF_2", "skill:R_ELF_1", "skill:R_ELF_2",
    "skill:R_ONI_1", "skill:R_ONI_2", "skill:R_BEAST_1", "skill:R_BEAST_2",
    "skill:C_MAGE_1", "skill:C_MAGE_2", "skill:C_SWORD_1", "skill:C_SWORD_2",
    "skill:C_ASSASSIN_1", "skill:C_ASSASSIN_2", "skill:C_TANKER_1", "skill:C_TANKER_2",
    "skill:C_ENCHANT_1", "skill:C_ENCHANT_2", "skill:C_PALADIN_1", "skill:C_PALADIN_2",
    "action:attack", "action:skill", "action:item", "action:confirm", "action:undo",
    "action:end_turn", "action:close", "action:shop", "action:target",
    "element:kim", "element:moc", "element:thuy", "element:hoa", "element:tho",
]

SIZE = 32
TRANSPARENT = (0, 0, 0, 0)
INK = (17, 24, 39, 255)
WHITE = (255, 255, 255, 255)


def pixel(canvas, x, y, color):
    if 0 <= x < SIZE and 0 <= y < SIZE:
        canvas[y][x] = color


def rect(canvas, x0, y0, x1, y1, color):
    for y in range(y0, y1):
        for x in range(x0, x1):
            pixel(canvas, x, y, color)


def line(canvas, x0, y0, x1, y1, color):
    dx, dy = abs(x1 - x0), -abs(y1 - y0)
    sx, sy = (1 if x0 < x1 else -1), (1 if y0 < y1 else -1)
    error = dx + dy
    while True:
        pixel(canvas, x0, y0, color)
        if x0 == x1 and y0 == y1:
            break
        doubled = 2 * error
        if doubled >= dy:
            error += dy
            x0 += sx
        if doubled <= dx:
            error += dx
            y0 += sy


def circle(canvas, cx, cy, radius, color):
    for y in range(cy - radius, cy + radius + 1):
        for x in range(cx - radius, cx + radius + 1):
            if (x - cx) ** 2 + (y - cy) ** 2 <= radius ** 2:
                pixel(canvas, x, y, color)


def png_bytes(canvas):
    raw = b"".join(b"\0" + bytes(channel for rgba in row for channel in rgba) for row in canvas)

    def chunk(name, data):
        return (struct.pack(">I", len(data)) + name + data +
                struct.pack(">I", zlib.crc32(name + data) & 0xFFFFFFFF))

    return (b"\x89PNG\r\n\x1a\n" +
            chunk(b"IHDR", struct.pack(">IIBBBBB", SIZE, SIZE, 8, 6, 0, 0, 0)) +
            chunk(b"IDAT", zlib.compress(raw, 9)) +
            chunk(b"IEND", b""))


def draw_icon(key):
    canvas = [[TRANSPARENT for _ in range(SIZE)] for _ in range(SIZE)]
    digest = hashlib.sha256(key.encode("ascii")).digest()
    color = (96 + digest[0] % 140, 96 + digest[1] % 140, 96 + digest[2] % 140, 255)
    rect(canvas, 4, 4, 28, 28, INK)
    rect(canvas, 6, 6, 26, 26, color)

    category = key.split(":", 1)[0]
    if category == "item":
        rect(canvas, 12, 8, 20, 12, WHITE)
        rect(canvas, 10, 12, 22, 23, WHITE)
        rect(canvas, 12, 23, 20, 25, INK)
        line(canvas, 12, 17, 19, 17, color)
        line(canvas, 13, 19, 18, 19, color)
    elif category == "skill":
        points = [(16, 8), (18, 13), (24, 13), (20, 17), (22, 23),
                  (16, 20), (10, 23), (12, 17), (8, 13), (14, 13)]
        for index, (x0, y0) in enumerate(points):
            x1, y1 = points[(index + 1) % len(points)]
            line(canvas, x0, y0, x1, y1, WHITE)
        circle(canvas, 16, 16, 3, INK)
    elif category == "action":
        line(canvas, 10, 22, 22, 10, WHITE)
        line(canvas, 9, 20, 12, 23, INK)
        line(canvas, 19, 9, 23, 13, WHITE)
        line(canvas, 8, 24, 12, 20, WHITE)
        rect(canvas, 20, 8, 24, 12, color)
    else:
        element_shapes = {
            "kim": lambda: circle(canvas, 16, 16, 7, WHITE),
            "moc": lambda: (line(canvas, 16, 23, 16, 10, WHITE),
                            line(canvas, 16, 15, 10, 10, WHITE),
                            line(canvas, 16, 18, 22, 12, WHITE)),
            "thuy": lambda: (line(canvas, 16, 8, 10, 18, WHITE),
                             line(canvas, 10, 18, 12, 23, WHITE),
                             line(canvas, 12, 23, 20, 23, WHITE),
                             line(canvas, 20, 23, 22, 18, WHITE),
                             line(canvas, 22, 18, 16, 8, WHITE)),
            "hoa": lambda: (circle(canvas, 16, 10, 4, WHITE),
                            circle(canvas, 10, 17, 4, WHITE),
                            circle(canvas, 22, 17, 4, WHITE),
                            circle(canvas, 16, 21, 4, WHITE)),
            "tho": lambda: (rect(canvas, 10, 19, 22, 23, WHITE),
                            rect(canvas, 12, 12, 20, 19, WHITE),
                            line(canvas, 8, 24, 24, 24, WHITE)),
        }
        element_shapes[key.split(":", 1)[1]]()

    # A two-bit marker gives each icon a stable, visible per-item identity.
    marker = digest[3] % 4
    rect(canvas, 7 + marker * 5, 7, 9 + marker * 5, 9, INK)
    return png_bytes(canvas)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", type=Path, default=Path("assets/ui/icons"))
    args = parser.parse_args()
    for key in ICON_KEYS:
        category, name = key.split(":", 1)
        destination = args.output / category / f"{name}.png"
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(draw_icon(key))
    print(f"Generated {len(ICON_KEYS)} UI icons in {args.output}")


if __name__ == "__main__":
    main()
