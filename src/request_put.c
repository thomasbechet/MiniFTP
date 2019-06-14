#include "request.h"

#include "tcp_connection.h"
#include "file_transfer.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

void client_put(const char* src, const char* dst, teakey_t key)
{
    int fd;
    size_t k, file_size;
    teablock_t block;
    struct stat file_stat;
    struct request request;
    struct answer answer;

    ////////////////////////////////////////////////////////////////////////////////////////

    fd = open(src, O_RDONLY, 0);
    if(fd != -1)
    {
        //Request initialization
        fstat(fd, &file_stat);
        memset(&request, 0, sizeof(struct request));
        request.kind = REQUEST_PUT;
        request.length = tea_block_count_from_size(file_stat.st_size);
        snprintf(request.path, MAX_PATH_LENGTH, "%s", dst);

        //Send request
        encrypt_request(&request, key);
        if(tcp_send(&request, sizeof(struct request)) != -1)
        {
            //Recv answer
            memset(&answer, 0, sizeof(struct answer));
            if(tcp_recv(&answer, sizeof(struct answer), &k) != -1 && k == sizeof(struct answer))
            {
                decrypt_answer(&answer, key);
                if(answer.ack == ANSWER_OK)
                {
                    if(send_file(fd, file_stat.st_size, key) != -1) //File transfer
                    {
                        //Recv confirmation
                        memset(&answer, 0, sizeof(struct answer));
                        if(tcp_recv(&answer, sizeof(struct answer), &k) != -1 && k == sizeof(struct answer))
                        {
                            decrypt_answer(&answer, key);
                            if(answer.ack == ANSWER_OK)
                            {
                                printf("File sended.\n");
                            }
                            else
                            {
                                fprintf(stderr, "Failed to send file. No ack answer\n");
                            }
                        }
                        else
                        {
                            fprintf(stderr, "No confirmation message received\n");
                        }
                    }
                    else
                    {
                        fprintf(stderr, "Failed to send file.\n");
                    }
                }
                else if(answer.ack == ANSWER_ERROR)
                {
                    fprintf(stderr, "Failed to send file to server.\n");
                }
            }
            else
            {
                fprintf(stderr, "Failed to receive answer from server.\n");
            }
        }   
        else
        {
            fprintf(stderr, "Failed to send request to the server.\n");
        }

        close(fd);
    }
    else
    {
        fprintf(stderr, "Failed to open file %s.\n", src);
    }
}
void server_put(const struct request* request, teakey_t key)
{   
    int fd;
    size_t k;
    size_t block_count;
    teablock_t block;
    struct answer answer;

    ////////////////////////////////////////////////////////////////////////////////////////

    fd = open(request->path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if(fd != -1)
    {
        memset(&answer, 0, sizeof(struct answer));
        answer.ack = ANSWER_OK;
        encrypt_answer(&answer, key);
        tcp_send(&answer, sizeof(struct answer));

        if(recv_file(fd, request->length, key) != 1) //File transfer
        {
            printf("Send ok\n");

            //No error, send confirmation to client
            memset(&answer, 0, sizeof(struct answer));
            answer.ack = ANSWER_OK;
            encrypt_answer(&answer, key);
            tcp_send(&answer, sizeof(struct answer));
        }
        else
        {
            printf("Send error\n");

            //Error, send to the client
            memset(&answer, 0, sizeof(struct answer));
            answer.ack = ANSWER_ERROR;
            answer.errnum = ERROR_RECV;
            encrypt_answer(&answer, key);
            tcp_send(&answer, sizeof(struct answer));
        }

        close(fd);
    }
    else
    {
        //Send error to client (failed to create/open file)
        memset(&answer, 0, sizeof(struct answer));
        answer.ack = ANSWER_ERROR;
        answer.errnum = ERROR_OPEN;
        tcp_send(&answer, sizeof(struct answer));
    }
}
