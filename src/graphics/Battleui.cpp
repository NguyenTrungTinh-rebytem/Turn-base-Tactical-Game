#include "BattleUI.h"
#include "graphics/UiCanvas.h"
#include "graphics/UiText.h"
#include "graphics/UiInput.h"
#include "graphics/UiWidgets.h"
#include "graphics/UiShapes.h"
#include "graphics/UiTheme.h"
#include "graphics/Strings.h"
#include "graphics/UiIcons.h"
#include "graphics/BattleAnchors.h"
#include "entities/Character.h"
#include "entities/Player.h"
#include "entities/Boss.h"
#include "data/Element.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <sstream>
#include <iostream>
#include <string>
#include <unordered_set>

namespace {
constexpr float HUD_SCALE = 1.5f;
constexpr Rectangle TURN_STRIP = UiTheme::turnStripHud;
constexpr Rectangle ROUND_CHIP = UiTheme::roundChipHud;
constexpr Rectangle PARTY_CARDS = UiTheme::partyCardsHud;
constexpr Rectangle COMMAND_PANEL = UiTheme::battleHud;
constexpr Rectangle LOG_BUTTON = UiTheme::battleLogButtonHud;
constexpr Rectangle LOG_PANEL = UiTheme::battleLogHud;
constexpr Rectangle LIST_PANEL = UiTheme::battleListHud;
constexpr float MENU_X = 16.0f, MENU_Y = 16.0f, MENU_W = 264.0f;
constexpr float ROW_H = 34.0f, ROW_GAP = 4.0f, MENU_PAD = 8.0f;
Rectangle menuRow(int i) {
    return { MENU_X + MENU_PAD, MENU_Y + MENU_PAD + i * (ROW_H + ROW_GAP),
             MENU_W - MENU_PAD * 2.0f, ROW_H };
}
constexpr float LIST_BOTTOM = 930.0f;
constexpr float LIST_HEADER = 64.0f;
constexpr float LIST_ROW_HEIGHT = 32.0f;
constexpr int SKILL_MAX_VISIBLE = 7;
constexpr int ITEM_MAX_VISIBLE = 7;
Rectangle listPlate(int visible) {
    return { MENU_X, menuRow(1).y - ROW_GAP, MENU_W,
             visible * (ROW_H + ROW_GAP) + ROW_GAP };
}

void drawRowHighlight(Rectangle row) {
    DrawRectangleRec(row, Color{ 0, 150, 190, 120 });
    DrawRectangle(static_cast<int>(row.x), static_cast<int>(row.y),
                  3, static_cast<int>(row.height), UiTheme::cyan);
    DrawPoly(Vector2{ row.x + row.width - 12.0f, row.y + row.height * 0.5f },
             3, 5.0f, 0.0f, UiTheme::cyan);
}
Rectangle animatedRow(float& highlightY, Rectangle target) {
    if (highlightY < 0.0f) highlightY = target.y;
    const float k = std::min(1.0f, GetFrameTime() * 18.0f);
    highlightY += (target.y - highlightY) * k;
    return Rectangle{ target.x, highlightY, target.width, target.height };
}
constexpr float FLOAT_SPEED = 45.0f;
constexpr float FLOAT_LIFE = 1.0f;

constexpr Color COL_PANEL = UiTheme::navyPanel;
constexpr Color COL_PANEL_EDGE = UiTheme::cyanEdge;
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
    UiShapes::runeBar(bounds, ratio, fill);
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

float fieldBarWidth(const Character& character, float spriteWidth) {
    (void)character;
    return std::max(40.0f, spriteWidth * 0.7f);   // 70% chiều rộng sprite
}

std::string localized(const Character* character) {
    return character ? Strings::localizedName(character->getName()) : std::string{};
}
}

