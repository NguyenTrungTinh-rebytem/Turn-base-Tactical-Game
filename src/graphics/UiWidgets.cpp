#include "UiWidgets.h"
#include "UiBlur.h"
#include "UiCanvas.h"
#include "UiIcons.h"
#include "UiInput.h"
#include "UiShapes.h"
#include "UiText.h"
#include "UiTheme.h"
#include "RaylibScope.h"
#include <algorithm>
#include <cmath>
#include <optional>
#include <utility>

namespace {
constexpr Color outline = UiTheme::secondaryBorder;
constexpr Color label = UiTheme::ash;
constexpr Color muted = UiTheme::disabled;
constexpr Color textOnAccent = UiTheme::primaryText;
constexpr Color accentHover = UiTheme::accent;
constexpr float borderWidth = 1.0f;
bool tooltipClaimed = false;
bool tooltipPending = false;
Rectangle tooltipAnchor{};
std::string tooltipText;
std::string toastMessage;
float toastRemaining = 0.0f;
std::optional<RaylibScope::ScissorMode> scrollAreaScissor;
float scrollDragGrab = 0.0f;
bool scrollDragging = false;

bool hovered(Rectangle bounds, const char* id = nullptr) {
    const bool over = UiInput::hovered(bounds, id);
    if (over) UiInput::setCursorType(UiInput::CursorType::POINTER);
    return over;
}

void drawLabel(Rectangle bounds, const std::string& text, int size, Color color,
               float centerXOffset, UiFontRole role, float letterSpacing);

Color withAlpha(Color color, float alpha) {
    color.a = static_cast<unsigned char>(static_cast<float>(color.a) * alpha);
    return color;
}

Color withOpacity(Color color, float opacity) {
    color.a = static_cast<unsigned char>(std::lround(255.0f * std::clamp(opacity, 0.0f, 1.0f)));
    return color;
}

bool styledButton(Rectangle bounds, const std::string& text, bool enabled,
                  Color top, Color bottom, Color foreground, Color normalBorder,
                  float opacity) {
    const bool over = hovered(bounds, text.c_str());
    if (over && !enabled) UiInput::setCursorType(UiInput::CursorType::DISABLED);
    const bool held = enabled && over && UiInput::down();
    const bool clicked = enabled && over && UiInput::pressed();
    if (clicked) {
        UiInput::consumeClick();
        UiInput::playClickSound();
    }
    Rectangle body = bounds;
    if (held) body.y += 2.0f;
    UiShapeEffects effects;
    effects.gradientBottom = enabled ? bottom : UiTheme::panelBottom;
    effects.shadowBlur = enabled ? 4.0f : 0.0f;
    effects.glow = UiTheme::ember;
    effects.glowRadius = over && enabled ? 6.0f : 0.0f;
    effects.glowStrength = over && enabled ? 0.55f : 0.0f;
    UiShapes::roundedRect(body, UiTheme::radiusSmall,
        enabled ? withAlpha(top, opacity)
                : withOpacity(UiTheme::panel, UiTheme::buttonAlpha),
        enabled ? (over ? UiTheme::accent : normalBorder) : UiTheme::disabled,
        borderWidth, effects);
    drawLabel(body, text, 16, enabled ? foreground : label, 0.0f,
              UiFontRole::HUD, 16.0f * 0.04f);
    return clicked;
}

void drawLabel(Rectangle bounds, const std::string& text, int size, Color color,
               float centerXOffset = 0.0f, UiFontRole role = UiFontRole::AUTO,
               float letterSpacing = 0.0f) {
    const int width = UiText::measure(text, size, role, letterSpacing);
    UiText::draw(text, static_cast<int>(bounds.x + (bounds.width - width) * 0.5f + centerXOffset),
                 static_cast<int>(bounds.y + (bounds.height - size) * 0.5f), size, color,
                 role, letterSpacing);
}

Rectangle inset(Rectangle bounds, float amount) {
    return { bounds.x + amount, bounds.y + amount,
             std::max(0.0f, bounds.width - 2.0f * amount),
             std::max(0.0f, bounds.height - 2.0f * amount) };
}

int wrappedLineCount(const std::string& text, int maxWidth, int fontSize, UiFontRole role) {
    int lineCount = 1;
    int currentWidth = 0;
    std::size_t start = 0;
    while (start < text.size()) {
        std::size_t end = text.find_first_of(" \n", start);
        if (end == std::string::npos) end = text.size();
        const int wordWidth = UiText::measure(text.substr(start, end - start), fontSize, role);
        if (currentWidth > 0 && currentWidth + wordWidth > maxWidth) {
            ++lineCount;
            currentWidth = 0;
        }
        currentWidth += wordWidth + UiText::measure(" ", fontSize, role);
        if (end < text.size() && text[end] == '\n') {
            ++lineCount;
            currentWidth = 0;
        }
        start = end + 1;
    }
    return lineCount;
}

float linearChannel(unsigned char channel) {
    const float value = static_cast<float>(channel) / 255.0f;
    return value <= 0.04045f ? value / 12.92f : std::pow((value + 0.055f) / 1.055f, 2.4f);
}

unsigned char srgbChannel(float value) {
    value = std::clamp(value, 0.0f, 1.0f);
    const float encoded = value <= 0.0031308f ? value * 12.92f
        : 1.055f * std::pow(value, 1.0f / 2.4f) - 0.055f;
    return static_cast<unsigned char>(std::lround(encoded * 255.0f));
}

std::pair<Color, Color> accessiblePillColors(Color fill) {
    float channels[] = { linearChannel(fill.r), linearChannel(fill.g), linearChannel(fill.b) };
    float luminance = 0.2126f * channels[0] + 0.7152f * channels[1] + 0.0722f * channels[2];
    Color foreground = UiTheme::ink;
    if (luminance < 0.08f || luminance >= 0.32f) {
        foreground = luminance < 0.08f ? WHITE : UiTheme::ink;
    } else if (luminance - 0.08f <= 0.32f - luminance) {
        const float factor = 0.08f / luminance;
        for (float& channel : channels) channel *= factor;
        foreground = WHITE;
    } else {
        const float factor = (0.32f - luminance) / (1.0f - luminance);
        for (float& channel : channels) channel += (1.0f - channel) * factor;
    }
    fill = Color{ srgbChannel(channels[0]), srgbChannel(channels[1]),
                  srgbChannel(channels[2]), 255 };
    return { fill, foreground };
}
}

