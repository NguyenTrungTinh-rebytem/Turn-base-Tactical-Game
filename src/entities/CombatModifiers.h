#pragma once
#include <array>
#include <cstddef>
#include <memory>

class Character;
class HeldGear;
struct Skill;

// Buff tạm thời do Booster tạo ra
enum class BuffType {
    DAMAGE_UP = 0,   // + % sát thương gây ra          (có thời hạn theo lượt)
    DAMAGE_REDUCE,   // - % sát thương nhận vào        (có thời hạn theo lượt)
    EXTRA_TURN,      // số lượt hành động thêm         (dùng 1 lần)
    CRIT_RATE,       // + % tỷ lệ chí mạng             (hết hiệu lực sau đòn đánh kế tiếp)
    COUNT
};

// Toàn bộ hiệu ứng của vật phẩm lên chiến đấu (buff + trang bị) nằm trong lớp này.
// Character chỉ giữ 1 đối tượng CombatModifiers và gọi vài hook -> ít đụng vào Character khi merge.
class CombatModifiers
{
public:
    static constexpr int BASE_CRIT_DAMAGE_PERCENT = 150;   // chí mạng cơ bản = 150% sát thương
    static constexpr int CRIT_GUARANTEED_RATE = 100;       // tỷ lệ chí mạng tối thiểu để chắc chắn chí mạng

    // ----- Buff -----
    // value: % (hoặc số lượt với EXTRA_TURN). turns: số lượt hiệu lực của DAMAGE_UP/DAMAGE_REDUCE.
    void applyBuff(BuffType type, int value, int turns = 0);
    void removeBuff(BuffType type);
    bool hasBuff(BuffType type) const;
    int  getBuffValue(BuffType type) const;       // 0 nếu không có
    int  getBuffTurnsLeft(BuffType type) const;
    bool consumeExtraTurn();                      // true nếu có lượt thêm (và trừ đi 1)

    // ----- Chí mạng -----
    // Chưa có tỷ lệ chí mạng cơ bản: chí mạng chỉ xảy ra khi tỷ lệ >= 100% (Precision Lens).
    int getCritRate() const;
    int getCritDamagePercent() const;             // 150 cơ bản, + bonus từ trang bị

    // ----- Trang bị (Held Gear: 1 món / nhân vật) -----
    // Trả về món cũ (nullptr nếu chưa có) để cất lại túi đồ.
    std::shared_ptr<HeldGear> equipGear(std::shared_ptr<HeldGear> gear);
    std::shared_ptr<HeldGear> unequipGear();
    const std::shared_ptr<HeldGear>& getGear() const { return m_gear; }

    // ----- Hook do Character gọi -----
    // Sát thương nhận vào (sau giáp/ngũ hành): Iron Shielding, Guardian Armor, Survival Sash
    int  applyIncoming(const Character& owner, int damage);
    // Gây sát thương theo pipeline: buff -> trang bị -> chí mạng -> takeDamage -> hút máu/phản đòn.
    // Trả về sát thương thực sự gây ra.
    int  dealDamage(Character& attacker, Character& target, const Skill& skill, int baseDamage);
    // Cuối lượt: giảm thời hạn buff, Leftover Band hồi máu
    void onTurnEnd(Character& owner);
    // Đầu trận: xóa buff, nạp lại hiệu ứng "1 lần/trận"
    void reset();

private:
    struct BuffState {
        int  m_value = 0;
        int  m_turnsLeft = 0;
        bool m_fresh = false;   // vừa áp dụng trong lượt này: chưa tính là đã qua 1 lượt
    };

    static bool isTimed(BuffType type);
    BuffState&       state(BuffType type)       { return m_buffs[static_cast<std::size_t>(type)]; }
    const BuffState& state(BuffType type) const { return m_buffs[static_cast<std::size_t>(type)]; }

    std::array<BuffState, static_cast<std::size_t>(BuffType::COUNT)> m_buffs{};
    std::shared_ptr<HeldGear> m_gear;
};
