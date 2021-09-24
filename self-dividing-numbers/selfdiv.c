#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

// sets digits used in prefix, returns lcm(digits)
static inline uint32_t process_prefix(bool *digits, uint64_t prefix)
{
    uint32_t i, rem, lcm = 1;
    for (i = 0; i < 10; ++i) digits[i] = false;
    while (prefix)
    {
        rem = prefix % 10;
        prefix /= 10;
        digits[rem] = true;
    }
    if (digits[2] || digits[4] || digits[6] || digits[8])
        lcm *= 2;
    if (digits[4] || digits[8])
        lcm *= 2;
    if (digits[8])
        lcm *= 2;
    if (digits[3] || digits[6] || digits[9])
        lcm *= 3;
    if (digits[9])
        lcm *= 3;
    if (digits[5])
        lcm *= 5;
    if (digits[7])
        lcm *= 7;
    return lcm;
}

static inline bool is_self_div(uint64_t x)
{
    uint64_t x2 = x, rem;
    while (x2)
    {
        rem = x2 % 10;
        x2 /= 10;
        if (!rem) return false;
        if (x % rem) return false;
    }
    return true;
}

void brute_force(uint64_t lo, uint64_t hi)
{
    while (lo <= hi)
    {
        if (is_self_div(lo))
            printf("%lu\n",lo);
        ++lo;
    }
}

// finds numbers in blocks given a prefix
// block size 10^3 = 1000 (3 digits not part of prefix)
void loop(uint64_t limit)
{
    uint64_t prefix_max, block_size, prefix, lo, hi, n, n2;
    uint32_t lcm, rem;
    bool digits[10];
    block_size = 1000;
    prefix_max = limit / block_size;
    brute_force(1,block_size-1);
    for (prefix = 1; prefix < prefix_max; ++prefix)
    {
        lcm = process_prefix(digits,prefix);
        if (digits[0]) continue;
        lo = prefix*block_size - 1;
        hi = (prefix+1)*block_size;
        lo += lcm - (lo%lcm);
        for (n = lo; n < hi; n += lcm)
        {
            n2 = n;
            rem = n2 % 10;
            if (!rem) continue;
            if (!digits[rem] && n % rem) continue;
            n2 /= 10;
            rem = n2 % 10;
            if (!rem) continue;
            if (!digits[rem] && n % rem) continue;
            n2 /= 10;
            rem = n2 % 10;
            if (!rem) continue;
            if (!digits[rem] && n % rem) continue;
            printf("%lu\n",n);
        }
    }
    brute_force(prefix_max*block_size,limit);
}

int main(int argc, char **argv)
{
    uint64_t limit;
    assert(argc >= 2);
    limit = atol(argv[1]);
    loop(limit);
    return 0;
}
