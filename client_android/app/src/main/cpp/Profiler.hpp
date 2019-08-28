#pragma once

#include <cmath>
#include <numeric>
#include <algorithm>
#include <fstream>
#include "Logger.hpp"
#include "Singleton.hpp"
#include "ProcessTime.hpp"



template <typename T>
class EventList{
private:
    const char *tag_ = nullptr;
    std::vector<std::tuple<uint32_t, T>> list_; // {uptime, delay}
    std::ofstream ofs_;
public:
    EventList(const char *tag) : tag_(tag) {
        ofs_.open(tag);
    }
    void addData(const T &data) {
        list_.push_back({ProcessTime::get().getProcessUptime(),data});
    }

    /// CSV format
    void dump() {
        LOGI << "dump_to: " << tag_;
        for (const auto &v : list_) {
            ofs_ << std::get<0>(v) << ", " << std::get<1>(v) << std::endl;
        }
    }
};




/// DEPRECATED....
/// try to dump data and analyze off-line
#if 0
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

    std::string calc() {
        if (durationList.empty()) {
            return "list is empty";
        }
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
#endif


class Profiler : public Singleton<Profiler> {
public:
    //Statistician arrivalAudioOffset_;
    EventList<int32_t> packageDelayList_;
    EventList<bool> emptyAudioOutBuffer_;
    EventList<size_t> audioOutBufferSize_;
public:
    Profiler()
        : packageDelayList_("packageDelayList_.csv")
        , emptyAudioOutBuffer_("emptyBuffer_.csv")
        , audioOutBufferSize_("audioOutBufferSize_.csv")
    { }

    void dump() {
        packageDelayList_.dump();
        emptyAudioOutBuffer_.dump();
        audioOutBufferSize_.dump();
    }
};
