#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdint.h>
typedef struct {
    uint64_t M;
    uint32_t s;
    int32_t add_indicator;
} Magic;


Magic magicu(uint32_t d);







#endif