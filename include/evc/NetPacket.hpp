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
    enum { header_length = 4+4+4+2+2 };
    enum { max_body_length = 1<<16 };
private:
    const std::string headMarker_ = "evc ";
    std::vector<char> wholePacket_;
    void init(const unsigned short payloadType, const unsigned short payloadSize){
        wholePacket_.resize(
                    header_length+payloadSize
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

        memcpy(wholePacket_.data() + header_length, payload.data(), payloadSize);
    }

    static NetPacket *makeOnePack(const char *p, const std::size_t size){
        if (size<header_length){
            return nullptr;
        }
        auto pack = new NetPacket();
//        pack->data()
        return pack;
    }
    bool decodeHeader(){
//         TODO: check
        unsigned short payloadSize;
        memcpy(&payloadSize, wholePacket_.data()+4*3+2, sizeof(decltype(payloadSize)));
        wholePacket_.resize(header_length + payloadSize);

        return true;
    }












    const char* data() const {return wholePacket_.data();}
    char* data() {return wholePacket_.data();}
    std::size_t length() const{return wholePacket_.size();}

    const char* body() const{ return wholePacket_.data()+header_length;}
    char* body() { return wholePacket_.data()+header_length;}
    std::size_t body_length() const{return wholePacket_.size()-header_length;}
};
