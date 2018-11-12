#pragma once

#include <cmath>
#include <numeric>

/* TODO
statistics media data
    - packet order
    - arrival time's Evenness 
    - 
*/

class Statistician {
private:
    std::vector<uint32_t> durationList;
    std::string result_;
public:
    Statistician(const char *tag) : result_(tag) {}
    Statistician(const Statistician&) = delete;
    
    void addDurationMs(const uint32_t dur) {
        durationList.push_back(dur);
    }

    std::string calc(){
        const auto &v = durationList;
        double sum = std::accumulate(v.begin(), v.end(), 0.0);
        double mean = sum / v.size();

        double sq_sum = std::inner_product(v.begin(), v.end(), v.begin(), 0.0);
        double stdev = std::sqrt(sq_sum / v.size() - mean * mean);

        result_ = result_ + "[" + std::to_string(mean) + "+/-"  + std::to_string(stdev) + "]";

        return result_;
    }

};


class Timer {
private:
    Statistician *stat_ = nullptr;
    decltype(std::chrono::system_clock::now()) startPoint_;
public:
    Timer(Statistician *stat)
        : stat_(stat)
    {
        startPoint_ = std::chrono::system_clock::now();
    }

    ~Timer() {
        auto dur = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startPoint_).count());
        if (stat_) {
            stat_->addDurationMs(dur);
        }
    }
};


class Profiler {
public:
    Statistician durationAudioCallback_;
    Statistician nsAecVolumeVadSend_;
public:
    Profiler()
        : durationAudioCallback_("durationAudioCallback_")
        , nsAecVolumeVadSend_("nsAecVolumeVadSend")
    { }

    void dumpOut() {
        LOGI << durationAudioCallback_.calc();
        LOGI << nsAecVolumeVadSend_.calc();
    }
};
