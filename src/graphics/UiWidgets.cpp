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
#include <cassert>
#include <cmath>
#include <iostream>
#include <optional>
#include <sstream>
#include <utility>

namespace {
constexpr Color outline{ 146, 164, 190, 255 };
constexpr Color label{ 225, 233, 245, 255 };
constexpr Color muted{ 194, 205, 222, 255 };
constexpr Color textOnAccent = UiTheme::silver;
constexpr float borderWidth = 1.0f;
bool tooltipClaimed = false;
std::string toastMessage;
float toastRemaining = 0.0f;
std::optional<RaylibScope::ScissorMode> scrollAreaScissor;

bool hovered(Rectangle bounds) {
    return CheckCollisionPointRec(UiInput::mousePosition(), bounds);
}

bool overlaps(Rectangle a, Rectangle b) {
    return a.width > 0.0f && a.height > 0.0f && b.width > 0.0f && b.height > 0.0f &&
        a.x < b.x + b.width && a.x + a.width > b.x &&
        a.y < b.y + b.height && a.y + a.height > b.y;
}

std::string fitLabel(const std::string& text, int size, float width) {
    if (UiText::measure(text, size) <= width) return text;
    constexpr const char* ellipsis = u8"…";
    const float available = width - UiText::measure(ellipsis, size);
    if (available <= 0.0f) return {};
    std::string fitted;
    for (const char* cursor = text.c_str(); *cursor;) {
        int bytes = 0;
        const int codepoint = GetCodepoint(cursor, &bytes);
        if (bytes <= 0) break;
        const std::string next = fitted + std::string(cursor, static_cast<std::size_t>(bytes));
        if (UiText::measure(next, size) > available) break;
        fitted = next;
        cursor += bytes;
        (void)codepoint;
    }
    return fitted + ellipsis;
}

void verifyRowLayout(Rectangle icon, Rectangle labelBounds, Rectangle trailing,
                     const std::string& id) {
#if defined(UI_DEV_TOOLS) && UI_DEV_TOOLS
    const bool valid = !overlaps(icon, labelBounds) && !overlaps(icon, trailing) &&
                       !overlaps(labelBounds, trailing);
    assert(valid && "row icon, label and trailing slot must not overlap");
    if (!valid) std::cerr << "[UI row overlap] " << id << '\n';
#else
    (void)icon;
    (void)labelBounds;
    (void)trailing;
    (void)id;
#endif
}

void drawLabel(Rectangle bounds, const std::string& text, int size, Color color,
               float centerXOffset = 0.0f) {
    int fittedSize = size;
    while (fittedSize > 10 && UiText::measure(text, fittedSize) > bounds.width - 16.0f)
        --fittedSize;
    const int width = UiText::measure(text, fittedSize);
    UiText::draw(text, static_cast<int>(bounds.x + (bounds.width - width) * 0.5f + centerXOffset),
                 static_cast<int>(bounds.y + (bounds.height - fittedSize) * 0.5f), fittedSize, color);
}

Rectangle inset(Rectangle bounds, float amount) {
    return { bounds.x + amount, bounds.y + amount,
             std::max(0.0f, bounds.width - 2.0f * amount),
             std::max(0.0f, bounds.height - 2.0f * amount) };
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
}

bool UiWidgets::Button(Rectangle bounds, const std::string& text, bool enabled) {
    return BladeButton(bounds, text, false, false, enabled);
}

bool UiWidgets::BladeButton(Rectangle bounds, const std::string& text, bool rightAnchored,
                            bool selected, bool enabled, const std::string& iconKey) {
    const bool over = hovered(bounds);
    const bool held = enabled && over && UiInput::down();
    const bool clicked = enabled && over && UiInput::pressed();
    Rectangle visualBounds = bounds;
    if (over && enabled) visualBounds.x += rightAnchored ? -7.0f : 7.0f;
    if (held) visualBounds.y += 2.0f;
    const Color fill = selected ? Color{ 50, 10, 17, 224 } : UiTheme::panel;
    UiShapes::bladeStrip(visualBounds, fill,
        enabled ? UiTheme::ash : muted, rightAnchored, selected || held, over, enabled);

    const int fontSize = 16;
    const float pad = 14.0f;
    const float trailingWidth = 0.0f;
    const float iconSize = iconKey.empty() ? 0.0f :
        std::min(24.0f, std::max(16.0f, bounds.height * 0.52f));
    const Rectangle iconBounds = iconSize > 0.0f
        ? Rectangle{ visualBounds.x + pad,
                     visualBounds.y + (visualBounds.height - iconSize) * 0.5f,
                     iconSize, iconSize } : Rectangle{};
    const float labelX = visualBounds.x + pad + iconSize +
                         (iconSize > 0.0f ? 12.0f : 0.0f);
    const Rectangle labelBounds{ labelX, visualBounds.y,
        std::max(0.0f, visualBounds.x + visualBounds.width - pad - trailingWidth - labelX),
        visualBounds.height };
    const Rectangle trailingBounds = trailingWidth > 0.0f
        ? Rectangle{ visualBounds.x + visualBounds.width - pad - trailingWidth,
                     visualBounds.y, trailingWidth, visualBounds.height } : Rectangle{};
    verifyRowLayout(iconBounds, labelBounds, trailingBounds, text);
    if (iconSize > 0.0f)
        UiIcons::draw(iconKey, iconBounds, enabled ? UiTheme::silver : muted);
    const std::string fitted = fitLabel(text, fontSize, labelBounds.width);
    UiText::draw(fitted, static_cast<int>(labelBounds.x),
        static_cast<int>(visualBounds.y + (visualBounds.height - fontSize) * 0.5f),
        fontSize, enabled ? UiTheme::silver : muted);
    return clicked;
}

bool UiWidgets::IconButton(Rectangle bounds, const std::string& iconKey, bool enabled) {
    const bool over = hovered(bounds);
    const bool held = enabled && over && UiInput::down();
    const bool clicked = enabled && over && UiInput::pressed();
    UiShapes::seal(bounds, !enabled ? muted : (over || held ? UiTheme::crimson : outline),
                   over || held);
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
        const bool over = hovered(tab);
        if (over && UiInput::pressed()) {
            selected = static_cast<int>(index);
            changed = true;
        }
        const bool active = static_cast<int>(index) == selected;
        UiShapes::angledTab(tab,
            active ? Color{ 50, 10, 17, 220 } : UiTheme::panel,
            outline, active, over);
        drawLabel(tab, labels[index], 15, active ? UiTheme::silver : UiTheme::text);
    }
    return changed;
}

