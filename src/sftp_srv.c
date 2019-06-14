#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

#include "tea.h"
#include "tcp_connection.h"
#include "diffie_hellman.h"
#include "request.h"

void handler(int command)
{
    tcp_close_server();
    tcp_terminate_connection();
    exit(0);
}

void server_start(void)
{
    teakey_t key;
    size_t k;
    struct request request;
    struct answer answer;
    int end;

    //////////////////////////////////

    end = 0;

    tea_obtain_key_as_server(&key);

    while(!end)
    {
        memset(&request, 0, sizeof(struct request));
        if(tcp_recv(&request, sizeof(struct request), &k) != -1 && k == sizeof(struct request))
        {
            decrypt_request(&request, key);

            if(request.kind == REQUEST_PUT)
            {
                server_put(&request, key);
            }   
            else if(request.kind == REQUEST_GET)
            {
                server_get(&request, key);   
            }
            else if(request.kind == REQUEST_DIR)
            {
                server_dir(&request, key);
            }
            else
            {
                //Send unknown
            }
        }
        else
        {
            fprintf(stderr, "Connection closed.\n");
            end = 1;
        }
    }
}

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        fprintf(stderr, "%s <service>\n", argv[0]);
        exit(-1);
    }

    signal(SIGCHLD, SIG_IGN); //Ignore defunct childs
    signal(SIGINT, handler);

    if(tcp_open_server(argv[1]) != -1)
    {
        while(1)
        {
            if(tcp_accept_connection() != -1)
            {
                printf("New connection.\n");
                if(!fork()) break;
                tcp_terminate_connection(); //Parent
            }
            else
            {
                printf("Connection failed.\n");
            }
        }
        tcp_close_server(); //Child only

        ///////////////////////////////////////////

        server_start();
    }

    return EXIT_SUCCESS;
}