#pragma once

template <typename T>
class Singleton {
public:
    static T& get() {
        static T instance; // local static variable initialization is thread-safe in C++11
        return instance;
    }
protected:
    virtual ~Singleton() { }
};
