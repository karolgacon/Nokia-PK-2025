#include "ConnectedState.hpp"
#include "NotConnectedState.hpp"

namespace ue
{

ConnectedState::ConnectedState(Context &context)
    : BaseState(context, "ConnectedState")
{
    logger.logInfo("Connected state entered");
    context.user.showConnected();
}

void ConnectedState::handleDisconnected()
{
    logger.logInfo("Connection to BTS lost");
    context.setState<NotConnectedState>();
}

void ConnectedState::handleSmsReceive(common::PhoneNumber from, std::string messageText)
{
    logger.logInfo("SMS received from: ", from, ", with text: ", messageText);

    std::size_t smsIndex = context.smsDatabase.addSms(from, messageText);
    logger.logDebug("SMS stored at index: ", smsIndex);

    context.user.showSms();
}



}
