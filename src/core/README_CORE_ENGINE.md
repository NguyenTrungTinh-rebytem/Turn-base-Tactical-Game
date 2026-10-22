# Core Engine / Command Pattern integration

Added under `src/core/`:
- `GameStateMachine.h/.cpp`: MENU, IN_BATTLE, VICTORY, GAME_OVER.
- `ICommand.h`: command interface.
- `AttackCommand`, `UseSkillCommand`, `UseItemCommand`: callback-backed commands with undo callbacks.
- `CommandStack`: owns commands with `std::unique_ptr`, supports `cancelLastAction()`.
- `GameEngine.h/.cpp`: Raylib loop in process-events -> update -> render order.

## Integration
Create a `main.cpp` in the Visual Studio startup project:
```cpp
#include "core/GameEngine.h"

int main()
{
    GameEngine game;
    game.run();
    return 0;
}
```
Ensure Raylib include/library paths and all `src/core/*.cpp` files are added to the VS project.

The current game engine uses placeholder keyboard transitions:
- MENU: Enter -> IN_BATTLE
- IN_BATTLE: V -> VICTORY, G -> GAME_OVER
- VICTORY/GAME_OVER: Enter -> MENU
- ESC in battle: undo/cancel the most recent command

To construct a command, supply both the action and its inverse. For example:
```cpp
game.queuePlayerCommand(std::make_unique<AttackCommand>(
    [&]() { /* apply attack */ },
    [&]() { /* restore affected state */ }
));
```
Commands use callbacks deliberately because the current `Character` API does not expose enough reversible state (HP/MP/item inventory) to implement a safe generic undo without changing Member 2's interfaces. Integrate real battle actions in these callbacks, and only permit cancellation before the turn is committed.
