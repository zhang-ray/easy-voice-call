#pragma once
#include <string>
#include <stdio.h>

class ReturnType {
public:
    ReturnType(int value) : ok_(value==0?true:false), errorMessage_(value==0?"":std::to_string(value)) { /*dump();*/}
    ReturnType(const char *error) : errorMessage_(error){ok_=false;}
    operator bool() const{return ok_;}
    const std::string &message(){return errorMessage_;}
    
private:
    void dump(){printf("%s:%s\n", ok_?"OK":"Not OK", errorMessage_.c_str());}
    bool ok_ = true;
    std::string errorMessage_ = "";
};
