#include "ViewingSmsListState.hpp"
#include "ViewingSmsState.hpp"
#include "ConnectedState.hpp"
#include "NotConnectedState.hpp"

namespace ue
{

    ViewingSmsListState::ViewingSmsListState(Context& context)
        : BaseState(context, "ViewingSmsListState")
    {
        showList();
    }

    void ViewingSmsListState::showList()
    {
        currentSmsList = context.smsDatabase.getAllSms();

        if (currentSmsList.empty())
        {
            logger.logInfo("Inbox is empty, returning to main menu");
            context.user.showAlert("Inbox", "No SMS messages to display.");
            context.setState<ConnectedState>();
            return;
        }

        logger.logInfo("Displaying SMS list, count: ", currentSmsList.size());
        context.user.showSmsList(currentSmsList);
    }

    void ViewingSmsListState::handleUiAction(std::optional<std::size_t> selectedIndex)
    {
        if (!selectedIndex.has_value())
        {
            logger.logInfo("UI action received, but no index was selected");
            return;
        }

        std::size_t index = selectedIndex.value();

        if (index < currentSmsList.size())
        {
            logger.logInfo("User selected SMS at index: ", index);
            context.setState<ViewingSmsState>(index);
        }
        else
        {
            logger.logError("Selected index ", index, " is out of range. List size: ", currentSmsList.size());
        }
    }

    void ViewingSmsListState::handleSib(common::BtsId btsId)
    {
        logger.logInfo("Ignoring SIB in ViewingSmsListState from BTS: ", btsId);
    }


    void ViewingSmsListState::handleUiBack()
    {
        logger.logInfo("Returning to main menu from SMS list");
        context.setState<ConnectedState>();
    }

    void ViewingSmsListState::handleDisconnected()
    {
        logger.logInfo("Disconnected while in SMS list view");
        context.setState<NotConnectedState>();
    }

    void ViewingSmsListState::handleSmsReceive(common::PhoneNumber from, std::string messageText)
    {
        logger.logInfo("Received SMS while browsing list, sender: ", from);
        std::size_t index = context.smsDatabase.addSms(from, messageText);
        logger.logDebug("Stored new SMS at index: ", index);
        context.user.showSms();

        // Reload the list to reflect the new message
        showList();
    }

}
