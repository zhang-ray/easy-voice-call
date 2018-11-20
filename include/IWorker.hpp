#pragma once

#include "AudioCommon.hpp"
#include "NetCommon.hpp"




// for property tree
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lockfree/spsc_queue.hpp>


#include <string>
#include <functional>

#include "ReturnType.hpp"

#include "Lib.hpp"

class IWorker {
public:
    virtual ~IWorker() {}
    virtual ReturnType init(
        const boost::property_tree::ptree &configRoot,
        std::function<void(const std::string &, const std::string &)> reportInfo,
        std::function<void(const AudioIoVolume)> reportVolume,
        std::function<void(const bool)> vadReporter
    ) = 0;

    virtual void setDurationReporter(std::function<void(const uint32_t)>) = 0;
    virtual void setMute(bool mute) = 0;
    virtual void asyncStart(std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState) = 0;
    virtual ReturnType syncStart(std::function<void(const NetworkState &newState, const std::string &extraMessage)> toggleState) = 0;
    ///// TODO: blocked when server down?
    virtual void syncStop() = 0;

    static EVC_API std::shared_ptr<IWorker> create();
};