void UiWidgets::beginFrame() {
    scrollAreaScissor.reset();
    tooltipClaimed = false;
    tooltipPending = false;
    tooltipText.clear();
    if (tooltipText.capacity() < 1024) tooltipText.reserve(1024);
}

bool UiWidgets::Button(Rectangle bounds, const std::string& text, bool enabled) {
    return styledButton(bounds, text, enabled, UiTheme::primaryTop,
                        UiTheme::primaryBottom, textOnAccent, UiTheme::ember,
                        UiTheme::primaryButtonAlpha);
}

bool UiWidgets::SecondaryButton(Rectangle bounds, const std::string& text, bool enabled) {
    return styledButton(bounds, text, enabled, UiTheme::buttonGlassTop,
                        UiTheme::buttonGlassBottom, UiTheme::text, UiTheme::secondaryBorder,
                        UiTheme::buttonAlpha);
}

bool UiWidgets::DangerButton(Rectangle bounds, const std::string& text, bool enabled) {
    return styledButton(bounds, text, enabled, UiTheme::dangerTop,
                        UiTheme::dangerBottom, UiTheme::text, UiTheme::danger,
                        UiTheme::primaryButtonAlpha);
}

bool UiWidgets::IconButton(Rectangle bounds, const std::string& iconKey, bool enabled) {
    const bool over = hovered(bounds, iconKey.c_str());
    if (over && !enabled) UiInput::setCursorType(UiInput::CursorType::DISABLED);
    const bool held = enabled && over && UiInput::down();
    const bool clicked = enabled && over && UiInput::pressed();
    if (clicked) {
        UiInput::consumeClick();
        UiInput::playClickSound();
    }
    Rectangle body = bounds;
    if (held) body.y += 2.0f;
    const Color fill = withOpacity(over && enabled ? UiTheme::buttonGlassBottom
                                                   : UiTheme::buttonGlassTop,
                                   UiTheme::buttonAlpha);
    UiShapeEffects effects;
    effects.gradientBottom = UiTheme::panelBottom;
    UiShapes::roundedRect(body, UiTheme::radiusSmall, fill,
                          !enabled ? muted : (over ? UiTheme::accent : outline),
                          borderWidth, effects);
    const float iconSize = std::min(bounds.width, bounds.height) * 0.62f;
    UiIcons::draw(iconKey, Rectangle{ bounds.x + (bounds.width - iconSize) * 0.5f,
                                      bounds.y + (bounds.height - iconSize) * 0.5f,
                                      iconSize, iconSize });
    return clicked;
}

