#pragma once
#include <memory>
#include <vector>
#include "ICommand.h"

class CommandStack
{
public:
    void executeCommand(std::unique_ptr<ICommand> command);
    bool cancelLastAction();
    void clear();
    bool empty() const;
    std::size_t size() const;

private:
    std::vector<std::unique_ptr<ICommand>> m_commands;
};
