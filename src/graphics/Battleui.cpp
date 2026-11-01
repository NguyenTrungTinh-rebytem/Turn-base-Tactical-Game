// Member 4 - Huynh Quang Khai | src/graphics/BattleUI.cpp
#include "BattleUI.h"
#include "graphics/UiCanvas.h"
#include "graphics/UiText.h"
#include "graphics/Strings.h"
#include "entities/Character.h"
#include <algorithm>
#include <cmath>

namespace {
// ---- Bo cuc (thiet ke cho cua so 1280x720) ----
constexpr int   PANEL_X = 20, PANEL_Y = 560, PANEL_W = 1240, PANEL_H = 140;
constexpr int   BANNER_Y = PANEL_Y + 15, HELP_Y = PANEL_Y + 50, LOG_Y = PANEL_Y + 90;
constexpr float BAR_W = 120.0f, HP_H = 12.0f, MP_H = 8.0f;
constexpr float TURN_BOX_W = 170.0f, TURN_BOX_H = 30.0f, TURN_GAP = 4.0f, TURN_MARGIN = 10.0f;
constexpr int   TURN_MAX_SHOWN = 8;
constexpr int   SKILL_BOX_W = 560, SKILL_BOX_Y = 290, SKILL_ROW_H = 35, SKILL_MAX_VISIBLE = 4;
constexpr int   ITEM_BOX_X = 340, ITEM_BOX_Y = 160, ITEM_BOX_W = 600, ITEM_BOX_H = 390;
constexpr float FLOAT_SPEED = 45.0f, FLOAT_LIFE = 1.0f;
constexpr float FRAME_H_REF = 64.0f;     // chieu cao frame tham chieu cho con tro muc tieu

// ---- Mau ----
constexpr Color COL_PANEL      { 15, 15, 30, 220 };
constexpr Color COL_SKILL_BOX  { 0, 40, 100, 240 };
constexpr Color COL_PANEL_EDGE { 200, 170, 80, 255 };
constexpr Color COL_BAR_BG     { 30, 30, 30, 220 };
constexpr Color COL_HP_HIGH    { 60, 200, 80, 255 };
constexpr Color COL_HP_MID     { 240, 200, 50, 255 };
constexpr Color COL_HP_LOW     { 220, 50, 50, 255 };
constexpr Color COL_MP         { 60, 120, 240, 255 };
constexpr Color COL_DAMAGE     { 255, 70, 70, 255 };
constexpr Color COL_HEAL       { 80, 255, 120, 255 };

float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }

float ratioOf(int cur, int maxV) {                       // chia 0 an toan
    return (maxV > 0) ? clamp01(static_cast<float>(cur) / static_cast<float>(maxV)) : 0.0f;
}

void drawBar(Vector2 pos, float w, float h, float ratio, Color fill) {
    Rectangle bg = { pos.x, pos.y, w, h };
    DrawRectangleRec(bg, COL_BAR_BG);
    DrawRectangleRec(Rectangle{ pos.x, pos.y, w * ratio, h }, fill);
    DrawRectangleLinesEx(bg, 1.5f, BLACK);
}

void drawButton(Rectangle bounds, const char* label, bool enabled = true) {
    const Color fill = enabled ? Color{ 45, 70, 105, 245 } : Color{ 55, 55, 65, 220 };
    const Color edge = enabled ? SKYBLUE : DARKGRAY;
    DrawRectangleRec(bounds, fill);
    DrawRectangleLinesEx(bounds, 1.5f, edge);
    const int textSize = 16;
    const int textWidth = UiText::measure(label, textSize);
    UiText::draw(label, static_cast<int>(bounds.x + (bounds.width - textWidth) / 2.0f),
             static_cast<int>(bounds.y + (bounds.height - textSize) / 2.0f),
             textSize, enabled ? RAYWHITE : GRAY);
}

const char* elementName(Element e) {
    switch (e) {
        case Element::KIM:  return Strings::elements[0];
        case Element::MOC:  return Strings::elements[1];
        case Element::THUY: return Strings::elements[2];
        case Element::HOA:  return Strings::elements[3];
        case Element::THO:  return Strings::elements[4];
    }
    return "?";
}

} // namespace

