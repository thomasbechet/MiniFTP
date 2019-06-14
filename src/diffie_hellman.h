#ifndef DIFFIE_HELLMAN_H
#define DIFFIE_HELLMAN_H

#include <stdint.h>

void dh_generate_key_step1(uint64_t* a, uint64_t* A);
void dh_generate_key_step2(uint64_t* key, uint64_t* a, uint64_t* B);

#endif