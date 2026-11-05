#include "UiDevGallery.h"
#include "UiBlur.h"
#include "UiCursor.h"
#include "UiIcons.h"
#include "UiInput.h"
#include "UiShapes.h"
#include "UiText.h"
#include "UiTheme.h"
#include "UiWidgets.h"
#include <algorithm>
#include <string>
#include <vector>

#ifndef UI_DEV_TOOLS
#define UI_DEV_TOOLS 0
#endif

namespace {
bool galleryVisible = false;
int galleryPage = 0;
int selectedTab = 0;
int quantity = 3;
float scroll = 0.0f;
float toastClock = 0.0f;
}

bool UiDevGallery::handleInput() {
#if UI_DEV_TOOLS
    if (IsKeyPressed(KEY_F10)) {
        galleryVisible = !galleryVisible;
        return true;
    }
    if (galleryVisible) {
        if (IsKeyPressed(KEY_ESCAPE)) galleryVisible = false;
        return true;
    }
    return false;
#else
    return false;
#endif
}

bool UiDevGallery::visible() {
#if UI_DEV_TOOLS
    return galleryVisible;
#else
    return false;
#endif
}

void UiDevGallery::draw() {
#if UI_DEV_TOOLS
    UiShapes::roundedRect(Rectangle{ 28, 24, 1224, 672 }, UiTheme::radiusMedium,
                          UiTheme::ink, UiTheme::accent, 2.0f);
    UiText::draw(u8"Thư viện thành phần giao diện (F10 đóng)", 52, 38, 24, UiTheme::text);
    UiText::draw(u8"Di chuột để xem hover/tooltip; nhấn để kiểm tra trạng thái tương tác.", 52, 72, 15, UiTheme::text);
    UiWidgets::TabBar(Rectangle{ 900, 42, 300, 36 },
                      { u8"Thành phần", u8"Blade & Seal" }, galleryPage);

    if (galleryPage == 1) {
        UiText::draw("S1 · blade strips — normal / hover / selected / disabled", 54, 118, 17, UiTheme::silver);
        const Color edge = UiTheme::ash;
        const Rectangle leftRows[] = {
            { 70, 152, 500, 40 }, { 70, 202, 500, 40 },
            { 70, 252, 500, 40 }, { 70, 302, 500, 40 }
        };
        const Rectangle rightRows[] = {
            { 710, 152, 500, 40 }, { 710, 202, 500, 40 },
            { 710, 252, 500, 40 }, { 710, 302, 500, 40 }
        };
        const char* const states[] = { "normal", "hover", "selected", "disabled" };
        for (int index = 0; index < 4; ++index) {
            UiShapes::bladeStrip(leftRows[index], UiTheme::panel, edge, false,
                index == 2, index == 1, index != 3);
            UiShapes::bladeStrip(rightRows[index], UiTheme::panel, edge, true,
                index == 2, index == 1, index != 3);
            UiText::draw(states[index], static_cast<int>(leftRows[index].x + 36), 164 + index * 50,
                         14, index == 2 ? UiTheme::silver : UiTheme::ashText);
            UiText::draw(states[index], static_cast<int>(rightRows[index].x + 366), 164 + index * 50,
                         14, index == 2 ? UiTheme::silver : UiTheme::ashText);
        }

        UiText::draw("S2 · plate", 60, 372, 15, UiTheme::silver);
        UiShapes::plate(Rectangle{ 62, 398, 270, 94 }, UiTheme::panelRaised, edge);
        UiText::draw("Thẻ / hộp thoại", 86, 430, 16, UiTheme::silver);
        UiText::draw("vát góc, viền trong", 86, 456, 13, UiTheme::ashText);

        UiText::draw("S3 · seals", 364, 372, 15, UiTheme::silver);
        UiShapes::seal(Rectangle{ 366, 398, 72, 72 }, edge);
        UiShapes::seal(Rectangle{ 462, 398, 72, 72 }, UiTheme::crimson, true);
        UiText::draw("thường", 366, 476, 12, UiTheme::ashText);
        UiText::draw("đang chọn", 462, 476, 12, UiTheme::ashText);

        UiText::draw("S4 · rune bars · S5 · diamond / divider", 560, 372, 15, UiTheme::silver);
        UiShapes::runeBar(Rectangle{ 560, 408, 400, 12 }, 0.72f, UiTheme::hpHigh,
                          UiTheme::danger);
        UiShapes::runeBar(Rectangle{ 560, 432, 400, 8 }, 0.46f, UiTheme::mpHigh);
        UiShapes::diamond(Vector2{ 990, 419 }, 18, UiTheme::crimsonGlow);
        UiShapes::divider(Vector2{ 1020, 420 }, Vector2{ 1200, 420 }, edge);
        UiText::draw("HP", 560, 449, 12, UiTheme::ashText);
        UiText::draw("MP", 560, 464, 12, UiTheme::ashText);

        UiText::draw("S6 · angled tabs", 62, 520, 15, UiTheme::silver);
        for (int index = 0; index < 3; ++index) {
            const Rectangle tab{ 62.0f + index * 145.0f, 548, 138, 38 };
            UiShapes::angledTab(tab, index == 1 ? Color{ 50, 10, 17, 220 } : UiTheme::panel,
                edge, index == 1, index == 2);
            UiText::draw(index == 0 ? u8"Đội hình" : index == 1 ? u8"Vật phẩm" : u8"Kỹ năng",
                static_cast<int>(tab.x + 18), 559, 14,
                index == 1 ? UiTheme::silver : UiTheme::ashText);
        }

        UiText::draw("S7 · icon seals", 560, 520, 15, UiTheme::silver);
        UiWidgets::IconButton(Rectangle{ 560, 548, 48, 48 }, "action:attack");
        UiWidgets::IconButton(Rectangle{ 624, 548, 48, 48 }, "action:pause");
        UiWidgets::IconButton(Rectangle{ 688, 548, 48, 48 }, "action:undo", false);
        UiWidgets::Tooltip(Rectangle{ 560, 548, 48, 48 }, u8"Nút biểu tượng · hover");
        UiText::draw("Ash", 760, 550, 13, UiTheme::ashText);
        UiShapes::divider(Vector2{ 760, 575 }, Vector2{ 1180, 575 }, UiTheme::ash);
        UiText::draw("chú thích: hairline + diamond", 760, 586, 13, UiTheme::ashText);
        UiWidgets::Toast();
        UiCursor::draw();
        return;
    }

    UiWidgets::Button(Rectangle{ 52, 112, 172, 44 }, u8"Nút thường");
    UiWidgets::Button(Rectangle{ 236, 112, 172, 44 }, u8"Vô hiệu hóa", false);
    UiWidgets::Tooltip(Rectangle{ 236, 112, 172, 44 }, u8"Nút không khả dụng");
    UiWidgets::IconButton(Rectangle{ 420, 112, 48, 44 }, "action:attack");
    UiWidgets::Tooltip(Rectangle{ 420, 112, 48, 44 }, u8"Tấn công");
    UiWidgets::TabBar(Rectangle{ 484, 112, 360, 44 },
                      { u8"Vật phẩm", u8"Kỹ năng", u8"Hành động" }, selectedTab);
    UiWidgets::ListRow(Rectangle{ 860, 112, 340, 56 }, u8"Phòng thủ",
                       u8"Đang chọn · trạng thái được bật", true);
    UiWidgets::ItemTile(Rectangle{ 52, 176, 112, 116 }, "item:ITM_HP_MINOR",
                        u8"Bình máu", "3");
    UiWidgets::Tooltip(Rectangle{ 52, 176, 112, 116 }, u8"Hồi phục HP");
    UiWidgets::Gauge(Rectangle{ 184, 188, 240, 32 }, 0.72f, "72 / 100",
                     Color{ 83, 196, 128, 255 });
    UiWidgets::Pill(Rectangle{ 444, 188, 96, 30 }, u8"HỎA", UiTheme::accent);
    UiWidgets::QuantityStepper(Rectangle{ 560, 182, 180, 42 }, quantity, 0, 9);
    UiWidgets::Button(Rectangle{ 760, 182, 140, 42 }, u8"Thông báo");
    UiWidgets::IconButton(Rectangle{ 1148, 182, 48, 42 }, "action:undo", false);
    UiWidgets::Tooltip(Rectangle{ 1148, 182, 48, 42 }, u8"Biểu tượng bị vô hiệu hóa");

    UiText::draw(u8"Vùng cuộn · con lăn chuột", 52, 318, 17, UiTheme::text);
    UiWidgets::ItemTile(Rectangle{ 764, 232, 82, 88 }, "item:ITM_ENERGY",
                        u8"Năng lượng", "0", false);
    UiWidgets::ListRow(Rectangle{ 860, 232, 340, 56 }, u8"Đã khóa",
                       u8"Trạng thái vô hiệu", false, false);
    UiWidgets::BeginScrollArea(Rectangle{ 52, 348, 360, 220 }, 430.0f, scroll);
    for (int row = 0; row < 7; ++row) {
        const Rectangle rowBounds{ 62, 356.0f + row * 56.0f - scroll, 340, 48 };
        UiWidgets::ListRow(rowBounds, u8"Mục danh sách " + std::to_string(row + 1),
                           u8"Cuộn để xem thêm", row == selectedTab);
    }
    UiWidgets::EndScrollArea();

    UiText::draw(u8"Biểu tượng phân loại", 444, 318, 17, UiTheme::text);
    const std::vector<std::string> iconKeys{
        "item:ITM_HP_MINOR", "skill:C_MAGE_1", "action:attack", "element:hoa"
    };
    for (std::size_t i = 0; i < iconKeys.size(); ++i) {
        const float x = 444.0f + static_cast<float>(i) * 70.0f;
        UiIcons::draw(iconKeys[i], Rectangle{ x, 350, 52, 52 });
        UiText::draw(iconKeys[i].substr(0, iconKeys[i].find(':')),
                     static_cast<int>(x - 3), 408, 11, UiTheme::text);
    }
    UiWidgets::Button(Rectangle{ 444, 448, 160, 42 }, u8"Hiện toast");
    UiWidgets::Button(Rectangle{ 620, 448, 160, 42 }, "Blur: LOW");
    UiWidgets::Button(Rectangle{ 796, 448, 160, 42 }, "Blur: HIGH");
    UiWidgets::Button(Rectangle{ 972, 448, 160, 42 }, "Blur: OFF");
    if (UiInput::pressed() && CheckCollisionPointRec(UiInput::mousePosition(),
            Rectangle{ 444, 448, 160, 42 })) {
        UiWidgets::showToast(u8"Thông báo giao diện mẫu");
    }
    if (UiInput::pressed() && CheckCollisionPointRec(UiInput::mousePosition(),
            Rectangle{ 620, 448, 160, 42 })) UiBlur::setQuality(UiBlurQuality::LOW);
    if (UiInput::pressed() && CheckCollisionPointRec(UiInput::mousePosition(),
            Rectangle{ 796, 448, 160, 42 })) UiBlur::setQuality(UiBlurQuality::HIGH);
    if (UiInput::pressed() && CheckCollisionPointRec(UiInput::mousePosition(),
            Rectangle{ 972, 448, 160, 42 })) UiBlur::setQuality(UiBlurQuality::OFF);
    UiWidgets::Dialog(Rectangle{ 444, 520, 520, 132 }, u8"Hộp thoại",
                      u8"Nền mờ hoặc lớp phủ dự phòng.", u8"Đóng");
    UiWidgets::Toast();
    UiCursor::draw();
#endif
}
