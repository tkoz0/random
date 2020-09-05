#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

// floor of each number divided by 2^67, total sum of these is 3551844396 < 2^32
uint32_t HI27[100] =
{
    38759044, 37187767, 32307642, 12576232, 29068655,
    53987386, 17435104,  8772463, 32284922,  9990541,
    17473610, 34976341, 15196670, 50647078, 42093171,
    33493147, 37379661, 39673415, 36006971, 45656375,
    29086356, 30279487, 17879996,  8530789, 30373108,
    59291325,  7566372, 26286571, 62425373, 22708771,
    60009474, 29360597, 16457860, 45661729, 59592855,
    20258956, 41453483, 18715053, 46649253, 58762600,
    63908004, 32447790, 24562310, 63438158, 67039713,
    40072288, 56337120, 15354099, 23562317, 42835131,
    12173148, 43139405, 57906279, 22783771, 57357505,
    19602765, 29115530, 58511440, 57023845, 15364183,
    31780078, 30010491, 48425179, 15408029, 45306997,
    21423281, 12994240, 23828266, 51158653, 16248644,
    31684753, 19193869, 59329187, 66635491, 35905193,
    66797105,  9969416, 24024831, 45640626, 14512491,
    57474555, 67499833, 67364448, 42298639, 29441804,
    51288472, 18815119, 41838678, 36507690, 36034091,
    44805620,  8910302, 16580349, 66317217, 19832218,
    25691703, 19188533, 22234867, 67455973, 23136461
};

// represent a subset as 100 bits, increased to 128 for memory alignment
// select 100 bits from 4 32 bit uint32_t in some way
struct subset { uint32_t a,b,c,d; };

// caching methods take a subset of the HI27 array (such as 20 of the numbers)
// and generate a cache storing the sums of every subset of those numbers
// the summing of a subset of all 100 can then be done with fewer additions at
// the expense of using more memory

// 4sum: 4x25num caches, 4*2^27 bytes = 512MiB (RAM)
// 5sum: 5x20num caches, 5*2^22 bytes = 20MiB (L3 on a E5-2670)
// 8sum: 4x12num + 4x13num caches, 4*2^14+4*2^15 bytes = 192KiB (L2)
// 12sum: 8x8num + 4x9num caches, 8*2^10+4*2^11 bytes = 16KiB (L1d)

#if CACHE_SUMS == 4 // RAM
uint32_t *cache0, *cache1, *cache2, *cache3;
#elif CACHE_SUMS == 5 // L3
uint32_t *cache0, *cache1, *cache2, *cache3, *cache4;
#elif CACHE_SUMS == 8 // L2
uint32_t *cache0, *cache1, *cache2, *cache3, *cache4, *cache5, *cache6, *cache7;
#elif CACHE_SUMS == 12 // L1
uint32_t *cache0, *cache1, *cache2, *cache3, *cache4, *cache5,
         *cache6, *cache7, *cache8, *cache9, *cache10, *cache11;
#else
#error CACHE_SUMS invalid, allowed values are 4, 5, 8, 12
#endif

