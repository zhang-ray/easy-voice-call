#pragma once

#include <cmath>
#include <numeric>
#include <algorithm>
#include <fstream>
#include "Logger.hpp"
#include "Singleton.hpp"
#include "ProcessTime.hpp"




template <typename T>
class DataDumper {
private:
    const std::string fileName_;
    std::ofstream ofs_;
public:
    DataDumper(const std::string fileName)
        : fileName_(fileName)
    {
        ofs_.open(fileName_);
    }

    void dump(const std::vector<T> data) {
        for (const auto &v : data) {
            ofs_ << v << std::endl;
        }
    }
};


/* TODO
statistics media data
    - packet order
    - arrival time's Evenness 
    - 
*/

class Statistician {
private:
    std::vector<int32_t> durationList;
    const char *tag_ = nullptr;
public:
    Statistician(const char *tag) : tag_(tag) {}
    Statistician(const Statistician&) = delete;
    void dump(){
        try {
            LOGI << calc();
            DataDumper<int32_t> dumper(tag_);
            dumper.dump(durationList);
        }
        catch (const std::exception &e) {
            LOGE_STD_EXCEPTION(e);
        }
    }
    void addData(const int32_t dur) {
        durationList.push_back(dur);
    }

    std::string calc(){
        const auto &v = durationList;
        double sum = std::accumulate(v.begin(), v.end(), 0.0);
        double mean = sum / v.size();

        double sq_sum = std::inner_product(v.begin(), v.end(), v.begin(), 0.0);
        double stdev = std::sqrt(sq_sum / v.size() - mean * mean);

        std::string result_="{@tag mean+/-std [min,max]#nbElements}";


        auto nbElements = v.size();
        auto minV = *std::min_element(std::begin(v), std::end(v));
        auto maxV = *std::max_element(std::begin(v), std::end(v));
        result_ = result_ + "\t{@" + tag_ + "\t" + std::to_string(mean) + "+/-" + std::to_string(stdev) + "[" + std::to_string(minV) + "," + std::to_string(maxV) + "]#" + std::to_string(nbElements) +"}\t";

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
        auto dur = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startPoint_).count());
        if (stat_) {
            stat_->addData(dur);
        }
    }
};



class TimestampMarker {
    using _TS = decltype(ProcessTime::get().getProcessUptime());
private:
    const char *tag_ = nullptr;
    std::vector<_TS> tsList_;
public:
    TimestampMarker(const char *tag) : tag_(tag) {}
    void dump(){
        try {
            LOGI << calc();
            DataDumper<_TS> dumper(tag_);
            dumper.dump(tsList_);
        }
        catch (const std::exception &e){
            LOGE_STD_EXCEPTION(e);
        }
    }
    void mark(const _TS &ts= ProcessTime::get().getProcessUptime()) { tsList_.push_back(ts); }
private:
    std::string calc() {
        std::string result_ = "{@tag [min,max]#nbElements}";
        const auto &v = tsList_;
        auto nbElements = v.size();
        auto minV = *std::min_element(std::begin(v), std::end(v));
        auto maxV = *std::max_element(std::begin(v), std::end(v));
        result_ = result_ + "\t{@" + tag_ + "[" + std::to_string(minV) + "," + std::to_string(maxV) + "]#" + std::to_string(nbElements) + "}";
        return result_;
    }
};


class Profiler : public Singleton<Profiler> {
public:
    Statistician arrivalAudioOffset_;
    TimestampMarker emptyAudioOutBufferTS_;
public:
    Profiler()
        : arrivalAudioOffset_("arrivalOffset(ms)")
        , emptyAudioOutBufferTS_("emptyBufferTS(uptime, ms)")
    { }
    void dump() {
        arrivalAudioOffset_.dump();
        emptyAudioOutBufferTS_.dump();
    }
};
