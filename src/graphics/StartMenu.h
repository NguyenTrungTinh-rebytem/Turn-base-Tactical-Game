#pragma once
// Member 4 - Huynh Quang Khai | src/graphics/StartMenu.h
// Man hinh Start Menu: nen + tieu de + nut bam (chuot va ban phim).
// Chi lo HIEN THI va doc input menu; viec chuyen state do GameEngine quyet dinh qua MenuAction.
#include "raylib.h"
#include <string>
#include <vector>

// Hanh dong menu tra ve khi nguoi choi bam nut. Sprint 2 se them: CREATE_HERO, SELECT_WAVE...
enum class MenuAction { NONE = 0, START_GAME, EXIT };

class StartMenu {
public:
    StartMenu();
    ~StartMenu();

    // Giu texture GPU => cam copy
    StartMenu(const StartMenu&) = delete;
    StartMenu& operator=(const StartMenu&) = delete;

    // Nap anh nen (PNG 16:9, vd 1280x720). Khong co file -> tu ve nen thu tuc (dem, trang, nui) => khong crash.
    bool loadBackground(const std::string& path);
    // Giai phong texture. Core goi trong shutdown(), TRUOC CloseWindow(). Goi nhieu lan van an toan.
    void unloadAll();

    // Dong trang thai nho o goc duoi trai, vd "Data: CSV OK" (isOk=false -> mau do)
    void setStatusText(const std::string& text, bool isOk);

    void update(float deltaTime);   // animation: fade-in, nhun tieu de, hieu ung nut
    MenuAction handleInput();       // doc chuot + phim, tra ve hanh dong khi bam (NONE neu chua bam)
    void draw() const;              // ve toan bo man hinh menu (goi trong BeginDrawing)

private:
    struct Button {
        std::string m_label;
        MenuAction  m_action = MenuAction::NONE;
        float       m_hover = 0.0f;   // 0..1 de lam muot hieu ung chon
    };

    Rectangle buttonRect(size_t index) const;
    void drawBackground() const;
    void drawProceduralBackground() const;
    void drawTitle() const;
    void drawButtons() const;
    void drawFooter() const;

    std::vector<Button> m_buttons;
    int         m_selected = 0;
    float       m_time = 0.0f;
    Texture2D   m_background{};
    bool        m_hasBackground = false;
    std::string m_statusText;
    bool        m_statusOk = true;
};