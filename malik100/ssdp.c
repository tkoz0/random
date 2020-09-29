// subset generator that uses modular sums instead
// finds subsets with sum congruent to 0 modulo 2^32
// requires about 32GiB of RAM to store the subset cache hash table
// writes output to stdout

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// each number modulo 2^32
const uint32_t LOW32[100] =
{
      82074477U, 1586815264U,  778726115U, 2061805352U, 4130077545U,
    2606359266U, 2168703000U, 4255121589U, 1308031970U, 1544731690U,
    1289595913U,  868877122U, 3612800087U,  116987510U, 3876356005U,
    3889886690U,  645486778U, 2051029555U, 1507265519U, 1052300163U,
     895304220U, 2019440024U, 3148890599U, 1477200983U, 1711691461U,
    4028233591U,  791565496U, 3055753303U, 3765097623U, 3579922176U,
    3836323256U, 1930885149U, 3114025839U,  816867777U, 3777450199U,
    3365493021U, 3190400804U, 3649957996U, 2270092331U, 4109351518U,
     702983013U, 2724544937U,  607270734U, 1326567269U, 1884823766U,
    1078220678U, 1608972911U, 3902615761U, 2007732502U,  876079986U,
    2243405305U, 1631418313U, 3423105634U, 2764474336U, 4172856759U,
     824208857U,  340384400U, 2415409225U, 3657369674U, 3429915498U,
    3058357339U, 1427306918U, 1257871469U, 2105153014U, 2549176117U,
     446589549U, 3177381921U, 3951835642U, 2401686493U, 2114144528U,
    1612715941U, 3374234390U, 3440701354U,  193149816U, 2825625492U,
    1480895682U, 3540865017U,  506354095U,  766429504U, 3530766950U,
    3336807512U, 2978415566U, 2548937384U, 2742559598U,    3751436U,
    1867853797U, 2162916303U, 4034890303U, 1750809323U,  822982675U,
      69605986U, 2672080659U, 2901006494U, 2740584250U,  547460821U,
    1011317875U, 1617581116U,  511661907U,  567455674U, 3846532814U
};

// cache0..7 for numbers 0..7,8..15,...,56..63
// cache8..11 for numbers 64..72,73..81,82..90,91..99
uint32_t *cache0, *cache1, *cache2, *cache3, *cache4, *cache5,
         *cache6, *cache7, *cache8, *cache9, *cache10, *cache11;

void cache_alloc()
{
    // use 1 malloc call, locate each cache pointer within
    cache0 = malloc(1<<14);
    cache1 = cache0 + (1<<8);
    cache2 = cache1 + (1<<8);
    cache3 = cache2 + (1<<8);
    cache4 = cache3 + (1<<8);
    cache5 = cache4 + (1<<8);
    cache6 = cache5 + (1<<8);
    cache7 = cache6 + (1<<8);
    cache8 = cache7 + (1<<8);
    cache9 = cache8 + (1<<9);
    cache10 = cache9 + (1<<9);
    cache11 = cache10 + (1<<9);
}

void cache_free()
{
    free(cache0);
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
            s0 += bit * LOW32[j];
            s1 += bit * LOW32[j+8];
            s2 += bit * LOW32[j+16];
            s3 += bit * LOW32[j+24];
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
            s0 += bit * LOW32[j+32];
            s1 += bit * LOW32[j+40];
            s2 += bit * LOW32[j+48];
            s3 += bit * LOW32[j+56];
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
            s0 += bit * LOW32[j+64];
            s1 += bit * LOW32[j+73];
            s2 += bit * LOW32[j+82];
            s3 += bit * LOW32[j+91];
        }
        cache8[i] = s0;
        cache9[i] = s1;
        cache10[i] = s2;
        cache11[i] = s3;
    }
}

