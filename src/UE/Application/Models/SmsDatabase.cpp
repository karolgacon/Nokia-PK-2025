#include "SmsDatabase.hpp"
#include <Messages/PhoneNumber.hpp>


SmsDatabase::SmsDatabase() {}

void SmsDatabase::addSms(std::string) {
    smsList.push_back(Sms());
}

std::vector<Sms> SmsDatabase::getSmsList() {
    return smsList;
}

std::optional<Sms> SmsDatabase::retrieveSms(int index) {
    if (index >= 0 && index < static_cast<int>(smsList.size())) {
        smsList[index].markAsRead();
        return smsList[index];
    }

    return std::nullopt;
}

