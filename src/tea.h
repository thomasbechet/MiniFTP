#ifndef TEA_H
#define TEA_H

#include <stdlib.h>
#include <stdint.h>

typedef uint64_t teablock_t;
typedef uint64_t teakey_t[2];

int tea_obtain_key_as_client(teakey_t* key);
int tea_obtain_key_as_server(teakey_t* key);
void tea_encrypt(teablock_t* v, teakey_t k);
void tea_decrypt(teablock_t* v, teakey_t k);

size_t tea_read_block_padding(teablock_t* block);
void tea_write_full_block_padding(teablock_t* block);
void tea_write_block_padding(teablock_t* block, size_t padding);
size_t tea_block_count_from_size(size_t size);
void tea_print_key(teakey_t k);

#endif