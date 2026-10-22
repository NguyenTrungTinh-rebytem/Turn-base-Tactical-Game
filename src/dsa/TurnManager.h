#pragma once
#include <queue>
#include <vector>
class Character;
class CharacterCompare
{
public:
    bool operator()(Character* lhs, Character* rhs) const;
};
class TurnManager
{
private:
    std::priority_queue<
        Character*,
        std::vector<Character*>,
        CharacterCompare
    > m_turnQueue;
    std::vector<Character*> m_characters;
    Character* m_currentCharacter;

public:
    TurnManager();
    void addCharacter(Character* character);
    Character* getNextTurn();
    void rebuildQueue();
    void removeCharacter(Character* character);
    void clear();
    bool isEmpty() const;
    const std::vector<Character*>& getCharacters() const;
    Character* getCurrentCharacter() const;
};
