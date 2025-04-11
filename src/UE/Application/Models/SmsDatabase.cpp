#include "SmsDatabase.hpp"

namespace ue {

    std::size_t SmsDatabase::addSms(common::PhoneNumber from, const std::string& text) {
        messages.emplace_back(from, text, false);
        return messages.size() - 1;
    }

    const std::vector<SmsMessage>& SmsDatabase::getAllSms() const {
        return messages;
    }

    bool SmsDatabase::markAsRead(std::size_t index) {
        if (index >= messages.size()) {
            return false;
        }

        messages[index].isRead = true;
        return true;
    }

    std::size_t SmsDatabase::getUnreadCount() const {
        std::size_t unread = 0;
        for (const auto& msg : messages) {
            if (!msg.isRead) {
                ++unread;
            }
        }
        return unread;
    }

} // namespace ue