bool UiWidgets::TabBar(Rectangle bounds, const std::vector<std::string>& labels, int& selected) {
    if (labels.empty()) return false;
    selected = std::clamp(selected, 0, static_cast<int>(labels.size()) - 1);
    const float tabWidth = bounds.width / static_cast<float>(labels.size());
    bool changed = false;
    for (std::size_t index = 0; index < labels.size(); ++index) {
        const Rectangle tab{ bounds.x + static_cast<float>(index) * tabWidth,
                             bounds.y, tabWidth, bounds.height };
        const bool over = hovered(tab, labels[index].c_str());
        if (over && UiInput::pressed()) {
            selected = static_cast<int>(index);
            changed = true;
            UiInput::consumeClick();
            UiInput::playClickSound();
        }
        const bool active = static_cast<int>(index) == selected;
        UiShapeEffects effects;
        effects.flatBottom = true;
        effects.gradientBottom = UiTheme::panelBottom;
        const Color fill = active ? withAlpha(UiTheme::primaryTop, UiTheme::buttonAlpha)
                                  : UiTheme::panel;
        UiShapes::roundedRect(tab, UiTheme::radiusSmall, fill,
                              active || over ? UiTheme::accent : outline,
                              borderWidth, effects);
        if (active) {
            UiShapes::roundedRect(Rectangle{ tab.x + 8.0f, tab.y + tab.height - 2.0f,
                                              tab.width - 16.0f, 2.0f },
                                  UiTheme::radiusBar, UiTheme::ember);
        }
        drawLabel(tab, labels[index], 16, active ? textOnAccent : UiTheme::text, 0.0f,
                  UiFontRole::HUD, 16.0f * 0.04f);
    }
    return changed;
}

bool UiWidgets::ListRow(Rectangle bounds, const std::string& title,
                        const std::string& detail, bool selected, bool enabled) {
    const bool over = hovered(bounds, title.c_str());
    const bool held = enabled && over && UiInput::down();
    UiShapeEffects effects;
    effects.gradientBottom = UiTheme::panelBottom;
    const Color rowFill = selected ? UiTheme::panelBottom : UiTheme::panel;
    UiShapes::roundedRect(bounds, UiTheme::radiusSmall, rowFill,
        held || selected ? UiTheme::accent : (over ? UiTheme::ember : outline), borderWidth, effects);
    const float textX = bounds.x + 14.0f;
    const Color tint = enabled ? UiTheme::text : muted;
    UiText::draw(title, static_cast<int>(textX), static_cast<int>(bounds.y + 7.0f), 16, tint,
                 UiFontRole::HUD);
    if (!detail.empty())
        UiText::draw(detail, static_cast<int>(textX), static_cast<int>(bounds.y + 29.0f), 16,
                     selected ? label : muted, UiFontRole::BODY);
    const bool clicked = enabled && over && UiInput::pressed();
    if (clicked) {
        UiInput::consumeClick();
        UiInput::playClickSound();
    }
    return clicked;
}

