#include "Application.hpp"
#include "ApplicationEnvironmentFactory.hpp"
#include "Ports/BtsPort.hpp"
#include "Ports/UserPort.hpp"
#include "Ports/TimerPort.hpp"

int main(int argc, char* argv[])
{
    using namespace ue;
    using namespace std::chrono_literals;

    auto appEnv = ue::createApplicationEnvironment(argc, argv);
    auto& logger = appEnv->getLogger();
    auto& tranport = appEnv->getTransportToBts();
    auto& gui = appEnv->getUeGui();
    auto phoneNumber = appEnv->getMyPhoneNumber();

    BtsPort bts(logger, tranport, phoneNumber);
    UserPort user(logger, gui, phoneNumber);
    TimerPort timer(logger);
    Application app(phoneNumber, logger, bts, user, timer);

    bts.start(app);
    user.start(app);
    timer.start(app);

    // SIMULATION  -  manual bts connection
    app.handleSib(common::BtsId{1});
    app.handleAttachAccept();

    // TEST  -  receive
    app.handleSmsReceive(common::PhoneNumber{122}, "Hello test z BTS");
    app.handleSmsReceive(common::PhoneNumber{45}, "Second message!");

    appEnv->startMessageLoop();

    bts.stop();
    user.stop();
    timer.stop();
}

