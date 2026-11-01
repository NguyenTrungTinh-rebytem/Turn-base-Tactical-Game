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
