#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdint.h>
typedef struct {
    uint64_t M;
    uint32_t s;
    int32_t add_indicator;
} Magic;

bool is_power_of_two(uint64_t x);
uint64_t next_power_of_two(uint64_t x);
Magic magicu(uint32_t d);



uint64_t alignTo8(uint64_t addr);



#endif