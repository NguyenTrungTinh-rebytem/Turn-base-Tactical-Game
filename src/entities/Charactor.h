#pragma once
#include <string>
struct CharacterStats
{
    int hp;
    int maxHp;
    int atk;
    int def;
    int spd;
};
class Character
{
protected:
    std::string m_name;
    CharacterStats m_stats;
    int m_currentHp;
    bool m_isStunned;

public:
    Character(
        const std::string& name,
        int hp,
        int atk,
        int def,
        int spd
    )
        : m_name(name),
          m_currentHp(hp),
          m_isStunned(false)
    {
        m_stats.hp = hp;
        m_stats.maxHp = hp;
        m_stats.atk = atk;
        m_stats.def = def;
        m_stats.spd = spd;
    }
    virtual ~Character() = default;
    const std::string& getName() const
    {
        return m_name;
    }
    const CharacterStats& getStats() const
    {
        return m_stats;
    }
    int getCurrentHp() const
    {
        return m_currentHp;
    }
    void setCurrentHp(int hp)
    {
        if (hp < 0)
        {
            hp = 0;
        }

        if (hp > m_stats.maxHp)
        {
            hp = m_stats.maxHp;
        }

        m_currentHp = hp;
        m_stats.hp = hp;
    }
    bool isAlive() const
    {
        return m_currentHp > 0;
    }
    bool isStunned() const
    {
        return m_isStunned;
    }
    void setStunned(bool stunned)
    {
        m_isStunned = stunned;
    }
    void takeDamage(int damage)
    {
        if (damage < 0)
        {
            damage = 0;
        }

        setCurrentHp(m_currentHp - damage);
    }
    void heal(int amount)
    {
        if (amount < 0)
        {
            amount = 0;
        }

        setCurrentHp(m_currentHp + amount);
    }
    void setSpeed(int speed)
    {
        if (speed < 0)
        {
            speed = 0;
        }

        m_stats.spd = speed;
    }
    bool canAct() const
    {
        return isAlive() && !m_isStunned;
    }
};
