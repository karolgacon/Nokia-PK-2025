#pragma once

#include "IUserPort.hpp"
#include "Logger/PrefixedLogger.hpp"
#include "IUeGui.hpp"
#include "Messages/PhoneNumber.hpp"
#include "IEventsHandler.hpp"

namespace ue
{

class UserPort : public IUserPort
{
public:
    UserPort(common::ILogger& logger, IUeGui& gui, common::PhoneNumber phoneNumber);
    void start(IEventsHandler& handler);
    void stop();

    void showNotConnected() override;
    void showConnecting() override;
    void showConnected() override;
    void showSms() override;
    void showSmsList(const std::vector<SmsMessage>& textMessages) override;
    void showSmsView(const SmsMessage& textMessage) override;
    void showAlert(const std::string& title, const std::string& textMessage) override;

private:
    void onAccept(); // reff to callback
    void onReject();

    common::PrefixedLogger logger;
    IUeGui& gui;
    common::PhoneNumber phoneNumber;
    IEventsHandler* handler = nullptr;
    details::GuiViewMode currentViewMode = details::GuiViewMode::Unknown;
};

}
