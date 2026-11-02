#pragma once

#include <string>
#include <string_view>

namespace Strings {
inline constexpr char gameTitle[] = u8"Chiến Thuật Theo Lượt";
inline constexpr char menuStart[] = u8"BẮT ĐẦU";
inline constexpr char menuExit[] = u8"THOÁT";
inline constexpr char victory[] = u8"CHIẾN THẮNG!";
inline constexpr char gameOver[] = u8"THẤT BẠI";
inline constexpr char turnOrder[] = u8"THỨ TỰ LƯỢT";
inline constexpr char yourTurn[] = u8"ĐẾN LƯỢT:";
inline constexpr char skillMenuTitle[] = u8"KỸ NĂNG";
inline constexpr char itemMenuTitle[] = u8"TÚI ĐỒ";
inline constexpr char shopTitle[] = u8"Cửa hàng & Trang bị";
inline constexpr char formation[] = u8"ĐỘI HÌNH";
inline constexpr char availableEnemies[] = u8"QUÁI CÓ THỂ GẶP";
inline constexpr char bossGuards[] = u8"QUÂN HỘ TỐNG";
inline constexpr char inventoryTitle[] = u8"TÚI ĐỒ MANG VÀO TRẬN";
inline constexpr char heroSkills[] = u8"KỸ NĂNG CỦA ANH HÙNG";
inline constexpr char turnLimit[] = u8"THỜI HẠN";
inline constexpr char unlimited[] = u8"Không giới hạn";
inline constexpr char cancelTarget[] = u8"HỦY";
inline constexpr char endTurn[] = u8"KẾT THÚC LƯỢT";
inline constexpr char undo[] = u8"HOÀN TÁC";
inline constexpr char attack[] = u8"TẤN CÔNG";
inline constexpr char skill[] = u8"KỸ NĂNG";
inline constexpr char item[] = u8"VẬT PHẨM";
inline constexpr char selectWave[] = u8"CHỌN MÀN";
inline constexpr char waveDataMissing[] = u8"Không có dữ liệu màn chơi (hãy kiểm tra thư mục data/)";
inline constexpr char waveCleared[] = u8"ĐÃ HOÀN THÀNH";
inline constexpr char heroStats[] = u8"CHỈ SỐ - %s";
inline constexpr char noSelectedItem[] = u8"Chưa chọn vật phẩm";
inline constexpr char noData[] = u8"Không có dữ liệu";
inline constexpr char gold[] = u8"%d vàng";
inline constexpr char stage[] = u8"MÀN %d";
inline constexpr char damage[] = u8"%s tấn công %s gây %d sát thương!";
inline constexpr char mpCost[] = u8"Không đủ MP để dùng %s!";
inline constexpr const char* elements[] = {
    u8"Kim", u8"Mộc", u8"Thủy", u8"Hỏa", u8"Thổ"
};
inline constexpr char back[] = u8"QUAY LẠI";
inline constexpr char confirm[] = u8"XÁC NHẬN";
inline constexpr char start[] = u8"Bắt đầu";
inline constexpr char battleLog[] = u8"NHẬT KÝ TRẬN ĐẤU: ";
inline constexpr char noSkill[] = u8"Chưa có kỹ năng";
inline constexpr char heroLead[] = u8"CHỌN ĐỘI TRƯỞNG (1/3)";
inline constexpr char targetPrompt[] = u8"Nhấp vào kẻ địch còn sống";
inline constexpr char skillPrompt[] = u8"CHỌN KỸ NĂNG";
inline constexpr char itemPrompt[] = u8"CHỌN VẬT PHẨM";
inline constexpr char battleLogTitle[] = u8"NHẬT KÝ TRẬN ĐẤU";
inline constexpr char noInventoryItems[] = u8"Túi đồ không có vật phẩm dùng được trong trận!";
inline constexpr char elementPrefix[] = u8"Hệ %s";

inline std::string localizedName(std::string_view name) {
    struct NamePair { std::string_view source; std::string_view translated; };
    static constexpr NamePair names[] = {
        { "Chien Binh", u8"Chiến Binh" }, { "Warrior", u8"Chiến binh" },
        { "Priest", u8"Tu sĩ" }, { "Mage", u8"Pháp sư" }, { "Swordman", u8"Kiếm sĩ" },
        { "Assassin", u8"Sát thủ" }, { "Tanker", u8"Đỡ đòn" },
        { "Enchanter", u8"Thuật sư" }, { "Paladin", u8"Hiệp sĩ" },
        { "Ogre", u8"Quỷ khổng lồ" }, { "Human", u8"Con người" },
        { "Dwarf", u8"Người lùn" }, { "Elf", u8"Tinh linh" },
        { "Oni", u8"Quỷ Oni" }, { "Beastman", u8"Thú nhân" },
        { "Iron Goblin", u8"Yêu tinh sắt" }, { "Forest Wolf", u8"Sói rừng" },
        { "Water Slime", u8"Chất nhầy nước" }, { "Fire Imp", u8"Tiểu quỷ lửa" },
        { "Clay Golem", u8"Người đá đất sét" }, { "Goblin Elite", u8"Yêu tinh tinh nhuệ" },
        { "Alpha Wolf", u8"Sói đầu đàn" }, { "Toxic Slime", u8"Chất nhầy độc" },
        { "Flame Imp", u8"Tiểu quỷ hỏa diệm" }, { "Stone Golem", u8"Người đá" },
        { "Ember Dragon", u8"Rồng tro lửa" }, { "Ma Vuong", u8"Ma Vương" },
        { "Ground Smash", u8"Địa chấn" }, { "Thick Hide", u8"Da dày" },
        { "Adaptive Strike", u8"Đòn thích ứng" }, { "Rally", u8"Khích lệ" },
        { "Hammer Blow", u8"Búa giáng" }, { "Stone Skin", u8"Da đá" },
        { "Moonlight Arrow", u8"Tên ánh trăng" }, { "Forest Blessing", u8"Phúc lành rừng xanh" },
        { "Demon Flame", u8"Lửa quỷ" }, { "Blood Rage", u8"Cơn thịnh nộ" },
        { "Savage Claw", u8"Móng vuốt cuồng nộ" }, { "Swift Howl", u8"Tiếng hú thần tốc" },
        { "Fireball", u8"Cầu lửa" }, { "Frost Chain", u8"Xích băng" },
        { "Power Slash", u8"Trảm mạnh" }, { "Sword Dance", u8"Kiếm vũ" },
        { "Backstab", u8"Đâm sau lưng" }, { "Venom Blade", u8"Đao độc" },
        { "Shield Bash", u8"Khiên kích" }, { "Fortress", u8"Pháo đài" },
        { "Healing Wave", u8"Sóng hồi phục" }, { "Curse", u8"Lời nguyền" },
        { "Holy Strike", u8"Thánh kích" }, { "Divine Shield", u8"Khiên thần" }
    };
    for (const NamePair& entry : names)
        if (entry.source == name) return std::string(entry.translated);
    return std::string(name);
}
}
