// Member 4 - Huynh Quang Khai | src/graphics/BattleUI.cpp
#include "BattleUI.h"
#include "graphics/UiCanvas.h"
#include "graphics/UiElementSigil.h"
#include "graphics/UiIcons.h"
#include "graphics/UiInput.h"
#include "graphics/UiShapes.h"
#include "graphics/UiText.h"
#include "graphics/UiWidgets.h"
#include "graphics/Strings.h"
#include "graphics/UiTheme.h"
#include "entities/Character.h"
#include "entities/Party.h"
#include "graphics/ItemIcons.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {
constexpr int PANEL_X = static_cast<int>(UiTheme::battleCommandCluster.x);
constexpr int PANEL_Y = static_cast<int>(UiTheme::battleCommandCluster.y);
constexpr int PANEL_W = static_cast<int>(UiTheme::battleCommandCluster.width);
constexpr int PANEL_H = static_cast<int>(UiTheme::battleCommandCluster.height);
constexpr int BANNER_Y = PANEL_Y + 8;
constexpr int HELP_Y = PANEL_Y + 36;
constexpr int SKILL_BOX_X = static_cast<int>(UiTheme::battleSkillList.x);
constexpr int SKILL_BOX_W = static_cast<int>(UiTheme::battleSkillList.width);
constexpr int SKILL_BOX_Y = static_cast<int>(UiTheme::battleSkillList.y);
constexpr int SKILL_ROW_H = 32, SKILL_ROW_STEP = 34, SKILL_MAX_VISIBLE = 7;
constexpr int ITEM_BOX_X = static_cast<int>(UiTheme::battleItemList.x);
constexpr int ITEM_BOX_Y = static_cast<int>(UiTheme::battleItemList.y);
constexpr int ITEM_BOX_W = static_cast<int>(UiTheme::battleItemList.width);
constexpr int ITEM_BOX_H = static_cast<int>(UiTheme::battleItemList.height);
constexpr float FLOAT_SPEED = 45.0f, FLOAT_LIFE = 1.0f;

// ---- Mau ----
constexpr Color COL_PANEL      = UiTheme::panel;
constexpr Color COL_SKILL_BOX  = UiTheme::panelBottom;
constexpr Color COL_PANEL_EDGE = UiTheme::border;
constexpr Color COL_BAR_BG     = UiTheme::obsidian;
constexpr Color COL_HP_HIGH    = UiTheme::hpTop;
constexpr Color COL_HP_MID     = UiTheme::accent;
constexpr Color COL_HP_LOW     = UiTheme::danger;
constexpr Color COL_MP         = UiTheme::mpTop;
constexpr Color COL_DAMAGE     = UiTheme::danger;
constexpr Color COL_HEAL       = UiTheme::heal;
constexpr Color COL_DISABLED   = UiTheme::disabled;

float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }

float ratioOf(int cur, int maxV) {                       // chia 0 an toan
    return (maxV > 0) ? clamp01(static_cast<float>(cur) / static_cast<float>(maxV)) : 0.0f;
}

void drawBar(Vector2 pos, float w, float h, float ratio, Color fill, Color bottom) {
    Rectangle bg = { pos.x, pos.y, w, h };
    UiShapes::roundedRect(bg, UiTheme::radiusBar, COL_BAR_BG, UiTheme::border, 1.0f);
    if (ratio > 0.0f) {
        UiShapeEffects effects;
        effects.gradientBottom = bottom;
        UiShapes::roundedRect(Rectangle{ pos.x, pos.y, w * ratio, h },
                              UiTheme::radiusBar, fill, BLANK, 0.0f, effects);
    }
}

