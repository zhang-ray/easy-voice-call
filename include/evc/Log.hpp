#pragma once
#include <stdio.h>

// TODO:
// use Boost.Log ?

#define log(format , ...)  printf("%s:%d " format "\n", __FILE__ , __LINE__ , ##__VA_ARGS__)