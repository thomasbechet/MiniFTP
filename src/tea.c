#include "tea.h"

#include "diffie_hellman.h"
#include "tcp_connection.h"

#include <stdlib.h>
#include <stdio.h>

#define DELTA 0x9e3779b9

int tea_obtain_key(uint64_t* key, int server)
{
    uint64_t a, A, B;
    size_t k;

    dh_generate_key_step1(&a, &A);

    if(server)
    {
        tcp_send(&A, sizeof(uint64_t));
        tcp_recv(&B, sizeof(uint64_t), &k);
    }   
    else
    {
        tcp_recv(&B, sizeof(uint64_t), &k);
        tcp_send(&A, sizeof(uint64_t));
    }
        
    dh_generate_key_step2(key, &a, &B); 
}
int tea_obtain_key_as_client(teakey_t* key)
{
    uint64_t k1, k2;

    tea_obtain_key(&k1, 0);
    tea_obtain_key(&k2, 0);

    (*key)[0] = k1;
    (*key)[1] = k2;

    return 0;
}
int tea_obtain_key_as_server(teakey_t* key)
{
    uint64_t k1, k2;

    tea_obtain_key(&k1, 1);
    tea_obtain_key(&k2, 1);

    (*key)[0] = k1;
    (*key)[1] = k2;

    return 0;
}
void tea_encrypt(teablock_t* v, teakey_t k)
{
    uint32_t v0 = ((uint32_t*)v)[0];
    uint32_t v1 = ((uint32_t*)v)[1];
    unsigned sum = 0;
    unsigned i;                                  
    uint32_t k0 = ((uint32_t*)k)[0];
    uint32_t k1 = ((uint32_t*)k)[1];
    uint32_t k2 = ((uint32_t*)k)[2];
    uint32_t k3 = ((uint32_t*)k)[3];
    for (i = 0; i < 32; i++) 
    {                                                     
        sum += DELTA;
        v0 += ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
        v1 += ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);
    }                                                    
    ((uint32_t*)v)[0] = v0; 
    ((uint32_t*)v)[1] = v1;
}
void tea_decrypt(teablock_t* v, teakey_t k)
{
    uint32_t v0 = ((uint32_t*)v)[0];
    uint32_t v1 = ((uint32_t*)v)[1];
    unsigned sum = DELTA * 32;
    unsigned i;                                  
    uint32_t k0 = ((uint32_t*)k)[0];
    uint32_t k1 = ((uint32_t*)k)[1];
    uint32_t k2 = ((uint32_t*)k)[2];
    uint32_t k3 = ((uint32_t*)k)[3]; 
    for (i = 0; i < 32; i++) 
    {                                                     
        v1 -= ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);
        v0 -= ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
        sum -= DELTA;
    }                                                    
    ((uint32_t*)v)[0] = v0; 
    ((uint32_t*)v)[1] = v1;
}

size_t tea_read_block_padding(teablock_t* block)
{
    return (size_t)(*block & 0x00000000000000FF);
}
void tea_write_full_block_padding(teablock_t* block)
{
    *block = (teablock_t)0x0000000000000008;
}
void tea_write_block_padding(teablock_t* block, size_t padding)
{
    *block |= (teablock_t)((char)padding);
}
size_t tea_block_count_from_size(size_t size)
{
    return (size / (size_t)(sizeof(teablock_t))) + 1;
}
void tea_print_key(teakey_t k)
{
    printf("0x%lx%lx\n", k[0], k[1]);
}