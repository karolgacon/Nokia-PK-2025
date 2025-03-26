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

}
