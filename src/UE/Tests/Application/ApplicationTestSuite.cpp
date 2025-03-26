#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Application.hpp"
#include "Mocks/ILoggerMock.hpp"
#include "Mocks/IBtsPortMock.hpp"
#include "Mocks/IUserPortMock.hpp"
#include "Mocks/ITimerPortMock.hpp"
#include "Messages/PhoneNumber.hpp"
#include <memory>

namespace ue
{
using namespace ::testing;

class ApplicationTests : public Test
{
protected:
    const common::PhoneNumber TEST_PHONE_NUMBER{112};
    NiceMock<common::ILoggerMock> mockLogger;
    StrictMock<IBtsPortMock> mockBtsPort;
    StrictMock<IUserPortMock> mockUserPort;
    StrictMock<ITimerPortMock> mockTimerPort;

    std::unique_ptr<Application> appInstance;

    void SetUp() override {
        // Oczekiwane działanie przy uruchomieniu aplikacji
        EXPECT_CALL(mockUserPort, showNotConnected());
    }

    void initializeApplication() {
        appInstance = std::make_unique<Application>(TEST_PHONE_NUMBER,
                                                    mockLogger,
                                                    mockBtsPort,
                                                    mockUserPort,
                                                    mockTimerPort);
    }

    void clearExpectations() {
        // Czyścimy oczekiwania po starcie aplikacji
        Mock::VerifyAndClearExpectations(&mockUserPort);
    }
};

struct AppDisconnectedTests : ApplicationTests
{};

TEST_F(ApplicationTests, ShouldAttachToBtsOnReceivingSib)
{
    // Uruchamiamy aplikację
    initializeApplication();
    clearExpectations();

    // GIVEN - BTS wysyła SIB
    common::BtsId testBtsId{42};

    // WHEN - Oczekujemy wysłania żądania przyłączenia
    EXPECT_CALL(mockBtsPort, sendAttachRequest(testBtsId));
    EXPECT_CALL(mockTimerPort, startTimer(std::chrono::milliseconds{500}));
    appInstance->handleSib(testBtsId);

    // WHEN - BTS akceptuje przyłączenie
    EXPECT_CALL(mockTimerPort, stopTimer());
    EXPECT_CALL(mockUserPort, showConnected());
    appInstance->handleAttachAccept();
}

TEST_F(ApplicationTests, ShouldHandleAttachRejection)
{
    // Inicjalizacja aplikacji
    initializeApplication();
    clearExpectations();

    // GIVEN - BTS wysyła SIB
    common::BtsId testBtsId{42};

    // WHEN - UE próbuje się połączyć
    EXPECT_CALL(mockBtsPort, sendAttachRequest(testBtsId));
    EXPECT_CALL(mockTimerPort, startTimer(std::chrono::milliseconds{500}));
    appInstance->handleSib(testBtsId);

    // WHEN - BTS odrzuca połączenie
    EXPECT_CALL(mockTimerPort, stopTimer());
    EXPECT_CALL(mockUserPort, showNotConnected());
    appInstance->handleAttachReject();
}

TEST_F(ApplicationTests, ShouldHandleAttachTimeout)
{
    // Inicjalizacja aplikacji
    initializeApplication();
    clearExpectations();

    // GIVEN - BTS wysyła SIB
    common::BtsId testBtsId{42};

    // WHEN - UE próbuje się połączyć, ale BTS nie odpowiada
    EXPECT_CALL(mockBtsPort, sendAttachRequest(testBtsId));
    EXPECT_CALL(mockTimerPort, startTimer(std::chrono::milliseconds{500}));
    appInstance->handleSib(testBtsId);

    // WHEN - Upływa czas oczekiwania na odpowiedź BTS
    EXPECT_CALL(mockTimerPort, stopTimer());
    EXPECT_CALL(mockUserPort, showNotConnected());
    appInstance->handleTimeout();
}

TEST_F(ApplicationTests, ShouldHandleDisconnection)
{
    // Inicjalizacja aplikacji
    initializeApplication();
    clearExpectations();

    // Połączenie z BTS
    common::BtsId testBtsId{42};
    EXPECT_CALL(mockBtsPort, sendAttachRequest(testBtsId));
    EXPECT_CALL(mockTimerPort, startTimer(std::chrono::milliseconds{500}));
    appInstance->handleSib(testBtsId);

    // BTS akceptuje połączenie
    EXPECT_CALL(mockTimerPort, stopTimer());
    EXPECT_CALL(mockUserPort, showConnected());
    appInstance->handleAttachAccept();

    // UE zostaje rozłączony
    EXPECT_CALL(mockUserPort, showNotConnected());
    appInstance->handleDisconnected();
}

}
