#pragma once

struct Stats
{
    int m_hp  = 0;
    int m_mp  = 0;
    int m_a   = 0;
    int m_ma  = 0;
    int m_d   = 0;
    int m_md  = 0;
    int m_spd = 0;

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

    bool operator<(const Stats& other) const
    {
        return m_spd < other.m_spd;
    }
};
