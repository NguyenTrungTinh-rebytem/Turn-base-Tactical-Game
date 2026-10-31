#!/usr/bin/env python3
"""Regenerate optional ornamental textures and a font-coverage specimen.

Dependencies: Pillow, fonttools (optional for codepoint audit).
Run from the repository root: python tools/gen_ui_assets.py
All results are PNG files. Revision 3: muted highlights; icons are procedural
in UiWidgets.cpp, so no external/third-party icon sprite assets are needed.
"""
from __future__ import annotations
import math
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'assets/ui'
FONTS = ROOT / 'assets/fonts'
OUT.mkdir(parents=True,exist_ok=True)

def fract(x: float)->float: return x-math.floor(x)
def rand(i: int)->float: return fract(math.sin(i*12.9898+78.233)*43758.5453)

def texture():
    w=h=256
    im=Image.new('RGBA',(w,h));p=im.load()
    for y in range(h):
        for x in range(w):
            n=rand(y*w+x)
            p[x,y]=(210,169,127,int(3+n*9))
    im.save(OUT/'obsidian_grain.png')
    # Soft ember bloom to layer behind active medallions.
    bloom=Image.new('RGBA',(128,128));bp=bloom.load()
    for y in range(128):
        for x in range(128):
            d=((x-64)**2+(y-64)**2)**0.5 / 64
            a=max(0,1-d)**2
            bp[x,y]=(195,141,91,round(75*a))
    bloom.save(OUT/'ember_glow.png')

def font_specimen():
    w,h=1920,1080
    im=Image.new('RGB',(w,h),(11,10,16));d=ImageDraw.Draw(im)
    d.rectangle((60,55,1860,1020), fill=(21,18,31), outline=(113,101,116),width=3)
    files={'display':FONTS/'NotoSerifDisplay-SemiBold.ttf',
           'hud':FONTS/'NotoSans-CondensedSemiBold.ttf',
           'body':FONTS/'NotoSans-Medium.ttf'}
    title=ImageFont.truetype(files['display'],72)
    d.text((95,77),'KIỂM TRA FONT  |  OBSIDIAN & EMBER',font=title,fill=(220,194,150),stroke_width=1,stroke_fill=(0,0,0))
    sample='Chú bé quàng khăn đỏ vượt đèo, ngắm ánh trăng và dòng sông.'
    accents='Ấ Ầ Ẩ Ẫ Ậ Ế Ề Ể Ễ Ệ Ố Ồ Ổ Ỗ Ộ Ớ Ờ Ở Ỡ Ợ Ứ Ừ Ử Ữ Ự'
    y=200
    for role,size in [('hud',50),('body',38),('display',42),('hud',32),('body',28)]:
        f=ImageFont.truetype(files[role],size)
        d.text((95,y),f'{role.upper()} / {size}px',font=ImageFont.truetype(files['hud'],24),fill=(117,188,179))
        d.text((320,y-5),sample,font=f,fill=(241,233,218),stroke_width=1,stroke_fill=(0,0,0))
        y+=128
    for j,line in enumerate([accents[:len(accents)//2],accents[len(accents)//2:]]):
        f=ImageFont.truetype(files['body'],36)
        d.text((120,855+j*63),line,font=f,fill=(241,233,218))
    im.save(OUT/'font_test_vietnamese.png')

def audit():
    try:
        from fontTools.ttLib import TTFont
    except ImportError:
        print('fonttools not available; skip Unicode coverage audit')
        return
    needed='Tấn công Kỹ năng Vật phẩm Đội hình Túi đồ Thông tin công Thủ lĩnh Chỉ số Thứ tự lượt Chiến thắng Ấ Ầ Ẩ Ẫ Ậ Ế Ề Ể Ễ Ệ Ố Ồ Ổ Ỗ Ộ Ớ Ờ Ở Ỡ Ợ Ứ Ừ Ử Ữ Ự – — … “ ” ×'
    font_paths={"display":FONTS/"NotoSerifDisplay-SemiBold.ttf", "hud":FONTS/"NotoSans-CondensedSemiBold.ttf", "body":FONTS/"NotoSans-Medium.ttf"}
    for name,path in font_paths.items():
        if not path.is_file(): continue
        font=TTFont(path)
        cps=set().union(*[set(t.cmap) for t in font['cmap'].tables])
        missing=sorted({f'U+{ord(c):04X}' for c in needed if ord(c) not in cps})
        print(name,'OK' if not missing else ', '.join(missing))

if __name__=='__main__':
    texture()
    font_specimen()
    audit()