Rectangle BattleUI::turnStripBounds() { return toDesign(TURN_STRIP); }
Rectangle BattleUI::roundChipBounds() { return toDesign(ROUND_CHIP); }
Rectangle BattleUI::partyCardBounds(int index) {
    if (index < 0 || index >= 3) return {};
    constexpr float w = 120.0f, h = 76.0f, gap = 6.0f;
    const float x0 = 16.0f;
    return { x0 + index * (w + gap), 720.0f - 16.0f - h, w, h };
}

Rectangle BattleUI::fieldHpBounds(const Character& character, Vector2 feet) {
    const float width = fieldBarWidth(character, 128.0f);
    return { feet.x - width * 0.5f, feet.y + UiTheme::fieldBarOffsetY, width, 6.0f };
}

Rectangle BattleUI::fieldHpBounds(const Character& character, const BattleAnchor& anchor) {
    const float width = fieldBarWidth(character, anchor.spriteBounds.width);
    return { anchor.feet.x - width * 0.5f, anchor.feet.y + UiTheme::fieldBarOffsetY, width, 6.0f };
}

Rectangle BattleUI::buttonBounds(BattleUIButton button) {
    switch (button) {
        case BattleUIButton::ATTACK:        return menuRow(0);
        case BattleUIButton::SKILL:         return menuRow(1);
        case BattleUIButton::ITEM:          return menuRow(2);
        case BattleUIButton::PAUSE:         return menuRow(3);   // nút "THOÁT"
        case BattleUIButton::END_TURN:      return menuRow(0);
        case BattleUIButton::UNDO:          return menuRow(1);
        case BattleUIButton::CANCEL_TARGET: return menuRow(0);
        case BattleUIButton::LOG_BUTTON:    return logButtonBounds();
    }
    return {};
}

Rectangle BattleUI::controlPanelBounds() { return { MENU_X, MENU_Y, MENU_W, MENU_PAD * 2.0f + 4.0f * ROW_H + 3.0f * ROW_GAP }; }
Rectangle BattleUI::itemMenuBounds() { return listPlate(ITEM_MAX_VISIBLE); }
Rectangle BattleUI::listPanelBounds(int rowCount) {
    const int rows = visibleRows(rowCount, ITEM_MAX_VISIBLE);
    const float height = std::min(LIST_PANEL.height, LIST_HEADER + rows * LIST_ROW_HEIGHT);
    return toDesign(Rectangle{ LIST_PANEL.x, LIST_BOTTOM - height, LIST_PANEL.width, height });
}
Rectangle BattleUI::logButtonBounds() { return {}; }
Rectangle BattleUI::logPanelBounds() { return toDesign(LOG_PANEL); }

Rectangle BattleUI::skillRowBounds(int skillIndex, int skillCount, int selectedIndex) {
    const int visible = visibleRows(skillCount, SKILL_MAX_VISIBLE);
    const int first = firstVisibleRow(selectedIndex, skillCount, visible);
    if (skillIndex < first || skillIndex >= first + visible) return {};
    return menuRow(skillIndex - first + 1);       // hàng 0 là nút HỦY
}

Rectangle BattleUI::itemRowBounds(int itemIndex, int itemCount, int selectedIndex) {
    const int visible = visibleRows(itemCount, ITEM_MAX_VISIBLE);
    const int first = firstVisibleRow(selectedIndex, itemCount, visible);
    if (itemIndex < first || itemIndex >= first + visible) return {};
    return menuRow(itemIndex - first + 1);
}

