#pragma once
#include <cstring>
#include <string>
#include <vector>

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
        UserInfo,
        TextMessage,
        AudioMessage,
    };

private:
    const std::string headMarker_ = "evc ";
    std::vector<char> wholePacket_;
    void init(const PayloadType payloadType, const unsigned short payloadSize){
        wholePacket_.resize(
                    FixHeaderLength+payloadSize
                    );

        memcpy(wholePacket_.data() + 0,       headMarker_.data(), 4);
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
        unsigned short payloadSize = payload.size();
        init(payloadType, payloadSize);

        memcpy(wholePacket_.data() + FixHeaderLength, payload.data(), payloadSize);
    }


    bool checkHeader(){
        //         TODO: check
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
};
