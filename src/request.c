#include "request.h"

#define REQUEST_BLOCK_ITERATION (sizeof(struct request)) / (sizeof(teablock_t))
#define ANSWER_BLOCK_ITERATION (sizeof(struct answer)) / (sizeof(teablock_t))

void encrypt_request(struct request* request, teakey_t key)
{
    teablock_t* ptr;
    int i;

    ptr = (teablock_t*)request;
    for(i = 0; i < REQUEST_BLOCK_ITERATION; i++)
        tea_encrypt(&ptr[i], key);
}
void decrypt_request(struct request* request, teakey_t key)
{
    teablock_t* ptr;
    int i;

    ptr = (teablock_t*)request;
    for(i = 0; i < REQUEST_BLOCK_ITERATION; i++)
        tea_decrypt(&ptr[i], key);
}
void encrypt_answer(struct answer* answer, teakey_t key)
{
    teablock_t* ptr;
    int i;

    ptr = (teablock_t*)answer;
    for(i = 0; i < ANSWER_BLOCK_ITERATION; i++)
        tea_encrypt(&ptr[i], key);
}
void decrypt_answer(struct answer* answer, teakey_t key)
{
    teablock_t* ptr;
    int i;

    ptr = (teablock_t*)answer;
    for(i = 0; i < ANSWER_BLOCK_ITERATION; i++)
        tea_decrypt(&ptr[i], key);
}