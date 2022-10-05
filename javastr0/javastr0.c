#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"

const uint8_t *CHARSET = (uint8_t*)" 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
size_t CHARSETLEN;
#define CACHESTRLEN 6

void errw(const char *f, ...)
{
    va_list args;
    va_start(args,f);
    vfprintf(stderr,f,args);
    va_end(args);
}

uint32_t *bitarr;

uint8_t *genarr;
size_t generated_total = 0;
void genstrs(uint32_t i, uint32_t h)
{
    if (i == CACHESTRLEN)
    {
        ++generated_total;
        bitarr[h>>5] |= 1<<(h&31);
    }
    else
    {
        const uint8_t *p = CHARSET;
        while (*p)
        {
            genarr[i] = *p;
            genstrs(i+1,31*h+*p);
            ++p;
        }
    }
}

void test_hash_uniq()
{
    genarr = malloc(CACHESTRLEN+1);
    bitarr = calloc(1<<27,4);
    genarr[CACHESTRLEN] = '\0';
    genstrs(0,0);
    errw("generated %lu strings of length %u\n",generated_total,CACHESTRLEN);
    size_t bitcount = 0;
    for (size_t i = 0; i < 1<<27; ++i)
        bitcount += __builtin_popcount(bitarr[i]);
    errw("filled %lu buckets of the hash table\n",bitcount);
    free(genarr);
    free(bitarr);
}

int main(int argc, char **argv)
{
    //printf("%u\n",hash("creashaks organzine"));
    CHARSETLEN = strlen((char*)CHARSET);
    errw("using charset with %u chars\n",CHARSETLEN);
    test_hash_uniq();
    return 0;
}
