#pragma once
namespace Strings {
enum class Key { START, EXIT, STAY, EXIT_CONFIRM, CONFIRM, BACK, SQUAD, SKILLS, INVENTORY, WAVE_INFO,
    WAVE, TURN, ATTACK, SKILL, ITEM, CANCEL, MENU, RESUME, MAIN_MENU, VICTORY, DEFEAT, RETRY,
    LOG, TARGET_HINT, COMMAND_HINT, TURN_ORDER, LEADER, STATS, CHOOSE_LEADER, NO_ITEMS, EMPTY_SLOT,
    HP, MP, ENEMY_TURN, END_TURN, UNDO, FONT_TEST };
enum class Language { VI, EN };
void setLanguage(Language lang);
Language language();
const char* get(Key key);
}
