#pragma once
#include "ICommand.h"
#include <functional>

class UseItemCommand final : public ICommand
{
public:
    UseItemCommand(std::function<void()> executeAction,
                   std::function<void()> undoAction);

    void execute() override;
    void undo() override;

private:
    std::function<void()> m_executeAction;
    std::function<void()> m_undoAction;
    bool m_executed = false;
};