void drawButton(Rectangle bounds, const char* label, const char* iconKey,
                bool enabled = true) {
    const bool hovered = CheckCollisionPointRec(UiInput::mousePosition(), bounds);
    if (hovered)
        UiInput::setCursorType(enabled ? UiInput::CursorType::POINTER : UiInput::CursorType::DISABLED);
    UiShapeEffects effects;
    effects.gradientBottom = hovered && enabled ? UiTheme::primaryBottom : UiTheme::panelBottom;
    UiShapes::roundedRect(bounds, UiTheme::radiusSmall,
                          hovered && enabled ? UiTheme::primaryTop : UiTheme::panel,
                          enabled ? (hovered ? UiTheme::accent : UiTheme::border) : UiTheme::disabled,
                          1.0f, effects);
    constexpr int textSize = 13;
    const float iconSize = 18.0f;
    UiIcons::draw(iconKey, Rectangle{ bounds.x + 5.0f,
        bounds.y + (bounds.height - iconSize) * 0.5f, iconSize, iconSize });
    UiText::drawEllipsis(label, static_cast<int>(bounds.x + 25.0f),
        static_cast<int>(bounds.y + (bounds.height - textSize) * 0.5f),
        static_cast<int>(bounds.width - 28.0f), textSize,
        hovered && enabled ? UiTheme::primaryText : (enabled ? UiTheme::text : COL_DISABLED),
        UiFontRole::HUD);
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

float normalizedStat(int current, int maximum) {
    return maximum > 0
        ? std::max(0.0f, std::min(1.0f, static_cast<float>(current) / maximum))
        : 0.0f;
}

void drawCompactGauge(Rectangle bounds, float value, Color top, Color bottom) {
    UiShapes::roundedRect(bounds, UiTheme::radiusBar, UiTheme::obsidian,
                          UiTheme::border, 0.5f);
    if (value <= 0.0f) return;
    UiShapeEffects effects;
    effects.gradientBottom = bottom;
    UiShapes::roundedRect(Rectangle{ bounds.x, bounds.y, bounds.width * value, bounds.height },
                          UiTheme::radiusBar, top, BLANK, 0.0f, effects);
}

constexpr float LIST_SCROLL_TOP = 30.0f;
constexpr float LIST_SCROLL_BOTTOM = 26.0f;

Rectangle listScrollbarTrack(Rectangle panel) {
    return Rectangle{ panel.x + panel.width - 7.0f, panel.y + LIST_SCROLL_TOP,
                      3.0f, panel.height - LIST_SCROLL_TOP - LIST_SCROLL_BOTTOM };
}

void drawListScrollbar(Rectangle panel, int total, int visible, int first) {
    if (total <= visible || visible <= 0) return;
    const Rectangle track = listScrollbarTrack(panel);
    UiShapes::roundedRect(track, UiTheme::radiusBar, UiTheme::obsidian, BLANK, 0.0f);
    const float thumbHeight = std::max(16.0f, track.height * visible / total);
    const float travel = track.height - thumbHeight;
    const float fraction = static_cast<float>(first) / std::max(1, total - visible);
    const Rectangle thumb{ track.x, track.y + travel * fraction, track.width, thumbHeight };
    UiShapes::roundedRect(thumb, UiTheme::radiusBar, UiTheme::accent, BLANK, 0.0f);
}

int scrollbarSelectedIndex(Vector2 position, Rectangle panel, int total, int visible) {
    if (total <= visible || visible <= 0) return -1;
    const Rectangle track = listScrollbarTrack(panel);
    Rectangle hitArea{ track.x - 8.0f, track.y, 19.0f, track.height };
    if (!CheckCollisionPointRec(position, hitArea)) return -1;
    const float fraction = std::max(0.0f, std::min(1.0f, (position.y - track.y) / track.height));
    return static_cast<int>(std::lround(fraction * (total - visible)));
}

} // namespace

Rectangle BattleUI::buttonBounds(BattleUIButton button) {
    constexpr float y = UiTheme::battleCommandCluster.y + 74.0f;
    constexpr float x = UiTheme::battleCommandCluster.x + 7.0f;
    switch (button) {
        case BattleUIButton::ATTACK:        return { x, y, 72.0f, 34.0f };
        case BattleUIButton::SKILL:         return { x + 76.0f, y, 72.0f, 34.0f };
        case BattleUIButton::ITEM:          return { x + 152.0f, y, 72.0f, 34.0f };
        case BattleUIButton::END_TURN:      return { x, y, 108.0f, 34.0f };
        case BattleUIButton::UNDO:          return { x + 116.0f, y, 108.0f, 34.0f };
        case BattleUIButton::CANCEL_TARGET: return { x, y, 140.0f, 34.0f };
    }
    return {};
}

Rectangle BattleUI::controlPanelBounds() {
    return UiTheme::battleCommandCluster;
}

Rectangle BattleUI::itemMenuBounds() {
    return UiTheme::battleItemList;
}

Rectangle BattleUI::itemCancelBounds() {
    const Rectangle box = UiTheme::battleItemList;
    return { box.x + box.width - 27.0f, box.y + 4.0f, 22.0f, 22.0f };
}

Rectangle BattleUI::partyCardBounds(int memberIndex) {
    if (memberIndex < 0 || memberIndex >= 3) return {};
    return UiTheme::battlePartyCards[memberIndex];
}

Rectangle BattleUI::battleLogButtonBounds() { return UiTheme::battleLogButton; }
Rectangle BattleUI::battleLogPanelBounds() { return UiTheme::battleLogPanel; }

