#pragma once

#include <string>
#include <string_view>

namespace Strings {
inline constexpr char gameTitle[] = u8"Chiến Thuật Theo Lượt";
inline constexpr char windowTitle[] = "The First Step";
inline constexpr char menuStart[] = u8"BẮT ĐẦU";
inline constexpr char menuExit[] = u8"THOÁT";
inline constexpr char exitConfirmTitle[] = u8"Thoát trò chơi?";
inline constexpr char exitConfirmBody[] = u8"Bạn có chắc muốn rời khỏi The First Step?";
inline constexpr char cancel[] = u8"HỦY";
inline constexpr char confirmExit[] = u8"THOÁT";
inline constexpr char victory[] = u8"CHIẾN THẮNG!";
inline constexpr char gameOver[] = u8"THẤT BẠI";
inline constexpr char returnToMenu[] = u8"VỀ MENU";
inline constexpr char turnOrder[] = u8"THỨ TỰ LƯỢT";
inline constexpr char battleStageTurnTemplate[] = u8"Màn %d · Lượt %d";
inline constexpr char enemyTurn[] = u8"Lượt địch";
inline constexpr char battleLogTitle[] = u8"NHẬT KÝ";
inline constexpr char battleLogEmpty[] = u8"Chưa có diễn biến";
inline constexpr char yourTurn[] = u8"ĐẾN LƯỢT:";
inline constexpr char skillMenuTitle[] = u8"KỸ NĂNG";
inline constexpr char itemMenuTitle[] = u8"TÚI ĐỒ";
inline constexpr char shopTitle[] = u8"CỬA HÀNG & TRANG BỊ";
inline constexpr char formation[] = u8"ĐỘI HÌNH";
inline constexpr char availableEnemies[] = u8"QUÁI CÓ THỂ GẶP";
inline constexpr char bossGuards[] = u8"QUÂN HỘ TỐNG";
inline constexpr char inventoryTitle[] = u8"TÚI ĐỒ MANG VÀO TRẬN";
inline constexpr char heroSkills[] = u8"KỸ NĂNG CỦA ANH HÙNG";
inline constexpr char turnLimit[] = u8"THỜI HẠN";
inline constexpr char unlimited[] = u8"Không giới hạn";
inline constexpr char cancelTarget[] = u8"HỦY CHỌN MỤC TIÊU";
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
inline constexpr char targetPrompt[] = u8"Nhấp vào mục tiêu còn sống";
inline constexpr char skillPrompt[] = u8"CHỌN KỸ NĂNG";
inline constexpr char itemPrompt[] = u8"CHỌN VẬT PHẨM";
inline constexpr char noInventoryItems[] = u8"Túi đồ không có vật phẩm dùng được trong trận!";
inline constexpr char elementPrefix[] = u8"Hệ %s";

inline constexpr char statHp[] = u8"MÁU";
inline constexpr char statMp[] = u8"NĂNG LƯỢNG";
inline constexpr char statAttack[] = u8"CÔNG";
inline constexpr char statMagic[] = u8"PHÉP";
inline constexpr char statDefense[] = u8"THỦ";
inline constexpr char statMagicDefense[] = u8"KHÁNG PHÉP";
inline constexpr char statSpeed[] = u8"TỐC ĐỘ";
inline constexpr char raceLabel[] = u8"TỘC";
inline constexpr char classLabel[] = u8"LỚP";
inline constexpr char noHeroData[] = u8"Chưa có dữ liệu tướng (data/heroes.csv, races.csv, classes.csv)";
inline constexpr char startMenuSubtitle[] = u8"RPG chiến thuật theo lượt 2.5D";
inline constexpr char preparationTitle[] = u8"CHUẨN BỊ TRẬN ĐẤU";
inline constexpr char preparationHero[] = u8"%s  -  Anh hùng: %s";
inline constexpr char partyLeaderChange[] = u8"ĐỘI TRƯỞNG - NHẤP ĐỂ ĐỔI";
inline constexpr char partyMemberChange[] = u8"ĐỒNG ĐỘI - NHẤP ĐỂ ĐỔI";
inline constexpr char partyMember[] = u8"ĐỒNG ĐỘI %d";
inline constexpr char damageLabel[] = u8"SÁT THƯƠNG";
inline constexpr char gateLabel[] = u8"CỔNG";
inline constexpr char turnsToDefeatGate[] = u8"lượt để hạ gác cổng";
inline constexpr char elementCycleTitle[] = u8"VÒNG KHẮC HỆ";
inline constexpr char elementAdvantage[] = u8"Khắc hệ: x1,5 sát thương";
inline constexpr char elementDisadvantage[] = u8"Bị khắc hệ: x0,5 sát thương";
inline constexpr char elementCycle[] = u8"(Kim > Mộc > Thổ > Thủy > Hỏa > Kim)";
inline constexpr char boss[] = u8"TRÙM";
inline constexpr char enterBattle[] = u8"VÀO TRẬN";
inline constexpr char skillTypeDamage[] = u8"Sát thương";
inline constexpr char skillTypeHeal[] = u8"Hồi máu";
inline constexpr char skillTypeBuff[] = u8"Tăng cường";
inline constexpr char skillTypeDebuff[] = u8"Giảm sức mạnh";
inline constexpr char effectAttackBoost[] = u8"Tăng tấn công";
inline constexpr char effectDefenseBoost[] = u8"Tăng phòng thủ";
inline constexpr char effectRegeneration[] = u8"Hồi máu mỗi lượt";
inline constexpr char effectStun[] = u8"Choáng (mất lượt)";
inline constexpr char effectBurn[] = u8"Bỏng (mất máu mỗi lượt)";
inline constexpr char effectSlow[] = u8"Làm chậm";
inline constexpr char effectWeakness[] = u8"Suy yếu";
inline constexpr char skillHeader[] = u8" - Hệ ";
inline constexpr char magicDamageType[] = u8" (Phép)";
inline constexpr char physicalDamageType[] = u8" (Vật lý)";
inline constexpr char skillCostTemplate[] = u8"Tốn năng lượng: %d";
inline constexpr char skillDamageTemplate[] = u8"Sát thương: %d%% %s";
inline constexpr char magicAttack[] = u8"Công phép";
inline constexpr char physicalAttack[] = u8"Công vật lý";
inline constexpr char estimatedDamageTemplate[] = u8"Ước tính: ~%d (chưa tính khắc hệ)";
inline constexpr char healingTemplate[] = u8"Hồi phục: %d máu";
inline constexpr char effectPrefix[] = u8"Hiệu ứng: ";
inline constexpr char strongAgainstPrefix[] = u8"Mạnh trước: ";
inline constexpr char weakAgainstPrefix[] = u8"Yếu trước: ";
inline constexpr char turnsSuffix[] = u8" lượt)";
inline constexpr char waitingConfirm[] = u8" (Chờ xác nhận)";
inline constexpr char thinking[] = u8"%s (Đang suy nghĩ...)";
inline constexpr char healSuffix[] = u8" máu";
inline constexpr char energyPowerTemplate[] = u8"   Năng lượng %d  ·  %d%s";
inline constexpr char speedTemplate[] = u8"TỐC %d";
inline constexpr char consumablesCategory[] = u8"Tiêu hao";
inline constexpr char powerCategory[] = u8"Tăng sức mạnh";
inline constexpr char equipmentCategory[] = u8"Trang bị";
inline constexpr char goldBalanceTemplate[] = u8"VÀNG: %d";
inline constexpr char shopTab[] = u8"CỬA HÀNG";
inline constexpr char inventoryEquipmentTab[] = u8"TÚI ĐỒ / TRANG BỊ";
inline constexpr char close[] = u8"ĐÓNG";
inline constexpr char shopEmpty[] = u8"Cửa hàng đang trống";
inline constexpr char inventoryEmptyShopHint[] = u8"Túi đồ trống";
inline constexpr char soldOut[] = u8"ĐÃ ĐỦ";
inline constexpr char ownedCountTemplate[] = u8"Đang có %d/%d";
inline constexpr char ownedUnlimitedTemplate[] = u8"Đang có %d";
inline constexpr char priceTemplate[] = u8"Giá: %d vàng";
inline constexpr char buyPriceTemplate[] = u8"MUA · %d vàng";
inline constexpr char itemOwnedLimitTemplate[] = u8"Có %d/%d";
inline constexpr char itemOwnedTemplate[] = u8"Có %d";
inline constexpr char itemOwnedDetailLimitTemplate[] = u8"Sở hữu: %d/%d";
inline constexpr char itemOwnedDetailTemplate[] = u8"Sở hữu: %d";
inline constexpr char soldOutFull[] = u8"ĐÃ ĐỦ SỐ LƯỢNG";
inline constexpr char buy[] = u8"MUA";
inline constexpr char insufficientGold[] = u8"KHÔNG ĐỦ VÀNG";
inline constexpr char noEquipmentEquipped[] = u8"(chưa đeo trang bị)";
inline constexpr char unequip[] = u8"THÁO";
inline constexpr char waveLocked[] = u8"Màn này chưa mở khóa! Hãy hoàn thành màn trước đó.";
inline constexpr char heroSummaryTemplate[] = u8"Anh hùng: %s";
inline constexpr char additionalEnemiesTemplate[] = u8"+%d quái khác";
inline constexpr char unlockWaveTemplate[] = u8"Hoàn thành màn %d\nđể mở khóa";
inline constexpr char unavailableScreen[] = u8"Không thể mở khung giao diện";
inline constexpr char loadingBattle[] = u8"ĐANG TẢI TRẬN ĐẤU";
inline constexpr char objectiveTemplate[] = u8"Mục tiêu: %s";
inline constexpr char emptySlot[] = u8"Trống";
inline constexpr char healthTemplate[] = u8"MÁU %d";
inline constexpr char bossHealthTemplate[] = u8"HP %d";
inline constexpr char noFollowingWave[] = u8"Đây là màn cuối cùng";
inline constexpr char previousWave[] = u8"Màn trước";
inline constexpr char nextWave[] = u8"Màn tiếp theo";
inline constexpr char goldPriceTemplate[] = u8"%d vàng";

inline std::string localizedItemDescription(std::string_view description) {
    struct Phrase {
        std::string_view source;
        std::string_view translated;
    };
    static constexpr Phrase phrases[] = {
        { "Hoi sinh 1 dong doi voi ", u8"Hồi sinh 1 đồng đội với " },
        { "Dang day HP ma nhan don chi mang: giu lai ", u8"Đang đầy HP mà nhận đòn chí mạng: giữ lại " },
        { "moi sat thuong", u8"mỗi sát thương" },
        { "sat thuong gay ra", u8"sát thương gây ra" },
        { "ty le chi mang", u8"tỷ lệ chí mạng" },
        { "cho don ke tiep", u8"cho đòn kế tiếp" },
        { "luot hanh dong", u8"lượt hành động" },
        { "ngay lap tuc", u8"ngay lập tức" },
        { "cuoi moi luot", u8"cuối mỗi lượt" },
        { "HP bang", u8"HP bằng" },
        { "nhan trong", u8"nhận trong" },
        { "nhan vao", u8"nhận vào" },
        { "muc tieu duoi", u8"mục tiêu dưới" },
        { "don chi mang", u8"đòn chí mạng" },
        { "bi danh can chien", u8"bị đánh cận chiến" },
        { "1 lan/tran", u8"1 lần/trận" },
        { "Hoi ngay ", u8"Hồi ngay " },
        { "Phuc hoi ", u8"Phục hồi " },
        { "Tang ", u8"Tăng " },
        { "Giam ", u8"Giảm " },
        { "Them ", u8"Thêm " },
        { "Hoi ", u8"Hồi " },
        { "Phan lai ", u8"Phản lại " },
        { "sat thuong", u8"sát thương" },
        { "ty le", u8"tỷ lệ" },
        { "chi mang", u8"chí mạng" },
        { "gay ra", u8"gây ra" },
        { "luot", u8"lượt" },
        { "toi da", u8"tối đa" },
        { "mat ", u8"mất " }
    };

    std::string result(description);
    for (const Phrase& phrase : phrases) {
        std::size_t position = 0;
        while ((position = result.find(phrase.source, position)) != std::string::npos) {
            result.replace(position, phrase.source.size(), phrase.translated);
            position += phrase.translated.size();
        }
    }
    return result;
}

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
