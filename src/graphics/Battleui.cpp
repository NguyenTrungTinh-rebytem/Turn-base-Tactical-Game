#include "BattleUI.h"
#include "graphics/UiCanvas.h"
#include "graphics/UiText.h"
#include "graphics/UiInput.h"
#include "graphics/UiWidgets.h"
#include "graphics/UiShapes.h"
#include "graphics/UiTheme.h"
#include "graphics/Strings.h"
#include "graphics/UiIcons.h"
#include "entities/Character.h"
#include "entities/Player.h"
#include "entities/Boss.h"
#include "data/Element.h"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace {
constexpr float HUD_SCALE = 1.5f;
constexpr Rectangle TURN_STRIP = UiTheme::turnStripHud;
constexpr Rectangle ROUND_CHIP = UiTheme::roundChipHud;
constexpr Rectangle PARTY_CARDS = UiTheme::partyCardsHud;
constexpr Rectangle COMMAND_PANEL = UiTheme::battleHud;
constexpr Rectangle LOG_BUTTON = UiTheme::battleLogButtonHud;
constexpr Rectangle LOG_PANEL = UiTheme::battleLogHud;
constexpr Rectangle LIST_PANEL = UiTheme::battleListHud;
constexpr float LIST_BOTTOM = 810.0f;
constexpr float LIST_HEADER = 64.0f;
constexpr float LIST_ROW_HEIGHT = 32.0f;
constexpr int SKILL_MAX_VISIBLE = 7;
constexpr int ITEM_MAX_VISIBLE = 7;
constexpr float FIELD_HP_H = 6.0f;
constexpr float FLOAT_SPEED = 45.0f;
constexpr float FLOAT_LIFE = 1.0f;

constexpr Color COL_PANEL = UiTheme::panel;
constexpr Color COL_PANEL_EDGE = UiTheme::ash;
constexpr Color COL_BAR_BG{ 14, 12, 20, 230 };
constexpr Color COL_HP_HIGH = UiTheme::hpHigh;
constexpr Color COL_HP_MID = UiTheme::hpHigh;
constexpr Color COL_HP_LOW = UiTheme::danger;
constexpr Color COL_MP = UiTheme::mpHigh;
constexpr Color COL_DAMAGE = UiTheme::danger;
constexpr Color COL_HEAL = UiTheme::heal;

Rectangle toDesign(Rectangle bounds) {
    return Rectangle{ bounds.x / HUD_SCALE, bounds.y / HUD_SCALE,
                      bounds.width / HUD_SCALE, bounds.height / HUD_SCALE };
}

int visibleRows(int rowCount, int maximum) {
    return std::max(0, std::min(rowCount, maximum));
}

int firstVisibleRow(int selectedIndex, int rowCount, int visible) {
    if (visible <= 0) return 0;
    return std::max(0, std::min(selectedIndex - visible + 1, rowCount - visible));
}

Rectangle rowBounds(int index, int rowCount, int selectedIndex, int maximum) {
    if (index < 0 || index >= rowCount || rowCount <= 0) return {};
    const int visible = visibleRows(rowCount, maximum);
    const int first = firstVisibleRow(selectedIndex, rowCount, visible);
    if (index < first || index >= first + visible) return {};
    Rectangle panel = LIST_PANEL;
    panel.height = std::min(LIST_PANEL.height, LIST_HEADER + visible * LIST_ROW_HEIGHT);
    panel.y = LIST_BOTTOM - panel.height;
    return toDesign(Rectangle{ panel.x + 12.0f,
        panel.y + LIST_HEADER + (index - first) * LIST_ROW_HEIGHT,
        panel.width - 24.0f, LIST_ROW_HEIGHT - 1.0f });
}

float ratioOf(int current, int maximum) {
    if (maximum <= 0) return 0.0f;
    return std::max(0.0f, std::min(1.0f,
        static_cast<float>(current) / static_cast<float>(maximum)));
}