bool BattleUI::isVisibleHudAt(Vector2 position, bool commandVisible,
                              bool skillListVisible, bool itemListVisible,
                              bool battleLogVisible, bool tickerVisible) {
    if (CheckCollisionPointRec(position, UiTheme::battleTurnStrip) ||
        CheckCollisionPointRec(position, UiTheme::battleRoundChip) ||
        CheckCollisionPointRec(position, UiTheme::battleLogButton)) return true;
    if (tickerVisible && CheckCollisionPointRec(position, UiTheme::battleLogTicker)) return true;
    for (const Rectangle card : UiTheme::battlePartyCards)
        if (CheckCollisionPointRec(position, card)) return true;
    if (battleLogVisible && CheckCollisionPointRec(position, UiTheme::battleLogPanel)) return true;
    if (commandVisible &&
        CheckCollisionPointRec(position, UiTheme::battleCommandCluster)) return true;
    if (skillListVisible &&
        CheckCollisionPointRec(position, UiTheme::battleSkillList)) return true;
    return itemListVisible &&
        CheckCollisionPointRec(position, UiTheme::battleItemList);
}

void BattleUI::drawPartyCards(const std::array<Character*, 3>& party, int partyCount,
                              const Character* active, const Character* selectedAlly) const {
    const Vector2 mouse = UiInput::mousePosition();
    for (int i = 0; i < 3; ++i) {
        const Rectangle card = UiTheme::battlePartyCards[i];
        const Character* character = i < partyCount ? party[static_cast<std::size_t>(i)] : nullptr;
        const bool isActive = character && character == active;
        const bool selected = isActive || (character && character == selectedAlly);
        const bool hovered = character && CheckCollisionPointRec(mouse, card);
        if (hovered) UiInput::setCursorType(UiInput::CursorType::POINTER);
        UiShapeEffects effects;
        effects.gradientBottom = UiTheme::panelBottom;
        effects.glow = UiTheme::ember;
        effects.glowRadius = isActive ? 4.0f : 0.0f;
        effects.glowStrength = isActive ? 0.28f : 0.0f;
        UiShapes::roundedRect(card, UiTheme::radiusCard, UiTheme::panel,
            isActive ? UiTheme::accent :
                (character == selectedAlly ? UiTheme::verdigris :
                 (hovered ? UiTheme::secondaryBorder : UiTheme::border)),
            selected ? 1.5f : 1.0f, effects);
        if (!character) continue;

        const Color tint = character->isAlive() ? WHITE : COL_DISABLED;
        const Rectangle portrait{ card.x + 7.0f, card.y + 17.0f, 38.0f, 38.0f };
        UiShapes::roundedRect(portrait, 12.0f,
            selected ? UiTheme::panelRaised : UiTheme::obsidian,
            selected ? UiTheme::accent : UiTheme::border, 1.0f);
        UiElementSigil::draw(character->getElement(),
            Rectangle{ portrait.x + 7.0f, portrait.y + 7.0f, 24.0f, 24.0f }, tint);

        const std::string name = Strings::localizedName(character->getName());
        UiText::drawEllipsis(character->isAlive() ? name : u8"Gục ngã",
            static_cast<int>(card.x + 51.0f), static_cast<int>(card.y + 6.0f),
            static_cast<int>(card.width - 58.0f), 14, tint, UiFontRole::HUD);

        const float valueX = card.x + card.width - 54.0f;
        const float gaugeX = card.x + 68.0f;
        const float gaugeWidth = std::max(24.0f, valueX - gaugeX - 7.0f);
        const float hpRatio = normalizedStat(character->getCurrentHp(), character->getStats().m_hp);
        const float mpRatio = normalizedStat(character->getCurrentMp(), character->getStats().m_mp);
        UiText::draw("HP", static_cast<int>(card.x + 51.0f), static_cast<int>(card.y + 25.0f),
                     11, UiTheme::ash, UiFontRole::HUD);
        drawCompactGauge(Rectangle{ gaugeX, card.y + 30.0f, gaugeWidth, 4.0f },
                         hpRatio, UiTheme::hpTop, UiTheme::hpBottom);
        UiText::drawEllipsis(TextFormat("%d/%d", std::max(0, character->getCurrentHp()),
                             character->getStats().m_hp),
                     static_cast<int>(valueX), static_cast<int>(card.y + 23.0f), 51,
                     11, tint, UiFontRole::NUMERALS);

        UiText::draw("MP", static_cast<int>(card.x + 51.0f), static_cast<int>(card.y + 46.0f),
                     11, UiTheme::ash, UiFontRole::HUD);
        drawCompactGauge(Rectangle{ gaugeX, card.y + 51.0f, gaugeWidth, 4.0f },
                         mpRatio, UiTheme::mpTop, UiTheme::mpBottom);
        UiText::drawEllipsis(TextFormat("%d/%d", std::max(0, character->getCurrentMp()),
                             character->getStats().m_mp),
                     static_cast<int>(valueX), static_cast<int>(card.y + 44.0f), 51,
                     11, tint, UiFontRole::NUMERALS);

        if (hovered) {
            const std::string tooltip = name + TextFormat(" · HP %d/%d · MP %d/%d",
                character->getCurrentHp(), character->getStats().m_hp,
                character->getCurrentMp(), character->getStats().m_mp);
            UiWidgets::Tooltip(card, tooltip);
        }
    }
}

