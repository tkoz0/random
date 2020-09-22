// subset scanner
// expects input to be ordered from smallest to largest sum
// this sorted order is validated by assertion
// reads data from stdin, writes duplicate list to stdout

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define LOMASK ((1UL<<56) - 1)

// stores lower 56 bits in lo and all higher bits in hi
typedef struct { uint64_t lo, hi; } NUM_T;

// numbers modulo 2^56
const uint64_t LO56[100] =
{
    49358056305482605, 62425860880523552, 63772143341169379,  5166998042748712,
    29821718707829609, 20514003362636514, 64896906471655448, 22902651073069237,
    49224737451604962,  1917627132984362,  6493686898469897, 30922184852112194,
    24071877967672407, 24288392363185782, 33397274433060773, 53023917779645922,
    25073482848622778, 69817744874289715, 55675456406883311, 59156488290620291,
     5285160016495132, 51651901491393944, 22960988507619815, 40972546372029527,
    17965381759424197, 54213112757876599, 66068400374174904, 16360976595228759,
    26520778277835927, 47880599643440896, 67740979648112056, 63058947993958429,
     3726643157549935, 15329421996091841, 68773773195038935, 60358037769640221,
    42440069690795812, 47194775040684140, 19104978875374635, 22159385361889886,
    36129587719482213, 69935303097663913, 67545426558072654, 55342901203423077,
    70413993542951126, 69910034158737286, 16707188167211631, 34744506915827921,
    64894453884358934, 43042793022024562, 22005692386223609, 52330424331761609,
    59347792799756898, 23654470722684896, 25195781274779063, 25456740139036121,
    71035581140425360, 60379833778583625, 68080453684234314, 49162071705672554,
     3806894565807195, 48420672448428454, 66213484835739757,  8727298341213686,
    43092766639609653, 29652935694510701, 18682681418252321, 69053122337323514,
     3767680857723869, 58691318624504080, 67184216518956965, 22536229352291094,
    36604991797260202,  9960536158714744, 29131962835509140,  5379173141228738,
    24453348301033465, 22083635715660207, 12969900057217344, 21172701946395238,
    55690945888494680, 19535481335645134, 47393630925468328,    97713248543598,
    49190569682484748, 41304590684336101, 46636109977582543, 10107728529689151,
     4555046496451307,  9357201257642003, 62502494464776802, 51464474066204435,
    65203842746800286,  5700515264069434, 14658101158450901, 32435606915611763,
    52165686552319036, 60143067507677011, 23113349891010490,  2662952289529550
};

// numbers divided by 2^56 (integer part)
const uint64_t HIBITS[100] =
{
     79378523109,  76160548462,  66166052244,  25756124452,  59532606963,
    110566166802,  35707093915,  17966005654,  66119521459,  20460628217,
     35785954251,  71631547710,  31122780727, 103725217506,  86206814927,
     68593967082,  76553546275,  81251154687,  73742278560,  93504256479,
     59568858380,  62012391275,  36618232499,  17471056042,  62204126950,
    121428635511,  15495930343,  53834898672, 127847165237,  46507564241,
    122899403470,  60130504559,  33705699098,  93515221495, 122046167230,
     41490342353,  84896734475,  38328430511,  95537671482, 120345805798,
    130883593369,  66453074496,  50303611938, 129921348183, 137297333448,
     82068046661, 115378422768,  31445195593,  48255625730,  87726348581,
     24930608894,  88349501766, 118592060416,  46661163630, 117468172222,
     40146463436,  59628606741, 119831431128, 116784835179,  31465847350,
     65085601458,  61461485733,  99174768213,  31555644397,  92788731161,
     43874880070,  26612204464,  48800290715, 104772921682,  33277223689,
     64890374432,  39309045482, 121506176966, 136469486069,  73533835318,
    136800472050,  20417364248,  49202855322,  93472002595,  29721582891,
    117707888676, 138239659112, 137962391461,  86627614713,  60296816314,
    105038791036,  38533364830,  85685614124,  74767749374,  73797819985,
     91761910785,  18248299922,  33956555168, 135817660989,  40616383017,
     52616609326,  39298116360,  45537009627, 138149833169,  47383474138
};

// locache0..9 for caching 0..9,10..19,...,90..99 from LO56
// hicache0..9 for caching 0..9,10..19,...,90..99 from HIBITS
uint64_t *locache0, *locache1, *locache2, *locache3, *locache4,
         *locache5, *locache6, *locache7, *locache8, *locache9,
         *hicache0, *hicache1, *hicache2, *hicache3, *hicache4,
         *hicache5, *hicache6, *hicache7, *hicache8, *hicache9;

