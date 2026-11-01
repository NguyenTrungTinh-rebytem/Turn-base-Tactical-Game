#pragma once

#include "raylib.h"
#include <string>
#include <vector>

namespace UiWidgets {
void beginFrame();
bool Button(Rectangle bounds, const std::string& label, bool enabled = true);
bool SecondaryButton(Rectangle bounds, const std::string& label, bool enabled = true);
bool DangerButton(Rectangle bounds, const std::string& label, bool enabled = true);
bool IconButton(Rectangle bounds, const std::string& iconKey, bool enabled = true);
bool TabBar(Rectangle bounds, const std::vector<std::string>& labels, int& selected);
bool ListRow(Rectangle bounds, const std::string& title, const std::string& detail = {},
             bool selected = false, bool enabled = true);
bool ItemTile(Rectangle bounds, const std::string& iconKey, const std::string& label,
              const std::string& quantity = {}, bool enabled = true);
void BeginScrollArea(Rectangle bounds, float contentHeight, float& scrollOffset);
void EndScrollArea();
bool Dialog(Rectangle bounds, const std::string& title, const std::string& body,
            const std::string& actionLabel);
enum class DialogAction { NONE, CONFIRM, CANCEL };
DialogAction DialogWithCancel(Rectangle bounds, const std::string& title,
                              const std::string& body, const std::string& actionLabel,
                              const std::string& cancelLabel);
void Tooltip(Rectangle anchor, const std::string& text);
void DrawTooltip();
void Pill(Rectangle bounds, const std::string& text, Color fill);
bool Breadcrumb(Rectangle bounds, const std::vector<std::string>& labels, int& selected);
void Gauge(Rectangle bounds, float value, const std::string& label, Color fill);
bool QuantityStepper(Rectangle bounds, int& value, int minimum, int maximum);
void showToast(const std::string& message, float duration = 2.5f);
void update(float deltaTime);
void Toast();
}