void BattleUI::drawRoundChip(int stage, int turn) const {
    const Rectangle chip = UiTheme::battleRoundChip;
    UiShapeEffects effects;
    effects.gradientBottom = UiTheme::panelBottom;
    UiShapes::roundedRect(chip, UiTheme::radiusChip, UiTheme::panel,
                          UiTheme::border, 1.0f, effects);
    UiIcons::draw("action:target", Rectangle{ chip.x + 7.0f, chip.y + 6.0f, 20.0f, 20.0f });
    UiText::draw(TextFormat(Strings::battleStageTurnTemplate, stage, turn),
                 static_cast<int>(chip.x + 33.0f), static_cast<int>(chip.y + 8.0f),
                 13, UiTheme::text, UiFontRole::NUMERALS);
}

void BattleUI::drawEnemyTurnChip(const Character* current) const {
    const Rectangle chip = UiTheme::battleEnemyTurnChip;
    UiShapeEffects effects;
    effects.gradientBottom = UiTheme::panelBottom;
    UiShapes::roundedRect(chip, UiTheme::radiusChip, UiTheme::panel,
                          UiTheme::border, 1.0f, effects);
    UiElementSigil::draw(current ? current->getElement() : Element::HOA,
                         Rectangle{ chip.x + 9.0f, chip.y + 8.0f, 22.0f, 22.0f });
    const std::string name = current ? Strings::localizedName(current->getName()) : std::string{};
    UiText::drawEllipsis(name.empty() ? Strings::enemyTurn : name,
                         static_cast<int>(chip.x + 38.0f), static_cast<int>(chip.y + 10.0f),
                         static_cast<int>(chip.width - 48.0f), 14, UiTheme::ash, UiFontRole::HUD);
}

Rectangle BattleUI::skillRowBounds(int skillIndex, int skillCount, int selectedIndex) {
    if (skillIndex < 0 || skillIndex >= skillCount || skillCount <= 0) return {};
    const int visible = std::min(skillCount, SKILL_MAX_VISIBLE);
    const int first = std::max(0, std::min(selectedIndex - visible + 1, skillCount - visible));
    if (skillIndex < first || skillIndex >= first + visible) return {};
    const int visibleRow = skillIndex - first;
    return { static_cast<float>(SKILL_BOX_X + 8),
             static_cast<float>(SKILL_BOX_Y + 30 + visibleRow * SKILL_ROW_STEP),
             static_cast<float>(SKILL_BOX_W - 24), static_cast<float>(SKILL_ROW_H) };
}

Rectangle BattleUI::itemRowBounds(int itemIndex) {
    if (itemIndex < 0 || itemIndex >= 7) return {};
    return { static_cast<float>(ITEM_BOX_X + 8),
             static_cast<float>(ITEM_BOX_Y + 30 + itemIndex * 34),
             static_cast<float>(ITEM_BOX_W - 24), 32.0f };
}

int BattleUI::itemIndexAt(Vector2 position, int itemCount, int selectedIndex) {
    if (itemCount <= 0 || !CheckCollisionPointRec(position, UiTheme::battleItemList)) return -1;
    const int visible = std::min(itemCount, 7);
    const int first = std::max(0, std::min(selectedIndex - visible + 1, itemCount - visible));
    for (int row = 0; row < visible; ++row)
        if (CheckCollisionPointRec(position, itemRowBounds(row))) return first + row;
    return -1;
}

int BattleUI::skillScrollbarIndexAt(Vector2 position, int skillCount) {
    const int visible = std::min(skillCount, SKILL_MAX_VISIBLE);
    const int first = scrollbarSelectedIndex(position, UiTheme::battleSkillList, skillCount, visible);
    return first < 0 ? -1 : std::min(skillCount - 1, first + visible - 1);
}

int BattleUI::itemScrollbarIndexAt(Vector2 position, int itemCount) {
    const int visible = std::min(itemCount, 7);
    const int first = scrollbarSelectedIndex(position, UiTheme::battleItemList, itemCount, visible);
    return first < 0 ? -1 : std::min(itemCount - 1, first + visible - 1);
}

