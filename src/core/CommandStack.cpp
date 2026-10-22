#include "CommandStack.h"
#include <utility>

void CommandStack::executeCommand(std::unique_ptr<ICommand> command)
{
    if (!command)
        return;

    command->execute();
    m_commands.push_back(std::move(command));
}

bool CommandStack::cancelLastAction()
{
    if (m_commands.empty())
        return false;

    m_commands.back()->undo();
    m_commands.pop_back();
    return true;
}

void CommandStack::clear()
{
    m_commands.clear();
}

bool CommandStack::empty() const
{
    return m_commands.empty();
}

std::size_t CommandStack::size() const
{
    return m_commands.size();
}