bool UiWidgets::ItemTile(Rectangle bounds, const std::string& iconKey, const std::string& title,
                         const std::string& quantity, bool enabled) {
    const bool over = hovered(bounds, title.c_str());
    const bool held = enabled && over && UiInput::down();
    UiShapeEffects effects;
    effects.gradientBottom = UiTheme::panelBottom;
    effects.glow = UiTheme::ember;
    effects.glowRadius = over && enabled ? 4.0f : 0.0f;
    effects.glowStrength = over && enabled ? 0.4f : 0.0f;
    UiShapes::roundedRect(bounds, UiTheme::radiusCard,
        UiTheme::panel,
        held || over ? UiTheme::accent : outline, borderWidth, effects);
    const float iconSize = std::min(bounds.height - 34.0f, bounds.width - 20.0f);
    UiIcons::draw(iconKey, Rectangle{ bounds.x + (bounds.width - iconSize) * 0.5f,
                                      bounds.y + 8.0f, iconSize, iconSize },
                  enabled ? WHITE : muted);
    UiText::draw(title, static_cast<int>(bounds.x + 10.0f),
                 static_cast<int>(bounds.y + bounds.height - 23.0f), 16,
                 enabled ? UiTheme::text : muted, UiFontRole::HUD);
    if (!quantity.empty())
        UiShapes::roundedRect(Rectangle{ bounds.x + bounds.width - 34.0f, bounds.y + 5.0f, 28.0f, 22.0f },
                              UiTheme::radiusChip, UiTheme::accent);
    if (!quantity.empty())
        drawLabel(Rectangle{ bounds.x + bounds.width - 34.0f, bounds.y + 5.0f, 28.0f, 22.0f },
                  quantity, 16, textOnAccent, 0.0f, UiFontRole::NUMERALS);
    const bool clicked = enabled && over && UiInput::pressed();
    if (clicked) {
        UiInput::consumeClick();
        UiInput::playClickSound();
    }
    return clicked;
}

void UiWidgets::BeginScrollArea(Rectangle bounds, float contentHeight, float& scrollOffset) {
    const bool over = hovered(bounds, "scroll-area");
    if (over) scrollOffset -= UiInput::wheel() * 32.0f;
    scrollOffset = std::clamp(scrollOffset, 0.0f, std::max(0.0f, contentHeight - bounds.height));
    UiShapes::roundedRect(bounds, UiTheme::radiusSmall, UiTheme::panel, outline, borderWidth);
    const float maxOffset = std::max(0.0f, contentHeight - bounds.height);
    if (maxOffset > 0.0f) {
        const Rectangle track{ bounds.x + bounds.width - 12.0f, bounds.y + 8.0f,
                               6.0f, bounds.height - 16.0f };
        const float thumbHeight = std::max(30.0f, track.height * bounds.height / contentHeight);
        const float thumbTravel = std::max(0.0f, track.height - thumbHeight);
        Rectangle thumb{ track.x, track.y + thumbTravel * scrollOffset / maxOffset,
                         track.width, thumbHeight };
        const bool overTrack = hovered(track, "scrollbar");
        if (overTrack && UiInput::pressed()) {
            if (CheckCollisionPointRec(UiInput::mousePosition(), thumb)) {
                scrollDragging = true;
                scrollDragGrab = UiInput::mousePosition().y - thumb.y;
            } else {
                scrollOffset = std::clamp(
                    (UiInput::mousePosition().y - track.y - thumbHeight * 0.5f) /
                        std::max(1.0f, thumbTravel) * maxOffset,
                    0.0f, maxOffset);
                scrollDragging = true;
                scrollDragGrab = thumbHeight * 0.5f;
            }
            UiInput::consumeClick();
        }
        if (scrollDragging && UiInput::down()) {
            scrollOffset = std::clamp(
                (UiInput::mousePosition().y - track.y - scrollDragGrab) /
                    std::max(1.0f, thumbTravel) * maxOffset,
                0.0f, maxOffset);
        }
        if (UiInput::released()) scrollDragging = false;
        const Color thumbColor = overTrack || scrollDragging ? UiTheme::accent : outline;
        UiShapes::roundedRect(thumb, UiTheme::radiusChip, thumbColor);
    } else {
        scrollDragging = false;
    }
    const float scale = UiCanvas::scale();
    scrollAreaScissor.emplace(static_cast<int>(bounds.x * scale), static_cast<int>(bounds.y * scale),
                              static_cast<int>(bounds.width * scale), static_cast<int>(bounds.height * scale));
}

