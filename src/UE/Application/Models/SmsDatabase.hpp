#pragma once
#include "Sms.hpp"
#include <vector>
#include <string>
#include <optional>

class SmsDatabase {
private:
    std::vector<Sms> smsList;

public:
    SmsDatabase();

    void addSms(std::string);
    std::vector<Sms> getSmsList();
    std::optional<Sms> retrieveSms(int index);
};