void drawBar(Rectangle bounds, float ratio, Color fill) {
    bounds = toDesign(bounds);
    DrawRectangleRec(bounds, COL_BAR_BG);
    Rectangle active = bounds;
    active.width *= ratio;
    if (active.width > 0.0f) DrawRectangleRec(active, fill);
    DrawRectangleLinesEx(bounds, 1.0f, BLACK);
}

std::string elementKey(Element element) {
    switch (element) {
        case Element::KIM: return "element:kim";
        case Element::MOC: return "element:moc";
        case Element::THUY: return "element:thuy";
        case Element::HOA: return "element:hoa";
        case Element::THO: return "element:tho";
    }
    return "element:kim";
}

float fieldBarWidth(const Character& character, float feetY) {
    const float depth = std::clamp((feetY / UiCanvas::height() - 0.60f) / 0.12f, 0.0f, 1.0f);
    const float perspectiveScale = 1.6f + 0.4f * depth;
    float spriteWidth = 64.0f * perspectiveScale * 1.3f;
    if (dynamic_cast<const Player*>(&character))
        spriteWidth = 128.0f * perspectiveScale * 1.3f;
    if (dynamic_cast<const Boss*>(&character))
        spriteWidth = 64.0f * perspectiveScale * 3.2f;
    const float maximum = dynamic_cast<const Boss*>(&character) ? 240.0f : 180.0f;
    return std::clamp(spriteWidth * 0.8f, 96.0f, maximum);
}

std::string localized(const Character* character) {
    return character ? Strings::localizedName(character->getName()) : std::string{};
}
}

Rectangle BattleUI::turnStripBounds() { return toDesign(TURN_STRIP); }
Rectangle BattleUI::roundChipBounds() { return toDesign(ROUND_CHIP); }
Rectangle BattleUI::partyCardBounds(int index) {
    if (index < 0 || index >= 3) return {};
    return toDesign(Rectangle{ PARTY_CARDS.x + index * 224.0f, PARTY_CARDS.y,
                               212.0f, PARTY_CARDS.height });
}

Rectangle BattleUI::fieldHpBounds(const Character& character, Vector2 feet) {
    const float width = fieldBarWidth(character, feet.y);
    return { feet.x - width * 0.5f, feet.y + 10.0f, width, FIELD_HP_H };
}

Rectangle BattleUI::buttonBounds(BattleUIButton button) {
    switch (button) {
        case BattleUIButton::ATTACK:        return toDesign(Rectangle{ 1440, 858, 440, 48 });
        case BattleUIButton::SKILL:         return toDesign(Rectangle{ 1440, 918, 440, 48 });
        case BattleUIButton::ITEM:          return toDesign(Rectangle{ 1440, 978, 440, 48 });
        case BattleUIButton::END_TURN:      return toDesign(Rectangle{ 1440, 978, 210, 48 });
        case BattleUIButton::UNDO:           return toDesign(Rectangle{ 1660, 978, 220, 48 });
        case BattleUIButton::CANCEL_TARGET: return toDesign(Rectangle{ 1660, 858, 220, 48 });
        case BattleUIButton::PAUSE:         return toDesign(Rectangle{ 1848, 820, 40, 40 });
        case BattleUIButton::LOG_BUTTON:    return logButtonBounds();
    }
    return {};
}

Rectangle BattleUI::controlPanelBounds() { return toDesign(COMMAND_PANEL); }
Rectangle BattleUI::itemMenuBounds() { return listPanelBounds(ITEM_MAX_VISIBLE); }
Rectangle BattleUI::listPanelBounds(int rowCount) {
    const int rows = visibleRows(rowCount, ITEM_MAX_VISIBLE);
    const float height = std::min(LIST_PANEL.height, LIST_HEADER + rows * LIST_ROW_HEIGHT);
    return toDesign(Rectangle{ LIST_PANEL.x, LIST_BOTTOM - height, LIST_PANEL.width, height });
}
Rectangle BattleUI::logButtonBounds() { return toDesign(LOG_BUTTON); }
Rectangle BattleUI::logPanelBounds() { return toDesign(LOG_PANEL); }