void UiWidgets::EndScrollArea() {
    scrollAreaScissor.reset();
}

bool UiWidgets::Dialog(Rectangle bounds, const std::string& title, const std::string& body,
                       const std::string& actionLabel) {
    UiBlur::draw(bounds, UiTheme::radiusLarge, withOpacity(UiTheme::panel, UiTheme::modalAlpha));
    UiText::draw(title, static_cast<int>(bounds.x + 22.0f), static_cast<int>(bounds.y + 18.0f),
                 22, UiTheme::accent);
    UiText::draw(body, static_cast<int>(bounds.x + 22.0f), static_cast<int>(bounds.y + 58.0f),
                 16, UiTheme::text);
    const Rectangle action{ bounds.x + bounds.width - 174.0f,
                            bounds.y + bounds.height - 64.0f, 152.0f, 48.0f };
    if (Button(action, actionLabel)) return true;
    if (UiInput::pressed()) UiInput::consumeClick();
    return false;
}

UiWidgets::DialogAction UiWidgets::DialogWithCancel(Rectangle bounds, const std::string& title,
                                                     const std::string& body,
                                                     const std::string& actionLabel,
                                                     const std::string& cancelLabel) {
    UiBlur::draw(bounds, UiTheme::radiusLarge, withOpacity(UiTheme::panel, UiTheme::modalAlpha));
    UiText::draw(title, static_cast<int>(bounds.x + 22.0f), static_cast<int>(bounds.y + 18.0f),
                 22, UiTheme::accent);
    UiText::draw(body, static_cast<int>(bounds.x + 22.0f), static_cast<int>(bounds.y + 58.0f),
                 16, UiTheme::text);
    const Rectangle cancel{ bounds.x + bounds.width - 342.0f,
                            bounds.y + bounds.height - 64.0f, 152.0f, 48.0f };
    const Rectangle confirm{ bounds.x + bounds.width - 174.0f,
                             bounds.y + bounds.height - 64.0f, 152.0f, 48.0f };
    if (SecondaryButton(cancel, cancelLabel)) return DialogAction::CANCEL;
    if (Button(confirm, actionLabel)) return DialogAction::CONFIRM;
    if (UiInput::rightClicked()) {
        UiInput::consumeClick(MOUSE_BUTTON_RIGHT);
        return DialogAction::CANCEL;
    }
    if (UiInput::pressed()) UiInput::consumeClick();
    return DialogAction::NONE;
}

void UiWidgets::Tooltip(Rectangle anchor, const std::string& text) {
    if (tooltipClaimed || !hovered(anchor, "tooltip-trigger")) return;
    tooltipClaimed = true;
    tooltipPending = true;
    tooltipAnchor = anchor;
    tooltipText.assign(text.data(), std::min(text.size(), static_cast<std::size_t>(1023)));
}