void BattleUI::drawHpMpBar(const Character& character, Vector2 head, int enemyIndex) const {    
    if (dynamic_cast<const Player*>(&character)) return;   // hero: không vẽ thanh máu trên sân
    const int maxHp = character.getStats().m_hp;
    const float hpRatio = ratioOf(character.getCurrentHp(), maxHp);
    const bool boss = dynamic_cast<const Boss*>(&character) != nullptr;
    const bool player = dynamic_cast<const Player*>(&character) != nullptr;
    const int frameSize = boss ? 192 : player ? 128 : 64;
    const float scaleMultiplier = boss ? 1.0666667f : player ? 0.78f : 1.3f;
    const BattleAnchor anchor = BattleAnchors::fromHead(head, frameSize, frameSize,
                                                        scaleMultiplier);
    const Rectangle hpBounds = fieldHpBounds(character, anchor);
    const float x = hpBounds.x;
    const float y = hpBounds.y;
    const float barWidth = hpBounds.width;
    const Color base = player ? UiTheme::hpHigh : UiTheme::hpEnemy;
    const Color hpColor = hpRatio > 0.25f ? base : UiTheme::danger;
    UiShapes::runeBar(hpBounds, hpRatio, hpColor);
    if (enemyIndex >= 0) {
        const char label[2] = { static_cast<char>('A' + enemyIndex % 26), '\0' };
        UiText::draw(label, static_cast<int>(x - 14.0f), static_cast<int>(y - 4.0f), 13,
                     UiTheme::hpEnemy);
    } else {
        const Rectangle sealBounds{ x - 13.0f, y - 5.0f, 16.0f, 16.0f };
        UiShapes::seal(sealBounds, UiTheme::ash);
        UiIcons::draw(elementKey(character.getElement()),
                      Rectangle{ sealBounds.x + 3.0f, sealBounds.y + 3.0f, 10.0f, 10.0f });
    }

    const Rectangle hoverBounds = anchor.spriteBounds;
#if defined(UI_DEV_TOOLS) && UI_DEV_TOOLS
    static std::unordered_set<const Character*> logged;
    static std::unordered_set<const Character*> violations;
    const Character* unitId = &character;
    const float centerX = hpBounds.x + hpBounds.width * 0.5f;
    const float barGap = hpBounds.y - anchor.feet.y;
    const bool valid = std::abs(centerX - anchor.feet.x) <= 4.0f &&
                       barGap >= 8.0f && barGap <= 40.0f;
    if (logged.insert(unitId).second) {
        const float windowX = static_cast<float>(GetScreenWidth()) /
                              static_cast<float>(UiCanvas::width());
        const float windowY = static_cast<float>(GetScreenHeight()) /
                              static_cast<float>(UiCanvas::height());
        const auto logRect = [](const char* space, Rectangle bounds, float scaleX,
                                float scaleY) {
            std::cerr << space << '=' << bounds.x * scaleX << ',' << bounds.y * scaleY
                      << ',' << bounds.width * scaleX << ',' << bounds.height * scaleY;
        };
        std::cerr << "[UI anchor] unit-id=" << static_cast<const void*>(unitId)
                  << " name=" << character.getName()
                  << " feet(canvas 1280x720)=" << anchor.feet.x << ',' << anchor.feet.y
                  << " feet(canvas 1920x1080)=" << anchor.feet.x * 1.5f << ','
                  << anchor.feet.y * 1.5f << " feet(window px)="
                  << anchor.feet.x * windowX << ',' << anchor.feet.y * windowY
                  << " slot(pre-layout canvas)=unavailable; estimated from core head API ";
        logRect("sprite(canvas 1280x720)", anchor.spriteBounds, 1.0f, 1.0f);
        std::cerr << ' ';
        logRect("sprite(canvas 1920x1080)", anchor.spriteBounds, 1.5f, 1.5f);
        std::cerr << ' ';
        logRect("sprite(window px)", anchor.spriteBounds, windowX, windowY);
        std::cerr << ' ';
        logRect("bar(canvas 1280x720)", hpBounds, 1.0f, 1.0f);
        std::cerr << ' ';
        logRect("bar(canvas 1920x1080)", hpBounds, 1.5f, 1.5f);
        std::cerr << ' ';
        logRect("bar(window px)", hpBounds, windowX, windowY);
        std::cerr << " source=head(canvas 1280x720)=" << head.x << ',' << head.y
                  << " scale-multiplier=" << scaleMultiplier << '\n';
    }
    if (!valid && violations.insert(unitId).second)
        std::cerr << "[UI anchor violation] unit-id=" << static_cast<const void*>(unitId)
                  << " name=" << character.getName()
                  << " center-error=" << std::abs(centerX - anchor.feet.x)
                  << " feet-gap=" << barGap << '\n';
    assert(valid && "field HP bar is not anchored to its combatant feet");
#endif
    const std::string name = localized(&character);
    const char* elementName = Strings::elements[static_cast<int>(character.getElement())];
    UiWidgets::Tooltip(hoverBounds, TextFormat("%s · %s · HP %d/%d · MP %d/%d",
        name.c_str(), elementName, std::max(0, character.getCurrentHp()), maxHp,
        std::max(0, character.getCurrentMp()), character.getStats().m_mp));
}

