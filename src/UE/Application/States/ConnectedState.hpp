#pragma once

#include "BaseState.hpp"

namespace ue
{

class ConnectedState : public BaseState
{
public:
    explicit ConnectedState(Context &context);
    void handleDisconnected() override;
    void handleSmsReceive(common::PhoneNumber from, std::string messageText) override;

};

}
