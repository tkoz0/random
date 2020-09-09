#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
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
    cache0 = malloc(1<<14); // use 1 malloc call instead
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

// parameters for a thread
struct params_t { uint32_t t_num; int fd; };

// globals for work unit request management
pthread_mutex_t wu_lock;
int wu_fd;

// return 96 bytes of binary data from work set file
// these represent the 24 uint32_t parameters for the LCGs
uint32_t *get_work_unit()
{
    if (pthread_mutex_lock(&wu_lock))
    {
        printf("ERROR locking mutex\n");
    }
    uint32_t *buf = malloc(96);
    ssize_t bytes = read(wu_fd,buf,96);
    if (bytes < 96)
    {
        if (bytes > 0)
        {
            printf("ERROR expected 96 byte work unit, got %lu bytes\n",bytes);
        }
        free(buf);
        buf = NULL;
    }
    if (pthread_mutex_unlock(&wu_lock))
    {
        printf("ERROR unlocking mutex\n");
    }
    return buf;
}

// size of a manually managed file buffer
#define TFILE_BUF (1<<22)

// SIMD version, using SSE2 and SSE4.1, only a little faster
// the L1 cache accesses for subset summing are much slower
// lcgs must contain 24 uint32_t values (96 bytes)
// satisfying subsets are written to the provided file descriptor
void *subset_generator_sse(void *lcgs, int fd, uint32_t t_num, uint8_t *buf)
{
    // using SSE2 instructions to parallelize LCGs
    // 8 LCGs, 2^32-1 numbers each (~34 billion numbers generated total)
    __m128i A0 = _mm_loadu_si128(lcgs+0);
    __m128i C0 = _mm_loadu_si128(lcgs+4);
    __m128i X0 = _mm_loadu_si128(lcgs+8);
    __m128i A1 = _mm_loadu_si128(lcgs+12);
    __m128i C1 = _mm_loadu_si128(lcgs+16);
    __m128i X1 = _mm_loadu_si128(lcgs+20);
    uint32_t counter = 0xFFFFFFFF; // loop counter
    uint32_t set0[4], set1[4];
    uint32_t sum0, sum1; // subset sum
    uint8_t *bytes; // pointer for accessing bytes in set data
    uint8_t *buf_ptr = buf, *buf_end = buf+TFILE_BUF;
    uint32_t sscount = 0;
    while (counter--)
    {
        // advance state of the 4 LCGs, X stores the new subset
        X0 = _mm_add_epi32(_mm_mullo_epi32(X0,A0),C0); // SSE4.1 and SSE2
        _mm_storeu_si128((void*)set0,X0); // SSE2
        X1 = _mm_add_epi32(_mm_mullo_epi32(X1,A1),C1);
        _mm_storeu_si128((void*)set1,X1);
        bytes = (uint8_t*) set0;
        sum0 = cache0[bytes[0]] + cache1[bytes[1]]
             + cache2[bytes[2]] + cache3[bytes[3]]
             + cache4[bytes[4]] + cache5[bytes[5]]
             + cache6[bytes[6]] + cache7[bytes[7]]
             + cache8[set0[2]&0x1FF] + cache9[set0[2]>>23]
             + cache10[set0[3]&0x1FF] + cache11[set0[3]>>23];
        bytes = (uint8_t*) set1;
        sum1 = cache0[bytes[0]] + cache1[bytes[1]]
             + cache2[bytes[2]] + cache3[bytes[3]]
             + cache4[bytes[4]] + cache5[bytes[5]]
             + cache6[bytes[6]] + cache7[bytes[7]]
             + cache8[set1[2]&0x1FF] + cache9[set1[2]>>23]
             + cache10[set1[3]&0x1FF] + cache11[set1[3]>>23];
        if (sum0 >> 12 == 435939)
        {
            ++sscount;
            memcpy(buf_ptr,set0,16);
            buf_ptr += 16;
            if (buf_ptr == buf_end)
            {
                if (write(fd,buf,TFILE_BUF) != TFILE_BUF)
                    printf("[thread %u] ERROR writing subsets\n",t_num);
                buf_ptr = buf;
            }
        }
        if (sum1 >> 12 == 435939)
        {
            ++sscount;
            memcpy(buf_ptr,set1,16);
            buf_ptr += 16;
            if (buf_ptr == buf_end)
            {
                if (write(fd,buf,TFILE_BUF) != TFILE_BUF)
                    printf("[thread %u] ERROR writing subsets\n",t_num);
                buf_ptr = buf;
            }
        }
    }
    if (write(fd,buf,buf_ptr-buf) != buf_ptr-buf)
        printf("[thread %u] ERROR writing subsets\n",t_num);
    uint32_t *retval = malloc(4);
    *retval = sscount;
    return retval;
}

