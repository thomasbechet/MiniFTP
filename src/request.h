#ifndef REQUEST_H
#define REQUEST_H

#include "request.h"
#include "tea.h"

#define REQUEST_PUT 0
#define REQUEST_GET 1
#define REQUEST_DIR 2

#define ANSWER_OK      0
#define ANSWER_UNKNOWN 1
#define ANSWER_ERROR   2

#define ERROR_OPEN 0
#define ERROR_RECV 1

#define MAX_PATH_LENGTH 256

struct request 
{
    int    kind;
    size_t length;        		  // pour PUT seulement (nombre de block)
    char   path[MAX_PATH_LENGTH];
};

struct answer
{
	int     ack;
	size_t  length;  			// pour GET seulement (nombre de block)
	int     errnum;   			// significatif ssi != 0 et ack == ANSWER_ERROR
	int     _pad[1];   			// aligne la taille sur un multiple de 8 octests
};

void client_put(const char* src, const char* dst, teakey_t key);
void server_put(const struct request* request, teakey_t key);

void client_get(const char* src, const char* dst, teakey_t key);
void server_get(const struct request* request, teakey_t key);

void client_dir(const char* dst, teakey_t key);
void server_dir(const struct request* request, teakey_t key);

void encrypt_request(struct request* request, teakey_t key);
void decrypt_request(struct request* request, teakey_t key);
void encrypt_answer(struct answer* answer, teakey_t key);
void decrypt_answer(struct answer* answer, teakey_t key);

#endif