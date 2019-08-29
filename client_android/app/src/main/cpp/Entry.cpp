extern "C" {
#include <jni.h>
}

#include <string>
#include "NetPacket.hpp"



namespace {

    std::shared_ptr<NetPacket> makePacket(const char *data, std::size_t bytes_recvd)
    {
        if (bytes_recvd < NetPacket::FixHeaderLength) {
            return nullptr;
        }

        auto ret = std::make_shared<NetPacket>();
        std::memcpy(ret->header(), data, NetPacket::FixHeaderLength);
        if (!ret->checkHeader()) {
            return nullptr;
        }


        if (bytes_recvd < NetPacket::FixHeaderLength + ret->payloadLength()) {
            return nullptr;
        }

        std::memcpy(ret->payload(), data + NetPacket::FixHeaderLength, ret->payloadLength());
        return ret;
    }

}

extern "C" JNIEXPORT jstring JNICALL
Java_com_zhang_1ray_easyvoicecall_Worker_getVersion(JNIEnv* env, jobject) {
    std::string version = "v0.0.1";
    return env->NewStringUTF(version.c_str());
}


extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_zhang_1ray_easyvoicecall_Worker_createNetPacket(JNIEnv* env, jobject, jint type) {
    NetPacket netPacket((NetPacket::PayloadType)type);
    auto jarrRV = env->NewByteArray(netPacket.wholePackLength());


    env->SetByteArrayRegion(jarrRV, 0, netPacket.wholePackLength(), (const jbyte*)(netPacket.header()));

    return jarrRV;
}


extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_zhang_1ray_easyvoicecall_Worker_createNetPacketWithSN(JNIEnv* env, jobject, jint type, jint serialNumber) {
    NetPacket netPacket((NetPacket::PayloadType)type, (uint32_t) serialNumber);
    auto jarrRV = env->NewByteArray(netPacket.wholePackLength());

    env->SetByteArrayRegion(jarrRV, 0, netPacket.wholePackLength(), (const jbyte*)(netPacket.header()));

    return jarrRV;
}


extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_zhang_1ray_easyvoicecall_Worker_createNetPacketWithPayload(JNIEnv* env, jobject, jint type, jint serialNumber, jbyteArray data) {
    jbyte* bytedata = (env)->GetByteArrayElements(data, 0);
    jsize nbInputByte = (env)->GetArrayLength(data);

    std::vector<int8_t> payload(nbInputByte);
    memcpy(payload.data(), bytedata, nbInputByte);

    NetPacket netPacket((NetPacket::PayloadType)type, (uint32_t) serialNumber, payload);
    auto jarrRV = env->NewByteArray(netPacket.wholePackLength());

    env->SetByteArrayRegion(jarrRV, 0, netPacket.wholePackLength(), (const jbyte*)(netPacket.header()));

    return jarrRV;
}



extern "C" JNIEXPORT jint JNICALL
Java_com_zhang_1ray_easyvoicecall_Worker_getNetPacketType(JNIEnv* env, jobject, jbyteArray jbyteArrayNetPacket) {
    jbyte* bytedata = (env)->GetByteArrayElements(jbyteArrayNetPacket, 0);
    jsize nbInputByte = (env)->GetArrayLength(jbyteArrayNetPacket);

    auto netPacket = makePacket((const char*)bytedata, nbInputByte);

    return (jint) (netPacket->payloadType());
}




extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_zhang_1ray_easyvoicecall_Worker_getNetPacketPayload(JNIEnv* env, jobject, jbyteArray jbyteArrayNetPacket) {
    jbyte* bytedata = (env)->GetByteArrayElements(jbyteArrayNetPacket, 0);
    jsize nbInputByte = (env)->GetArrayLength(jbyteArrayNetPacket);

    auto netPacket = makePacket((const char*)bytedata, nbInputByte);

    auto jarrRV = env->NewByteArray(netPacket->payloadLength());

    env->SetByteArrayRegion(jarrRV, 0, netPacket->payloadLength(), (const jbyte*)(netPacket->payload()));

    return jarrRV;
}