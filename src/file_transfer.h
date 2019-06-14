#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

#include "tea.h"

#include <stdlib.h>

int send_file(int fd, size_t filesize, teakey_t key);
int recv_file(int fd, size_t block_count, teakey_t key);

#endif