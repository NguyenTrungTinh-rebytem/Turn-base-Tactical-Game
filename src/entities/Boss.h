// Boss: trâu hơn và đau hơn Enemy thường.
class Boss : public Character
{
public:
    Boss(const std::string& name, const Stats& stats, Element element);

    void takeDamage(int amount, Element atkElement, bool isMagic) override;  // Chỉ nhận 80% sát thương
    void executeSkill(Skill* skill, Character* target) override;             // Gây 125% sát thương

private:
    static constexpr float DAMAGE_TAKEN_RATE = 0.8f;   // Hệ số sát thương nhận vào
    static constexpr float DAMAGE_DEALT_RATE = 1.25f;  // Hệ số sát thương gây ra
};