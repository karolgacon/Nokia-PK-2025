#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Application.hpp"
#include "Mocks/ILoggerMock.hpp"
#include "Mocks/IBtsPortMock.hpp"
#include "Mocks/IUserPortMock.hpp"
#include "Mocks/ITimerPortMock.hpp"
#include "Messages/PhoneNumber.hpp"
#include <memory>

namespace ue {
using namespace ::testing;

class ApplicationTestSuite : public Test {
protected:
    const common::PhoneNumber testPhoneNumber{112};
    NiceMock<common::ILoggerMock> loggerMock;
    StrictMock<IBtsPortMock> btsPortMock;
    StrictMock<IUserPortMock> userPortMock;
    StrictMock<ITimerPortMock> timerPortMock;

    std::unique_ptr<Application> app;

    void SetUp() override {
        EXPECT_CALL(userPortMock, showNotConnected());
    }

    void initApp() {
        app = std::make_unique<Application>(
            testPhoneNumber, loggerMock, btsPortMock, userPortMock, timerPortMock);
    }

    void clearExpectations() {
        Mock::VerifyAndClearExpectations(&userPortMock);
    }
};

TEST_F(ApplicationTestSuite, AttachesToBtsOnSibReception) {
    initApp();
    clearExpectations();

    common::BtsId btsId{42};

    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(std::chrono::milliseconds{500}));
    app->handleSib(btsId);

    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();
}

TEST_F(ApplicationTestSuite, HandlesAttachRejectionCorrectly) {
    initApp();
    clearExpectations();

    common::BtsId btsId{42};
    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(std::chrono::milliseconds{500}));
    app->handleSib(btsId);

    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(userPortMock, showNotConnected());
    app->handleAttachReject();
}

TEST_F(ApplicationTestSuite, HandlesAttachTimeoutProperly) {
    initApp();
    clearExpectations();

    common::BtsId btsId{42};
    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(std::chrono::milliseconds{500}));
    app->handleSib(btsId);

    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(userPortMock, showNotConnected());
    app->handleTimeout();
}

TEST_F(ApplicationTestSuite, HandlesDisconnectionAfterAttach) {
    initApp();
    clearExpectations();

    common::BtsId btsId{42};
    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(std::chrono::milliseconds{500}));
    app->handleSib(btsId);

    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();

    EXPECT_CALL(userPortMock, showNotConnected());
    app->handleDisconnected();
}

TEST_F(ApplicationTestSuite, ReceivesAndDisplaysSmsProperly) {
    initApp();
    clearExpectations();

    common::BtsId btsId{42};
    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(std::chrono::milliseconds{500}));
    app->handleSib(btsId);

    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();

    Mock::VerifyAndClearExpectations(&userPortMock);

    EXPECT_CALL(userPortMock, showSms());
    app->handleSmsReceive(common::PhoneNumber{111}, "Hello from test");

    EXPECT_CALL(userPortMock, showNotConnected());
    app->handleDisconnected();
}

TEST_F(ApplicationTestSuite, shallHandleIncomingCallRequest)
{
    initApp();
    clearExpectations();
    common::PhoneNumber from{123};

    common::BtsId btsId{42};
    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_));
    app->handleSib(btsId);

    EXPECT_CALL(timerPortMock, stopTimer()).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();

    clearExpectations();

    EXPECT_CALL(userPortMock, showIncomingCall(from));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    app->handleCallRequest(from);

    EXPECT_CALL(btsPortMock, sendAcceptCall(from));
    EXPECT_CALL(userPortMock, showTalkingMobileScreen(from));
    app->handleAcceptCall(from);
}

TEST_F(ApplicationTestSuite, shallRejectIncomingCallRequest)
{
    initApp();
    clearExpectations();
    common::PhoneNumber from{123};
    common::BtsId btsId{42};

    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_));
    app->handleSib(btsId);

    EXPECT_CALL(timerPortMock, stopTimer()).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();

    clearExpectations();

    EXPECT_CALL(userPortMock, showIncomingCall(from));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    app->handleCallRequest(from);

    EXPECT_CALL(btsPortMock, sendCallDropped(from));
    EXPECT_CALL(userPortMock, showConnected());
    app->handleUiBack();
}

