#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

#include "tea.h"
#include "tcp_connection.h"
#include "diffie_hellman.h"
#include "request.h"

#define COMMAND_HELP "help"
#define COMMAND_QUIT "quit"
#define COMMAND_EXIT "exit"
#define COMMAND_GET "get"
#define COMMAND_PUT "put"
#define COMMAND_DIR "ls"

void handler(int command)
{
    tcp_terminate_connection();
    exit(0);
}

void print_help(void)
{
    printf("help            : display all commands.\n");
    printf("quit            : close the connection and leave.\n");
    printf("exit            : close the connection and leave.\n");
    printf("get <src> <dst> : Retrieve a file from server.\n");
    printf("put <src> <dst> : Transfer a file to the server.\n");
    printf("ls  <dir>       : Display a distant directory.\n");
}

void client_start(void)
{
    teakey_t key;
    char command[256];
    char* args[10];
    int end, r;

    /////////////////////////////////

    tea_obtain_key_as_client(&key);

    end = 0;
    do
    {
        printf(">");
        if(fgets(command, 256, stdin) != NULL && command[0] != '\n')
        {
            command[strcspn(command, "\n")] = 0;
            r = 0;
            char* p = strtok(command, " ");
            while(p != NULL)
            {
                args[r++] = p;
                p = strtok(NULL, " ");
            }

            if(strcmp(args[0], COMMAND_HELP) == 0) 
            {
                if(r == 1) print_help();
            }
            else if(strcmp(args[0], COMMAND_QUIT) == 0)
            {
                if(r == 1) end = 1;
            }
            else if(strcmp(args[0], COMMAND_EXIT) == 0)
            {
                if(r == 1) end = 1;
            }
            else if(strcmp(args[0], COMMAND_PUT) == 0)
            {
                if(r != 3)
                    printf("%s <source> <destination>\n", COMMAND_PUT);
                else
                    client_put(args[1], args[2], key);
            }
            else if(strcmp(args[0], COMMAND_GET) == 0)
            {
                if(r != 3)
                    printf("%s <source> <destination>\n", COMMAND_GET);
                else
                    client_get(args[1], args[2], key);
            }
            else if(strcmp(args[0], COMMAND_DIR) == 0)
            {
                if(r == 2)
                    client_dir(args[1], key);
                else if(r == 1)
                    client_dir(".", key);
                else
                    printf("%s <dir>\n", COMMAND_DIR);
            }
            else
            {
                printf("Unknown command \"%s\"\n", command);
            }
        }
    }
    while(!end);
}

int main(int argc, char* argv[])
{
    if(argc < 3)
    {
        fprintf(stderr, "%s <address> <service>\n", argv[0]);
        exit(-1);
    }

    signal(SIGINT, handler);

    if(tcp_clt_connect(argv[1], argv[2]) != -1)
    {
        client_start();

        tcp_clt_disconnect();
    }

    return EXIT_SUCCESS;
}