#pragma once

#include "Ports/IBtsPort.hpp"
#include "Ports/IUserPort.hpp"
#include "Ports/ITimerPort.hpp"

namespace ue
{
    namespace details {
        enum class GuiViewMode {
            Unknown,
            MainMenu,
            SmsList,
            SmsView

            //TODO add more here
        };
    }

class IEventsHandler : public IBtsEventsHandler,
                       public ITimerEventsHandler,
                       public IUserEventsHandler
{
public:
    virtual ~IEventsHandler() = default;
    virtual void handleUiAction(std::optional<std::size_t> ind) = 0;
    virtual void handleUiBack() = 0;
};

}
