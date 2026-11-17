#pragma once
// src/graphics/SettingsScreen.h
// MAN HINH CAI DAT (ve chong len menu): Am thanh / Ngon ngu / Cot truyen & Boi canh / Thanh vien.
// Chi lo HIEN THI + doc input, doc/ghi truc tiep GameSettings::get(); GameEngine ap dung (am luong, shader) khi nhan CHANGED.
#include "raylib.h"
#include <string>
#include <vector>

enum class SettingsAction { NONE = 0, CHANGED, SFX_PREVIEW, SAVE, LOAD, DELETE_SAVE, CLOSE };

// Thong tin tab "Luu game" do GameEngine dien (Settings chi hien thi)
struct SaveSummary {
    bool m_exists = false;
    std::string m_savedAt;                 // "17/11/2026 08:59"
    int m_nextWave = 0;
    int m_gold = 0;
    std::vector<std::string> m_heroLines;  // vd "Kael  HP 80  MP 20"
    bool m_canSave = false;                // dang o moc co the luu tay (man Chuan bi giua 2 wave)
};

class SettingsScreen {
public:
    void open();
    bool isOpen() const { return m_open; }
    void close() { m_open = false; }

    void setSaveInfo(const SaveSummary& info) { m_save = info; m_confirmDelete = false; }
    void setMessage(const std::string& text, bool ok) { m_message = text; m_messageOk = ok; m_messageTime = 4.0f; }

    void update(float deltaTime);
    SettingsAction handleInput();
    void draw() const;

    // Nut "Cai dat" nho o goc duoi phai man hinh menu chinh
    static Rectangle menuButtonRect();
    static void drawMenuButton(bool hover);
    // Nut "Tiep tuc" canh nut Cai dat (chi ve khi co file luu)
    static Rectangle continueButtonRect();
    static void drawContinueButton(bool hover);

private:
    enum Tab { TAB_AUDIO = 0, TAB_LANGUAGE, TAB_SAVE, TAB_STORY, TAB_TEAM, TAB_COUNT };

    static Rectangle panelRect();
    static Rectangle tabRect(int index);
    static Rectangle closeRect();
    static Rectangle contentRect();
    static Rectangle sliderTrack(int row);
    static Rectangle toggleRect();
    static Rectangle languageRect(int index);
    static Rectangle saveButtonRect(int index);   // 0 Luu, 1 Tai, 2 Xoa

    void drawAudio(const Rectangle& area) const;
    void drawLanguage(const Rectangle& area) const;
    void drawSave(const Rectangle& area) const;
    void drawStory(const Rectangle& area) const;
    void drawTeam(const Rectangle& area) const;

    bool  m_open = false;
    float m_time = 0.0f;
    int   m_tab = TAB_AUDIO;
    int   m_row = 0;          // dong dang chon o tab Am thanh (0 nhac, 1 hieu ung, 2 shader)
    int   m_drag = -1;        // thanh truot dang keo
    SaveSummary m_save;
    std::string m_message;
    bool  m_messageOk = true;
    float m_messageTime = 0.0f;
    bool  m_confirmDelete = false;   // bam Xoa lan 1 -> cho bam lan 2
};