void BattleUI::drawRoundChip(int stage, const Character* current, int gold) const {
    if (m_lastRoundStage != stage) {
        m_lastRoundStage = stage;
        m_lastRoundCharacter = nullptr;
        m_turnNumber = 0;
    }
    if (current && current != m_lastRoundCharacter) {
        m_lastRoundCharacter = current;
        ++m_turnNumber;
    }
    const std::string round = TextFormat(Strings::roundTurn, stage,
                                         std::max(1, m_turnNumber));
    const std::string coinText = TextFormat(Strings::gold, gold);
    const float rowWidth = static_cast<float>(UiText::measure(round, 16));
    const float coinWidth = static_cast<float>(UiText::measure(coinText, 16)) + 42.0f;
    const float chipWidth = std::max(160.0f, std::max(rowWidth, coinWidth) + 24.0f);
    const Rectangle bounds{ 1280.0f - 16.0f - chipWidth, 84.0f, chipWidth, 52.0f };
    UiShapes::bladeStrip(bounds, COL_PANEL, COL_PANEL_EDGE, true);
    UiText::draw(round, static_cast<int>(bounds.x + bounds.width - rowWidth - 12.0f),
                 static_cast<int>(bounds.y + 5.0f), 16, UiTheme::silver);
    const float goldIconSize = 16.0f;
    const float coinRowWidth = coinWidth;
    const Rectangle coin{ bounds.x + bounds.width - coinRowWidth - 8.0f,
                          bounds.y + 29.0f, coinRowWidth, 20.0f };
    UiShapes::bladeStrip(coin, UiTheme::ink, UiTheme::gold, true);
    UiIcons::draw("action:coin", Rectangle{ coin.x + 6.0f, coin.y + 2.0f,
                                             goldIconSize, goldIconSize }, UiTheme::gold);
    UiText::draw(coinText, static_cast<int>(coin.x + coin.width -
                     UiText::measure(coinText, 16) - 7.0f),
                 static_cast<int>(coin.y + 2.0f), 16, UiTheme::gold);
}

void BattleUI::drawPartyCards(const std::vector<Character*>& party, const Character* current,
                              const Character* selectedAlly) const {
    const int shown = std::min(static_cast<int>(party.size()), 3);
    for (int index = 0; index < shown; ++index) {
        const Character* character = party[static_cast<std::size_t>(index)];
        if (!character) continue;
        const Rectangle b = partyCardBounds(index);
        const bool hot = character == current || character == selectedAlly;
        UiShapes::plate(b, COL_PANEL, hot ? UiTheme::cyan : COL_PANEL_EDGE, hot ? 2.0f : 1.0f);

        const int hp = std::max(0, character->getCurrentHp());
        const int mp = std::max(0, character->getCurrentMp());
        const int maxHp = character->getStats().m_hp;
        const int maxMp = character->getStats().m_mp;
        UiText::draw(localized(character), static_cast<int>(b.x + 8.0f),
                     static_cast<int>(b.y + 6.0f), 12,
                     character->isAlive() ? UiTheme::silver : UiTheme::ashText);
        UiText::draw(TextFormat("HP %d/%d", hp, maxHp), static_cast<int>(b.x + 8.0f),
                     static_cast<int>(b.y + 22.0f), 10, UiTheme::silver);
        UiShapes::runeBar(Rectangle{ b.x + 8.0f, b.y + 36.0f, b.width - 16.0f, 6.0f },
                          ratioOf(hp, maxHp), UiTheme::hpHigh);
        UiText::draw(TextFormat("MP %d/%d", mp, maxMp), static_cast<int>(b.x + 8.0f),
                     static_cast<int>(b.y + 46.0f), 10, UiTheme::mpHigh);
        UiShapes::runeBar(Rectangle{ b.x + 8.0f, b.y + 60.0f, b.width - 16.0f, 5.0f },
                          ratioOf(mp, maxMp), UiTheme::mpLow);
        UiWidgets::Tooltip(b, "Chọn đồng đội làm mục tiêu của vật phẩm");
    }
}

