#ifndef UTILITY_H
#define UTILITY_H

#include <stdint.h>

int is_little_endian(void);
uint64_t swap_uint64(uint64_t val);

#endif