Rectangle BattleUI::buttonBounds(BattleUIButton button) {
    constexpr float y = 607.0f;
    switch (button) {
        case BattleUIButton::ATTACK:        return { 40.0f, y, 128.0f, 36.0f };
        case BattleUIButton::SKILL:         return { 180.0f, y, 128.0f, 36.0f };
        case BattleUIButton::ITEM:          return { 320.0f, y, 128.0f, 36.0f };
        case BattleUIButton::END_TURN:      return { 40.0f, y, 180.0f, 36.0f };
        case BattleUIButton::UNDO:          return { 232.0f, y, 140.0f, 36.0f };
        case BattleUIButton::CANCEL_TARGET: return { 40.0f, y, 200.0f, 36.0f };
    }
    return {};
}

Rectangle BattleUI::controlPanelBounds() {
    return { static_cast<float>(PANEL_X), static_cast<float>(PANEL_Y),
             static_cast<float>(PANEL_W), static_cast<float>(PANEL_H) };
}

Rectangle BattleUI::itemMenuBounds() {
    return { static_cast<float>(ITEM_BOX_X), static_cast<float>(ITEM_BOX_Y),
             static_cast<float>(ITEM_BOX_W), static_cast<float>(ITEM_BOX_H) };
}

Rectangle BattleUI::skillRowBounds(int skillIndex, int skillCount, int selectedIndex) {
    if (skillIndex < 0 || skillIndex >= skillCount || skillCount <= 0) return {};
    const int visible = std::min(skillCount, SKILL_MAX_VISIBLE);
    const int first = std::max(0, std::min(selectedIndex - visible + 1, skillCount - visible));
    if (skillIndex < first || skillIndex >= first + visible) return {};
    const int boxX = (UiCanvas::width() - SKILL_BOX_W) / 2;
    const int rowY = SKILL_BOX_Y + 50 + (skillIndex - first) * SKILL_ROW_H;
    return { static_cast<float>(boxX + 20), static_cast<float>(rowY - 5),
             static_cast<float>(SKILL_BOX_W - 40), static_cast<float>(SKILL_ROW_H) };
}

Rectangle BattleUI::itemRowBounds(int itemIndex) {
    if (itemIndex < 0 || itemIndex >= 9) return {};
    return { static_cast<float>(ITEM_BOX_X + 10),
             static_cast<float>(ITEM_BOX_Y + 80 + itemIndex * 28),
             static_cast<float>(ITEM_BOX_W - 20), 28.0f };
}

// ============ HP / MP tren dau nhan vat ============
void BattleUI::drawHpMpBar(const Character& c, Vector2 anchor) const {
    const int maxHp = c.getStats().m_hp;
    const int maxMp = c.getStats().m_mp;
    const float hpRatio = ratioOf(c.getCurrentHp(), maxHp);

    const float x = anchor.x - BAR_W / 2.0f;
    const float y = anchor.y - 40.0f;

    const std::string name = Strings::localizedName(c.getName());
    UiText::draw(TextFormat("%s [%s]", name.c_str(), elementName(c.getElement())),
             (int)x, (int)y - 16, 14, c.isAlive() ? RAYWHITE : DARKGRAY);

    Color hpColor = (hpRatio > 0.5f) ? COL_HP_HIGH : (hpRatio > 0.25f) ? COL_HP_MID : COL_HP_LOW;
    drawBar({ x, y }, BAR_W, HP_H, hpRatio, hpColor);
    const char* hpText = TextFormat("%d/%d", std::max(0, c.getCurrentHp()), maxHp);
    UiText::draw(hpText, (int)(x + (BAR_W - UiText::measure(hpText, 10)) / 2.0f), (int)(y + (HP_H - 10) / 2.0f), 10, WHITE);

    drawBar({ x, y + HP_H + 3.0f }, BAR_W, MP_H, ratioOf(c.getCurrentMp(), maxMp), COL_MP);
}

