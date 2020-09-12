#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
// SIMD
#include <emmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>

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

// most efficient cache method of those in "cachetest.c"
// 12sum: 8x8num + 4x9num caches, 8*2^10+4*2^11 bytes = 16KiB (L1d)

uint32_t *cache0, *cache1, *cache2, *cache3, *cache4, *cache5,
         *cache6, *cache7, *cache8, *cache9, *cache10, *cache11;

void cache_alloc()
{
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
}

void cache_free()
{
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
}

void cache_generate()
{
    // i = cache index
    // i2 = copy i (for bit extraction)
    // j = loop counter for summing from HI27 array
    // bit = extracted bit from i
    // s? = store sum computed before writing to cache
    uint32_t i, i2, j, bit, s0, s1, s2, s3;
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
}

// represent a subset as 100 bits, increased to 128 for memory alignment
// select 100 bits from 4 32 bit uint32_t in some way
struct subset_t { uint32_t a,b,c,d; };

// compute the sum of a subset as represented by the struct subset type
uint32_t hi27_sum(struct subset_t s)
{
    // a(0..7,8..15,16..23,24..31) b(0..7,8..15,16..23,24..31)
    // c(0..8,23..31) d(0..8,23..31)
    return cache0[s.a&0xFF] + cache1[(s.a>>8)&0xFF]
         + cache2[(s.a>>16)&0xFF] + cache3[s.a>>24]
         + cache4[s.b&0xFF] + cache5[(s.b>>8)&0xFF]
         + cache6[(s.b>>16)&0xFF] + cache7[s.b>>24]
         + cache8[s.c&0x1FF] + cache9[s.c>>23]
         + cache10[s.d&0x1FF] + cache11[s.d>>23];
}

uint32_t hi27_sum_sse(uint32_t set[4])
{
    uint8_t *bytes = (uint8_t*) set;
    return cache0[bytes[0]] + cache1[bytes[1]]
         + cache2[bytes[2]] + cache3[bytes[3]]
         + cache4[bytes[4]] + cache5[bytes[5]]
         + cache6[bytes[6]] + cache7[bytes[7]]
         + cache8[set[2]&0x1FF] + cache9[set[2]>>23]
         + cache10[set[3]&0x1FF] + cache11[set[3]>>23];
}

// parameters for a thread, including the LCG parameters
struct params_t { uint32_t a, c, t_num; };

// advance LCG state and extract the next number
#define NEXT_X(X) (X = (a * X + c))

// SIMD version, using SSE2 and SSE4.1, only a little faster
// the L1 cache accesses for subset summing are much slower
void subset_generator_sse(uint32_t a, uint32_t c)
{
    // using SSE2 instructions to parallelize LCGs
    __m128i A0 = _mm_set_epi32(a,a+4,a+8,a+12);
    __m128i C0 = _mm_set_epi32(c,c+2,c+4,c+6);
    __m128i X0 = _mm_set_epi32(0,0,0,0);
    __m128i A1 = _mm_set_epi32(a+16,a+20,a+24,a+28);
    __m128i C1 = _mm_set_epi32(c+8,c+10,c+12,c+14);
    __m128i X1 = _mm_set_epi32(0,0,0,0);
    //__m128i A2 = _mm_set_epi32(a+32,a+36,a+40,a+44);
    //__m128i C2 = _mm_set_epi32(c+16,c+18,c+20,c+22);
    //__m128i X2 = _mm_set_epi32(0,0,0,0);
    //__m128i A3 = _mm_set_epi32(a+48,a+52,a+56,a+60);
    //__m128i C3 = _mm_set_epi32(c+24,c+26,c+28,c+30);
    //__m128i X3 = _mm_set_epi32(0,0,0,0);
    uint32_t counter = (1<<29);
    uint32_t set0[4], set1[4];
    //uint32_t set2[4], set3[4];
    uint32_t total = 0; // used to prevent optimizing out the loop
    uint32_t sum0, sum1; // subset sum
    //uint32_t sum2, sum3;
    while (counter--)
    {
        // advance state of the 4 LCGs, X stores the new subset
        X0 = _mm_add_epi32(_mm_mullo_epi32(X0,A0),C0); // SSE4.1 and SSE2
        _mm_storeu_si128((void*)set0,X0); // SSE2
        X1 = _mm_add_epi32(_mm_mullo_epi32(X1,A1),C1);
        _mm_storeu_si128((void*)set1,X1);
        //X2 = _mm_add_epi32(_mm_mullo_epi32(X2,A2),C2);
        //_mm_storeu_si128((void*)set2,X2);
        //X3 = _mm_add_epi32(_mm_mullo_epi32(X3,A3),C3);
        //_mm_storeu_si128((void*)set3,X3);
        sum0 = hi27_sum_sse(set0);
        sum1 = hi27_sum_sse(set1);
        //sum2 = hi27_sum_sse(set2);
        //sum3 = hi27_sum_sse(set3);
        // use variables to prevent optimizing out the loop
        total += sum0+sum1;
        //total += sum2+sum3;
    }
    printf("total = %u\n",total);
}

// non SIMD version, not much slower
void subset_generator(uint32_t a, uint32_t c)
{
    uint32_t X = 0; // LCG state, use 0 as a seed
    uint32_t counter = (1 << 30); // 2^30 subsets created from 2^32 integers
    uint32_t sum, setcount = 0;
    uint64_t total = 0;
    struct subset_t S;
    while (counter--)
    {
        S.a = NEXT_X(X);
        S.b = NEXT_X(X);
        S.c = NEXT_X(X);
        S.d = NEXT_X(X);
        sum = hi27_sum(S);
        total += sum;
        // approximate sum by adding each number divided by 2^67
        // here, divide by 2^10 so sums fit in a range of size about 2^77
        // 1743757 * 2^77 = 263509107554218365724764667904 ~= 2.6 * 10^29
        // this is fairly close to the middle of possible subset sums
        //if (sum >> 10 == 1743757) ++setcount;
        if (sum >> 12 == 435939) ++setcount;
        // TODO write subsets to file output buffer
    }
    printf("total = %lu\n",total);
    printf("setcount = %u\n",setcount);
}

// setup buffer for writing to file and call subset generator
void worker_function(void *params)
{
    struct params_t *params_cast = (struct params_t*) params;
    uint32_t a = params_cast->a;
    uint32_t c = params_cast->c;
    uint32_t t_num = params_cast->t_num;
    printf("[%02u] a = %u, c = %u, starting\n",t_num,a,c);
    clock_t start = clock();
    subset_generator(a,c);
    clock_t end = clock();
    double elapsed = ((double)(end-start)) / CLOCKS_PER_SEC;
    printf("[%02u] a = %u, c = %u, done (%lf sec)\n",t_num,a,c,elapsed);
}

int main(int argc, char **argv)
{
    cache_alloc();
    cache_generate();
    //struct params_t params = { 29, 101, 0 };
    struct params_t params = { 1000000009, 1003, 0 };
    worker_function(&params);
    return 42;
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
//        times[i] = time_subset_generator(a,c);
        total_time += times[i];
    }
    printf("total_time = %lf\n",total_time);
    printf("average = %lf\n",total_time/tests);
    cache_free();
    return 42;
}