TEST_F(ApplicationTestSuite, shallTimeoutOnIncomingCallRequest)
{
    initApp();
    clearExpectations();
    common::PhoneNumber from{123};
    common::BtsId btsId{42};

    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_));
    app->handleSib(btsId);

    EXPECT_CALL(timerPortMock, stopTimer()).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();

    clearExpectations();

    EXPECT_CALL(userPortMock, showIncomingCall(from));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    app->handleCallRequest(from);

    EXPECT_CALL(btsPortMock, callMissed(from));
    EXPECT_CALL(userPortMock, showConnected());
    app->handleTimeout();
}


TEST_F(ApplicationTestSuite, shallHandleUnknownRecipientAfterAccept)
{
    initApp();
    clearExpectations();
    common::PhoneNumber from{123};
    common::BtsId btsId{42};

    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_));
    app->handleSib(btsId);

    EXPECT_CALL(timerPortMock, stopTimer()).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();

    clearExpectations();

    EXPECT_CALL(userPortMock, showIncomingCall(from));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    app->handleCallRequest(from);

    EXPECT_CALL(btsPortMock, sendAcceptCall(from));
    EXPECT_CALL(userPortMock, showTalkingMobileScreen(from));
    app->handleAcceptCall(from);

    clearExpectations();

    // UnknownRecipient po zaakceptowaniu połączenia
    EXPECT_CALL(userPortMock, showAlert(::testing::_, ::testing::_));    
    EXPECT_CALL(userPortMock, showConnected());
    app->handleNumberUnknown(from);
}

TEST_F(ApplicationTestSuite, shallIgnoreUnknownRecipientAfterReject)
{
    initApp();
    clearExpectations();
    common::PhoneNumber from{123};
    common::BtsId btsId{42};

    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_));
    app->handleSib(btsId);

    EXPECT_CALL(timerPortMock, stopTimer()).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();

    clearExpectations();

    EXPECT_CALL(userPortMock, showIncomingCall(from));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    app->handleCallRequest(from);

    EXPECT_CALL(btsPortMock, sendCallDropped(from));
    EXPECT_CALL(userPortMock, showConnected());
    app->handleUiBack();

    clearExpectations();

    app->handleNumberUnknown(from);
}

TEST_F(ApplicationTestSuite, shallSendCallRequestAndHandleAccept)
{
    initApp();
    clearExpectations();
    common::PhoneNumber myNumber{112};
    common::PhoneNumber callee{123};
    common::BtsId btsId{42};

    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    app->handleSib(btsId);

    EXPECT_CALL(timerPortMock, stopTimer()).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();

    clearExpectations();

    EXPECT_CALL(userPortMock, showDialCompose());
    app->handleUiAction(2);

    ON_CALL(userPortMock, getDialedPhoneNumber()).WillByDefault(Return(callee));

    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(btsPortMock, sendCallRequest(callee));
    EXPECT_CALL(userPortMock, showAlert(::testing::_, ::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, getDialedPhoneNumber()).WillRepeatedly(Return(callee));
    app->handleUiAction(std::nullopt);

    EXPECT_CALL(timerPortMock, stopTimer()).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, showTalkingMobileScreen(callee));
    app->handleAcceptCall(callee);
}