// ============ Turn order ============
void BattleUI::drawTurnOrder(const std::vector<Character*>& upcoming, const Character* current) const {
    float x = UiCanvas::width() - TURN_BOX_W - TURN_MARGIN;
    float y = TURN_MARGIN;
    UiText::draw(Strings::turnOrder, (int)x, (int)y, 16, GOLD);
    y += 22.0f;

    int shown = 0;
    for (const Character* c : upcoming) {
        if (!c || shown >= TURN_MAX_SHOWN) break;
        Rectangle box = { x, y, TURN_BOX_W, TURN_BOX_H };
        const bool isCurrent = (c == current);
        const bool alive = c->isAlive();

        DrawRectangleRec(box, isCurrent ? Color{ 90, 70, 20, 230 } : Color{ 25, 25, 40, 200 });
        DrawRectangleLinesEx(box, isCurrent ? 2.5f : 1.0f, isCurrent ? GOLD : GRAY);

        Color textColor = alive ? RAYWHITE : DARKGRAY;
        const std::string name = Strings::localizedName(c->getName());
        UiText::draw(TextFormat("%d. %s", shown + 1, name.c_str()), (int)x + 8, (int)y + 8, 14, textColor);
        const char* spd = TextFormat("SPD %d", c->getSpeed());
        UiText::draw(spd, (int)(x + TURN_BOX_W - UiText::measure(spd, 12) - 8), (int)y + 9, 12, alive ? SKYBLUE : DARKGRAY);

        y += TURN_BOX_H + TURN_GAP;
        ++shown;
    }
}

// ============ Khung dieu khien ============
void BattleUI::drawControlPanel() const {
    Rectangle panel = { (float)PANEL_X, (float)PANEL_Y, (float)PANEL_W, (float)PANEL_H };
    DrawRectangleRec(panel, COL_PANEL);
    DrawRectangleLinesEx(panel, 2.0f, COL_PANEL_EDGE);
}

void BattleUI::drawTurnBanner(int stage, const Character* current, bool isPlayerTurn, bool waitingConfirm) const {
    UiText::draw(TextFormat(Strings::stage, stage), PANEL_X + PANEL_W - 150, BANNER_Y, 20, SKYBLUE);
    UiText::draw(Strings::yourTurn, PANEL_X + 20, BANNER_Y, 20, YELLOW);
    if (!current) return;
    if (isPlayerTurn) {
        const std::string name = Strings::localizedName(current->getName());
        UiText::draw(TextFormat(u8"%s%s", name.c_str(), waitingConfirm ? u8" (Chờ xác nhận)" : ""),
                 PANEL_X + 140, BANNER_Y, 20, GREEN);
    } else {
        UiText::draw(TextFormat(u8"%s (Đang suy nghĩ...)", current->getName().c_str()), PANEL_X + 140, BANNER_Y, 20, RED);
    }
}

void BattleUI::drawCommandHelp(const std::string& helpText) const {
    const char* text = helpText.empty()
        ? u8"[A] Tấn công | [S] Kỹ năng | [I] Vật phẩm | [TAB] Đổi mục tiêu | [ESC] Menu"
        : helpText.c_str();
    UiText::draw(text, PANEL_X + 480, HELP_Y, 18, LIGHTGRAY);
}

void BattleUI::drawBattleControls(bool waitingConfirm, bool selectingTarget, bool canUndo) const {
    if (waitingConfirm) {
        drawButton(buttonBounds(BattleUIButton::END_TURN), Strings::endTurn);
        drawButton(buttonBounds(BattleUIButton::UNDO), Strings::undo, canUndo);
        drawCommandHelp(u8"[ENTER] Kết thúc lượt | [ESC/Z] Hoàn tác");
    } else if (selectingTarget) {
        drawButton(buttonBounds(BattleUIButton::CANCEL_TARGET), Strings::cancelTarget);
        drawCommandHelp(Strings::targetPrompt);
    } else {
        drawButton(buttonBounds(BattleUIButton::ATTACK), Strings::attack);
        drawButton(buttonBounds(BattleUIButton::SKILL), Strings::skill);
        drawButton(buttonBounds(BattleUIButton::ITEM), Strings::item);
        drawCommandHelp();
    }
}

void BattleUI::drawBattleLog(const std::string& message) const {
    if (message.empty()) return;
    UiText::draw(message.c_str(), PANEL_X + 20, LOG_Y, 20, ORANGE);
}