// ============ Compact field status marker ============
void BattleUI::drawFieldStatus(const Character& c, Vector2 feet, bool hovered) const {
    const int maxHp = c.getStats().m_hp;
    const int maxMp = c.getStats().m_mp;
    const float hpRatio = ratioOf(c.getCurrentHp(), maxHp);
    constexpr float barWidth = 72.0f;
    const float x = feet.x - barWidth / 2.0f;
    const float y = feet.y + 3.0f;
    Color hpColor = (hpRatio > 0.5f) ? COL_HP_HIGH : (hpRatio > 0.25f) ? COL_HP_MID : COL_HP_LOW;
    drawBar({ x, y }, barWidth, 6.0f, hpRatio, hpColor,
            hpRatio <= 0.25f ? UiTheme::danger : UiTheme::hpBottom);
    UiElementSigil::draw(c.getElement(), Rectangle{ feet.x - 7.0f, y + 8.0f, 14.0f, 14.0f });

    if (!hovered) return;
    const Vector2 mouse = UiCanvas::mousePosition();
    const std::string name = Strings::localizedName(c.getName());
    const Rectangle tooltip{ std::min(mouse.x + 16.0f, UiCanvas::width() - 208.0f),
        std::min(mouse.y + 16.0f, UiCanvas::height() - 82.0f), 192.0f, 66.0f };
    UiShapeEffects effects;
    effects.gradientBottom = UiTheme::panelBottom;
    UiShapes::roundedRect(tooltip, UiTheme::radiusSmall, UiTheme::panel,
                          UiTheme::border, 1.0f, effects);
    UiText::drawEllipsis(name, static_cast<int>(tooltip.x + 10.0f),
                         static_cast<int>(tooltip.y + 7.0f), 172, 18,
                         UiTheme::text, UiFontRole::HUD);
    UiText::draw(TextFormat("%s  HP %d/%d", elementName(c.getElement()),
                            std::max(0, c.getCurrentHp()), maxHp),
                 static_cast<int>(tooltip.x + 10.0f), static_cast<int>(tooltip.y + 30.0f),
                 16, UiTheme::text, UiFontRole::BODY);
    UiText::draw(TextFormat("MP %d/%d", std::max(0, c.getCurrentMp()), maxMp),
                 static_cast<int>(tooltip.x + 10.0f), static_cast<int>(tooltip.y + 48.0f),
                 16, UiTheme::mpTop, UiFontRole::NUMERALS);
}

// ============ Khung dieu khien ============
void BattleUI::drawControlPanel() const {
    Rectangle panel = UiTheme::battleCommandCluster;
    UiShapeEffects effects;
    effects.gradientBottom = UiTheme::panelBottom;
    effects.shadowBlur = 4.0f;
    UiShapes::roundedRect(panel, UiTheme::radiusLarge, COL_PANEL,
                          COL_PANEL_EDGE, 1.0f, effects);
}

void BattleUI::drawTurnBanner(int stage, const Character* current, bool isPlayerTurn, bool waitingConfirm) const {
    (void)stage;
    if (!current) return;
    UiElementSigil::draw(current->getElement(), Rectangle{ PANEL_X + 10.0f, BANNER_Y, 20.0f, 20.0f });
    const std::string name = Strings::localizedName(current->getName());
    const char* status = waitingConfirm ? Strings::waitingConfirm : (isPlayerTurn ? "" : Strings::enemyTurn);
    UiText::drawEllipsis(TextFormat("%s%s%s", name.c_str(), *status ? " · " : "", status),
        PANEL_X + 36, BANNER_Y + 2, PANEL_W - 48, 14,
        isPlayerTurn ? UiTheme::text : UiTheme::ash, UiFontRole::HUD);
}

void BattleUI::drawCommandHelp(const std::string& helpText) const {
    if (helpText.empty()) return;
    UiText::drawEllipsis(helpText, PANEL_X + 12, HELP_Y, PANEL_W - 24, 16,
                         LIGHTGRAY, UiFontRole::BODY);
}

void BattleUI::drawBattleControls(bool waitingConfirm, bool selectingTarget, bool canUndo) const {
    if (selectingTarget) UiInput::setCursorType(UiInput::CursorType::TARGET);
    if (waitingConfirm) {
        drawButton(buttonBounds(BattleUIButton::END_TURN), Strings::endTurn, "action:end_turn");
        drawButton(buttonBounds(BattleUIButton::UNDO), Strings::undo, "action:undo", canUndo);
    } else if (selectingTarget) {
        drawButton(buttonBounds(BattleUIButton::CANCEL_TARGET), Strings::cancel, "action:close");
        drawCommandHelp(Strings::targetPrompt);
    } else {
        drawButton(buttonBounds(BattleUIButton::ATTACK), Strings::attack, "action:attack");
        drawButton(buttonBounds(BattleUIButton::SKILL), Strings::skill, "action:skill");
        drawButton(buttonBounds(BattleUIButton::ITEM), Strings::item, "action:item");
    }
}

void BattleUI::observeBattleLog(const std::string& message) {
    if (message.empty() || message == m_lastBattleLog) return;
    m_lastBattleLog = message;
    m_battleLog.push_back(message);
    if (m_battleLog.size() > 80) m_battleLog.erase(m_battleLog.begin());
    m_tickerAge = 0.0f;
    m_logUnread = true;
}