// setup buffer for writing to file and call subset generator
void *worker_function(void *params)
{
    struct params_t *params_cast = (struct params_t*) params;
    uint32_t t_num = params_cast->t_num, *lcgparams;
    uint32_t completed = 0, *retval;
    uint64_t found = 0;
    uint8_t *file_buf = malloc(TFILE_BUF);
    int fd = params_cast->fd;
    clock_t beg,end;
    double time_used = 0.0;
    printf("[thread %u] starting\n",t_num);
    beg = clock();
    for (;;)
    {
        lcgparams = get_work_unit();
        if (!lcgparams) break; // NULL means none left
        retval = subset_generator_sse(lcgparams,fd,t_num,file_buf);
        found += *retval;
        ++completed;
        free(lcgparams);
        printf("[thread %u] unit done, %u subsets\n",t_num,*retval);
        free(retval);
    }
    end = clock();
    free(file_buf);
    time_used = (double)(end-beg) / CLOCKS_PER_SEC;
    printf("[thread %u] FINISHED, %u units, ~%lf sec each, %lu subsets found\n",
            t_num,completed,time_used/completed,found);
    return NULL;
}

// usage: <a.out> <threads> <workset_file> <output_dir>
int main(int argc, char **argv)
{
    cache_alloc();
    cache_generate();
    assert(argc >= 4);
    uint32_t thread_count = atoi(argv[1]);
    assert(thread_count > 0 && thread_count < 100); // sane numbers
    printf("thread_count = %u\n",thread_count);
    pthread_mutex_init(&wu_lock,NULL);
    wu_fd = open(argv[2],O_RDONLY);
    assert(wu_fd != -1);
    printf("opened workset from \"%s\"\n",argv[2]);
    DIR *outdir = opendir(argv[3]);
    if (outdir)
    {
        closedir(outdir); // exists
        printf("ERROR dir exists, assuming workset completed\n");
        exit(1);
    }
    else if (mkdir(argv[3],0777))
    {
        printf("ERROR creating output dir\n");
        exit(1);
    }
    printf("writing output to dir \"%s\"\n",argv[3]);
    pthread_t *threads = malloc(thread_count * sizeof(*threads));
    struct params_t *params = malloc(thread_count * sizeof(*params));
    char filepath[strlen(argv[3])+32];
    for (uint32_t t = 0; t < thread_count; ++t)
    {
        printf("creating thread %u\n",t);
        params[t].t_num = t;
        sprintf(filepath,"%s/t%02u.subsets_raw",argv[3],t);
        //sprintf(filepath,"t%02u.subsets_raw",t);
        printf("opening file \"%s\"\n",filepath);
        params[t].fd = open(filepath,O_WRONLY|O_CREAT,0777);
        assert(params[t].fd != -1); // error
        if (pthread_create(threads+t,NULL,worker_function,params+t))
        {
            printf("ERROR creating thread %u\n",t);
        }
    }
    for (uint32_t t = 0; t < thread_count; ++t)
    {
        if (pthread_join(threads[t],NULL))
        {
            printf("ERROR joining thread %u\n",t);
        }
        close(params[t].fd);
        printf("joined thread %u\n",t);
    }
    printf("done\n");
    free(threads);
    free(params);
    pthread_mutex_destroy(&wu_lock);
    close(wu_fd);
    cache_free();
    return 0;
}
