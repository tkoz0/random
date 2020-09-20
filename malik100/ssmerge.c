// subset merger (uses a multi way merge of several files)
// similar subset sum caches
// expects input files to already be sorted

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

#define INPUT_BUF_SIZE (1<<22)

// ptr points to memory in buf
// when ptr == end, refill the buffer and move ptr back to beginning
// leave ptr == end to indicate that the file is done
typedef struct { int fd; uint8_t *buf, *ptr, *end;
                 uint64_t file_len, bytes_read; } HEAP_ITEM;

// sets up a heap item from a file, has a manually managed buffer
HEAP_ITEM *make_heap_item(const char *filename)
{
    HEAP_ITEM *result = malloc(sizeof(*result));
    result->fd = open(filename,O_RDONLY);
    result->file_len = lseek(result->fd,0,SEEK_END);
    assert(result->file_len != -1);
    lseek(result->fd,0,SEEK_SET);
    assert((result->file_len & 15) == 0); // multiple of 16
    result->bytes_read = 0;
    result->buf = malloc(INPUT_BUF_SIZE);
    uint64_t read_result = read(result->fd,result->buf,INPUT_BUF_SIZE);
    assert(read_result > 0 && ((read_result & 15) == 0));
    result->bytes_read += read_result;
    result->end = result->buf + read_result;
    result->ptr = result->buf;
    return result;
}

// extracts a subset from the file, 16 bytes copied to sptr
// returns 1 if a subset was extracted, 0 otherwise
int subset_pop(HEAP_ITEM *hi, void *sptr)
{
    if (hi->ptr == hi->end) return 0; // no more subsets
    memcpy(sptr,hi->ptr,16);
    hi->ptr += 16;
    if (hi->ptr == hi->end) // refill buffer if possible
    {
        if (hi->bytes_read < hi->file_len) // read more data from file
        {
            uint64_t read_result = read(hi->fd,hi->buf,INPUT_BUF_SIZE);
            assert(read_result > 0 && ((read_result & 15) == 0));
            hi->bytes_read += read_result;
            hi->end = hi->buf + read_result;
            hi->ptr = hi->buf; // return to beginning
        }
    }
    return 1;
}

// returns a pointer to the next subset without extracting it
void *subset_peek(HEAP_ITEM *hi)
{
    if (hi->ptr == hi->end) return NULL; // no more subsets
    return hi->ptr;
}

// for keeping the heap properly organized
int heap_compare(const void *a, const void *b)
{
    HEAP_ITEM *ha = (HEAP_ITEM*)a;
    HEAP_ITEM *hb = (HEAP_ITEM*)b;
    // subsets to compare
    void *sa = subset_peek(ha), *sb = subset_peek(hb);
    // item a is done, consider it >= item b result
    if (!sa) return (sb != NULL);
    // item a is nonnull, if item b is null return -1, otherwise compare sums
    if (!sb) return -1;
    return subset_compare(sa,sb); // both nonnull, use subset sums
}

HEAP_ITEM **heap;
uint32_t heap_len;

#define LEFT(i) (2*i+1)
#define RIGHT(i) (2*i+2)
#define PARENT(i) ((i-1)/2)

// move heap item down into correct position
// swap with smaller child to build a min heap
void sift_down(uint32_t i)
{
    uint32_t child, swap;
    HEAP_ITEM *tmp;
    while ((child = LEFT(i)) < heap_len)
    {
        swap = i;
        if (heap_compare(heap[swap],heap[child]) > 0)
            swap = child; // left child is smaller
        ++child; // check if other child is smaller
        if (child < heap_len && heap_compare(heap[swap],heap[child]) > 0)
            swap = child;
        if (swap == i) return;
        tmp = heap[i]; // swap heap items
        heap[i] = heap[swap];
        heap[swap] = tmp;
        i = swap;
    }
}

// checks heap for debugging (in a min heap, each node >= its parent)
void assert_heap()
{
    for (uint32_t i = 1; i < heap_len; ++i)
        assert(heap_compare(heap[i],heap[PARENT(i)]) >= 0);
}

// rearranges the elements into a heap, uses heapify down method
void heapify()
{
    uint32_t i = PARENT(heap_len-1);
    while (i) sift_down(i--);
    sift_down(0);
//assert_heap();
}

// takes the next subset in sequence, copies 16 bytes to sptr
// returns 1 if a subset is extracted, 0 otherwise
int next_subset(void *sptr)
{
    int result = subset_pop(heap[0],sptr);
    sift_down(0);
//assert_heap();
    return result;
}

#define OUTPUT_BUF_SIZE (1<<24)

// usage: <a.out> <output file> <input_0> [input_1] [input_2] ...
int main(int argc, char **argv)
{
    // initialize caches
    cache_alloc();
    cache_generate();
    assert(argc >= 3);
    // open output file
    fprintf(stderr,"opening output file...\n");
    int fdout;
    uint8_t *fdoutbuf = malloc(OUTPUT_BUF_SIZE);
    uint8_t *fdoutptr = fdoutbuf, *fdoutend = fdoutbuf + OUTPUT_BUF_SIZE;
    if (strcmp(argv[1],"-"))
    {
        fprintf(stderr,"output file: \"%s\"\n",argv[1]);
        fdout = open(argv[1],O_WRONLY|O_CREAT,0777);
    }
    else // stdout
    {
        fprintf(stderr,"output file: stdout\n");
        fdout = 1;
    }
    // setup heap for the input files
    heap_len = argc-2;
    heap = malloc(sizeof(*heap) * heap_len);
    for (int argi = 2; argi < argc; ++argi)
        heap[argi-2] = make_heap_item(argv[argi]);
    heapify();
    // stream through the subsets
    uint8_t subset[16];
    while (next_subset(subset)) // extract subsets until none left
    {
        memcpy(fdoutptr,subset,16);
        fdoutptr += 16;
        if (fdoutptr == fdoutend) // write buffer
        {
            if (write(fdout,fdoutbuf,OUTPUT_BUF_SIZE) != OUTPUT_BUF_SIZE)
            {
                fprintf(stderr,"ERROR writing output\n");
                return 1;
            }
            fdoutptr = fdoutbuf; // back to beginning
        }
    }
    if (write(fdout,fdoutbuf,fdoutptr-fdoutbuf) != fdoutptr-fdoutbuf)
    {
        fprintf(stderr,"ERROR writing output\n");
        return 1;
    }
    // resource cleanup
    close(fdout);
    free(fdoutbuf);
    for (int i = 0; i < heap_len; ++i)
    {
        close(heap[i]->fd);
        free(heap[i]->buf);
        free(heap[i]);
    }
    cache_free();
    return 0;
}