void BattleUI::clearBattleLog() {
    m_battleLog.clear();
    m_lastBattleLog.clear();
    m_tickerAge = 3.0f;
    m_logScroll = 0;
    m_logUnread = false;
}

void BattleUI::drawBattleLogButton(const std::string& ticker, bool unread) const {
    const Rectangle button = UiTheme::battleLogButton;
    const bool hovered = CheckCollisionPointRec(UiInput::mousePosition(), button);
    if (hovered) UiInput::setCursorType(UiInput::CursorType::POINTER);
    UiShapeEffects effects;
    effects.gradientBottom = UiTheme::panelBottom;
    UiShapes::roundedRect(button, UiTheme::radiusSmall,
        hovered ? UiTheme::panelRaised : UiTheme::panel,
        hovered ? UiTheme::accent : UiTheme::border, 1.0f, effects);
    for (int line = 0; line < 3; ++line)
        DrawRectangle(static_cast<int>(button.x + 8.0f),
            static_cast<int>(button.y + 9.0f + line * 5.0f), 12, 2, UiTheme::accent);
    UiText::draw(Strings::battleLogTitle, static_cast<int>(button.x + 29.0f),
        static_cast<int>(button.y + 8.0f), 12, UiTheme::text, UiFontRole::HUD);
    if (unread) DrawCircle(static_cast<int>(button.x + button.width - 8.0f),
                           static_cast<int>(button.y + 8.0f), 3.0f, UiTheme::ember);
    if (!ticker.empty() && m_tickerAge < 3.0f) {
        const Rectangle tickerBounds = UiTheme::battleLogTicker;
        UiShapes::roundedRect(tickerBounds, UiTheme::radiusSmall, UiTheme::panel,
                              UiTheme::border, 1.0f, effects);
        UiText::drawEllipsis(ticker, static_cast<int>(tickerBounds.x + 8.0f),
            static_cast<int>(tickerBounds.y + 8.0f),
            static_cast<int>(tickerBounds.width - 16.0f), 12, UiTheme::ash, UiFontRole::BODY);
    }
}

void BattleUI::drawBattleLogPanel(bool open) {
    if (!open) return;
    const Rectangle panel = UiTheme::battleLogPanel;
    UiShapeEffects effects;
    effects.gradientBottom = UiTheme::panelBottom;
    UiShapes::roundedRect(panel, UiTheme::radiusLarge, UiTheme::panel,
                          UiTheme::border, 1.0f, effects);
    UiText::draw(Strings::battleLogTitle, static_cast<int>(panel.x + 10.0f),
                 static_cast<int>(panel.y + 7.0f), 13, UiTheme::accent, UiFontRole::HUD);
    const Rectangle close{ panel.x + panel.width - 27.0f, panel.y + 4.0f, 22.0f, 22.0f };
    if (CheckCollisionPointRec(UiInput::mousePosition(), close))
        UiInput::setCursorType(UiInput::CursorType::POINTER);
    UiIcons::draw("action:close", close);
    constexpr int rows = 4;
    for (int row = 0; row < rows; ++row) {
        const int index = static_cast<int>(m_battleLog.size()) - 1 - m_logScroll - row;
        if (index < 0) break;
        const std::string& entry = m_battleLog[static_cast<std::size_t>(index)];
        Color entryColor = row == 0 ? UiTheme::text : UiTheme::ash;
        if (entry.find(u8"gây") != std::string::npos) entryColor = UiTheme::danger;
        else if (entry.find(u8"hồi") != std::string::npos ||
                 entry.find(u8"dùng") != std::string::npos) entryColor = UiTheme::heal;
        else if (entry.find(u8"▲") != std::string::npos) entryColor = UiTheme::accent;
        else if (entry.find(u8"▼") != std::string::npos) entryColor = UiTheme::disabled;
        UiText::drawEllipsis(m_battleLog[static_cast<std::size_t>(index)],
            static_cast<int>(panel.x + 10.0f), static_cast<int>(panel.y + 33.0f + row * 19.0f),
            static_cast<int>(panel.width - 20.0f), 12, entryColor, UiFontRole::BODY);
    }
    if (m_battleLog.empty())
        UiText::draw(Strings::battleLogEmpty, static_cast<int>(panel.x + 10.0f),
            static_cast<int>(panel.y + 38.0f), 12, UiTheme::ash, UiFontRole::BODY);
    const int maxScroll = std::max(0, static_cast<int>(m_battleLog.size()) - rows);
    drawListScrollbar(panel, static_cast<int>(m_battleLog.size()), rows, maxScroll - m_logScroll);
}

void BattleUI::scrollBattleLog(float wheelDelta) {
    if (!CheckCollisionPointRec(UiInput::mousePosition(), UiTheme::battleLogPanel) || wheelDelta == 0.0f)
        return;
    const int maxScroll = std::max(0, static_cast<int>(m_battleLog.size()) - 4);
    m_logScroll = std::max(0, std::min(maxScroll, m_logScroll + (wheelDelta > 0.0f ? 1 : -1)));
}

