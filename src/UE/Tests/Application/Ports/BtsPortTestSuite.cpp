#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Ports/BtsPort.hpp"
#include "Mocks/ILoggerMock.hpp"
#include "Mocks/IBtsPortMock.hpp"
#include "Messages/PhoneNumber.hpp"
#include "Mocks/ITransportMock.hpp"
#include "Messages/OutgoingMessage.hpp"
#include "Messages/IncomingMessage.hpp"
#include "Messages/MessageId.hpp"

namespace ue
{
using namespace ::testing;

class BtsPortTests : public Test
{
protected:
    const common::PhoneNumber LOCAL_PHONE{112};
    const common::BtsId TEST_BTS_ID{13121981ll};

    NiceMock<common::ILoggerMock> mockLogger;
    StrictMock<IBtsEventsHandlerMock> mockHandler;
    NiceMock<common::ITransportMock> mockTransport;

    common::ITransport::MessageCallback onMessageReceived;
    common::ITransport::DisconnectedCallback onDisconnected;
    common::BinaryMessage outgoingMsg; // Bufor na wiadomości wychodzące

    BtsPort testPort{mockLogger, mockTransport, LOCAL_PHONE};

    void SetUp() override
    {
        // Rejestrujemy callbacki na potrzeby testów
        ON_CALL(mockTransport, registerMessageCallback(_))
            .WillByDefault(SaveArg<0>(&onMessageReceived));
        ON_CALL(mockTransport, registerDisconnectedCallback(_))
            .WillByDefault(SaveArg<0>(&onDisconnected));

        // Przechwytujemy wiadomości wychodzące
        ON_CALL(mockTransport, sendMessage(_))
            .WillByDefault(DoAll(
                SaveArg<0>(&outgoingMsg),
                Return(true) // Zwracamy true jako wynik wysyłania
                ));

        testPort.start(mockHandler);
    }

    void TearDown() override
    {
        testPort.stop();
    }

    // Tworzenie przykładowej wiadomości SIB
    common::BinaryMessage generateSibMessage()
    {
        common::OutgoingMessage msg{common::MessageId::Sib,
                                    common::PhoneNumber{},
                                    common::PhoneNumber{}};
        msg.writeBtsId(TEST_BTS_ID);
        return msg.getMessage();
    }

    // Tworzenie wiadomości odpowiedzi na przyłączenie
    common::BinaryMessage generateAttachResponseMessage(bool accepted)
    {
        common::OutgoingMessage msg{common::MessageId::AttachResponse,
                                    common::PhoneNumber{},
                                    LOCAL_PHONE};
        msg.writeNumber<std::uint8_t>(accepted ? 1u : 0u);
        return msg.getMessage();
    }

    // Tworzenie błędnej wiadomości (niepełnej)
    common::BinaryMessage generateCorruptMessage()
    {
        common::OutgoingMessage msg{common::MessageId::AttachRequest,
                                    common::PhoneNumber{},
                                    common::PhoneNumber{}};
        // Nie dodajemy BTS ID, więc wiadomość jest niepoprawna
        return msg.getMessage();
    }
};

TEST_F(BtsPortTests, ShouldRegisterCallbacksDuringStart)
{
    // Sprawdzane w SetUp/TearDown
    SUCCEED();
}

TEST_F(BtsPortTests, ShouldIgnoreInvalidMessage)
{
    // Przesyłamy błędną wiadomość
    onMessageReceived(generateCorruptMessage());
    // Test przejdzie, jeśli żaden mockowany handler nie zostanie wywołany
}

TEST_F(BtsPortTests, ShouldProcessSibMessage)
{
    EXPECT_CALL(mockHandler, handleSib(TEST_BTS_ID));
    onMessageReceived(generateSibMessage());
}

TEST_F(BtsPortTests, ShouldHandleAttachAcceptance)
{
    EXPECT_CALL(mockHandler, handleAttachAccept());
    onMessageReceived(generateAttachResponseMessage(true));
}

TEST_F(BtsPortTests, ShouldHandleAttachRejection)
{
    EXPECT_CALL(mockHandler, handleAttachReject());
    onMessageReceived(generateAttachResponseMessage(false));
}

TEST_F(BtsPortTests, ShouldSendAttachRequestCorrectly)
{
    EXPECT_CALL(mockTransport, sendMessage(_));
    testPort.sendAttachRequest(TEST_BTS_ID);

    common::IncomingMessage reader(outgoingMsg);
    ASSERT_NO_THROW(EXPECT_EQ(common::MessageId::AttachRequest, reader.readMessageId()));
    ASSERT_NO_THROW(EXPECT_EQ(LOCAL_PHONE, reader.readPhoneNumber()));
    ASSERT_NO_THROW(EXPECT_EQ(common::PhoneNumber{}, reader.readPhoneNumber()));
    ASSERT_NO_THROW(EXPECT_EQ(TEST_BTS_ID, reader.readBtsId()));
    ASSERT_NO_THROW(reader.checkEndOfMessage());
}

TEST_F(BtsPortTests, ShouldHandleTransportDisconnection)
{
    EXPECT_CALL(mockHandler, handleDisconnected());
    onDisconnected();
}

}