Rectangle BattleUI::skillRowBounds(int skillIndex, int skillCount, int selectedIndex) {
    return rowBounds(skillIndex, skillCount, selectedIndex, SKILL_MAX_VISIBLE);
}

Rectangle BattleUI::itemRowBounds(int itemIndex, int itemCount, int selectedIndex) {
    return rowBounds(itemIndex, itemCount, selectedIndex, ITEM_MAX_VISIBLE);
}

void BattleUI::drawHpMpBar(const Character& character, Vector2 anchor) const {
    const int maxHp = character.getStats().m_hp;
    const float hpRatio = ratioOf(character.getCurrentHp(), maxHp);
    const Rectangle hpBounds = fieldHpBounds(character, anchor);
    const float x = hpBounds.x;
    const float y = hpBounds.y;
    const float barWidth = hpBounds.width;
    const Color hpColor = hpRatio > 0.5f ? COL_HP_HIGH :
        (hpRatio > 0.25f ? COL_HP_MID : COL_HP_LOW);
    drawBar(hpBounds, hpRatio, hpColor);
    UiIcons::draw(elementKey(character.getElement()),
                  Rectangle{ x - 11.0f, y - 4.0f, 14.0f, 14.0f });

    const float depth = std::clamp((anchor.y / UiCanvas::height() - 0.60f) / 0.12f, 0.0f, 1.0f);
    const float spriteHeight = (dynamic_cast<const Boss*>(&character) ? 192.0f
        : dynamic_cast<const Player*>(&character) ? 128.0f : 64.0f) *
        (1.6f + 0.4f * depth) * (dynamic_cast<const Boss*>(&character) ? 3.2f * 64.0f / 192.0f : 1.3f);
    const Rectangle hoverBounds{ anchor.x - barWidth * 0.5f,
                                 anchor.y - spriteHeight,
                                 barWidth, spriteHeight };
    const std::string name = localized(&character);
    const char* elementName = Strings::elements[static_cast<int>(character.getElement())];
    UiWidgets::Tooltip(hoverBounds, TextFormat(u8"%s · %s · HP %d/%d · MP %d/%d",
        name.c_str(), elementName, std::max(0, character.getCurrentHp()), maxHp,
        std::max(0, character.getCurrentMp()), character.getStats().m_mp));
}

void BattleUI::drawRoundChip(int stage, const Character* current, int gold) const {
    const Rectangle bounds = roundChipBounds();
    UiShapes::roundedRect(bounds, 8.0f, COL_PANEL, COL_PANEL_EDGE, 1.0f);
    UiText::draw(TextFormat(u8"Màn %d · Lượt", stage), static_cast<int>(bounds.x + 12.0f),
                 static_cast<int>(bounds.y + 10.0f), 17, UiTheme::silver);
    const std::string name = localized(current);
    UiText::draw(name.empty() ? u8"Đang chờ lượt" : name,
                 static_cast<int>(bounds.x + 12.0f), static_cast<int>(bounds.y + 33.0f),
                 13, UiTheme::ashText);
    const Rectangle coin{ bounds.x + bounds.width - 104.0f, bounds.y + 8.0f, 92.0f, 34.0f };
    UiShapes::roundedRect(coin, 8.0f, UiTheme::ink, UiTheme::gold, 1.0f);
    UiText::draw(TextFormat("%d", gold), static_cast<int>(coin.x + 12.0f),
                 static_cast<int>(coin.y + 9.0f), 16, UiTheme::gold);
}

