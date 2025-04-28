#include "SmsDatabase.hpp"

namespace ue {

std::size_t SmsDatabase::addIncomingSms(common::PhoneNumber sender, const std::string& content)
{
    smsStorage.emplace_back(sender, content);
    return smsStorage.size() - 1;
}

std::size_t SmsDatabase::addOutgoingSms(common::PhoneNumber receiver, const std::string& content, SmsMessage::Status initialStatus)
{
    smsStorage.emplace_back(receiver, content, initialStatus);
    lastOutgoingIndex = smsStorage.size() - 1;
    return smsStorage.size() - 1;
}

bool SmsDatabase::markAsRead(std::size_t smsIndex)
{
    if (smsIndex >= smsStorage.size())
        return false;

    SmsMessage& sms = smsStorage[smsIndex];
    if (sms.direction == SmsMessage::Direction::Incoming && sms.status == SmsMessage::Status::ReceivedUnread)
    {
        sms.status = SmsMessage::Status::ReceivedRead;
        return true;
    }
    return false;
}

bool SmsDatabase::markLastOutgoingAsFailed()
{
    if (lastOutgoingIndex && *lastOutgoingIndex < smsStorage.size())
    {
        SmsMessage& lastSms = smsStorage[*lastOutgoingIndex];
        if (lastSms.direction == SmsMessage::Direction::Outgoing)
        {
            lastSms.status = SmsMessage::Status::Failed;
            lastOutgoingIndex.reset();
            return true;
        }
    }

    for (int i = static_cast<int>(smsStorage.size()) - 1; i >= 0; --i)
    {
        if (smsStorage[i].direction == SmsMessage::Direction::Outgoing &&
            smsStorage[i].status == SmsMessage::Status::Sent)
        {
            smsStorage[i].status = SmsMessage::Status::Failed;
            return true;
        }
    }

    return false;
}

const std::vector<SmsMessage> &SmsDatabase::fetchAllMessages() const
{
    return smsStorage;
}


std::size_t SmsDatabase::countUnreadMessages() const
{
    std::size_t unreadCount = 0;
    for (const auto& sms : smsStorage)
    {
        if (sms.direction == SmsMessage::Direction::Incoming && sms.status == SmsMessage::Status::ReceivedUnread)
        {
            ++unreadCount;
        }
    }
    return unreadCount;
}

} // namespace ue
