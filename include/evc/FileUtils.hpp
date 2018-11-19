#pragma once


#include <string>

inline std::vector<int16_t> loadPcmFile(const std::string &filePath) {
    std::ifstream ifs(filePath.c_str(), std::ios::binary | std::ios::ate);
    auto fileSize = ifs.tellg();
    if (fileSize <= 0) {
        return std::vector<int16_t>();
    }
    std::vector<int16_t > wholePcm_;
    ifs.seekg(0, std::ios::beg);
    wholePcm_.resize((size_t)fileSize / sizeof(int16_t));
    if (!ifs.read((char*)wholePcm_.data(), fileSize)) {
        return std::vector<int16_t>();
    }
    wholePcm_.shrink_to_fit();

    return std::move(wholePcm_);
}