bool UiWidgets::ListRow(Rectangle bounds, const std::string& title,
                        const std::string& detail, bool selected, bool enabled, float textInset,
                        bool rightAnchored) {
    rightAnchored = rightAnchored ||
        bounds.x + bounds.width * 0.5f > UiCanvas::width() * 0.5f;
    const bool over = hovered(bounds);
    const bool held = enabled && over && UiInput::down();
    Rectangle visualBounds = bounds;
    if (over && enabled) visualBounds.x += 6.0f;
    if (held) visualBounds.y += 2.0f;
    UiShapes::bladeStrip(visualBounds,
        selected ? Color{ 50, 10, 17, 224 } : UiTheme::panel,
        enabled ? outline : muted, rightAnchored, selected || held, over, enabled);
    const int titleSize = bounds.height < 30.0f ? 14 : bounds.height < 45.0f ? 16 : 16;
    const float left = bounds.x + std::max(14.0f, textInset);
    const float right = bounds.x + bounds.width - 14.0f;
    const Rectangle titleBounds{ left, bounds.y + 2.0f, std::max(0.0f, right - left),
                                  std::max(0.0f, bounds.height - (detail.empty() ? 4.0f : 22.0f)) };
    const Rectangle detailBounds{ left, bounds.y + bounds.height - 20.0f,
                                  std::max(0.0f, right - left), 18.0f };
    verifyRowLayout({}, titleBounds, detail.empty() ? Rectangle{} : detailBounds, title);
    const Color tint = enabled ? UiTheme::text : muted;
    UiText::draw(fitLabel(title, titleSize, titleBounds.width),
                 static_cast<int>(titleBounds.x),
                 static_cast<int>(titleBounds.y + (titleBounds.height - titleSize) * 0.5f),
                 titleSize, tint);
    if (!detail.empty())
        UiText::draw(fitLabel(detail, 14, detailBounds.width),
                     static_cast<int>(detailBounds.x),
                     static_cast<int>(detailBounds.y + (detailBounds.height - 14.0f) * 0.5f),
                     14, selected ? label : muted);
    return enabled && over && UiInput::pressed();
}

bool UiWidgets::ItemTile(Rectangle bounds, const std::string& iconKey, const std::string& title,
                         const std::string& quantity, bool enabled,
                         const std::function<void(Rectangle)>& drawIcon) {
    const bool over = hovered(bounds);
    const bool held = enabled && over && UiInput::down();
    UiShapes::plate(bounds, over ? UiTheme::panelRaised : UiTheme::panel,
    held || over ? UiTheme::crimson : outline, held ? 2.0f : borderWidth);
    const float iconSize = std::min(bounds.height - 34.0f, bounds.width - 20.0f);
    const Rectangle iconBounds{ bounds.x + (bounds.width - iconSize) * 0.5f,
                                bounds.y + 8.0f, iconSize, iconSize };
    if (drawIcon) drawIcon(iconBounds);
    else UiIcons::draw(iconKey, iconBounds, enabled ? WHITE : muted);
    UiText::draw(title, static_cast<int>(bounds.x + 10.0f),
                 static_cast<int>(bounds.y + bounds.height - 23.0f), 13, enabled ? UiTheme::text : muted);
    if (!quantity.empty())
        UiShapes::bladeStrip(Rectangle{ bounds.x + bounds.width - 34.0f, bounds.y + 5.0f, 28.0f, 22.0f },
                             Color{ 50, 10, 17, 240 }, UiTheme::crimson, false);
    if (!quantity.empty())
        drawLabel(Rectangle{ bounds.x + bounds.width - 34.0f, bounds.y + 5.0f, 28.0f, 22.0f },
                  quantity, 12, textOnAccent);
    return enabled && over && UiInput::pressed();
}

