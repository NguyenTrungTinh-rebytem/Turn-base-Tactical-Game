#pragma once
// src/graphics/SettingsScreen.h
// MAN HINH CAI DAT (ve chong len menu): Am thanh / Ngon ngu / Cot truyen & Boi canh / Thanh vien.
// Chi lo HIEN THI + doc input, doc/ghi truc tiep GameSettings::get(); GameEngine ap dung (am luong, shader) khi nhan CHANGED.
#include "raylib.h"

enum class SettingsAction { NONE = 0, CHANGED, SFX_PREVIEW, CLOSE };

class SettingsScreen {
public:
    void open();
    bool isOpen() const { return m_open; }
    void close() { m_open = false; }

    void update(float deltaTime);
    SettingsAction handleInput();
    void draw() const;

    // Nut "Cai dat" nho o goc duoi phai man hinh menu chinh
    static Rectangle menuButtonRect();
    static void drawMenuButton(bool hover);

private:
    enum Tab { TAB_AUDIO = 0, TAB_LANGUAGE, TAB_STORY, TAB_TEAM, TAB_COUNT };

    static Rectangle panelRect();
    static Rectangle tabRect(int index);
    static Rectangle closeRect();
    static Rectangle contentRect();
    static Rectangle sliderTrack(int row);
    static Rectangle toggleRect();
    static Rectangle languageRect(int index);

    void drawAudio(const Rectangle& area) const;
    void drawLanguage(const Rectangle& area) const;
    void drawStory(const Rectangle& area) const;
    void drawTeam(const Rectangle& area) const;

    bool  m_open = false;
    float m_time = 0.0f;
    int   m_tab = TAB_AUDIO;
    int   m_row = 0;          // dong dang chon o tab Am thanh (0 nhac, 1 hieu ung, 2 shader)
    int   m_drag = -1;        // thanh truot dang keo
};