void BattleUI::drawControlPanel() const {
    UiShapes::plate(controlPanelBounds(), COL_PANEL, COL_PANEL_EDGE, 1.0f);
}

void BattleUI::drawTurnBanner(int, const Character*, bool, bool) const {}   // thông tin đã nằm ở bảng đội


void BattleUI::drawCommandHelp(const std::string& helpText) const {
    const Rectangle menu = controlPanelBounds();
    const Rectangle box{ menu.x, menu.y + menu.height + 6.0f, menu.width, 64.0f };
    UiShapes::plate(box, COL_PANEL, COL_PANEL_EDGE, 1.0f);
    const std::string text = helpText.empty() ? std::string("Chọn một hành động.") : helpText;
    const int maxWidth = static_cast<int>(box.width - 20.0f);
    std::istringstream words(text);
    std::string word, line;
    int lineNumber = 0;
    const auto flush = [&](const std::string& value) {
        UiText::draw(value, static_cast<int>(box.x + 10.0f),
                     static_cast<int>(box.y + 8.0f + lineNumber * 18.0f), 12, UiTheme::ashText);
        ++lineNumber;
    };
    while (words >> word && lineNumber < 3) {
        const std::string candidate = line.empty() ? word : line + " " + word;
        if (!line.empty() && UiText::measure(candidate, 12) > maxWidth) {
            flush(line);
            line = word;
        } else {
            line = candidate;
        }
    }
    if (!line.empty() && lineNumber < 3) flush(line);
}
void BattleUI::drawBattleControls(bool waitingConfirm, bool selectingTarget,
                                  bool selectingList, bool canUndo) const {
    if (!selectingList) m_highlightY = -1.0f;
    const Vector2 mouse = UiInput::mousePosition();
    std::string help;
    const auto row = [&](BattleUIButton id, const char* label, const char* action,
                         bool enabled, const char* hint) {
        const Rectangle b = buttonBounds(id);
        const bool hot = enabled && CheckCollisionPointRec(mouse, b);
        UiWidgets::BladeButton(b, label, true, hot, enabled, action);
        if (hot) help = hint;
    };

    if (waitingConfirm) {
        row(BattleUIButton::END_TURN, Strings::endTurn, "action:end_turn", true,
            "Kết thúc lượt của nhân vật này.");
        row(BattleUIButton::UNDO, Strings::undo, "action:undo", canUndo,
            "Hoàn tác chỉ khả dụng trước khi kết thúc lượt.");
    } else if (selectingTarget || selectingList) {
        row(BattleUIButton::CANCEL_TARGET, Strings::cancelTarget, "action:close", true,
            "Quay lại menu lệnh.");
        if (help.empty())
            help = selectingTarget ? Strings::targetPrompt : "Chọn một mục trong danh sách.";
    } else {
        row(BattleUIButton::ATTACK, Strings::attack, "action:attack", true,
            "Chọn kẻ địch rồi tấn công.");
        row(BattleUIButton::SKILL, Strings::skill, "action:skill", true,
            "Mở danh sách kỹ năng.");
        row(BattleUIButton::ITEM, Strings::item, "action:item", true,
            "Mở túi vật phẩm.");
        row(BattleUIButton::PAUSE, "THOÁT", "action:pause", true,
            "Tạm dừng và thoát trận đấu.");
    }
    if (!selectingList) drawCommandHelp(help);
}