void BattleUI::drawSkillMenu(const std::vector<Skill>& skills, int selectedIndex, int currentMp) const {
    if (skills.empty()) return;
    const int total = static_cast<int>(skills.size());
    selectedIndex = std::max(0, std::min(selectedIndex, total - 1));

    // Cuon de dong dang chon luon nam trong vung nhin thay
    const int visible = std::min(total, SKILL_MAX_VISIBLE);
    const int first = std::max(0, std::min(selectedIndex - visible + 1, total - visible));

    const int boxX = (UiCanvas::width() - SKILL_BOX_W) / 2;
    const int boxH = 70 + visible * SKILL_ROW_H;
    Rectangle box = { (float)boxX, (float)SKILL_BOX_Y, (float)SKILL_BOX_W, (float)boxH };
    DrawRectangleRec(box, COL_SKILL_BOX);
    DrawRectangleLinesEx(box, 2.0f, SKYBLUE);
    UiText::draw(Strings::skillPrompt, boxX + 20, SKILL_BOX_Y + 15, 16, YELLOW);

    int y = SKILL_BOX_Y + 50;
    for (int i = first; i < first + visible; ++i, y += SKILL_ROW_H) {
        const Skill& s = skills[static_cast<size_t>(i)];
        Color color = WHITE;
        if (currentMp < s.m_mpCost) color = GRAY;               // thieu MP => mo
        else if (i == selectedIndex) color = GREEN;

        const char* suffix = (s.m_type == SkillType::HEAL) ? " HP" : "%";
        const std::string skillName = Strings::localizedName(s.m_name);
        UiText::draw(TextFormat("[%d] %s  (MP %d, %d%s)", i + 1, skillName.c_str(), s.m_mpCost, s.m_power, suffix),
                 boxX + 30, y, 18, color);
    }
    if (total > visible)
        UiText::draw(TextFormat("%d/%d", selectedIndex + 1, total), boxX + SKILL_BOX_W - 60, SKILL_BOX_Y + 15, 14, GRAY);
}

void BattleUI::drawTargetCursor(const BattleSlot& slot) const {
    const float bob = std::sin(static_cast<float>(GetTime()) * 6.0f) * 5.0f;   // nhun len xuong
    const float tipY = slot.m_feetPos.y - FRAME_H_REF * slot.m_scale - 14.0f + bob;
    const float cx = slot.m_feetPos.x;
    DrawTriangle(Vector2{ cx - 12.0f, tipY - 18.0f }, Vector2{ cx, tipY }, Vector2{ cx + 12.0f, tipY - 18.0f }, GOLD);
}

// ============ Chu bay ============
void BattleUI::spawnFloatingText(Vector2 pos, int value, bool isHeal) {
    FloatingText ft;
    ft.m_text = TextFormat(isHeal ? "+%d" : "-%d", std::abs(value));
    ft.m_pos = pos;
    ft.m_color = isHeal ? COL_HEAL : COL_DAMAGE;
    ft.m_life = ft.m_maxLife = FLOAT_LIFE;
    m_floatingTexts.push_back(std::move(ft));
}

void BattleUI::update(float deltaTime) {
    for (auto& t : m_floatingTexts) {
        t.m_life -= deltaTime;
        t.m_pos.y -= FLOAT_SPEED * deltaTime;
    }
    m_floatingTexts.erase(
        std::remove_if(m_floatingTexts.begin(), m_floatingTexts.end(),
                       [](const FloatingText& t) { return t.m_life <= 0.0f; }),
        m_floatingTexts.end());
}

void BattleUI::drawFloatingTexts() const {
    for (const auto& t : m_floatingTexts) {
        Color c = t.m_color;
        c.a = static_cast<unsigned char>(255 * clamp01(t.m_life / t.m_maxLife));
        const int size = 26;
        const int w = UiText::measure(t.m_text.c_str(), size);
        const int px = (int)(t.m_pos.x - w / 2.0f), py = (int)t.m_pos.y;
        UiText::draw(t.m_text.c_str(), px + 2, py + 2, size, Color{ 0, 0, 0, c.a });
        UiText::draw(t.m_text.c_str(), px, py, size, c);
    }
}

void BattleUI::unloadAll() { m_floatingTexts.clear(); }