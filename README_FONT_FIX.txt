FONT FIX for Windows 11 / MSYS2 UCRT64

1. Copy src/graphics/ui/FontManager.cpp to D:\CODE\Turn-base-Tactical-Game\src\graphics\ui\FontManager.cpp (overwrite).
2. Rebuild with cmake --build build-ucrt64 --parallel 4
3. Game loads Noto fonts if installed, or tries Windows Segoe UI/Georgia fonts.
4. For original Ember & Rune typography, install licensed NotoSans-Medium.ttf and NotoSerifDisplay-SemiBold.ttf under assets/fonts/ via their official publishers.
5. This does not solve source text that was saved in the wrong encoding or overlapping UI labels.
