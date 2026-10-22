#include "UseItemCommand.h"
#include <utility>

UseItemCommand::UseItemCommand(std::function<void()> executeAction,
                               std::function<void()> undoAction)
    : m_executeAction(std::move(executeAction)),
      m_undoAction(std::move(undoAction))
{
}

void UseItemCommand::execute()
{
    if (m_executed || !m_executeAction)
        return;

    m_executeAction();
    m_executed = true;
}

void UseItemCommand::undo()
{
    if (!m_executed)
        return;

    if (m_undoAction)
        m_undoAction();

    m_executed = false;
}