// sums a subset of the first 32 numbers
uint32_t partial_subset_sum(uint32_t partial_subset)
{
    uint32_t result = 0;
    result += cache0[partial_subset & 0xFF];
    partial_subset >>= 8;
    result += cache1[partial_subset & 0xFF];
    partial_subset >>= 8;
    result += cache2[partial_subset & 0xFF];
    partial_subset >>= 8;
    result += cache3[partial_subset];
    return result;
}

// 2^32 subsets of the first 32 numbers need to be stored
// the empty subset is reserved for indicating empty hash table cells
// this value must be >= 2 so empty hash table buckets are frequent
// memory size needed is 16GiB * HT_SIZE_MULT
#define HT_SIZE_MULT 2
#define HT_SIZE (HT_SIZE_MULT * (1UL << 32))

// performance testing on 2x E5-2670 with 96GiB of PC3L-10600R
// HT_SIZE_MULT=2 --> ~6min to setup hash table
// HT_SIZE_MULT=4 --> ~11min to setup hash table

uint32_t *HT;

uint64_t hash(uint32_t partial_subset)
{
    return HT_SIZE_MULT * ((uint64_t)partial_subset_sum(partial_subset));
}

// forces allocation of HT by does unnecessary operations to it
void force_allocate()
{
    HT = calloc(HT_SIZE,sizeof(*HT));
    // set 2 equal values at the start of each 4KiB page
    for (uint32_t *ptr = HT+1024; ptr < HT+HT_SIZE; ptr += 1024)
        ptr[0] = ptr[1] = *(ptr-1024)+1;
    // zero those values
    for (uint32_t *ptr = HT; ptr < HT+HT_SIZE; ptr += 1024)
        ptr[0] -= ptr[1], ptr[1] = 0;
}

// hash table of subsets of the first 32 numbers
// uses linear probing with a few modifications
void setup_hash_table()
{
    fprintf(stderr,"allocating hash table...\n");
    force_allocate();
    if (!HT)
    {
        fprintf(stderr,"ERROR allocating hash table\n");
        exit(1);
    }
    fprintf(stderr,"inserting subsets...\n");
    uint64_t subset, index;
    for (subset = 0; subset < (1UL<<32); ++subset) // insert all 2^32 subsets
    {
        if ((subset & ((1<<24)-1)) == 0) // progress message
            fprintf(stderr,"hash table setup: %lu / 4294967296\n",subset);
        index = hash((uint32_t)subset);
        while ((index < HT_SIZE) && HT[index]) ++index; // find empty bucket
        if (index < HT_SIZE) HT[index] = (uint32_t)subset; // insert subset
        // if index == HT_SIZE then skip insertion, barely hurts performance
        // worst it does is skip a few subsets summing to migher modulus values
    }
    HT[HT_SIZE-1] = 0; // ensure 0 at end for terminating linear probing
    // may delete a subset but that will barely hurt performance
    // it prevents having to wrap around to the beginning of the hash table
    fprintf(stderr,"hash table setup done\n");
}

#define BUF_SIZE (1<<22)
pthread_mutex_t stdout_lock;