void BattleUI::drawLogButton(bool) const {}

void BattleUI::drawListPanel(const std::string&, int, int) const {}

void BattleUI::drawBattleLog(const std::string& message) const {
    const Rectangle bounds = logPanelBounds();
    UiShapes::plate(bounds, COL_PANEL, COL_PANEL_EDGE, 1.0f);
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
        const Rectangle box = listPlate(1);
        UiShapes::plate(box, COL_PANEL, COL_PANEL_EDGE, 1.0f);
        UiText::draw(Strings::noSkill, static_cast<int>(box.x + 12.0f),
                     static_cast<int>(box.y + 12.0f), 13, UiTheme::ashText);
        return;
    }

    const int count = static_cast<int>(skills.size());
    selectedIndex = std::max(0, std::min(selectedIndex, count - 1));
    const int visible = visibleRows(count, SKILL_MAX_VISIBLE);
    const int first = firstVisibleRow(selectedIndex, count, visible);
    UiShapes::plate(listPlate(visible), COL_PANEL, COL_PANEL_EDGE, 1.0f);

    for (int index = first; index < first + visible; ++index) {
        const Skill& skill = skills[static_cast<std::size_t>(index)];
        const Rectangle row = skillRowBounds(index, count, selectedIndex);
        const bool selected = (index == selectedIndex);
        const bool enabled = currentMp >= skill.m_mpCost;

        if (selected) drawRowHighlight(animatedRow(m_highlightY, row));

        UiIcons::drawSkill(skill.m_element, skill.m_type, skill.m_isMagic,
                           Rectangle{ row.x + 8.0f, row.y + 6.0f, 22.0f, 22.0f });
        UiText::draw(Strings::localizedName(skill.m_name), static_cast<int>(row.x + 38.0f),
                     static_cast<int>(row.y + 3.0f), 13,
                     enabled ? UiTheme::silver : UiTheme::ashText);

        const std::string detail = (skill.m_type == SkillType::HEAL)
            ? "MP " + std::to_string(skill.m_mpCost) + "  ·  +" + std::to_string(skill.m_power) + " HP"
            : "MP " + std::to_string(skill.m_mpCost) + "  ·  " + std::to_string(skill.m_power) + "%";
        UiText::draw(detail, static_cast<int>(row.x + 38.0f), static_cast<int>(row.y + 18.0f), 11,
                     enabled ? UiTheme::mpHigh : UiTheme::danger);
    }
}
void BattleUI::drawItemMenu(const std::vector<std::string>& labels, int selectedIndex) const {
    if (labels.empty()) return;
    const int count = static_cast<int>(labels.size());
    selectedIndex = std::max(0, std::min(selectedIndex, count - 1));
    const int visible = visibleRows(count, ITEM_MAX_VISIBLE);
    const int first = firstVisibleRow(selectedIndex, count, visible);
    UiShapes::plate(listPlate(visible), COL_PANEL, COL_PANEL_EDGE, 1.0f);

    for (int index = first; index < first + visible; ++index) {
        const Rectangle row = itemRowBounds(index, count, selectedIndex);
        const bool selected = (index == selectedIndex);
        if (selected) drawRowHighlight(animatedRow(m_highlightY, row));       
        UiText::draw(labels[static_cast<std::size_t>(index)], static_cast<int>(row.x + 12.0f),
                     static_cast<int>(row.y + 9.0f), 13,
                     selected ? UiTheme::silver : UiTheme::ashText);
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