void BattleUI::drawPartyCards(const std::vector<Character*>& party, const Character* current,
                              const Character* selectedAlly) const {
    const int shown = std::min(static_cast<int>(party.size()), 3);
    for (int index = 0; index < shown; ++index) {
        const Character* character = party[static_cast<std::size_t>(index)];
        if (!character) continue;
        const Rectangle bounds = partyCardBounds(index);
        const bool active = character == current;
        const bool selected = character == selectedAlly;
        UiShapes::roundedRect(bounds, 8.0f,
            active ? UiTheme::panelRaised : COL_PANEL,
            selected || active ? UiTheme::crimson : UiTheme::ash,
            selected || active ? 2.0f : 1.0f);

        const std::string name = localized(character);
        UiIcons::draw(elementKey(character->getElement()),
                      Rectangle{ bounds.x + 8.0f, bounds.y + 8.0f, 28.0f, 28.0f });
        UiText::draw(name, static_cast<int>(bounds.x + 42.0f),
                     static_cast<int>(bounds.y + 8.0f), 13,
                     character->isAlive() ? UiTheme::silver : UiTheme::ashText);
        const char* hp = TextFormat(u8"HP %d/%d", std::max(0, character->getCurrentHp()),
                                    character->getStats().m_hp);
        UiText::draw(hp, static_cast<int>(bounds.x + 8.0f),
                     static_cast<int>(bounds.y + 39.0f), 10, UiTheme::silver);
        const float hpRatio = ratioOf(character->getCurrentHp(), character->getStats().m_hp);
        drawBar(Rectangle{ bounds.x * HUD_SCALE + 10.0f, bounds.y * HUD_SCALE + 82.0f,
                           192.0f, 8.0f }, hpRatio, COL_HP_HIGH);
        const char* mp = TextFormat("MP %d/%d", std::max(0, character->getCurrentMp()),
                                    character->getStats().m_mp);
        UiText::draw(mp, static_cast<int>(bounds.x + 8.0f),
                     static_cast<int>(bounds.y + 68.0f), 10, UiTheme::mpHigh);
        drawBar(Rectangle{ bounds.x * HUD_SCALE + 10.0f, bounds.y * HUD_SCALE + 128.0f,
                           192.0f, 6.0f },
                ratioOf(character->getCurrentMp(), character->getStats().m_mp), UiTheme::mpLow);
        UiWidgets::Tooltip(bounds, u8"Chọn đồng đội làm mục tiêu của vật phẩm");
    }
}

void BattleUI::drawControlPanel() const {
    UiShapes::roundedRect(controlPanelBounds(), 8.0f, COL_PANEL, COL_PANEL_EDGE, 1.0f);
}

void BattleUI::drawTurnBanner(int stage, const Character* current, bool isPlayerTurn,
                              bool waitingConfirm) const {
    (void)stage;
    const Rectangle panel = controlPanelBounds();
    const std::string name = localized(current);
    UiText::draw(name.empty() ? u8"Đang chờ lượt" : name,
                 static_cast<int>(panel.x + 14.0f),
                 static_cast<int>(panel.y + 10.0f), 16,
                 isPlayerTurn ? UiTheme::silver : UiTheme::ashText);
    const Rectangle pauseBounds = buttonBounds(BattleUIButton::PAUSE);
    UiWidgets::IconButton(pauseBounds, "action:pause");
    UiWidgets::Tooltip(pauseBounds, u8"Tạm dừng trận đấu");
    if (waitingConfirm)
        UiText::draw(u8"Chờ xác nhận", static_cast<int>(panel.x + 14.0f),
                     static_cast<int>(panel.y + 32.0f), 11, UiTheme::ashText);
}

void BattleUI::drawCommandHelp(const std::string& helpText) const {
    const std::string text = helpText.empty()
        ? std::string(u8"Chọn một hành động.")
        : helpText;
    const Rectangle panel = controlPanelBounds();
    UiText::draw(text, static_cast<int>(panel.x + 12.0f),
                 static_cast<int>(panel.y + panel.height - 18.0f), 11, UiTheme::ashText);
}