// compute the sum of a subset as represented by the struct subset type
uint32_t hi27_sum(struct subset s)
{
#if CACHE_SUMS == 4 // RAM
    // a(7..31) b(7..31) c(7..31) d(7..31)
    return cache0[s.a>>7] + cache1[s.b>>7] + cache2[s.c>>7] + cache3[s.d>>7];
#elif CACHE_SUMS == 5 // L3
    // bits used:
    // cache0: s.a 0..19
    // cache1: s.a 22..31 and s.b 0..9
    // cache2: s.b 12..31
    // cache3: s.c 0..19
    // cache4: s.c 22..31 and s.d 0..9
    return cache0[s.a & 0xFFFFF]
         + cache1[(s.a >> 22) | ((s.b & 0x3FF) << 10)]
         + cache2[s.b >> 12]
         + cache3[s.c & 0xFFFFF]
         + cache4[(s.c >> 22) | ((s.d & 0x3FF) << 10)];
#elif CACHE_SUMS == 8 // L2
    // a(0..12,20..31) b(0..12,20..31) c(0..12,20..31) d(0..12,20..31)
    return cache0[s.a & 0x1FFF] + cache1[s.a >> 20]
         + cache2[s.b & 0x1FFF] + cache3[s.b >> 20]
         + cache4[s.c & 0x1FFF] + cache5[s.c >> 20]
         + cache6[s.d & 0x1FFF] + cache7[s.d >> 20];
#elif CACHE_SUMS == 12 // L1
    // a(0..7,8..15,16..23,24..31) b(0..7,8..15,16..23,24..31)
    // c(0..8,23..31) d(0..8,23..31)
    return cache0[s.a&0xFF] + cache1[(s.a>>8)&0xFF]
         + cache2[(s.a>>16)&0xFF] + cache3[s.a>>24]
         + cache4[s.b&0xFF] + cache5[(s.b>>8)&0xFF]
         + cache6[(s.b>>16)&0xFF] + cache7[s.b>>24]
         + cache8[s.c&0x1FF] + cache9[s.c>>23]
         + cache10[s.d&0x1FF] + cache11[s.d>>23];
#else
#error CACHE_SUMS invalid, allowed values are 4, 5, 8, 12
#endif
}

void cache_alloc()
{
#if CACHE_SUMS == 4 // RAM
    cache0 = malloc(1<<27);
    cache1 = malloc(1<<27);
    cache2 = malloc(1<<27);
    cache3 = malloc(1<<27);
#elif CACHE_SUMS == 5 // L3
    cache0 = malloc(1<<22);
    cache1 = malloc(1<<22);
    cache2 = malloc(1<<22);
    cache3 = malloc(1<<22);
    cache4 = malloc(1<<22);
#elif CACHE_SUMS == 8 // L2
    cache0 = malloc(1<<15);
    cache1 = malloc(1<<14);
    cache2 = malloc(1<<15);
    cache3 = malloc(1<<14);
    cache4 = malloc(1<<15);
    cache5 = malloc(1<<14);
    cache6 = malloc(1<<15);
    cache7 = malloc(1<<14);
#elif CACHE_SUMS == 12 // L1
    cache0 = malloc(1<<10);
    cache1 = malloc(1<<10);
    cache2 = malloc(1<<10);
    cache3 = malloc(1<<10);
    cache4 = malloc(1<<10);
    cache5 = malloc(1<<10);
    cache6 = malloc(1<<10);
    cache7 = malloc(1<<10);
    cache8 = malloc(1<<11);
    cache9 = malloc(1<<11);
    cache10 = malloc(1<<11);
    cache11 = malloc(1<<11);
#else
#error CACHE_SUMS invalid, allowed values are 4, 5, 8, 12
#endif
}

void cache_free()
{
#if CACHE_SUMS == 4 // RAM
    free(cache0);
    free(cache1);
    free(cache2);
    free(cache3);
#elif CACHE_SUMS == 5 // L3
    free(cache0);
    free(cache1);
    free(cache2);
    free(cache3);
    free(cache4);
#elif CACHE_SUMS == 8 // L2
    free(cache0);
    free(cache1);
    free(cache2);
    free(cache3);
    free(cache4);
    free(cache5);
    free(cache6);
    free(cache7);
#elif CACHE_SUMS == 12 // L1
    free(cache0);
    free(cache1);
    free(cache2);
    free(cache3);
    free(cache4);
    free(cache5);
    free(cache6);
    free(cache7);
    free(cache8);
    free(cache9);
    free(cache10);
    free(cache11);
#else
#error CACHE_SUMS invalid, allowed values are 4, 5, 8, 12
#endif
}