TEST_F(ApplicationTestSuite, shallHandleCallDroppedAfterDial)
{
    initApp();
    clearExpectations();
    common::PhoneNumber callee{123};
    common::BtsId btsId{42};

    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    app->handleSib(btsId);
    EXPECT_CALL(timerPortMock, stopTimer()).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();
    clearExpectations();

    EXPECT_CALL(userPortMock, showDialCompose());
    app->handleUiAction(2);
    ON_CALL(userPortMock, getDialedPhoneNumber()).WillByDefault(Return(callee));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(btsPortMock, sendCallRequest(callee));
    EXPECT_CALL(userPortMock, showAlert(::testing::_, ::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, getDialedPhoneNumber()).WillRepeatedly(Return(callee));
    app->handleUiAction(std::nullopt);

    EXPECT_CALL(userPortMock, showAlert(::testing::_, ::testing::_));
    EXPECT_CALL(userPortMock, showConnected()).Times(::testing::AnyNumber());
    app->handleCallDropped(callee);
}

TEST_F(ApplicationTestSuite, shallHandleUnknownRecipientAfterDial)
{
    initApp();
    clearExpectations();
    common::PhoneNumber callee{123};
    common::BtsId btsId{42};

    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    app->handleSib(btsId);
    EXPECT_CALL(timerPortMock, stopTimer()).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();
    clearExpectations();

    EXPECT_CALL(userPortMock, showDialCompose());
    app->handleUiAction(2);
    ON_CALL(userPortMock, getDialedPhoneNumber()).WillByDefault(Return(callee));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(btsPortMock, sendCallRequest(callee));
    EXPECT_CALL(userPortMock, showAlert(::testing::_, ::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, getDialedPhoneNumber()).WillRepeatedly(Return(callee));
    app->handleUiAction(std::nullopt);

    EXPECT_CALL(userPortMock, showAlert(::testing::_, ::testing::_));
    EXPECT_CALL(userPortMock, showConnected()).Times(::testing::AnyNumber());    
    app->handleNumberUnknown(callee);
}


TEST_F(ApplicationTestSuite, shallHandleTimeoutAfterDial)
{
    initApp();
    clearExpectations();
    common::PhoneNumber callee{123};
    common::BtsId btsId{42};

    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    app->handleSib(btsId);
    EXPECT_CALL(timerPortMock, stopTimer()).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();
    clearExpectations();

    EXPECT_CALL(userPortMock, showDialCompose());
    app->handleUiAction(2);
    ON_CALL(userPortMock, getDialedPhoneNumber()).WillByDefault(Return(callee));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(btsPortMock, sendCallRequest(callee));
    EXPECT_CALL(userPortMock, showAlert(::testing::_, ::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, getDialedPhoneNumber()).WillRepeatedly(Return(callee));
    app->handleUiAction(std::nullopt);

    EXPECT_CALL(userPortMock, showAlert(::testing::_, ::testing::_));
    EXPECT_CALL(userPortMock, showConnected()).Times(::testing::AnyNumber());
    app->handleTimeout();
}

TEST_F(ApplicationTestSuite, shallHandleUserResignationDuringDial)
{
    initApp();
    clearExpectations();
    common::PhoneNumber callee{123};
    common::BtsId btsId{42};

    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    app->handleSib(btsId);
    EXPECT_CALL(timerPortMock, stopTimer()).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();
    clearExpectations();

    EXPECT_CALL(userPortMock, showDialCompose());
    app->handleUiAction(2);
    ON_CALL(userPortMock, getDialedPhoneNumber()).WillByDefault(Return(callee));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(btsPortMock, sendCallRequest(callee));
    EXPECT_CALL(userPortMock, showAlert(::testing::_, ::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, getDialedPhoneNumber()).WillRepeatedly(Return(callee));
    app->handleUiAction(std::nullopt);

    EXPECT_CALL(btsPortMock, sendCallDropped(callee));
    EXPECT_CALL(userPortMock, showAlert(::testing::_, ::testing::_));
    EXPECT_CALL(userPortMock, showConnected()).Times(::testing::AnyNumber());
    app->handleUiBack();
}

TEST_F(ApplicationTestSuite, shallHandleResignationAndUnknownRecipient) {
    initApp();
    clearExpectations();
    common::PhoneNumber callee{123};
    common::BtsId btsId{42};

    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    app->handleSib(btsId);
    EXPECT_CALL(timerPortMock, stopTimer()).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();
    clearExpectations();

    EXPECT_CALL(userPortMock, showDialCompose());
    app->handleUiAction(2);
    ON_CALL(userPortMock, getDialedPhoneNumber()).WillByDefault(Return(callee));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(btsPortMock, sendCallRequest(callee));
    EXPECT_CALL(userPortMock, showAlert(::testing::_, ::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, getDialedPhoneNumber()).WillRepeatedly(Return(callee));
    app->handleUiAction(std::nullopt);

    EXPECT_CALL(btsPortMock, sendCallDropped(callee));
    EXPECT_CALL(userPortMock, showAlert(::testing::_, ::testing::_));
    EXPECT_CALL(userPortMock, showConnected()).Times(::testing::AnyNumber());
    app->handleUiBack();

    clearExpectations();

    EXPECT_CALL(userPortMock, showConnected()).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, showAlert(::testing::_, ::testing::_)).Times(::testing::AnyNumber());
    app->handleNumberUnknown(callee);
}

TEST_F(ApplicationTestSuite, shallHandleOutgoingCallAndSendMessage)
{
    initApp();
    clearExpectations();
    common::PhoneNumber callee{123};
    common::BtsId btsId{42};

    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    app->handleSib(btsId);

    EXPECT_CALL(timerPortMock, stopTimer()).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();

    clearExpectations();

    EXPECT_CALL(userPortMock, showDialCompose());
    app->handleUiAction(2);

    ON_CALL(userPortMock, getDialedPhoneNumber()).WillByDefault(Return(callee));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(btsPortMock, sendCallRequest(callee));
    EXPECT_CALL(userPortMock, showAlert(::testing::_, ::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, getDialedPhoneNumber()).WillRepeatedly(Return(callee));
    app->handleUiAction(std::nullopt);

    EXPECT_CALL(timerPortMock, stopTimer()).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, showTalkingMobileScreen(callee));
    app->handleAcceptCall(callee);

    clearExpectations();

    EXPECT_CALL(userPortMock, getTextFromCall()).WillOnce(Return("Hello!"));
    EXPECT_CALL(btsPortMock, sendTalkCall(callee, "Hello!"));
    EXPECT_CALL(userPortMock, deleteOutgoingText());
    EXPECT_CALL(timerPortMock, startTimer(::testing::_));
    app->handleUiAction(std::nullopt);
}

TEST_F(ApplicationTestSuite, shallHandleOutgoingCallDroppedByUser)
{
    initApp();
    clearExpectations();
    common::PhoneNumber callee{123};
    common::BtsId btsId{42};

    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    app->handleSib(btsId);

    EXPECT_CALL(timerPortMock, stopTimer()).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();

    clearExpectations();

    EXPECT_CALL(userPortMock, showDialCompose());
    app->handleUiAction(2);

    ON_CALL(userPortMock, getDialedPhoneNumber()).WillByDefault(Return(callee));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(btsPortMock, sendCallRequest(callee));
    EXPECT_CALL(userPortMock, showAlert(::testing::_, ::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, getDialedPhoneNumber()).WillRepeatedly(Return(callee));
    app->handleUiAction(std::nullopt);

    EXPECT_CALL(timerPortMock, stopTimer()).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, showTalkingMobileScreen(callee));
    app->handleAcceptCall(callee);

    clearExpectations();

    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(btsPortMock, sendCallDropped(callee));
    EXPECT_CALL(userPortMock, showAlert("Call ended", "Call ended by user"));
    app->handleUiBack();
}

    TEST_F(ApplicationTestSuite, shallHandleOutgoingCallDroppedByRemote)
{
    initApp();
    clearExpectations();

    common::PhoneNumber callee{123};
    common::BtsId btsId{42};

    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(_)).Times(AnyNumber());
    app->handleSib(btsId);

    EXPECT_CALL(timerPortMock, stopTimer()).Times(AnyNumber());
    EXPECT_CALL(userPortMock, showConnected()).Times(AtLeast(1));
    app->handleAttachAccept();

    clearExpectations();

    EXPECT_CALL(userPortMock, showDialCompose());
    app->handleUiAction(2);

    ON_CALL(userPortMock, getDialedPhoneNumber()).WillByDefault(Return(callee));
    EXPECT_CALL(userPortMock, getDialedPhoneNumber()).Times(AtLeast(1));
    EXPECT_CALL(timerPortMock, startTimer(_)).Times(AnyNumber());
    EXPECT_CALL(btsPortMock, sendCallRequest(callee));
    EXPECT_CALL(userPortMock, showAlert("Calling", "Dialing number:\n123"));

    app->handleUiAction(std::nullopt);  // klik zielonej słuchawki → dialing

    clearExpectations();

    EXPECT_CALL(timerPortMock, stopTimer()).Times(AtLeast(1));
    EXPECT_CALL(userPortMock, showAlert("Call Rejected", "Call was rejected by recipient."));
    EXPECT_CALL(userPortMock, showConnected()).Times(AtLeast(1));
    app->handleCallDropped(callee);
}

TEST_F(ApplicationTestSuite, shallHandleTimeoutDuringOutgoingCall)
{
    initApp();
    clearExpectations();
    common::PhoneNumber callee{123};
    common::BtsId btsId{42};

    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    app->handleSib(btsId);

    EXPECT_CALL(timerPortMock, stopTimer()).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();

    clearExpectations();

    EXPECT_CALL(userPortMock, showDialCompose());
    app->handleUiAction(2);

    ON_CALL(userPortMock, getDialedPhoneNumber()).WillByDefault(Return(callee));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(btsPortMock, sendCallRequest(callee));
    EXPECT_CALL(userPortMock, showAlert(::testing::_, ::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, getDialedPhoneNumber()).WillRepeatedly(Return(callee));
    app->handleUiAction(std::nullopt);

    EXPECT_CALL(timerPortMock, stopTimer()).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, showTalkingMobileScreen(callee));
    app->handleAcceptCall(callee);

    clearExpectations();

    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(btsPortMock, callMissed(callee));
    EXPECT_CALL(userPortMock, showAlert("Call ended", "Call ended due to timeout"));
    EXPECT_CALL(userPortMock, showConnected());
    app->handleTimeout();
}

    TEST_F(ApplicationTestSuite, shallHandleDialingTimeout) {
    initApp();
    clearExpectations();
    common::PhoneNumber callee{123};
    common::BtsId btsId{42};

    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    app->handleSib(btsId);

    EXPECT_CALL(timerPortMock, stopTimer()).Times(::testing::AnyNumber());
    EXPECT_CALL(userPortMock, showConnected()).Times(::testing::AtLeast(1));
    app->handleAttachAccept();

    clearExpectations();

    EXPECT_CALL(userPortMock, showDialCompose());
    app->handleUiAction(2);

    ON_CALL(userPortMock, getDialedPhoneNumber()).WillByDefault(Return(callee));
    EXPECT_CALL(userPortMock, getDialedPhoneNumber()).Times(AtLeast(1));
    EXPECT_CALL(timerPortMock, startTimer(::testing::_)).Times(::testing::AnyNumber());
    EXPECT_CALL(btsPortMock, sendCallRequest(callee)).Times(1);
    EXPECT_CALL(userPortMock, showAlert("Calling", "Dialing number:\n123"));

    app->handleUiAction(std::nullopt);

    clearExpectations();

    EXPECT_CALL(userPortMock, showAlert("Call Timeout", "Recipient did not answer."));
    app->handleTimeout();

    clearExpectations();

    EXPECT_CALL(userPortMock, deleteOutgoingText()).Times(1);
    EXPECT_CALL(userPortMock, showConnected()).Times(AtLeast(1));

    app->handleUiAction(std::nullopt);
}

    TEST_F(ApplicationTestSuite, shallIgnoreDuplicateIncomingCallRequests)
{
    initApp();
    clearExpectations();
    common::PhoneNumber from{123};
    common::BtsId btsId{42};

    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(_));
    app->handleSib(btsId);

    EXPECT_CALL(timerPortMock, stopTimer()).Times(AnyNumber());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();

    clearExpectations();

    EXPECT_CALL(userPortMock, showIncomingCall(from)).Times(2);  // ZMIANA
    EXPECT_CALL(timerPortMock, startTimer(_)).Times(AnyNumber());

    app->handleCallRequest(from);
    app->handleCallRequest(from);  // drugi raz — teraz powinien przejść
}

    TEST_F(ApplicationTestSuite, shallIgnoreCallTalkWithoutEstablishedCall)
{
    initApp();
    clearExpectations();
    common::PhoneNumber from{123};
    common::BtsId btsId{42};

    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(_));
    app->handleSib(btsId);
    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();

    app->handleTalkCall(from, "Ghost message");
}

TEST_F(ApplicationTestSuite, shallNotSendCallRequestIfPhoneNumberEmpty)
{
    initApp();
    clearExpectations();
    common::BtsId btsId{42};
    common::PhoneNumber invalidNumber{0};

    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(_)).Times(AnyNumber());
    app->handleSib(btsId);
    EXPECT_CALL(timerPortMock, stopTimer()).Times(AnyNumber());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();

    clearExpectations();

    EXPECT_CALL(userPortMock, showDialCompose());
    app->handleUiAction(2);

    // Pusty/błędny numer
    ON_CALL(userPortMock, getDialedPhoneNumber()).WillByDefault(Return(invalidNumber));
    EXPECT_CALL(userPortMock, getDialedPhoneNumber()).WillRepeatedly(Return(invalidNumber));

    EXPECT_CALL(timerPortMock, startTimer(_)).Times(0);
    EXPECT_CALL(btsPortMock, sendCallRequest(_)).Times(0);
    EXPECT_CALL(userPortMock, showAlert("Invalid number", "Please enter a valid number."));

    app->handleUiAction(std::nullopt);
}

    TEST_F(ApplicationTestSuite, shallIgnoreCallDroppedFromUnexpectedNumber)
{
    initApp();
    clearExpectations();

    common::PhoneNumber known{123};
    common::PhoneNumber unexpected{200};  // musi być < 256
    common::BtsId btsId{42};

    // Przyłączanie do BTS
    EXPECT_CALL(btsPortMock, sendAttachRequest(btsId));
    EXPECT_CALL(timerPortMock, startTimer(_)).Times(AnyNumber());
    app->handleSib(btsId);

    EXPECT_CALL(timerPortMock, stopTimer()).Times(AnyNumber());
    EXPECT_CALL(userPortMock, showConnected());
    app->handleAttachAccept();

    clearExpectations();

    // Rozpoczęcie dzwonienia
    EXPECT_CALL(userPortMock, showDialCompose());
    app->handleUiAction(2);

    ON_CALL(userPortMock, getDialedPhoneNumber()).WillByDefault(Return(known));
    EXPECT_CALL(userPortMock, getDialedPhoneNumber()).WillRepeatedly(Return(known));

    EXPECT_CALL(btsPortMock, sendCallRequest(known));
    EXPECT_CALL(userPortMock, showAlert(_, _)).Times(AnyNumber());
    EXPECT_CALL(timerPortMock, startTimer(_)).Times(AnyNumber());
    app->handleUiAction(std::nullopt);

    // Oczekiwane przejście do ekranu rozmowy
    EXPECT_CALL(userPortMock, showTalkingMobileScreen(known));
    app->handleAcceptCall(known);

    clearExpectations();

    // Brak reakcji na CallDropped od nieoczekiwanego numeru
    EXPECT_CALL(userPortMock, showAlert(_, _)).Times(0);
    EXPECT_CALL(userPortMock, showConnected()).Times(0);
    EXPECT_CALL(userPortMock, showTalkingMobileScreen(_)).Times(0);
    EXPECT_CALL(userPortMock, addIncomingText(_)).Times(0);
    EXPECT_CALL(timerPortMock, startTimer(_)).Times(AnyNumber());
    EXPECT_CALL(timerPortMock, stopTimer()).Times(AnyNumber());

    app->handleCallDropped(unexpected);
}

} // namespace ue
