#pragma once

// Bộ chỉ số của một nhân vật. Dùng chung cho race, class, buff...
struct Stats
{
    int m_hp  = 0;   // Máu tối đa
    int m_mp  = 0;   // Mana tối đa
    int m_a   = 0;   // Tấn công vật lý
    int m_ma  = 0;   // Tấn công phép
    int m_d   = 0;   // Phòng thủ vật lý
    int m_md  = 0;   // Phòng thủ phép
    int m_spd = 0;   // Tốc độ, TurnManager dùng để xếp thứ tự lượt

    // Cộng từng chỉ số của hai bộ Stats, ví dụ: raceStats + classStats.
    Stats operator+(const Stats& other) const
    {
        Stats result;
        result.m_hp  = m_hp  + other.m_hp;
        result.m_mp  = m_mp  + other.m_mp;
        result.m_a   = m_a   + other.m_a;
        result.m_ma  = m_ma  + other.m_ma;
        result.m_d   = m_d   + other.m_d;
        result.m_md  = m_md  + other.m_md;
        result.m_spd = m_spd + other.m_spd;
        return result;
    }

    // CHỈ so sánh theo tốc độ (m_spd), không phải toàn bộ chỉ số.
    // Khi sắp xếp giảm dần cho TurnManager: dùng (b < a) hoặc std::greater.
    bool operator<(const Stats& other) const
    {
        return m_spd < other.m_spd;
    }
};