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
        mag.M = 0x100000000ULL; // 2^32
        mag.s = 0;
        mag.add_indicator = 0;
        return mag;
    }

    uint64_t p, nc, delta, q1, r1, q2, r2;
    const uint64_t two32 = 0x100000000ULL; // <-- use 2^32

    mag.add_indicator = 0;
    p = 32; // <-- start from 32
    nc = two32 - 1 - ((two32 - 1) % d); // <-- Hacker's Delight formula

    q1 = two32 / (nc + 1);
    r1 = two32 - q1 * (nc + 1);

    q2 = two32 / d;
    r2 = two32 - q2 * d;

    do {
        p++;
        if (r1 >= nc - r1 + 1) {
            q1 = 2 * q1 + 1;
            r1 = 2 * r1 - nc - 1;
        } else {
            q1 = 2 * q1;
            r1 = 2 * r1;
        }

        if (r2 + 1 >= d - r2) {
            if (q2 >= two32 - 1) mag.add_indicator = 1;
            q2 = 2 * q2 + 1;
            r2 = 2 * r2 + 1 - d;
        } else {
            q2 = 2 * q2;
            r2 = 2 * r2 + 1;
        }

        delta = d - 1 - r2;
    } while (p < 64 && (q1 < delta || (q1 == delta && r1 == 0)));

    mag.M = q2 + 1;
    mag.s = p - 32;
    return mag;
}

Magic magicu_g(uint32_t d) {
    Magic mag;
    if (d == 1) {
        mag.M = 0x100000000ULL; // requires 64-bit type to hold 2^32
        mag.s = 0;
        mag.add_indicator = 0;
        return mag;
    }

    uint64_t p, nc, delta, q1, r1, q2, r2;
    const uint64_t two31 = 0x80000000;

    mag.add_indicator = 0;
    p = 31;
    nc = (two31 - 1) / d * d - 1;

    q1 = two31 / (nc + 1);
    r1 = two31 - q1 * (nc + 1);

    q2 = two31 / d;
    r2 = two31 - q2 * d;

    do {
        p++;
        if (r1 >= nc - r1 + 1) {
            q1 = 2 * q1 + 1;
            r1 = 2 * r1 - nc - 1;
        } else {
            q1 = 2 * q1;
            r1 = 2 * r1;
        }

        if (r2 + 1 >= d - r2) {
            if (q2 >= two31 - 1) mag.add_indicator = 1;
            q2 = 2 * q2 + 1;
            r2 = 2 * r2 + 1 - d;
        } else {
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
