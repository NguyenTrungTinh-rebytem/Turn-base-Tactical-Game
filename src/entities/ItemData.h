#pragma once
#include <string>

// Kiểu dữ liệu vật phẩm. Nằm riêng trong entities/ (không đụng data/DataTypes.h) để
// phần vật phẩm tách biệt hoàn toàn khỏi phần Data/Core của thành viên khác.

enum class ItemCategory { CONSUMABLE, BOOSTER, HELD };

enum class ItemEffect {
    // CONSUMABLE (dùng ngay, trong và ngoài trận)
    HEAL_HP_PCT,          // value = % HP tối đa
    RESTORE_MP_PCT,       // value = % MP tối đa
    REVIVE_PCT,           // value = % HP khi hồi sinh đồng đội đang gục
    // BOOSTER (dùng trong trận, tốn 1 lượt)
    BOOST_DAMAGE_UP,      // value = % sát thương gây ra,    duration = số lượt
    BOOST_DAMAGE_REDUCE,  // value = % sát thương nhận giảm, duration = số lượt
    BOOST_EXTRA_TURN,     // value = số lượt hành động thêm
    BOOST_CRIT_RATE,      // value = % tỷ lệ chí mạng cộng thêm cho đòn đánh kế tiếp
    // HELD (bị động, trang bị trước trận)
    HELD_REGEN,           // value = % HP tối đa hồi cuối mỗi lượt
    HELD_LIFESTEAL,       // value = % sát thương gây ra được hồi lại thành HP
    HELD_SURVIVE,         // value = số HP giữ lại; 1 lần/trận; chỉ khi đang đầy HP
    HELD_DAMAGE_REDUCE,   // value = % mọi sát thương nhận vào giảm
    HELD_BERSERK,         // value = % sát thương tăng; value2 = % HP tối đa mất sau khi đánh
    HELD_EXECUTE,         // value = % sát thương tăng; value2 = ngưỡng % HP của mục tiêu
    HELD_CRIT_DAMAGE,     // value = % sát thương chí mạng cộng thêm
    HELD_COUNTER          // value = % sát thương phản lại khi bị tấn công cận chiến (vật lý)
};

inline ItemCategory categoryOfEffect(ItemEffect effect) {
    switch (effect) {
    case ItemEffect::HEAL_HP_PCT:
    case ItemEffect::RESTORE_MP_PCT:
    case ItemEffect::REVIVE_PCT:
        return ItemCategory::CONSUMABLE;
    case ItemEffect::BOOST_DAMAGE_UP:
    case ItemEffect::BOOST_DAMAGE_REDUCE:
    case ItemEffect::BOOST_EXTRA_TURN:
    case ItemEffect::BOOST_CRIT_RATE:
        return ItemCategory::BOOSTER;
    default:
        return ItemCategory::HELD;
    }
}

struct ItemData {
    std::string m_id;
    std::string m_name;          // tên tiếng Anh
    std::string m_nameVi;        // tên tiếng Việt
    std::string m_descriptionVi; // mô tả (tùy chọn) do designer viết trong CSV
    ItemCategory m_category = ItemCategory::CONSUMABLE;
    ItemEffect m_effect = ItemEffect::HEAL_HP_PCT;
    int m_value = 0;
    int m_value2 = 0;
    int m_duration = 0;          // số lượt (Booster có thời hạn)
    int m_maxCarry = 0;          // số lượng tối đa mang theo trong túi (0 = không giới hạn)
};
