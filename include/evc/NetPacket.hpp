#pragma once
#include <cstring>
#include <string>
#include <vector>

#include "ProcessTime.hpp"
#include "git_info.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief The NetPacket class
///
/// nbByte                       | meaning
///  4                           |   fix byte array: 'evc '
///  4                           |   EVC version
///  4                           |   packet created timestamp (UNIX timestamp)
///  2                           |   packet type: heartBeat, userInfo, audioPacket
///  2                           |   nbByte of packet payload
///  nbByte of packet payload    |   payload
///
///
/// (little endian)
//////////////////////////////////////////////////////////////////////////////////////////////////////



class NetPacket{
public:
    enum { FixHeaderLength = 4+4+4+2+2 };
    enum { MaxPayloadLength = 1<<16 };

    enum class PayloadType : unsigned short {
        Undefined = 0,
        HeartBeatRequest,
        HeartBeatResponse,
        LoginRequest,
        LoginResponse,
        UserInfo,
        TextMessage,
        AudioMessage,
        LogoutRequest,
        LogoutResponse,
    };

    static auto getDescription(const PayloadType payloadType) -> const char * {
        switch (payloadType){
            case NetPacket::PayloadType::HeartBeatRequest   : return "HeartBeatRequest";
            case NetPacket::PayloadType::HeartBeatResponse  : return "HeartBeatResponse";
            case NetPacket::PayloadType::LoginRequest       : return "LoginRequest";
            case NetPacket::PayloadType::LoginResponse      : return "LoginResponse";
            case NetPacket::PayloadType::UserInfo           : return "UserInfo";
            case NetPacket::PayloadType::TextMessage        : return "TextMessage";
            case NetPacket::PayloadType::AudioMessage       : return "AudioMessage";
            case NetPacket::PayloadType::LogoutRequest      : return "LogoutRequest";
            case NetPacket::PayloadType::LogoutResponse     : return "LogoutResponse";
            
            case NetPacket::PayloadType::Undefined: 
            default:
                return "Undefined";
        }
    };

private:
    const std::string headMarker_ = "evc ";
    const std::string version_ = GIT_TAG; // TODO re-format
    std::vector<char> wholePacket_;
    void init(
        const PayloadType payloadType, 
        const unsigned short payloadSize, 
        decltype(ProcessTime::get().getProcessUptime()) upTime = (ProcessTime::get().getProcessUptime())
    ) {
        wholePacket_.resize(
                    FixHeaderLength+payloadSize
                    );

        memcpy(wholePacket_.data() + 0,       headMarker_.data(), 4);
        std::memcpy(wholePacket_.data() + 4 * 1, version_.data(), 4);
        std::memcpy(wholePacket_.data() + 4 * 2, &upTime, 4);
        memcpy(wholePacket_.data() + 4*3,     &payloadType,   sizeof(decltype(payloadType)));
        memcpy(wholePacket_.data() + 4*3+2,   &payloadSize,   sizeof(decltype(payloadSize)));
    }

public:
    NetPacket(){
        init(PayloadType::Undefined, 0);
    }

    NetPacket(const PayloadType payloadType){
        init(payloadType, 0);
    }

    NetPacket(const PayloadType payloadType, const std::vector<char> payload){
        auto payloadSize = payload.size();
        if (payloadSize > (uint16_t)(-1)) {
            throw "payloadSize larger than max of uint16_t";
        }
        init(payloadType, (uint16_t)payloadSize);

        memcpy(wholePacket_.data() + FixHeaderLength, payload.data(), (uint16_t)payloadSize);
    }


    bool checkHeader(){
        unsigned short payloadSize;
        memcpy(&payloadSize, wholePacket_.data()+4*3+2, sizeof(decltype(payloadSize)));
        wholePacket_.resize(FixHeaderLength + payloadSize);

        return true;
    }


    PayloadType payloadType() const {return *((PayloadType*)(wholePacket_.data()+ 4*3));}












    const char* header() const {return wholePacket_.data();}
    char* header() {return wholePacket_.data();}
    std::size_t wholePackLength() const{return wholePacket_.size();}


    const char* payload() const{ return wholePacket_.data()+FixHeaderLength;}
    char* payload() { return wholePacket_.data()+FixHeaderLength;}
    std::size_t payloadLength() const{return wholePacket_.size()-FixHeaderLength;}


    uint32_t timestamp() const { return *(uint32_t *)(wholePacket_.data() + (4 * 2)); }



    std::string info() const {
        std::string ret;
        ret.append("[version=");
        ret += wholePacket_[4 + 0];
        ret += wholePacket_[4 + 1];
        ret += wholePacket_[4 + 2];
        ret += wholePacket_[4 + 3];
        ret.append("] ");
        ret.append("[type=");
        ret.append(NetPacket::getDescription(payloadType()));
        ret.append("] ");
        ret.append("[wholePackLength(byte)=");
        ret.append(std::to_string(wholePackLength()));
        ret.append("] ");
        ret.append("[timestamp(s)=");
        ret.append(std::to_string(timestamp() / 1000.0));
        ret.append("] ");
        return ret;
    }
};
