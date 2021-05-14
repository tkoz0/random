#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>

// 25 billion requires 35 bits
#define LIMIT 25000000000L
#define BIT_MASK_21 ((1L << 21) - 1)

// (a*a) % n, assuming a uses at most 42 bits
// n is also restricted to 42 bits
uint64_t mod_square(uint64_t a, uint64_t n)
{
    uint64_t r = (a & BIT_MASK_21) * a; // lower 21 bits
    r += (a >> 21) * ((a << 21) % n); // higher 21 bits
    return r % n;
}

// (a*b) % n, all 3 variables up to 42 bits
uint64_t mod_multiply(uint64_t a, uint64_t b, uint64_t n)
{
    uint64_t r = (a & BIT_MASK_21) * b;
    r += (a >> 21) * ((b << 21) % n);
    return r % n;
}

// computes a^(n-1) % n == 1, using a=2, n should be odd input > 1
bool prp(uint64_t n, uint64_t a)
{
    uint64_t e = n-1;
    uint64_t b = a;
    uint64_t output = 1;
    //if (e&1) output = b;
    while ((e >>= 1))
    {
        b = mod_square(b,n);
        if (e&1) output = mod_multiply(output,b,n);
    }
    return output == 1;
}

// strong prp test, using a=2, n should be odd input > 1
bool sprp(uint64_t n, uint64_t a)
{
    uint32_t s = -1; // guaranteed s >= 0, setting to -1 for loop at bottom
    uint64_t d = n-1;
    while (!(d & 1)) // compute s,d (actually s-1)
        ++s, d >>= 1;
    // compute a^d mod n
    uint64_t b = a;
    uint64_t res = b; // since d is odd
    //if (d&1) res = b;
    while ((d >>= 1))
    {
        b = mod_square(b,n);
        if (d&1) res = mod_multiply(res,b,n);
    }
    if (res == 1 || res == n-1) return true;
    // squarings to handle a^(d * 2^r) mod n for 1 <= r <= s-1
    while (s--)
        if ((res = mod_square(res,n)) == n-1)
            return true;
    return false;
}

uint32_t *ps = NULL; // odd only prime sieve
size_t ps_len;
#define PS_GET(n) (ps[n >> 5] & (1 << (n & 31)))
#define PS_SET(n) (ps[n >> 5] |= (1 << (n & 31)))
#define PRIME(n) (PS_GET(n>>1) == 0)

// initializes prime sieve and returns prime count
// this could be much better, primesieve can compute pi(2.5E10) ~40x faster
uint64_t init_ps()
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
        count -= __builtin_popcountll(ps[i]);
    for (size_t j = (ps_len-1)<<5; 2*j+1 <= LIMIT; ++j)
        if (!PS_GET(j)) ++count;
    return count+1; // 2 is skipped so include it
}

int main(int argc, char **argv)
{
    printf("sieving primes...\n");
    time_t t1 = clock();
    printf("pi(%lu)=%lu\n",LIMIT,init_ps());
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
    
    printf("computing pseudoprimes...\n");
    t1 = clock();
    //sprintf(fname,"pseudoprimes.%lu.prp(2).txt",LIMIT);
    sprintf(fname,"pseudoprimes.%lu.sprp(2).txt",LIMIT);
    f = fopen(fname,"w");
    //setvbuf(f,NULL,_IOFBF,1<<20);
    for (uint64_t n = 3; n <= LIMIT; n += 2)
        //if (prp(n,2) && (!PRIME(n)))
        if (sprp(n,2) && (!PRIME(n))) // switching order should improve speed
            fprintf(f,"%lu\n",n);
    fclose(f);
    t2 = clock();
    printf("pseudoprimes_time=%lf\n",(t2-t1)/(double)CLOCKS_PER_SEC);
    free(ps);
    return 0;
}