void BattleUI::drawBattleControls(bool waitingConfirm, bool selectingTarget,
                                  bool selectingList, bool canUndo) const {
    if (waitingConfirm) {
        UiWidgets::Button(buttonBounds(BattleUIButton::END_TURN), Strings::endTurn);
        UiWidgets::Button(buttonBounds(BattleUIButton::UNDO), Strings::undo, canUndo);
        drawCommandHelp(u8"Hoàn tác chỉ khả dụng trước khi kết thúc lượt.");
    } else if (selectingTarget || selectingList) {
        UiWidgets::Button(buttonBounds(BattleUIButton::CANCEL_TARGET), Strings::cancelTarget);
        drawCommandHelp(selectingTarget ? Strings::targetPrompt : u8"Chọn một mục trong danh sách.");
    } else {
        const Rectangle attackBounds = buttonBounds(BattleUIButton::ATTACK);
        const Rectangle skillBounds = buttonBounds(BattleUIButton::SKILL);
        const Rectangle itemBounds = buttonBounds(BattleUIButton::ITEM);
        UiWidgets::Button(attackBounds, Strings::attack);
        UiWidgets::Button(skillBounds, Strings::skill);
        UiWidgets::Button(itemBounds, Strings::item);
        const Vector2 mouse = UiInput::mousePosition();
        if (CheckCollisionPointRec(mouse, attackBounds))
            drawCommandHelp(u8"Chọn kẻ địch rồi tấn công.");
        else if (CheckCollisionPointRec(mouse, skillBounds))
            drawCommandHelp(u8"Mở danh sách kỹ năng.");
        else if (CheckCollisionPointRec(mouse, itemBounds))
            drawCommandHelp(u8"Mở túi vật phẩm.");
        else
            drawCommandHelp();
    }
}

void BattleUI::drawLogButton(bool open) const {
    const Rectangle bounds = logButtonBounds();
    const bool clicked = UiWidgets::Button(bounds, open ? u8"ĐÓNG" : u8"NHẬT KÝ");
    (void)clicked;
    UiWidgets::Tooltip(bounds, u8"Bật hoặc tắt nhật ký trận đấu");
}

void BattleUI::drawListPanel(const std::string& title, int rowCount, int selectedIndex) const {
    const Rectangle bounds = listPanelBounds(rowCount);
    UiShapes::roundedRect(bounds, 12.0f, COL_PANEL, COL_PANEL_EDGE, 2.0f);
    UiText::draw(title, static_cast<int>(bounds.x + 12.0f),
                 static_cast<int>(bounds.y + 9.0f), 14, GOLD);
    if (rowCount > 0) {
        UiText::draw(TextFormat("%d/%d", selectedIndex + 1, rowCount),
                     static_cast<int>(bounds.x + bounds.width - 48.0f),
                     static_cast<int>(bounds.y + 10.0f), 12, LIGHTGRAY);
    }
}

void BattleUI::drawBattleLog(const std::string& message) const {
    const Rectangle bounds = logPanelBounds();
    UiShapes::roundedRect(bounds, 12.0f, COL_PANEL, COL_PANEL_EDGE, 2.0f);
    UiText::draw(Strings::battleLogTitle, static_cast<int>(bounds.x + 16.0f),
                 static_cast<int>(bounds.y + 12.0f), 15, GOLD);
    if (message.empty()) return;

    const int maxWidth = static_cast<int>(bounds.width - 32.0f);
    std::istringstream words(message);
    std::string word;
    std::string line;
    int lineNumber = 0;
    while (words >> word && lineNumber < 4) {
        const std::string candidate = line.empty() ? word : line + " " + word;
        if (!line.empty() && UiText::measure(candidate, 13) > maxWidth) {
            UiText::draw(line, static_cast<int>(bounds.x + 16.0f),
                         static_cast<int>(bounds.y + 48.0f + lineNumber * 20.0f), 13, RAYWHITE);
            ++lineNumber;
            line = word;
        } else {
            line = candidate;
        }
    }
    if (!line.empty() && lineNumber < 4)
        UiText::draw(line, static_cast<int>(bounds.x + 16.0f),
                     static_cast<int>(bounds.y + 48.0f + lineNumber * 20.0f), 13, RAYWHITE);
}

