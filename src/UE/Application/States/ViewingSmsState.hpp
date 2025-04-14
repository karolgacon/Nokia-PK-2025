#pragma once

#include "BaseState.hpp"

namespace ue
{

    class ViewingSmsState: public BaseState
    {
    public:
        ViewingSmsState(Context &context, std::size_t smsIndex);

        void handleUiBack() override;
        void handleDisconnected() override;
        void handleSmsReceive(common::PhoneNumber from, std::string messageText) override;
        void handleSib(common::BtsId btsId) override;

    private:
        std::size_t viewingSmsIndex;
    };

}