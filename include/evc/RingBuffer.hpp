#pragma once

#include <mutex>
#include <vector>
#include <cstring> // std::memory

//////////////////////////////////////////////////////////////////////////
// TODO: lock free impl
//////////////////////////////////////////////////////////////////////////

struct RingBuffer {
private:
    std::size_t posStart_ = 0;
    std::size_t currentSize_ = 0;
    std::size_t bytePerElement_;
    std::size_t maxSize_;
    std::vector<uint8_t> payload_;

private:
    std::mutex mutex_;

public:
    RingBuffer(const std::size_t bytePerElement, const std::size_t maxNbElement)
        : bytePerElement_(bytePerElement)
        , maxSize_(maxNbElement)
        , payload_(bytePerElement_ * maxSize_)
    {    }


    bool isEmpty() {return currentSize_ == 0;}
    bool isFull() {return currentSize_ == maxSize_;}
    std::size_t size() { return currentSize_; }
    std::size_t bytePerElement() { return bytePerElement_; }
    std::size_t sizeInByte() { return currentSize_*bytePerElement_; }

    bool popElements(uint8_t *data, std::size_t nbElement) {
        std::lock_guard<std::mutex> guard(mutex_);

        if (isEmpty()) {
            return false;
        }

        if (nbElement > currentSize_) {
            return false;
        }

        auto posEnd = posStart_ + nbElement;
        if (posEnd <= (int)maxSize_) {
            std::memcpy(data,&payload_[posStart_*bytePerElement_],nbElement*bytePerElement_);
        }
        else {
            auto _1 = maxSize_ - posStart_;
            auto _2 = nbElement - _1;
            std::memcpy(data,&payload_[posStart_*bytePerElement_],_1*bytePerElement_);
            std::memcpy(data + _1*bytePerElement_,&payload_[0],_2*bytePerElement_);
        }

        currentSize_ -= nbElement;
        posStart_ = (posStart_ + nbElement) % maxSize_;

        return true;
    }




    bool pushElements(uint8_t *data, std::size_t nbElement){
        std::lock_guard<std::mutex> guard(mutex_);

        /// TODO, erase tail element
        if (isFull()) {
            return false;
        }

        if ((currentSize_ + nbElement) > maxSize_) {
            return false;
        }

        auto posTail = posStart_ + currentSize_;
        if (maxSize_ > posTail) {
            auto tailCap = maxSize_ - posTail;
            if ((int)nbElement <= tailCap) {
                std::memcpy(&payload_[bytePerElement_*posTail],data,bytePerElement_ * nbElement);
            }
            else {
                std::memcpy(&payload_[bytePerElement_*posTail],data,bytePerElement_ * tailCap);
                std::memcpy(&payload_[bytePerElement_ * 0],data + bytePerElement_ * tailCap,bytePerElement_ * (nbElement - tailCap));
            }
        }
        else {
            posTail = (posStart_ + currentSize_) % maxSize_;
            std::memcpy(&payload_[bytePerElement_*posTail],data,bytePerElement_ * nbElement);
        }

        currentSize_ += nbElement;

        return true;
    }
};
