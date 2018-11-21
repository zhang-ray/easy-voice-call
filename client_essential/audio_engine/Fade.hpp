#pragma once

#include "AudioCommon.hpp"
#include <complex>
#include <vector>


enum class FadeInOut {
    FadeIn,
    FadeOut
};



class Fade {
public:
    virtual ~Fade() { }
};



class FadeExponential : public Fade {
    const size_t nbSegment_ = 2;
public:
    void fade(const FadeInOut inOrOut, std::vector<std::shared_ptr<PcmSegment>> segments) {
        assert(segments.size() >= nbSegment_);

        auto minFactor = 0.01;
        auto minX = std::log(minFactor);
        auto step = -minX / (blockSize*nbSegment_);


        auto lastFactor = std::numeric_limits<double>::max();
        for (int indexSegment = 0; indexSegment < segments.size(); indexSegment++) {
            auto currentSegmentIndex = segments.size() - nbSegment_ + indexSegment;
            for (int posOfOneSeg = 0; posOfOneSeg < blockSize; posOfOneSeg++) {
                auto indexSample = indexSegment*blockSize + posOfOneSeg;
                if (inOrOut == FadeInOut::FadeIn) {
                    indexSample = segments.size()*blockSize - indexSample;
                }
                auto finalX = step*indexSample;
                auto &currentSegment = segments[currentSegmentIndex];

                auto finalFactor = std::exp(-finalX);

                /// checking
                {
                    if (finalFactor > 1.0000 + std::numeric_limits<double>::epsilon()) {
                        throw;
                    }
                    if (inOrOut == FadeInOut::FadeIn) {
                        if (finalFactor < lastFactor) {
                            throw;
                        }
                    }
                    if (inOrOut == FadeInOut::FadeOut) {
                        if (finalFactor > lastFactor) {
                            throw;
                        }
                    }
                }
                
                currentSegment->at(posOfOneSeg) *= finalFactor;

                lastFactor = finalFactor;
                

            }
        }

    }
};