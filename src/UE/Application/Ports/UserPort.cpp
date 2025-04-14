#include "UserPort.hpp"
#include "UeGui/IListViewMode.hpp"
#include "UeGui/ITextMode.hpp"
 #include <string>

namespace ue
{

UserPort::UserPort(common::ILogger &logger, IUeGui &gui, common::PhoneNumber phoneNumber)
    : logger(logger, "[USER-PORT]"),
      gui(gui),
      phoneNumber(phoneNumber)
{}

void UserPort::start(IEventsHandler &handler)
{
    this->handler = &handler;
    gui.setTitle("UE Nokia " + common::to_string(phoneNumber));
    gui.setAcceptCallback(std::bind(&UserPort::onAccept, this));
    gui.setRejectCallback(std::bind(&UserPort::onReject, this));
}

void UserPort::stop()
{
    handler = nullptr;
    gui.setAcceptCallback(nullptr);
    gui.setRejectCallback(nullptr);
}

void UserPort::showNotConnected()
{
    currentViewMode = details::GuiViewMode::Unknown;
    gui.showNotConnected();
}

void UserPort::showConnecting()
{
    currentViewMode = details::GuiViewMode::Unknown;
    gui.showConnecting();
}

void UserPort::showConnected()
{
    currentViewMode = details::GuiViewMode::MainMenu;
    logger.logInfo("Displaying Main Menu");

    IUeGui::IListViewMode& menu = gui.setListViewMode();

    menu.clearSelectionList();
    menu.addSelectionListItem("New Message", "Compose and send an SMS");
    menu.addSelectionListItem("Check Messages", "Browse your SMS inbox");

    gui.showConnected();
}
void UserPort::showSms()
{
    logger.logInfo("New SMS message");
    gui.showNewSms(true);
}

void UserPort::showSmsList(const std::vector<SmsMessage>& messages)
{
    currentViewMode = details::GuiViewMode::SmsList;
    logger.logInfo("Showing inbox with ", messages.size(), " messages");

    IUeGui::IListViewMode& listView = gui.setListViewMode();
    listView.clearSelectionList();

    for (const auto& sms : messages)
    {
        std::string prefix = sms.isRead ? "  " : "* ";
        std::string label = prefix + "From: " + common::to_string(sms.from);
        listView.addSelectionListItem(label, sms.text);
    }
}

void UserPort::showSmsView(const SmsMessage& message)
{
    currentViewMode = details::GuiViewMode::SmsView;
    logger.logInfo("Showing SMS from: ", message.from);
    IUeGui::ITextMode& viewer = gui.setViewTextMode();
    std::string displayText = "From: " + common::to_string(message.from) + "\n\n" + message.text;
    viewer.setText(displayText);
}

void UserPort::showAlert(const std::string& title, const std::string& message)
{
    currentViewMode = details::GuiViewMode::Unknown;
    logger.logInfo("Alert - ", title);
    IUeGui::ITextMode& alertView = gui.setAlertMode();
    alertView.setText(title + "\n\n" + message);
}

void UserPort::onAccept()
{
    if (!handler) return;

    std::optional<std::size_t> selectedIndex;

    if (currentViewMode == details::GuiViewMode::MainMenu || currentViewMode == details::GuiViewMode::SmsList)
    {
        IUeGui::IListViewMode& currentList = gui.setListViewMode();
        auto [hasSelection, index] = currentList.getCurrentItemIndex();
        if (hasSelection)
            selectedIndex = index;
    }
    else if (currentViewMode == details::GuiViewMode::SmsView)
    {
        logger.logDebug("Accept pressed while viewing SMS - no action taken");
        return;
    }

    logger.logDebug("Accept triggered, ViewMode: ", static_cast<int>(currentViewMode), ", Index: ", selectedIndex ? std::to_string(*selectedIndex) : "none");
    handler->handleUiAction(selectedIndex);
}

void UserPort::onReject()
{
    if (!handler) return;
    logger.logDebug("Reject triggered, ViewMode: ", static_cast<int>(currentViewMode));
    handler->handleUiBack();
}


}
