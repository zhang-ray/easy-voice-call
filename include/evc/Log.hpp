#pragma once
#include <stdio.h>

#define log(format , ...) do{ printf("%s:%d @%s\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); }while(0);
