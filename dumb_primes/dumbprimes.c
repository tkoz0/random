#include <stdio.h>
#include <stdint.h>
#include <gmp.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

void rho(mpz_t n, uint32_t z, mpz_t ret)
{
    uint64_t iters = 0;
    time_t t = time(NULL);
    mpz_t x,y,d,dif;
    mpz_init_set_ui(x,2);
    mpz_init_set_ui(y,2);
    mpz_init_set_ui(d,1);
    mpz_init(dif);
    while (mpz_cmp_ui(d,1) == 0)
    {
        mpz_mul(x,x,x);
        mpz_add_ui(x,x,z);
        mpz_mod(x,x,n);
        mpz_mul(y,y,y);
        mpz_add_ui(y,y,z);
        mpz_mod(y,y,n);
        mpz_mul(y,y,y);
        mpz_add_ui(y,y,z);
        mpz_mod(y,y,n);
        mpz_sub(dif,x,y);
        mpz_abs(dif,dif);
        mpz_gcd(d,dif,n);
        ++iters;
        if ((iters & 0xFFFF) == 0)
        {
            printf("iters = %lu, time = %lu\n",iters,time(NULL)-t);
        }
    }
    assert(mpz_cmp(d,n) != 0);
    mpz_set(ret,d);
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    mpz_t n;
    //mpz_init_set_str(n,"143",10);
    mpz_init_set_str(n,"968781407364863083137710311141698230147190908917041451702241383953355271518921502882192737339838901020891735995810937664661421108789374588589346607874448213077354111309953517505519886124172729273077604131231288929464629246054705732462497177644931424626683663278210490850745697754500413234377340513471920096137534723980256656869243561933494899809681108978792553981365589",10);
    uint32_t z = atoi(argv[1]);
    mpz_t d;
    mpz_init(d);
    rho(n,z,d);
    char *ds = mpz_get_str(NULL,10,d);
    printf("factor = %s\n",ds);
    return 0;
}