void cache_alloc()
{
    // use 1 malloc call
    locache0 = malloc(20 * (1<<13));
    locache1 = locache0 + (1<<10);
    locache2 = locache1 + (1<<10);
    locache3 = locache2 + (1<<10);
    locache4 = locache3 + (1<<10);
    locache5 = locache4 + (1<<10);
    locache6 = locache5 + (1<<10);
    locache7 = locache6 + (1<<10);
    locache8 = locache7 + (1<<10);
    locache9 = locache8 + (1<<10);
    hicache0 = locache9 + (1<<10);
    hicache1 = hicache0 + (1<<10);
    hicache2 = hicache1 + (1<<10);
    hicache3 = hicache2 + (1<<10);
    hicache4 = hicache3 + (1<<10);
    hicache5 = hicache4 + (1<<10);
    hicache6 = hicache5 + (1<<10);
    hicache7 = hicache6 + (1<<10);
    hicache8 = hicache7 + (1<<10);
    hicache9 = hicache8 + (1<<10);
}

void cache_free()
{
    free(locache0);
}

void cache_generate()
{
    // i = cache index, i2 = copy i (for bit extraction)
    // j = loop counter for summation, bit = extracted bit from i
    // s? = store computed sum before writing to cache
    uint32_t i, i2, j, bit;
    uint64_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9;
    for (i = 0; i < (1<<10); ++i)
    {
        i2 = i;
        s0 = s1 = s2 = s3 = s4 = s5 = s6 = s7 = s8 = s9 = 0;
        for (j = 0; j < 10; ++j, i2 >>= 1)
        {
            bit = i2 & 1;
            s0 += bit * LO56[j];
            s1 += bit * LO56[j+10];
            s2 += bit * LO56[j+20];
            s3 += bit * LO56[j+30];
            s4 += bit * LO56[j+40];
            s5 += bit * LO56[j+50];
            s6 += bit * LO56[j+60];
            s7 += bit * LO56[j+70];
            s8 += bit * LO56[j+80];
            s9 += bit * LO56[j+90];
        }
        locache0[i] = s0;
        locache1[i] = s1;
        locache2[i] = s2;
        locache3[i] = s3;
        locache4[i] = s4;
        locache5[i] = s5;
        locache6[i] = s6;
        locache7[i] = s7;
        locache8[i] = s8;
        locache9[i] = s9;
        i2 = i;
        s0 = s1 = s2 = s3 = s4 = s5 = s6 = s7 = s8 = s9 = 0;
        for (j = 0; j < 10; ++j, i2 >>= 1)
        {
            bit = i2 & 1;
            s0 += bit * HIBITS[j];
            s1 += bit * HIBITS[j+10];
            s2 += bit * HIBITS[j+20];
            s3 += bit * HIBITS[j+30];
            s4 += bit * HIBITS[j+40];
            s5 += bit * HIBITS[j+50];
            s6 += bit * HIBITS[j+60];
            s7 += bit * HIBITS[j+70];
            s8 += bit * HIBITS[j+80];
            s9 += bit * HIBITS[j+90];
        }
        hicache0[i] = s0;
        hicache1[i] = s1;
        hicache2[i] = s2;
        hicache3[i] = s3;
        hicache4[i] = s4;
        hicache5[i] = s5;
        hicache6[i] = s6;
        hicache7[i] = s7;
        hicache8[i] = s8;
        hicache9[i] = s9;
    }
}

// expect pointer to 16 bytes of raw subset data
// computes the subset sum faster using the caches
NUM_T subset_sum(const void *subset)
{
    NUM_T result = { 0, 0 };
    uint64_t lower = *((uint64_t*)subset); // get lower 64 bits
    // use uint64_t, shifting 9 bits left by 27 will discard bits in uint32_t
    uint64_t hi1 = ((uint32_t*)subset)[2];
    uint64_t hi2 = ((uint32_t*)subset)[3];
    uint64_t higher = (hi1 & 0x1FF) | ((hi1 >> 23) << 9)
                    | ((hi2 & 0x1FF) << 18) | ((hi2 >> 23) << 27);
    result.lo += locache0[lower & 0x3FF];
    result.hi += hicache0[lower & 0x3FF];
    lower >>= 10;
    result.lo += locache1[lower & 0x3FF];
    result.hi += hicache1[lower & 0x3FF];
    lower >>= 10;
    result.lo += locache2[lower & 0x3FF];
    result.hi += hicache2[lower & 0x3FF];
    lower >>= 10;
    result.lo += locache3[lower & 0x3FF];
    result.hi += hicache3[lower & 0x3FF];
    lower >>= 10;
    result.lo += locache4[lower & 0x3FF];
    result.hi += hicache4[lower & 0x3FF];
    lower >>= 10;
    result.lo += locache5[lower & 0x3FF];
    result.hi += hicache5[lower & 0x3FF];
    lower >>= 10;
    lower |= higher << 4; // combine the remaining 4 bits with higher 36 bits
    result.lo += locache6[lower & 0x3FF];
    result.hi += hicache6[lower & 0x3FF];
    lower >>= 10;
    result.lo += locache7[lower & 0x3FF];
    result.hi += hicache7[lower & 0x3FF];
    lower >>= 10;
    result.lo += locache8[lower & 0x3FF];
    result.hi += hicache8[lower & 0x3FF];
    lower >>= 10;
    result.lo += locache9[lower];
    result.hi += hicache9[lower];
    // finish summation
    result.hi += result.lo >> 56;
    result.lo &= LOMASK;
    return result;
}

