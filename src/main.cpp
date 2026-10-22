#include "core/GameEngine.h"
#include <iostream>
#include <exception>

int main() {
    try {
        GameEngine engine;
        engine.run();
    } 
    catch (const std::exception& e) {
        std::cerr << "Game Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
