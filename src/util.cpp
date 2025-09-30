#include "util.hpp"


bool is_power_of_two(uint64_t x) {
    return x != 0 && (x & (x - 1)) == 0;
}

uint64_t next_power_of_two(uint64_t x) {
   // printf("next_power_of_two\n");
    if (x <= 1) return 1;
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    return x + 1;
}



Magic magicu(uint32_t d) {
    Magic mag;
    if (d == 1) {
        mag.M = 0x100000000ULL; // requires 64-bit type to hold 2^32
        mag.s = 0;
        mag.add_indicator = 0;
        return mag;
    }


    uint32_t p;
    uint32_t anc, delta, q1, r1, q2, r2, t;
    const uint32_t two31 = 0x80000000;

    mag.add_indicator = 0;
    p = 31;
    uint32_t nc = -1 - (-d) % d;

    q1 = two31 / nc;
    r1 = two31 - q1 * nc;

    q2 = two31 / d;
    r2 = two31 - q2 * d;

    do {
        p = p + 1;
        if (r1 >= nc - r1) {
            q1 = 2 * q1 + 1;
            r1 = 2 * r1 - nc;
        } else {
            q1 = 2 * q1;
            r1 = 2 * r1;
        }

        if (r2 + 1 >= d - r2) {
            if (q2 >= 0x7FFFFFFF) mag.add_indicator = 1;
            q2 = 2 * q2 + 1;
            r2 = 2 * r2 + 1 - d;
        } else {
            if (q2 >= 0x80000000) mag.add_indicator = 1;
            q2 = 2 * q2;
            r2 = 2 * r2 + 1;
        }

        delta = d - 1 - r2;
    } while (p < 64 && (q1 < delta || (q1 == delta && r1 == 0)));

    mag.M = q2 + 1;
    mag.s = p - 32;
    return mag;
}

uint64_t alignTo8(uint64_t addr)
{
    return ((addr + 7) / 8) * 8;
}