// each pointer should point to 16 bytes of subset data
// subset comparator for qsort
int subset_compare(const void *a, const void *b)
{
    NUM_T a_sum = subset_sum(a);
    NUM_T b_sum = subset_sum(b);
    if (a_sum.hi > b_sum.hi) return 1;
    if (a_sum.hi < b_sum.hi) return -1;
    if (a_sum.lo > b_sum.lo) return 1;
    if (a_sum.lo < b_sum.lo) return -1;
    return 0;
}

#define INPUT_BUF_SIZE (1<<24)

// does not use arguments
int main(int argc, char **argv)
{
    // initialize caches
    cache_alloc();
    cache_generate();
    uint64_t subset_prev[2]; // previous subset for comparing
    subset_prev[0] = subset_prev[1] = 0; // 0 sum subset initially
    NUM_T sum_prev, sum_curr;
    sum_prev.hi = 0;
    sum_prev.lo = 0;
    uint64_t *buf, *ptr, *end, index = 0;
    buf = malloc(INPUT_BUF_SIZE);
    ssize_t read_length;
    while ((read_length = read(0,buf,INPUT_BUF_SIZE)) > 0)
    {
        if (read_length & 15) // not a multiple of 16
        {
            fprintf(stderr,"ERROR expected multiple of 16 bytes from stdin\n");
            return 1;
        }
        end = buf + (read_length>>3);
        for (ptr = buf; ptr < end; ptr += 2, ++index)
        {
            sum_curr = subset_sum(ptr);
#ifdef WRITE_SUMS
printf("sum(subsets[%lu]) = (%lu,%lu)\n",index,sum_curr.hi,sum_curr.lo);
#endif
            // sum decreased, print details and exit
            if ((sum_prev.hi > sum_curr.hi) ||
                (sum_prev.hi == sum_curr.hi && sum_prev.lo > sum_curr.lo))
            {
                fprintf(stderr,"ERROR input is not sorted, index %lu\n",index);
                fprintf(stderr,"subset_prev = (%lu,%lu)\n",
                        subset_prev[0],subset_prev[1]);
                fprintf(stderr,"subset_curr = (%lu,%lu)\n",
                        ptr[0],ptr[1]);
                fprintf(stderr,"sum_prev = (%lu,%lu)\n",
                        sum_prev.hi,sum_prev.lo);
                fprintf(stderr,"sum_curr = (%lu,%lu)\n",
                        sum_curr.hi,sum_curr.lo);
                return 1;
            }
            // equal sum, print details
            if ((sum_prev.hi == sum_curr.hi) && (sum_prev.lo == sum_curr.lo))
            {
                printf("DUPLICATE (%lu,%lu)\n",index-1,index);
                printf("subsets[%lu] = (%lu,%lu)\n",index-1,
                        subset_prev[0],subset_prev[1]);
                printf("subsets[%lu] = (%lu,%lu)\n",index,
                        ptr[0],ptr[1]);
                printf("sum(subsets[%lu]) = (%lu,%lu)\n",index-1,
                        sum_prev.hi,sum_prev.lo);
                printf("sum(subsets[%lu]) = (%lu,%lu)\n",index,
                        sum_curr.hi,sum_curr.lo);
            }
            memcpy(subset_prev,ptr,16);
            sum_prev = sum_curr;
        }
    }
    if (read_length == -1)
    {
        fprintf(stderr,"ERROR reading stdin\n");
        return 1;
    }
    fprintf(stderr,"DONE\n");
    // resource cleanup
    free(buf);
    cache_free();
    return 0;
}
