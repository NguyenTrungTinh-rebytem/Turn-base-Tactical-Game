#pragma once

#include <string>
#include <string_view>

namespace Strings {
inline constexpr char gameTitle[] = "NĂM PHONG ẤN";
inline constexpr char menuTagline[] = "NĂM CỔNG. MỘT THẾ GIỚI. KHÔNG CƠ HỘI THỨ HAI.";
inline constexpr char menuLoreLine1[] = "Thế giới không kết thúc...";
inline constexpr char menuLoreLine2[] = "Nó đang bị phong ấn.";
inline constexpr char exitQuestion[] = "Thoát trò chơi?";
inline constexpr char exitWarning[] = "Tiến trình hiện tại sẽ không được lưu.";
inline constexpr char exitConfirm[] = "Thoát";
inline constexpr char stay[] = "Ở lại";
inline constexpr char menuStart[] = "BẮT ĐẦU";
inline constexpr char menuExit[] = "THOÁT";
inline constexpr char victory[] = "CHIẾN THẮNG!";
inline constexpr char gameOver[] = "THẤT BẠI";
inline constexpr char turnOrder[] = "THỨ TỰ LƯỢT";
inline constexpr char yourTurn[] = "ĐẾN LƯỢT:";
inline constexpr char skillMenuTitle[] = "KỸ NĂNG";
inline constexpr char itemMenuTitle[] = "TÚI ĐỒ";
inline constexpr char shopTitle[] = "Cửa hàng & Trang bị";
inline constexpr char formation[] = "ĐỘI HÌNH";
inline constexpr char availableEnemies[] = "QUÁI CÓ THỂ GẶP";
inline constexpr char bossGuards[] = "QUÂN HỘ TỐNG";
inline constexpr char inventoryTitle[] = "TÚI ĐỒ MANG VÀO TRẬN";
inline constexpr char heroSkills[] = "KỸ NĂNG CỦA ANH HÙNG";
inline constexpr char turnLimit[] = "THỜI HẠN";
inline constexpr char unlimited[] = "Không giới hạn";
inline constexpr char cancelTarget[] = "HỦY";
inline constexpr char endTurn[] = "KẾT THÚC LƯỢT";
inline constexpr char undo[] = "HOÀN TÁC";
inline constexpr char attack[] = "TẤN CÔNG";
inline constexpr char skill[] = "KỸ NĂNG";
inline constexpr char item[] = "VẬT PHẨM";
inline constexpr char selectWave[] = "CHỌN MÀN";
inline constexpr char waveDataMissing[] = "Không có dữ liệu màn chơi (hãy kiểm tra thư mục data/)";
inline constexpr char waveCleared[] = "ĐÃ HOÀN THÀNH";
inline constexpr char heroStats[] = "CHỈ SỐ - %s";
inline constexpr char noSelectedItem[] = "Chưa chọn vật phẩm";
inline constexpr char noData[] = "Không có dữ liệu";
inline constexpr char gold[] = "%d vàng";
inline constexpr char roundTurn[] = "Màn %d · Lượt %d";
inline constexpr char stage[] = "MÀN %d";
inline constexpr char damage[] = "%s tấn công %s gây %d sát thương!";
inline constexpr char mpCost[] = "Không đủ MP để dùng %s!";
inline constexpr const char* elements[] = {
    "Kim", "Mộc", "Thủy", "Hỏa", "Thổ"
};
inline constexpr char back[] = "QUAY LẠI";
inline constexpr char confirm[] = "XÁC NHẬN";
inline constexpr char start[] = "Bắt đầu";
inline constexpr char battleLog[] = "NHẬT KÝ TRẬN ĐẤU: ";
inline constexpr char noSkill[] = "Chưa có kỹ năng";
inline constexpr char heroLead[] = "CHỌN ĐỘI TRƯỞNG (1/3)";
inline constexpr char targetPrompt[] = "Nhấp vào kẻ địch còn sống";
inline constexpr char skillPrompt[] = "CHỌN KỸ NĂNG";
inline constexpr char itemPrompt[] = "CHỌN VẬT PHẨM";
inline constexpr char battleLogTitle[] = "NHẬT KÝ TRẬN ĐẤU";
inline constexpr char noInventoryItems[] = "Túi đồ không có vật phẩm dùng được trong trận!";
inline constexpr char elementPrefix[] = "Hệ %s";

inline std::string localizedName(std::string_view name) {
    struct NamePair { std::string_view source; std::string_view translated; };
    static constexpr NamePair names[] = {
        { "Chien Binh", "Chiến Binh" }, { "Warrior", "Chiến binh" },
        { "Priest", "Tu sĩ" }, { "Mage", "Pháp sư" }, { "Swordman", "Kiếm sĩ" },
        { "Assassin", "Sát thủ" }, { "Tanker", "Đỡ đòn" },
        { "Enchanter", "Thuật sư" }, { "Paladin", "Hiệp sĩ" },
        { "Ogre", "Quỷ khổng lồ" }, { "Human", "Con người" },
        { "Dwarf", "Người lùn" }, { "Elf", "Tinh linh" },
        { "Oni", "Quỷ Oni" }, { "Beastman", "Thú nhân" },
        { "Iron Goblin", "Yêu tinh sắt" }, { "Forest Wolf", "Sói rừng" },
        { "Water Slime", "Chất nhầy nước" }, { "Fire Imp", "Tiểu quỷ lửa" },
        { "Clay Golem", "Người đá đất sét" }, { "Goblin Elite", "Yêu tinh tinh nhuệ" },
        { "Alpha Wolf", "Sói đầu đàn" }, { "Toxic Slime", "Chất nhầy độc" },
        { "Flame Imp", "Tiểu quỷ hỏa diệm" }, { "Stone Golem", "Người đá" },
        { "Ember Dragon", "Rồng tro lửa" }, { "Ma Vuong", "Ma Vương" },
        { "Ground Smash", "Địa chấn" }, { "Thick Hide", "Da dày" },
        { "Adaptive Strike", "Đòn thích ứng" }, { "Rally", "Khích lệ" },
        { "Hammer Blow", "Búa giáng" }, { "Stone Skin", "Da đá" },
        { "Moonlight Arrow", "Tên ánh trăng" }, { "Forest Blessing", "Phúc lành rừng xanh" },
        { "Demon Flame", "Lửa quỷ" }, { "Blood Rage", "Cơn thịnh nộ" },
        { "Savage Claw", "Móng vuốt cuồng nộ" }, { "Swift Howl", "Tiếng hú thần tốc" },
        { "Fireball", "Cầu lửa" }, { "Frost Chain", "Xích băng" },
        { "Power Slash", "Trảm mạnh" }, { "Sword Dance", "Kiếm vũ" },
        { "Backstab", "Đâm sau lưng" }, { "Venom Blade", "Đao độc" },
        { "Shield Bash", "Khiên kích" }, { "Fortress", "Pháo đài" },
        { "Healing Wave", "Sóng hồi phục" }, { "Curse", "Lời nguyền" },
        { "Holy Strike", "Thánh kích" }, { "Divine Shield", "Khiên thần" }
    };
    for (const NamePair& entry : names)
        if (entry.source == name) return std::string(entry.translated);
    return std::string(name);
}
}
