#pragma once

#include "GeneratedWave.hpp"

class AudioInStub {
private:
    std::vector<int16_t> wholePcm_;
    size_t size_ = 0;
    size_t pos_ = 0;
    const bool loop_ = true;
public:
    AudioInStub(const decltype(wholePcm_) &wholePcm)
        : wholePcm_(wholePcm) 
        , size_(wholePcm_.size())
    { }

    const int16_t *get() {
        if (pos_ + blockSize >= size_) {
            pos_ = 0;
        }
        auto ret = wholePcm_.data() + pos_;
        pos_ += blockSize;
        return ret;
    }

    static std::shared_ptr<AudioInStub> loadFile(const std::string &filePathOrTag) {
        std::ifstream ifs(filePathOrTag.c_str(), std::ios::binary | std::ios::ate);
        auto fileSize = ifs.tellg();
        if (fileSize <= 0) {
            return nullptr;
        }
        std::vector<int16_t > wholePcm_;
        ifs.seekg(0, std::ios::beg);
        wholePcm_.resize((size_t)fileSize / sizeof(int16_t));
        if (!ifs.read((char*)wholePcm_.data(), fileSize)) {
            return nullptr;
        }
        wholePcm_.shrink_to_fit();

        return std::make_shared<AudioInStub>(wholePcm_);
    }


    static std::shared_ptr<AudioInStub> create(const std::string &filePathOrTag) {
        auto ret = loadFile(filePathOrTag);

        if (ret) {
            return ret;
        }
        
        if (filePathOrTag == "sine") {
            return std::make_shared<AudioInStub>(SineWave(sampleRate, 440, .8, 10));
        }

        return nullptr;
    }
};