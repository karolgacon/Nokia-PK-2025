#pragma once

#include "Messages/PhoneNumber.hpp"
#include <string>
#include <vector>

namespace ue {

    /**
     * @brief Structure representing a single SMS message
     */
    struct SmsMessage {
        common::PhoneNumber from;
        std::string text;
        bool isRead;

        SmsMessage(common::PhoneNumber sender, std::string content, bool read = false)
            : from(sender), text(std::move(content)), isRead(read) {}
    };

    class SmsDatabase {
    public:
        SmsDatabase() = default;

        // Add a new SMS to the database
        std::size_t addSms(common::PhoneNumber from, const std::string& text);

        // Get all SMS messages
        const std::vector<SmsMessage>& getAllSms() const;

        // Mark an SMS as read
        bool markAsRead(std::size_t index);

        // Get number of unread messages
        std::size_t getUnreadCount() const;

    private:
        std::vector<SmsMessage> messages;
    };

} // namespace ue