// generate subsets
// LCG0 is for generating random 68 bits for the higher 68 numbers
// LCG1 is for selecting from hash table at random when theres multiple options
void subset_generator(uint64_t a0, uint64_t c0, uint32_t a1, uint32_t c1)
{
    uint32_t *buf = malloc(BUF_SIZE);
    uint32_t *ptr = buf, *end = buf + (BUF_SIZE>>2);
    uint64_t X0 = 0; // LCG0 state
    uint32_t X1 = 0; // LCG1 state
    uint64_t subset[2], index;
    // pointers of different data types for conveniently accessing bytes
    uint32_t *subset_ptr4 = (uint32_t*)subset;
    uint8_t *subset_ptr1 = (uint8_t*)subset;
    uint32_t sum68, sum32, found, count;
    // loop forever, LCG0 wont run loop around in any practical amount of time
    for (;;)
    {
        // advance LCG0 for random subset
        subset[0] = a0*X0 + c0;
        subset[1] = a0*subset[0] + c0;
        X0 = subset[1];
        X1 = a1*X1 + c1; // advance LCG1 for hash table selection
        // compute sum of the 68 higher numbers
        sum68 = cache4[subset_ptr1[4]] + cache5[subset_ptr1[5]]
              + cache6[subset_ptr1[6]] + cache7[subset_ptr1[7]]
              + cache8[subset_ptr4[2]&0x1FF] + cache9[subset_ptr4[2]>>23]
              + cache10[subset_ptr4[3]&0x1FF] + cache11[subset_ptr4[3]>>23];
        sum32 = 0 - sum68; // needed value to find in hash table
        index = HT_SIZE_MULT * ((uint64_t)sum32);
        found = 0; // count subsets here with sum value of sum32
        while (HT[index])
        {
            if (partial_subset_sum(HT[index]) == sum32) ++found;
            ++index;
        }
        if (found) // possible to complete a subset
        {
            index = HT_SIZE_MULT * ((uint64_t)sum32);
            found = (X1 % found)+1; // num subsets of sum32 to iterate through
            for (;; ++index) // loop until that subset is found
            {
                if (partial_subset_sum(HT[index]) == sum32) --found;
                if (found == 0)
                {
                    subset_ptr4[0] = HT[index]; // include in subset bytes
                    break;
                }
            }
            memcpy(ptr,subset,16);
            ptr += 4;
            if (ptr == end) // flush buffer
            {
                if (pthread_mutex_lock(&stdout_lock))
                {
                    fprintf(stderr,"ERROR locking mutex\n");
                    exit(1);
                }
                if (write(1,buf,BUF_SIZE) != BUF_SIZE)
                {
                    fprintf(stderr,"ERROR writing buffer\n");
                    exit(1);
                }
                if (pthread_mutex_unlock(&stdout_lock))
                {
                    fprintf(stderr,"ERROR unlocking mutex\n");
                    exit(1);
                }
                ptr = buf; // return to start
            }
        }
    }
    free(buf); // cant be reached because the above loop is infinite
}

// pass LCG data to a thread
typedef struct { uint64_t a0, c0; uint32_t a1, c1; } LCG_T;

void *start_thread(void *lcgs_param)
{
    LCG_T *lcgs = (LCG_T*) lcgs_param;
    subset_generator(lcgs->a0,lcgs->c0,lcgs->a1,lcgs->c1);
    return NULL; // never reached
}

// parameters are for a 64 bit LCG to create parameters for randomization
// usage: <a.out> <threads> <multiplier> <addend>
int main(int argc, char **argv)
{
    // initialize things
    cache_alloc();
    cache_generate();
    setup_hash_table();
    pthread_mutex_init(&stdout_lock,NULL);
    // extract LCG params
    assert(argc >= 4);
    uint64_t a = atol(argv[2]);
    uint64_t c = atol(argv[3]);
    assert(a % 4 == 1);
    uint64_t X = 0;
    // start the threads
    uint32_t thread_count = atoi(argv[1]);
    assert(thread_count > 0 && thread_count < 100);
    pthread_t threads[thread_count];
    LCG_T lcg_params[thread_count];
    for (uint32_t i = 0; i < thread_count; ++i)
    {
        // fill LCG params for this thread
        while ((X = a*X + c) % 4 != 1);
        lcg_params[i].a0 = X;
        lcg_params[i].c0 = (X = a*X + c);
        while ((X = a*X + c) % 4 != 1);
        lcg_params[i].a1 = (uint32_t) X;
        lcg_params[i].c1 = (uint32_t) (X = a*X + c);
        pthread_create(threads+i,NULL,start_thread,lcg_params+i);
    }
    void *thread_result[1];
    for (uint32_t i = 0; i < thread_count; ++i) // never reached
        pthread_join(threads[i],thread_result);
// testing, just runs subset_generator with some LCG parameters
//subset_generator(5,7,9,11);
    // resource cleanup, never happens because infinite loops
    pthread_mutex_destroy(&stdout_lock);
    cache_free();
    free(HT);
    return 0;
}
