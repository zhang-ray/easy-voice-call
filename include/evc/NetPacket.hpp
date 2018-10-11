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
    enum { fixHeaderLength = 4+4+4+2+2 };
    enum { maxPayloadLength = 1<<16 };
private:
    const std::string headMarker_ = "evc ";
    std::vector<char> wholePacket_;
    void init(const unsigned short payloadType, const unsigned short payloadSize){
        wholePacket_.resize(
                    fixHeaderLength+payloadSize
                    );

        memcpy(wholePacket_.data() + 0,       headMarker_.data(), 4);
        memcpy(wholePacket_.data() + 4*3,     &payloadType,   sizeof(decltype(payloadType)));
        memcpy(wholePacket_.data() + 4*3+2,   &payloadSize,   sizeof(decltype(payloadSize)));
    }

public:
    NetPacket(){
        init(0, 0);
    }

    NetPacket(const unsigned short payloadType, const std::vector<char> payload){
        unsigned short payloadSize = payload.size();
        init(payloadType, payloadSize);

        memcpy(wholePacket_.data() + fixHeaderLength, payload.data(), payloadSize);
    }


    bool checkHeader(){
        //         TODO: check
        unsigned short payloadSize;
        memcpy(&payloadSize, wholePacket_.data()+4*3+2, sizeof(decltype(payloadSize)));
        wholePacket_.resize(fixHeaderLength + payloadSize);

        return true;
    }















    const char* header() const {return wholePacket_.data();}
    char* header() {return wholePacket_.data();}
    std::size_t wholePackLength() const{return wholePacket_.size();}


    const char* payload() const{ return wholePacket_.data()+fixHeaderLength;}
    char* payload() { return wholePacket_.data()+fixHeaderLength;}
    std::size_t payloadLength() const{return wholePacket_.size()-fixHeaderLength;}
};
