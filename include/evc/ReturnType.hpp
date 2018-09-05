#pragma once
#include <string>
#include <stdio.h>

class ReturnType {
public:
    ReturnType(int value) : ok_(value==0?true:false), errorMessage_(value==0?"":std::to_string(value)) { /*dump();*/}
    ReturnType(const char *error) : errorMessage_(error){}
    operator bool() const{return ok_;}

private:
    void dump(){printf("%s:%s\n", ok_?"OK":"Not OK", errorMessage_.c_str());}
    bool ok_ = true;
    const std::string errorMessage_ = "";
};