void UiWidgets::DrawTooltip() {
    if (!tooltipPending || tooltipText.empty()) return;
    const std::string& text = tooltipText;
    const float margin = static_cast<float>(UiCanvas::width()) * 24.0f /
                         static_cast<float>(UiTheme::CANVAS_WIDTH);
    const float width = std::min(340.0f, static_cast<float>(UiText::measure(text, 16) + 24));
    const int textHeight = wrappedLineCount(text, static_cast<int>(width - 24.0f),
                                            16, UiFontRole::BODY) * 22;
    const float height = static_cast<float>(std::max(36, textHeight + 20));
    const Vector2 mouse = UiInput::mousePosition();
    const Rectangle candidates[] = {
        { mouse.x + 14.0f, mouse.y + 18.0f, width, height },
        { mouse.x + 14.0f, mouse.y - height - 14.0f, width, height },
        { mouse.x - width - 14.0f, mouse.y + 18.0f, width, height },
        { mouse.x - width - 14.0f, mouse.y - height - 14.0f, width, height }
    };
    Rectangle bounds = candidates[0];
    bool placed = false;
    for (const Rectangle& candidate : candidates) {
        Rectangle clamped = candidate;
        clamped.x = std::clamp(clamped.x, margin,
                               static_cast<float>(UiCanvas::width()) - margin - clamped.width);
        clamped.y = std::clamp(clamped.y, margin,
                               static_cast<float>(UiCanvas::height()) - margin - clamped.height);
        if (CheckCollisionRecs(clamped, tooltipAnchor)) continue;
        bounds = clamped;
        placed = true;
        break;
    }
    if (!placed) {
        bounds = candidates[0];
        bounds.x = tooltipAnchor.x + tooltipAnchor.width + 8.0f;
        if (bounds.x + bounds.width > UiCanvas::width() - margin)
            bounds.x = tooltipAnchor.x - bounds.width - 8.0f;
        bounds.y = std::clamp(bounds.y, margin,
                              static_cast<float>(UiCanvas::height()) - margin - bounds.height);
        if (bounds.x < margin) {
            bounds.x = std::clamp(tooltipAnchor.x,
                                  margin,
                                  static_cast<float>(UiCanvas::width()) - margin - bounds.width);
            bounds.y = tooltipAnchor.y - bounds.height - 8.0f;
        }
    }
    if (CheckCollisionRecs(bounds, tooltipAnchor)) return;
    UiShapeEffects effects;
    effects.gradientBottom = UiTheme::panelBottom;
    effects.shadowBlur = 8.0f;
    UiShapes::roundedRect(bounds, UiTheme::radiusTooltip,
                          withAlpha(UiTheme::panel, UiTheme::tooltipAlpha), outline,
                          borderWidth, effects);
    UiText::drawWrapped(text, static_cast<int>(bounds.x + 12.0f),
                        static_cast<int>(bounds.y + 10.0f),
                        static_cast<int>(width - 24.0f), 16, UiTheme::text,
                        22, UiFontRole::BODY);
}

void UiWidgets::Pill(Rectangle bounds, const std::string& text, Color fill) {
    const auto colors = accessiblePillColors(fill);
    UiShapes::roundedRect(bounds, UiTheme::radiusChip, withAlpha(colors.first, UiTheme::chipAlpha));
    drawLabel(bounds, text, 16, colors.second, 0.0f, UiFontRole::HUD);
}

