#pragma once

#include "Participant.hpp"

extern bool isEchoMode;

/// TODO:
/// Response some voice like "waiting for someone else login" ?
///
class Room {
public:
    ReturnType join(ParticipantPointer participant) {
        auto size__ = participants_.size();
        if (participants_.size() >= 2) {
            return "participants_.count()>2";
        }
        participants_.insert(participant);

        return 0;
    }



    void leave(ParticipantPointer participant) {
        participants_.erase(participant);
    }



    void processClientMessagePayload(const NetPacket& msg, ParticipantPointer sender) {
        static uint32_t counterUserMessage_ = 0;
        auto payloadType = msg.payloadType();

        bool traceDump = true;
        switch (payloadType) {
        case NetPacket::PayloadType::HeartBeatRequest: {
            sender->deliver(NetPacket(NetPacket::PayloadType::HeartBeatResponse));
            break;
        }
        case NetPacket::PayloadType::TextMessage:
        case NetPacket::PayloadType::AudioMessage:
        {
            counterUserMessage_++;

            if (isEchoMode) {
                /// echo mode
                sender->deliver(msg);;
            }
            else {
                /// broadcast mode
                for (auto participant : participants_) {
                    if (participant == sender) {
                        continue;
                    }

                    participant->deliver(msg);
                }
            }

            traceDump = false;
            break;
        }
        case NetPacket::PayloadType::LoginRequest:
        {
            //// GOT login request!!!
            sender->deliver(NetPacket(NetPacket::PayloadType::LoginResponse));
            break;
        }
        case NetPacket::PayloadType::LogoutRequest:
        {
            leave(sender);
            break;
        }

        default:
            // WHAT?
            break;
        }

        if (traceDump || counterUserMessage_ % 100 == 0) {
            BOOST_LOG_TRIVIAL(trace) << "[" << sender->info() << "] " << msg.info() ;
            if (counterUserMessage_ % 100 == 0) {
                BOOST_LOG_TRIVIAL(trace) << "\t\tgot " << counterUserMessage_ << "\tUserMessage totally";
            }
        }
    }

private:
    std::set<ParticipantPointer> participants_;
};


