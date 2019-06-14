#include "diffie_hellman.h"

#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

typedef unsigned __int128 uint128;

#define SOPHIE_GERMAIN_P 4611686018427402023

uint64_t expm(uint64_t m, uint64_t e, uint64_t mod)
{
    uint128 _r = 1u;
    uint128 _m = (uint128)m;
    uint128 _mod = (uint128)mod;
    
    while(e)
    {
        if(e & 1) _r = (_r * _m) % _mod;
        _m = (_m * _m) % _mod;
        e >>= 1;
    }

    return (uint64_t)_r;
}
uint64_t generateur(uint64_t p)
{
    uint64_t q, temp, g;

    q = (p - 1) / 2;
    for(g = 2; g < p; g++)
    {
        temp = expm(g, q, p);
        if(temp != 1) return g;
    }

    return 0;
}

void dh_generate_key_step1(uint64_t* a, uint64_t* A)
{
    struct timeval time; 
    gettimeofday(&time, NULL);
    srand(time.tv_usec);

    *a = 2 + rand() % (SOPHIE_GERMAIN_P - 2 + 1);
    *A = expm(generateur(SOPHIE_GERMAIN_P), *a, SOPHIE_GERMAIN_P);
}
void dh_generate_key_step2(uint64_t* key, uint64_t* a, uint64_t* B)
{
    *key = expm(*B, *a, SOPHIE_GERMAIN_P);
}