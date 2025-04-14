#include "ViewingSmsState.hpp"
#include "ViewingSmsListState.hpp"
#include "NotConnectedState.hpp"
#include <stdexcept>

namespace ue
{

    ViewingSmsState::ViewingSmsState(Context& context, std::size_t smsIndex)
        : BaseState(context, "ViewingSmsState"),
          viewingSmsIndex(smsIndex)
    {
        const auto& allMessages = context.smsDatabase.getAllSms();

        if (viewingSmsIndex >= allMessages.size())
        {
            logger.logError("Invalid index: ", viewingSmsIndex, ", list size: ", allMessages.size());
            context.setState<ViewingSmsListState>();
            return;
        }

        const SmsMessage& selectedMessage = allMessages[viewingSmsIndex];

        logger.logInfo("Opening SMS from: ", selectedMessage.from, ", index: ", viewingSmsIndex);

        if (!selectedMessage.isRead)
        {
            logger.logDebug("Marking SMS as read at index: ", viewingSmsIndex);
            context.smsDatabase.markAsRead(viewingSmsIndex);
        }

        context.user.showSmsView(selectedMessage);
    }

    void ViewingSmsState::handleSib(common::BtsId btsId)
    {
        logger.logInfo("Ignoring SIB in ViewingSmsState from BTS: ", btsId);
    }

    void ViewingSmsState::handleUiBack()
    {
        logger.logInfo("Returning to SMS list from message view");
        context.setState<ViewingSmsListState>();
    }

    void ViewingSmsState::handleDisconnected()
    {
        logger.logInfo("Disconnected from BTS while viewing a message");
        context.setState<NotConnectedState>();
    }

    void ViewingSmsState::handleSmsReceive(common::PhoneNumber from, std::string messageText)
    {
        logger.logInfo("New SMS received while viewing another message. Sender: ", from);
        std::size_t index = context.smsDatabase.addSms(from, messageText);
        logger.logDebug("Saved incoming SMS at index: ", index);
        context.user.showSms();
    }

}
