/*

Usage: ./fibonacci <MAX_INDEX>

MAX_INDEX: used to compute the size of the addition buffers

Let F(n) be the nth fibonacci number.
F(n) = ((1.618...)^n-(0.618...)^n)/sqrt(5)
= floor(1.618^n / sqrt(5)) (also the closest integer since 1/sqrt(5) < 0.5)
Number of bits to store F(n) is (using upper bound 1.618^n/sqrt(5))
ceil(n*log2(1.618) - log2(sqrt(5)))

*/

// TODO rewrite with base 2^63

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
typedef uint64_t u64;
typedef u64 word_t;

#define LOG2_SQRT5 1.1609640474436813
#define LOG2_PHI   0.6942419136306174
#define BITS_F(n)  (1 + (int)(n*LOG2_PHI - LOG2_SQRT5))
#define WORDS_F(n) ((BITS_F(n)+(8*sizeof(word_t)-1))/(8*sizeof(word_t)))

word_t *buf_a, *buf_b;
uint64_t len, max_index, max_used;

// perform an addition, store a+b in a and then swap
void iterate()
{
    uint64_t *tmp_ptr, i = 0;
    word_t *ptr_a = buf_a, *ptr_b = buf_b;
    char carry = 0;
    while (i <= max_used)
    {
        ++i;
        __asm__
        (
            "addb %0,(%1)\n"
            "movb $0,%0\n"
            "jno 1f\n"
            "movb $1,%0\n"
            "1:\n"
            "movq (%2),%%rax\n"
            "addq %%rax,(%1)\n"
            "jno 2f\n"
            "movb $1,%0\n"
            "2:\n"
            : "=r"(carry)
            : "r"(ptr_a),"r"(ptr_b)
            : "%rax"
        );
        printf("%u\n",carry);
        /*
        *ptr_a += carry;
        carry = 0;
        __asm__ volatile
        (
            "jno of1\n"
            "movb $1,%0\n"
            "of1:\n"
            : "=r"(carry)
        );
        *ptr_a += *ptr_b;
        __asm__ volatile
        (
            "jno of2\n"
            "movb $1,%0\n"
            "of2:\n"
            : "=r"(carry)
        );
        */
        ++ptr_a;
        ++ptr_b;
    }
    if (carry)
    {
        ++(*ptr_a);
        ++max_used;
    }
    else
    {
        printf("here\n");
    }
    // swap
    tmp_ptr = buf_a;
    buf_a = buf_b;
    buf_b = tmp_ptr;
}

int main(int argc, char **argv)
{
    //size_t ret;
    assert(argc >= 2);
    max_index = atoll(argv[1]);
    assert(max_index);
    len = WORDS_F(max_index);
    buf_a = calloc(len,sizeof(word_t));
    buf_b = calloc(len,sizeof(word_t));
    buf_b[0] = 1;
    max_used = 0; // highest index used in integer representations so far
    while (--max_index)
    {
        iterate();
        printf("a,b = %lu,%lu\n",buf_a[0],buf_b[0]);
    }
    // buf_b now stores output
    //ret = fwrite(buf_b,sizeof(word_t),len,stdout);
    //assert(ret == len);
    // cleanup
    free(buf_a);
    free(buf_b);
    return 0;
}