void BattleUI::drawSkillMenu(const std::vector<Skill>& skills, int selectedIndex, int currentMp) const {
    if (skills.empty()) {
        drawListPanel(Strings::skillPrompt, 0, 0);
        const Rectangle panel = listPanelBounds(0);
        UiText::draw(Strings::noSkill, static_cast<int>(panel.x + 12.0f),
                     static_cast<int>(panel.y + 25.0f), 12, LIGHTGRAY);
        return;
    }
    selectedIndex = std::max(0, std::min(selectedIndex, static_cast<int>(skills.size()) - 1));
    drawListPanel(Strings::skillPrompt, static_cast<int>(skills.size()), selectedIndex);

    const int visible = visibleRows(static_cast<int>(skills.size()), SKILL_MAX_VISIBLE);
    const int first = firstVisibleRow(selectedIndex, static_cast<int>(skills.size()), visible);
    for (int index = first; index < first + visible; ++index) {
        const Skill& skill = skills[static_cast<std::size_t>(index)];
        const std::string name = Strings::localizedName(skill.m_name);
        const char* detail = TextFormat("MP %d  ·  %d%s", skill.m_mpCost, skill.m_power,
            skill.m_type == SkillType::HEAL ? " HP" : "%");
        const bool enabled = currentMp >= skill.m_mpCost;
        const Rectangle row = skillRowBounds(index, static_cast<int>(skills.size()), selectedIndex);
        UiWidgets::ListRow(row, TextFormat("%s  ·  %s", name.c_str(), detail), {},
                           index == selectedIndex, enabled);
        UiWidgets::Tooltip(row,
                           TextFormat(u8"%s · tiêu hao %d MP", name.c_str(), skill.m_mpCost));
    }
}

void BattleUI::drawTargetCursor(Rectangle targetBounds) const {
    const float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(GetTime()) * 5.0f);
    const float centerX = targetBounds.x + targetBounds.width * 0.5f;
    const Vector2 feet{ centerX, targetBounds.y + targetBounds.height };
    const float radius = std::max(18.0f,
        std::min(targetBounds.width * 0.38f, targetBounds.height * 0.14f));
    DrawPolyLines(feet, 24, radius + pulse * 4.0f, 0.0f,
                  UiTheme::crimsonGlow);
    DrawPolyLines(feet, 24, radius - 4.0f, 0.0f,
                  Color{ 255, 59, 59, 110 });
    const float bob = 5.0f + pulse * 3.0f;
    DrawTriangle(Vector2{ centerX - 7.0f, feet.y + bob + 10.0f },
                 Vector2{ centerX, feet.y + bob },
                 Vector2{ centerX + 7.0f, feet.y + bob + 10.0f },
                 UiTheme::crimsonGlow);
}

void BattleUI::spawnFloatingText(Vector2 pos, int value, bool isHeal) {
    FloatingText text;
    text.m_text = TextFormat(isHeal ? "+%d" : "-%d", std::abs(value));
    text.m_pos = pos;
    text.m_color = isHeal ? COL_HEAL : COL_DAMAGE;
    text.m_life = text.m_maxLife = FLOAT_LIFE;
    m_floatingTexts.push_back(std::move(text));
}

void BattleUI::update(float deltaTime) {
    for (FloatingText& text : m_floatingTexts) {
        text.m_life -= deltaTime;
        text.m_pos.y -= FLOAT_SPEED * deltaTime;
    }
    m_floatingTexts.erase(std::remove_if(m_floatingTexts.begin(), m_floatingTexts.end(),
        [](const FloatingText& text) { return text.m_life <= 0.0f; }), m_floatingTexts.end());
}

void BattleUI::drawFloatingTexts() const {
    for (const FloatingText& text : m_floatingTexts) {
        Color color = text.m_color;
        color.a = static_cast<unsigned char>(255 * std::max(0.0f,
            std::min(1.0f, text.m_life / text.m_maxLife)));
        const int size = 26;
        const int x = static_cast<int>(text.m_pos.x - UiText::measure(text.m_text, size) / 2.0f);
        const int y = static_cast<int>(text.m_pos.y);
        UiText::draw(text.m_text, x + 2, y + 2, size, Color{ 0, 0, 0, color.a });
        UiText::draw(text.m_text, x, y, size, color);
    }
}

void BattleUI::unloadAll() { m_floatingTexts.clear(); }
