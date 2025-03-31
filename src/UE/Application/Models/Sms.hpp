#pragma once
#include <string>
#include <chrono>
#include <Messages/PhoneNumber.hpp>

class Sms {
private:
    common::PhoneNumber messageFrom;
    common::PhoneNumber messageTo;

    std::string textMessage;

    bool messageRead = false;
    std::chrono::time_point<std::chrono::system_clock> messageReceived;

public:
    Sms();

    Sms(const std::string &textMessage, const common::PhoneNumber &from, const common::PhoneNumber &to, bool read, const std::chrono::time_point<std::chrono::system_clock> &receive);

    const common::PhoneNumber &getFrom() const;
    const common::PhoneNumber &getTo() const;
    const std::chrono::time_point<std::chrono::system_clock> &getReceive() const;

    void markAsRead();
    bool isRead() const;
    std::string getText() const;

};