void BattleUI::dragBattleLogScrollbar(Vector2 position) {
    const int first = scrollbarSelectedIndex(position, UiTheme::battleLogPanel,
        static_cast<int>(m_battleLog.size()), 4);
    if (first < 0) return;
    const int maxScroll = std::max(0, static_cast<int>(m_battleLog.size()) - 4);
    m_logScroll = std::max(0, std::min(maxScroll, maxScroll - first));
}

void BattleUI::drawSkillMenu(const std::vector<Skill>& skills, int selectedIndex, int currentMp) {
    if (skills.empty()) return;
    const int total = static_cast<int>(skills.size());
    selectedIndex = std::max(0, std::min(selectedIndex, total - 1));

    // Cuon de dong dang chon luon nam trong vung nhin thay
    const int visible = std::min(total, SKILL_MAX_VISIBLE);
    const int first = std::max(0, std::min(selectedIndex - visible + 1, total - visible));

    const int boxX = SKILL_BOX_X;
    Rectangle box = UiTheme::battleSkillList;
    UiShapeEffects boxEffects;
    boxEffects.gradientBottom = UiTheme::panelBottom;
    UiShapes::roundedRect(box, UiTheme::radiusLarge, COL_SKILL_BOX,
                          COL_PANEL_EDGE, 1.0f, boxEffects);
    UiText::draw(Strings::skillPrompt, boxX + 20, SKILL_BOX_Y + 7, 16, YELLOW, UiFontRole::HUD);

    std::string hoverCaption;
    for (int i = first; i < first + visible; ++i) {
        const Skill& s = skills[static_cast<size_t>(i)];
        Color color = WHITE;
        if (currentMp < s.m_mpCost) color = COL_DISABLED;
        else if (i == selectedIndex) color = GREEN;

        const std::string skillName = Strings::localizedName(s.m_name);
        const Rectangle row = skillRowBounds(i, total, selectedIndex);
        const Vector2 mouse = UiCanvas::mousePosition();
        const bool hovered = CheckCollisionPointRec(mouse, row);
        if (hovered) UiInput::setCursorType(UiInput::CursorType::POINTER);
        UiShapeEffects rowEffects;
        rowEffects.gradientBottom = UiTheme::panelBottom;
        UiShapes::roundedRect(row, UiTheme::radiusSmall,
            hovered || i == selectedIndex ? UiTheme::panelBottom : UiTheme::panel,
            i == selectedIndex ? UiTheme::accent : (hovered ? UiTheme::ember : UiTheme::border),
            1.0f, rowEffects);
        UiElementSigil::draw(s.m_element, Rectangle{ row.x + 6.0f, row.y + 9.0f, 24.0f, 24.0f });
        UiText::drawEllipsis(skillName, static_cast<int>(row.x + 38), static_cast<int>(row.y + 8),
                             static_cast<int>(row.width - 84), 14, color, UiFontRole::HUD);
        UiText::draw(TextFormat("MP %d", s.m_mpCost),
            static_cast<int>(row.x + row.width - 42.0f), static_cast<int>(row.y + 8),
            12, currentMp < s.m_mpCost ? COL_DISABLED : UiTheme::mpTop, UiFontRole::NUMERALS);
        if (hovered) {
            if (currentMp < s.m_mpCost)
                hoverCaption = TextFormat(Strings::mpCost, skillName.c_str());
            else
                hoverCaption = skillName + TextFormat(" · %d %s · %d",
                    s.m_mpCost, Strings::statMp, s.m_power);
        }
    }
    UiText::drawEllipsis(hoverCaption, boxX + 10, SKILL_BOX_Y + ITEM_BOX_H - 20,
                         SKILL_BOX_W - 20, 12, UiTheme::ash, UiFontRole::BODY);
    drawListScrollbar(box, total, visible, first);
}