void cache_generate()
{
    // i = cache index
    // i2 = copy i (for bit extraction)
    // j = loop counter for summing from HI27 array
    // bit = extracted bit from i
    // s? = store sum computed before writing to cache
    uint32_t i, i2, j, bit, s0, s1, s2, s3;
#if CACHE_SUMS == 4 // RAM
    for (i = 0; i < (1<<25); ++i)
    {
        i2 = i;
        s0 = s1 = s2 = s3 = 0;
        for (j = 0; j < 25; ++j, i2 >>= 1) // sum from 25 values
        {
            bit = i2 & 1;
            s0 += bit * HI27[j];
            s1 += bit * HI27[j+25];
            s2 += bit * HI27[j+50];
            s3 += bit * HI27[j+75];
        }
        cache0[i] = s0;
        cache1[i] = s1;
        cache2[i] = s2;
        cache3[i] = s3;
    }
#elif CACHE_SUMS == 5 // L3
    uint32_t s4;
    for (i = 0; i < (1<<20); ++i)
    {
        i2 = i;
        s0 = s1 = s2 = s3 = s4 = 0;
        for (j = 0; j < 20; ++j, i2 >>= 1) // sum from 20 values
        {
            bit = i2 & 1;
            s0 += bit * HI27[j];
            s1 += bit * HI27[j+20];
            s2 += bit * HI27[j+40];
            s3 += bit * HI27[j+60];
            s4 += bit * HI27[j+80];
        }
        cache0[i] = s0;
        cache1[i] = s1;
        cache2[i] = s2;
        cache3[i] = s3;
        cache4[i] = s4;
    }
#elif CACHE_SUMS == 8 // L2
    for (i = 0; i < (1<<13); ++i)
    {
        if (i >> 12) goto skip12bit; // too large for 12num caches
        i2 = i;
        s0 = s1 = s2 = s3 = 0;
        for (j = 0; j < 12; ++j, i2 >>= 1)
        {
            bit = i2 & 1;
            s0 += bit * HI27[j+13];
            s1 += bit * HI27[j+38];
            s2 += bit * HI27[j+63];
            s3 += bit * HI27[j+88];
        }
        cache1[i] = s0;
        cache3[i] = s1;
        cache5[i] = s2;
        cache7[i] = s3;
        skip12bit: // 13num caches below
        i2 = i;
        s0 = s1 = s2 = s3 = 0;
        for (j = 0; j < 13; ++j, i2 >>= 1)
        {
            bit = i2 & 1;
            s0 += bit * HI27[j];
            s1 += bit * HI27[j+25];
            s2 += bit * HI27[j+50];
            s3 += bit * HI27[j+75];
        }
        cache0[i] = s0;
        cache2[i] = s1;
        cache4[i] = s2;
        cache6[i] = s3;
    }
#elif CACHE_SUMS == 12 // L1
    for (i = 0; i < (1<<9); ++i)
    {
        if (i >> 8) goto skip8bit; // too large for 8num caches
        i2 = i;
        s0 = s1 = s2 = s3 = 0;
        for (j = 0; j < 8; ++j, i2 >>= 1)
        {
            bit = i2 & 1;
            s0 += bit * HI27[j];
            s1 += bit * HI27[j+8];
            s2 += bit * HI27[j+16];
            s3 += bit * HI27[j+24];
        }
        cache0[i] = s0;
        cache1[i] = s1;
        cache2[i] = s2;
        cache3[i] = s3;
        i2 = i;
        s0 = s1 = s2 = s3 = 0;
        for (j = 0; j < 8; ++j, i2 >>= 1)
        {
            bit = i2 & 1;
            s0 += bit * HI27[j+32];
            s1 += bit * HI27[j+40];
            s2 += bit * HI27[j+48];
            s3 += bit * HI27[j+56];
        }
        cache4[i] = s0;
        cache5[i] = s1;
        cache6[i] = s2;
        cache7[i] = s3;
        skip8bit: // 9num caches below
        i2 = i;
        s0 = s1 = s2 = s3 = 0;
        for (j = 0; j < 9; ++j, i2 >>= 1)
        {
            bit = i2 & 1;
            s0 += bit * HI27[j+64];
            s1 += bit * HI27[j+73];
            s2 += bit * HI27[j+82];
            s3 += bit * HI27[j+91];
        }
        cache8[i] = s0;
        cache9[i] = s1;
        cache10[i] = s2;
        cache11[i] = s3;
    }
#else
#error CACHE_SUMS invalid, allowed values are 4, 5, 8, 12
#endif
}

