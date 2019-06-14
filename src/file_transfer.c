#include "file_transfer.h"

#include "utility.h"
#include "tcp_connection.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int send_file(int fd, size_t filesize, teakey_t key)
{
    int i;
    size_t k, blockcount, last_block_size;
    teablock_t block;

    /////////////////////////////

    blockcount = tea_block_count_from_size(filesize);
    last_block_size = filesize - ((blockcount - 1) * (size_t)(sizeof(teablock_t))); 

    for(i = 0; i < blockcount; i++)
    {
        memset(&block, 0, sizeof(teablock_t));

        if(i == blockcount - 1) //Last block
        {
            if(last_block_size > 0) //Has padding
            {
                k = read(fd, &block, last_block_size);
                if(k != last_block_size)
                {
                    fprintf(stderr, "Failed to read last block.\n");
                    return -1;
                }

                if(is_little_endian()) block = swap_uint64(block);

                tea_write_block_padding(&block, (size_t)(sizeof(teablock_t) - last_block_size));
            }
            else
            {
                tea_write_full_block_padding(&block);
            }
        }
        else
        {
            k = read(fd, &block, sizeof(teablock_t));
            if(k != sizeof(teablock_t))
            {
                fprintf(stderr, "Failed to read block %u.\n", i);
                return -1;
            }

            if(is_little_endian()) block = swap_uint64(block);
        }

        tea_encrypt(&block, key);

        if(tcp_send(&block, sizeof(teablock_t)) == -1)
        {
            fprintf(stderr, "Failed to send block to server.\n");
            return -1;
        } 
    }

    return 0;
}

int recv_file(int fd, size_t block_count, teakey_t key)
{
    int i;
    size_t k, last_block_size;
    teablock_t block;

    //////////////////////

    for(i = 0; i < block_count; i++)
    {
        memset(&block, 0, sizeof(teablock_t));
        if(tcp_recv(&block, sizeof(teablock_t), &k) != -1 && k == sizeof(teablock_t))
        {
            tea_decrypt(&block, key);

            if(i == block_count - 1) //Last block
            {
                last_block_size = (size_t)(sizeof(teablock_t) - tea_read_block_padding(&block));

                if(is_little_endian()) block = swap_uint64(block);
                k = write(fd, &block, last_block_size);
                if(k == -1)
                {
                    fprintf(stderr, "Failed to write last block to file.\n");
                    return -1;
                }
            }
            else
            {
                if(is_little_endian()) block = swap_uint64(block);
                k = write(fd, &block, sizeof(teablock_t));
                if(k == -1)
                {
                    fprintf(stderr, "Failed to write to file.\n");
                    return -1;
                }
            }
        }
        else
        {
            fprintf(stderr, "Failed to recieve block.\n");
            return -1;
        }
    }

    return 0;
}