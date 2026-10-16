#include "GameEngine.h"

int main()
{
    GameEngine engine(1280, 720, "Turn-based Card RPG");
    engine.run();
    return 0;
}