void BattleUI::drawItemMenu(const std::vector<ItemPtr>& items, int selectedIndex,
                            const Character* selectedAlly, const Party& party,
                            const ItemIcons& icons) const {
    const Rectangle box = UiTheme::battleItemList;
    UiShapeEffects effects;
    effects.gradientBottom = UiTheme::panelBottom;
    UiShapes::roundedRect(box, UiTheme::radiusLarge, COL_SKILL_BOX, COL_PANEL_EDGE, 1.0f, effects);
    UiText::draw(Strings::itemPrompt, static_cast<int>(box.x + 10.0f),
                 static_cast<int>(box.y + 7.0f), 13, UiTheme::text, UiFontRole::HUD);
    UiIcons::draw("action:close", itemCancelBounds());
    if (CheckCollisionPointRec(UiInput::mousePosition(), itemCancelBounds()))
        UiInput::setCursorType(UiInput::CursorType::POINTER);
    if (selectedAlly) {
        const std::string allyName = Strings::localizedName(selectedAlly->getName());
        UiText::drawEllipsis(allyName, static_cast<int>(box.x + 100.0f),
            static_cast<int>(box.y + 7.0f), static_cast<int>(box.width - 110.0f),
            12, UiTheme::accent, UiFontRole::BODY);
    }
    const int total = static_cast<int>(items.size());
    if (total == 0) {
        UiText::draw(Strings::noInventoryItems, static_cast<int>(box.x + 10.0f),
            static_cast<int>(box.y + 42.0f), 12, UiTheme::ash, UiFontRole::BODY);
        return;
    }
    selectedIndex = std::max(0, std::min(selectedIndex, total - 1));
    const int visible = std::min(total, 7);
    const int first = std::max(0, std::min(selectedIndex - visible + 1, total - visible));
    std::string hoverCaption;
    for (int rowIndex = 0; rowIndex < visible; ++rowIndex) {
        const std::size_t i = static_cast<std::size_t>(first + rowIndex);
        const Rectangle row = itemRowBounds(rowIndex);
        const bool hovered = CheckCollisionPointRec(UiInput::mousePosition(), row);
        if (hovered) UiInput::setCursorType(UiInput::CursorType::POINTER);
        const bool selected = static_cast<int>(i) == selectedIndex;
        UiShapeEffects rowEffects;
        rowEffects.gradientBottom = UiTheme::panelBottom;
        UiShapes::roundedRect(row, UiTheme::radiusSmall,
            selected || hovered ? UiTheme::panelRaised : UiTheme::panel,
            selected ? UiTheme::accent : (hovered ? UiTheme::ember : UiTheme::border),
            1.0f, rowEffects);
        if (!items[i]) continue;
        icons.draw(items[i]->getId(), Rectangle{ row.x + 5.0f, row.y + 5.0f, 22.0f, 22.0f }, WHITE);
        UiText::drawEllipsis(items[i]->getNameVi(), static_cast<int>(row.x + 33.0f),
            static_cast<int>(row.y + 8.0f), static_cast<int>(row.width - 70.0f),
            12, selected ? UiTheme::accent :
                (selectedAlly && !items[i]->canUse(*selectedAlly) ? UiTheme::disabled : UiTheme::text),
            UiFontRole::BODY);
        const int quantity = static_cast<int>(party.countItem(items[i]->getId()));
        UiText::draw(TextFormat("×%d", quantity),
            static_cast<int>(row.x + row.width - 30.0f), static_cast<int>(row.y + 8.0f),
            12, UiTheme::verdigris, UiFontRole::NUMERALS);
        if (hovered) {
            if (selectedAlly && !items[i]->canUse(*selectedAlly))
                hoverCaption = std::string(u8"Không thể dùng vật phẩm này cho ") +
                    Strings::localizedName(selectedAlly->getName());
            else
                hoverCaption = items[i]->getDescription();
        }
    }
    UiText::drawEllipsis(hoverCaption, static_cast<int>(box.x + 10.0f),
        static_cast<int>(box.y + box.height - 19.0f), static_cast<int>(box.width - 22.0f),
        11, UiTheme::ash, UiFontRole::BODY);
    drawListScrollbar(box, total, visible, first);
}

void BattleUI::drawTargetCursor(const BattleSlot& slot, Rectangle spriteBounds, bool selected) const {
    const float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(GetTime()) * 6.0f);
    const float radius = std::max(18.0f, std::min(spriteBounds.width * 0.24f, 42.0f));
    DrawEllipse(static_cast<int>(slot.m_feetPos.x), static_cast<int>(slot.m_feetPos.y + 2.0f),
        radius, 6.0f, Color{ 242, 190, 92, static_cast<unsigned char>(90 + 100 * pulse) });
    const float cx = spriteBounds.x + spriteBounds.width * 0.5f;
    const float tipY = spriteBounds.y - 2.0f + pulse * 3.0f;
    const float halfWidth = std::max(7.0f, std::min(spriteBounds.width * 0.12f, 15.0f));
    const Color color = selected ? UiTheme::accent : UiTheme::ember;
    DrawTriangle(Vector2{ cx - halfWidth, tipY - halfWidth },
                 Vector2{ cx + halfWidth, tipY - halfWidth },
                 Vector2{ cx, tipY + halfWidth }, color);
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
    m_tickerAge = std::min(3.0f, m_tickerAge + deltaTime);
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
        UiText::drawOutlined(t.m_text, px, py, size, c, Color{ 8, 10, 17, c.a }, 2,
                             UiFontRole::NUMERALS);
    }
}

void BattleUI::unloadAll() { m_floatingTexts.clear(); }