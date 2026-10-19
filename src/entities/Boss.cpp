// Giảm sát thương gốc trước, rồi để Character::takeDamage lo phần hệ và phòng thủ.
void Boss::takeDamage(int amount, Element atkElement, bool isMagic)
{
    const int reducedAmount = static_cast<int>(static_cast<float>(amount) * DAMAGE_TAKEN_RATE);
    Character::takeDamage(reducedAmount, atkElement, isMagic);
}

// Giống Enemy nhưng sát thương gốc được nhân DAMAGE_DEALT_RATE.
void Boss::executeSkill(Skill* skill, Character* target)
{
    if (!canAct(skill, target)) return;
    const int damage = static_cast<int>(static_cast<float>(calcSkillDamage(skill)) * DAMAGE_DEALT_RATE);
    target->takeDamage(damage, skill->m_element, skill->m_isMagic);
}