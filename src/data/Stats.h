#pragma once

// Bộ chỉ số thống nhất cho toàn bộ nhân vật
struct Stats
{
    int m_hp  = 0;   // Máu tối đa
    int m_mp  = 0;   // Mana tối đa
    int m_a   = 0;   // Tấn công vật lý
    int m_ma  = 0;   // Tấn công phép
    int m_d   = 0;   // Phòng thủ vật lý
    int m_md  = 0;   // Phòng thủ phép
    int m_spd = 0;   // Tốc độ (TurnManager dùng xếp lượt)

    Stats operator+(const Stats& other) const {
        return Stats{ 
            m_hp + other.m_hp, m_mp + other.m_mp, 
            m_a + other.m_a, m_ma + other.m_ma, 
            m_d + other.m_d, m_md + other.m_md, 
            m_spd + other.m_spd 
        };
    }

    bool operator<(const Stats& other) const { 
        return m_spd < other.m_spd; 
    }
};