void UiWidgets::BeginScrollArea(Rectangle bounds, float contentHeight, float& scrollOffset) {
    const bool over = hovered(bounds);
    if (over) scrollOffset -= UiInput::wheel() * 32.0f;
    scrollOffset = std::clamp(scrollOffset, 0.0f, std::max(0.0f, contentHeight - bounds.height));
    UiShapes::plate(bounds, UiTheme::panel, outline, borderWidth);
    const float scale = UiCanvas::scale();
    scrollAreaScissor.emplace(static_cast<int>(bounds.x * scale), static_cast<int>(bounds.y * scale),
                              static_cast<int>(bounds.width * scale), static_cast<int>(bounds.height * scale));
}

void UiWidgets::EndScrollArea() { scrollAreaScissor.reset(); }

bool UiWidgets::Dialog(Rectangle bounds, const std::string& title, const std::string& body,
                       const std::string& actionLabel) {
    UiBlur::draw(bounds, UiTheme::radiusMedium, UiTheme::glassModal);
    UiShapes::plate(bounds, BLANK, UiTheme::crimson, 2.0f);
    UiText::draw(title, static_cast<int>(bounds.x + 22.0f), static_cast<int>(bounds.y + 18.0f),
                 22, UiTheme::silver);
    UiText::draw(body, static_cast<int>(bounds.x + 22.0f), static_cast<int>(bounds.y + 58.0f),
                 16, UiTheme::text);
    const Rectangle action{ bounds.x + bounds.width - 174.0f,
                            bounds.y + bounds.height - 58.0f, 152.0f, 40.0f };
    return Button(action, actionLabel);
}

void UiWidgets::Tooltip(Rectangle anchor, const std::string& text) {
    if (tooltipClaimed || !hovered(anchor)) return;
    tooltipClaimed = true;
    const Vector2 mouse = UiInput::mousePosition();
    constexpr float maxWidth = 340.0f;
    constexpr float lineHeight = 18.0f;
    const float width = std::min(maxWidth, static_cast<float>(UiText::measure(text, 14) + 24));
    std::vector<std::string> lines;
    std::istringstream words(text);
    std::string word;
    std::string line;
    while (words >> word) {
        const std::string candidate = line.empty() ? word : line + " " + word;
        if (!line.empty() && UiText::measure(candidate, 14) > width - 24.0f) {
            lines.push_back(std::move(line));
            line = word;
        } else {
            line = candidate;
        }
    }
    if (!line.empty()) lines.push_back(std::move(line));
    const float height = std::max(36.0f, static_cast<float>(lines.size()) * lineHeight + 16.0f);
    Rectangle bounds{ mouse.x + 14.0f, mouse.y + 18.0f, width, height };
    if (bounds.x + bounds.width > UiCanvas::width() - 8.0f) bounds.x = mouse.x - width - 14.0f;
    if (bounds.y + bounds.height > UiCanvas::height() - 8.0f)
        bounds.y = std::max(8.0f, mouse.y - bounds.height - 12.0f);
    UiShapes::plate(bounds, UiTheme::glassTooltip, outline, borderWidth);
    UiShapes::diamond(Vector2{ bounds.x, bounds.y + bounds.height * 0.5f },
                      8.0f, outline, false);
    for (std::size_t index = 0; index < lines.size(); ++index)
        UiText::draw(lines[index], static_cast<int>(bounds.x + 12.0f),
                     static_cast<int>(bounds.y + 8.0f + index * lineHeight), 14, UiTheme::text);
}

void UiWidgets::Pill(Rectangle bounds, const std::string& text, Color fill) {
    const auto colors = accessiblePillColors(fill);
    UiShapes::bladeStrip(bounds, colors.first, outline, false);
    drawLabel(bounds, text, 13, colors.second);
}

void UiWidgets::Gauge(Rectangle bounds, float value, const std::string& text, Color fill) {
    value = std::clamp(value, 0.0f, 1.0f);
    UiShapes::runeBar(bounds, value, fill);
    const int labelWidth = UiText::measure(text, 13) + 16;
    const Rectangle badge{ bounds.x + (bounds.width - labelWidth) * 0.5f,
                           bounds.y + (bounds.height - 22.0f) * 0.5f,
                           static_cast<float>(labelWidth), 22.0f };
    UiShapes::bladeStrip(badge, UiTheme::ink, UiTheme::ash, false);
    drawLabel(badge, text, 13, UiTheme::text);
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
    UiShapes::plate(Rectangle{ decrement.x + decrement.width + 3.0f, bounds.y,
                               bounds.width - 2.0f * buttonWidth - 6.0f, bounds.height },
                    UiTheme::panel, outline, borderWidth);
    drawLabel(bounds, std::to_string(value), 16, UiTheme::text);
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
    UiShapes::plate(bounds, UiTheme::panelRaised, outline, borderWidth);
    drawLabel(bounds, toastMessage, 16, UiTheme::text);
}