// advance LCG state and extract the next number
#define NEXT_X(X) (X = (a * X + c))

// compute sums for several subsets generated by a LCG, measure time taken
// if LCG_PERIOD_TEST is defined, it will test that the period length of the
// LCG is the full 2^32, requires allocating 2^32 bits (512MiB)
double test_mod_sum(uint32_t a, uint32_t c)
{
#ifdef LCG_PERIOD_TEST
    // marks for numbers that have occurred in the LCG sequence
    uint32_t *marks = calloc(1<<27,sizeof(uint32_t));
#endif
    clock_t start = clock();
    uint32_t X = 0; // LCG state
    // use a = 3740067437, c = 11 (based on java.util.Random)
    // period length = 2^32
    uint32_t counter = (1 << 30); // 2^30 to extract
    struct subset S;
    uint32_t sum = 0;
    while (counter--)
    {
        S.a = NEXT_X(X); // take next 4 random numbers
        S.b = NEXT_X(X);
        S.c = NEXT_X(X);
        S.d = NEXT_X(X);
#ifdef LCG_PERIOD_TEST
        uint32_t b0, b1, b2, b3;
        b0 = marks[S.a>>5]&(1<<(S.a&0x1F)); // extract marks
        b1 = marks[S.b>>5]&(1<<(S.b&0x1F));
        b2 = marks[S.c>>5]&(1<<(S.c&0x1F));
        b3 = marks[S.d>>5]&(1<<(S.d&0x1F));
        assert(!b0); // should not have occurred yet
        assert(!b1);
        assert(!b2);
        assert(!b3);
        marks[S.a>>5] |= 1<<(S.a&0x1F); // set marks
        marks[S.b>>5] |= 1<<(S.b&0x1F);
        marks[S.c>>5] |= 1<<(S.c&0x1F);
        marks[S.d>>5] |= 1<<(S.d&0x1F);
#endif
        //printf("{%10u,%10u,%10u,%10u}\n",S.a,S.b,S.c,S.d);
        //printf("hi27_sum = %u\n",hi27_sum(S));
        sum += hi27_sum(S);
    }
    clock_t elapsed = clock() - start;
    printf("sum = %u\n",sum); // use the result so -O3 doesnt optimize it out
    return ((double) elapsed) / CLOCKS_PER_SEC;
}

int main(int argc, char **argv)
{
    assert(argc >= 2);
    uint32_t tests = atoi(argv[1]);
    assert(tests && tests < 1024); // sane amount of tests 1-1023
    double times[tests];
    double total_time = 0.0;
    cache_alloc();
    cache_generate();
    uint32_t X = 0; // LCG state (seed = 0)
    for (uint32_t i = 0; i < tests; ++i)
    {
        // generate randomized LCG parameters for testing
        // this uses parameters based on java.util.Random
        uint32_t a = (X = (3740067437U * X + 11));
        uint32_t c = (X = (3740067437U * X + 11));
        a = (a << 2) | 1; // ensure a=1 (mod 4)
        c = (c << 1) | 1; // ensure c is odd
        times[i] = test_mod_sum(a,c);
        total_time += times[i];
    }
    printf("total_time = %lf\n",total_time);
    printf("average = %lf\n",total_time/tests);
    cache_free();
    return 42;
}
