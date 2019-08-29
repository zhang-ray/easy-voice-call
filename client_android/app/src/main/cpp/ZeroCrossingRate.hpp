#pragma once

#include <cmath>
#include <vector>


inline double ZeroCrossingRate(const PcmSegment &oneSegment) {
    int sum = 0;
    std::vector<bool> positiveArray(blockSize);

    for (int i = 0; i < positiveArray.size(); i++) {
        positiveArray[i] = (oneSegment[i] > 0);
    }
    for (int i = 1; i < oneSegment.size(); i++) {
        if (positiveArray[i - 1] != positiveArray[i]) {
            sum++;
        }
    }
    return (double)sum / positiveArray.size();
}