void UiWidgets::Gauge(Rectangle bounds, float value, const std::string& text, Color fill) {
    value = std::clamp(value, 0.0f, 1.0f);
    UiShapes::roundedRect(bounds, UiTheme::radiusBar, withAlpha(UiTheme::panelBottom, UiTheme::panelAlpha),
                          outline, borderWidth);
    if (value > 0.0f) {
        Rectangle active = inset(bounds, 2.0f);
        active.width = std::max(0.0f, active.width) * value;
        UiShapeEffects effects;
        effects.gradientBottom = fill;
        UiShapes::roundedRect(active, UiTheme::radiusBar, fill, BLANK, 0.0f, effects);
    }
    const int labelWidth = UiText::measure(text, 16) + 16;
    const Rectangle badge{ bounds.x + (bounds.width - labelWidth) * 0.5f,
                           bounds.y + (bounds.height - 22.0f) * 0.5f,
                           static_cast<float>(labelWidth), 22.0f };
    UiShapes::roundedRect(badge, UiTheme::radiusChip, UiTheme::obsidian);
    drawLabel(badge, text, 16, UiTheme::text, 0.0f, UiFontRole::NUMERALS);
}

bool UiWidgets::QuantityStepper(Rectangle bounds, int& value, int minimum, int maximum) {
    if (minimum > maximum) std::swap(minimum, maximum);
    value = std::clamp(value, minimum, maximum);
    const float buttonWidth = std::min(bounds.height, bounds.width * 0.3f);
    const Rectangle decrement{ bounds.x, bounds.y, buttonWidth, bounds.height };
    const Rectangle increment{ bounds.x + bounds.width - buttonWidth, bounds.y, buttonWidth, bounds.height };
    bool changed = false;
    if (Button(decrement, u8"−", value > minimum)) { --value; changed = true; }
    if (Button(increment, "+", value < maximum)) { ++value; changed = true; }
    UiShapes::roundedRect(Rectangle{ decrement.x + decrement.width + 3.0f, bounds.y,
                                     bounds.width - 2.0f * buttonWidth - 6.0f, bounds.height },
                          UiTheme::radiusSmall, UiTheme::panel, outline, borderWidth);
    drawLabel(bounds, std::to_string(value), 16, UiTheme::text);
    return changed;
}

bool UiWidgets::Breadcrumb(Rectangle bounds, const std::vector<std::string>& labels, int& selected) {
    if (labels.empty()) return false;
    selected = std::clamp(selected, 0, static_cast<int>(labels.size()) - 1);
    const float segmentWidth = bounds.width / static_cast<float>(labels.size());
    bool changed = false;
    for (std::size_t i = 0; i < labels.size(); ++i) {
        const Rectangle segment{ bounds.x + static_cast<float>(i) * segmentWidth,
                                 bounds.y, segmentWidth, bounds.height };
        const bool over = hovered(segment, labels[i].c_str());
        if (over && UiInput::pressed()) {
            selected = static_cast<int>(i);
            changed = true;
            UiInput::consumeClick();
        }
        const bool active = selected == static_cast<int>(i);
        UiShapes::roundedRect(segment, UiTheme::radiusChip,
                              active ? UiTheme::panelBottom : UiTheme::panel,
                              active ? UiTheme::accent : outline, borderWidth);
        drawLabel(segment, labels[i], 16, active ? UiTheme::accent : UiTheme::text,
                  0.0f, UiFontRole::HUD);
    }
    return changed;
}

void UiWidgets::showToast(const std::string& message, float duration) {
    toastMessage = message;
    toastRemaining = std::max(0.1f, duration);
}

void UiWidgets::update(float deltaTime) {
    toastRemaining = std::max(0.0f, toastRemaining - std::max(0.0f, deltaTime));
}

void UiWidgets::Toast() {
    if (toastRemaining <= 0.0f || toastMessage.empty()) return;
    Rectangle bounds{ (UiCanvas::width() - 440.0f) * 0.5f, 24.0f, 440.0f, 46.0f };
    UiShapeEffects effects;
    effects.gradientBottom = UiTheme::panelBottom;
    effects.shadowBlur = 8.0f;
    UiShapes::roundedRect(bounds, UiTheme::radiusLarge, UiTheme::panelRaised,
                          outline, borderWidth, effects);
    drawLabel(bounds, toastMessage, 16, UiTheme::text);
}
