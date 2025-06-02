#pragma once

#include "Messages/PhoneNumber.hpp"
#include <string>
#include <vector>
#include <optional>

namespace ue {

    /**
     * @brief Structure representing a single SMS message.
     */
    struct SmsMessage
    {
        enum class Direction { Incoming, Outgoing };
        enum class Status { ReceivedUnread, ReceivedRead, Sent, Failed };

        common::PhoneNumber contact;
        std::string body;
        Direction direction;
        Status status;

        SmsMessage(common::PhoneNumber from, std::string bodyText)
            : contact(from), body(std::move(bodyText)), direction(Direction::Incoming), status(Status::ReceivedUnread)
        {}

        SmsMessage(common::PhoneNumber to, std::string bodyText, Status initialStatus)
            : contact(to), body(std::move(bodyText)), direction(Direction::Outgoing), status(initialStatus)
        {}
    };

    /**
     * @brief Class responsible for managing SMS storage.
     */
    class SmsDatabase
    {
    public:
        SmsDatabase() = default;

        std::size_t addIncomingSms(common::PhoneNumber sender, const std::string& content);
        std::size_t addOutgoingSms(common::PhoneNumber receiver, const std::string& content, SmsMessage::Status initialStatus = SmsMessage::Status::Sent);

        bool markAsRead(std::size_t smsIndex);
        bool markLastOutgoingAsFailed();

        const std::vector<SmsMessage>& fetchAllMessages() const;
        std::size_t countUnreadMessages() const;

    private:
        std::vector<SmsMessage> smsStorage;
        std::optional<std::size_t> lastOutgoingIndex;
    };

} // namespace ue
