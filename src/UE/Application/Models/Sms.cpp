#include "Sms.hpp"

Sms::Sms()
{
    textMessage = "123 test 321";
}

Sms::Sms(const std::string &textMessage,
         const common::PhoneNumber &from,
         const common::PhoneNumber &to,
         bool read,
         const std::chrono::time_point<std::chrono::system_clock> &receive)
    : textMessage(textMessage),
      messageFrom(from),
      messageTo(to),
      messageRead(read),
      messageReceived(receive) {}

const common::PhoneNumber &Sms::getFrom() const {
    return messageFrom;
}

const common::PhoneNumber &Sms::getTo() const {
    return messageTo;
}

const std::chrono::time_point<std::chrono::system_clock> &Sms::getReceive() const {
    return messageReceived;
}

void Sms::markAsRead() {
    messageRead = true;
}

bool Sms::isRead() const {
    return messageRead;
}

std::string Sms::getText() const {
    return textMessage;
}
