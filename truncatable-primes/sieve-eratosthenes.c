#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>

#define LIMIT 1000000000L

uint32_t *ps = NULL;
size_t ps_len;
#define PS_GET(n) (ps[n >> 5] & (1 << (n & 31)))
#define PS_SET(n) (ps[n >> 5] |= (1 << (n & 31)))

// {1} (2)
uint64_t wheel_2()
{
    size_t bits = (LIMIT+1)/2;
    ps_len = (bits + 31) / 32;
    ps = calloc(ps_len,sizeof(*ps)); // ps[i] bit j represents 2*(32*i+j) + 1
    // 0 bit means prime, 1 bit means not prime (works conveniently with calloc)
    ps[0] = 1; // 1 is not prime
    // 2 is prime but ignored for the odd only sieve
    // loop to find primes up to square root and cross out from sieve
    for (size_t i = 3; i*i <= LIMIT; i += 2)
    {
        if (PS_GET(i/2)) continue; // not prime
        // loop j over bit indexes in the array
        // increment i to go up by 2*i values which skips even numbers
        for (size_t j = (i*i)>>1; j < bits; j += i)
            PS_SET(j);
    }
    uint64_t count = (ps_len-1)<<5;
    for (size_t i = 0; i < ps_len-1; ++i)
        count -= __builtin_popcount(ps[i]);
    for (size_t j = (ps_len-1)<<5; 2*j+1 <= LIMIT; ++j)
        if (!PS_GET(j)) ++count;
    return count+1; // 2 is skipped so include it
}

// {1,5} (6)
uint64_t wheel_2_3()
{
    size_t bits = (LIMIT+5)/3;
    ps_len = (bits+31) / 32;
    return 0;
}

// {1,7,11,13,17,19,23,29} (30)
uint64_t wheel_2_3_5()
{
    static const uint32_t wheel[8] = {1,7,11,13,17,19,23,29};
    static const uint32_t whsqindex[8] = {0,5,0,5,5,0,5,0};
    static const uint32_t step[8] = {6,4,2,4,2,4,6,2};
    size_t bits = (LIMIT+29)*4/15;
    ps_len = (bits+31) / 32;
    ps = calloc(ps_len,sizeof(*ps));
    ps[0] = 1; // 1 is not prime
    for (size_t i = 8;; ++i)
    {
        if (PS_GET(i)) continue; // not prime
        uint64_t p = 30*(i>>3) + wheel[i&7];
        p *= p;
        uint32_t i = whsqindex[i&7];
        if (p > LIMIT) break;
        for (;;)
        {
            while (i < 8 && p <= LIMIT)
            {
                ; // TODO not sure how to do wheel sieving properly
            }
            i = 0;
        }
    }
    uint64_t count = (ps_len-1)<<5;
    for (size_t i = 0; i < ps_len-1; ++i)
        count -= __builtin_popcount(ps[i]);
    for (size_t j = (ps_len-1)<<5; 30*(j>>3)+wheel[j&7] <= LIMIT; ++j)
        if (!PS_GET(j)) ++count;
    return count+3; // 2,3,5 skipped
}

int main(int argc, char **argv)
{
    printf("sieving primes...\n");
    time_t t1 = clock();
    printf("pi(%lu)=%lu\n",LIMIT,wheel_2());
    time_t t2 = clock();
    printf("sieve_time=%lf\n",(t2-t1)/(double)CLOCKS_PER_SEC);
    
    printf("writing sieve...\n");
    t1 = clock();
    char fname[100];
    sprintf(fname,"sieve.%lu.wheel(2).bin",LIMIT);
    FILE *f = fopen(fname,"wb");
    fwrite(ps,sizeof(*ps),ps_len,f);
    fclose(f);
    t2 = clock();
    printf("write_time=%lf\n",(t2-t1)/(double)CLOCKS_PER_SEC);
    
    free(ps);
    return 0;
}
