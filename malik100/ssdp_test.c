// DP version for subset generation
// uses DP to find subsets congruent to 0 modulo a power of 2
// the modulus used determines the RAM requirements (bits = 100 * modulus)
// only supports up to 2^32 due to using uint32_t (50GiB max)

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

#define MODBITS 24
#define MODMASK ((uint32_t)((1UL<<MODBITS)-1))
#define ROWBITS ((uint64_t)(1UL<<MODBITS))
#define GET_BIT(arr,i) ((arr)[(i)>>5] & (1<<((i)&31)))
#define SET_BIT(arr,i) ((arr)[(i)>>5] |= (1<<((i)&31)))
#define UNSET_BIT(arr,i) ((arr)[(i)>>5] &= ((1UL<<32)-(1<<((i)&31))))

uint32_t NUMS[100];
uint32_t *DP;

void make_dp_table()
{
    // mask numbers to number of bits used
    assert(MODBITS > 5);
    for (uint32_t i = 0; i < 100; ++i)
        NUMS[i] = LOW32[i] & MODMASK;
    // generate DP table (allocate as a single block of memory)
    printf("allocating DP table (%lu bytes)\n",100*(ROWBITS>>3));
    DP = calloc(100*(ROWBITS>>5),sizeof(uint32_t));
    if (!DP)
    {
        printf("ERROR DP table allocation failed\n");
        exit(1);
    }
    uint32_t *row = DP, *prevrow, num;
    // generate row 0 separately
    // DP[0][i] = DP[-1][NUMS[i]](false) or DP[-1][i-NUMS[0]](true if i=NUMS[0])
    SET_BIT(row,0);
    SET_BIT(row,NUMS[0]);
    // generate future rows based on previous row
    for (uint32_t r = 1; r < 100; ++r)
    {
        printf("generating DP[%u]\n",r);
        prevrow = row;
        row += (ROWBITS >> 5);
        assert(prevrow + (ROWBITS >> 5) == row);
        num = NUMS[r];
        for (uint64_t i = 0; i < ROWBITS; ++i)
            if (GET_BIT(prevrow,i))
            {
                SET_BIT(row,i);
                SET_BIT(row,(i+num)&MODMASK);
            }
    }
    for (uint32_t r = 0; r < 100; ++r)
    {
        row = DP + (r * (ROWBITS >> 5));
        uint64_t bits_set = 0;
        for (uint64_t i = 0; i < ROWBITS; ++i)
            if (GET_BIT(row,i)) ++bits_set;
        printf("DP[%u] has %lu ones\n",r,bits_set);
    }
}

int main(int argc, char **argv)
{
    make_dp_table();
    // cleanup
    free(DP);
    return 0;
}
