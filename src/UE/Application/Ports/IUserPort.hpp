#pragma once

#include <Models/SmsDatabase.hpp>

namespace ue
{

class IUserEventsHandler
{
public:
    virtual ~IUserEventsHandler() = default;
};

class IUserPort
{
public:
    virtual ~IUserPort() = default;

    virtual void showNotConnected() = 0;
    virtual void showConnecting() = 0;
    virtual void showConnected() = 0;

    //virtual void showSmsList() = 0;
    //virtual void showSms(int index) = 0;
    //virtual SmsDatabase& getSmsDatabase() = 0;

    //TODO: here
};

}
