#pragma once
#include <stdio.h>

#define log(format , ...)  printf("%s:%d " format "\n", __FILE__ , __LINE__ , ##__VA_ARGS__)