#pragma once

#include <chrono>

#include "Singleton.hpp"



class ProcessTime : public Singleton<ProcessTime> {
private:
    decltype(std::chrono::system_clock::now()) processStartingPoint_;
public:
    ProcessTime() : processStartingPoint_(std::chrono::system_clock::now()) {
    }

    auto getProcessUptime() {
        // assume ***.count() less than uint32_t 
        return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - processStartingPoint_).count());
    }


};

