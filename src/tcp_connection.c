#include "tcp_connection.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BACKLOG 64

int _socket_listen;
int _socket;

int sock_bind(struct addrinfo** res, struct addrinfo* s);
int sock_connect(struct addrinfo** res, struct addrinfo* s); 
int print_addrinfo(struct addrinfo s, char* buf, size_t max);
struct addrinfo get_criteria(void);

int tcp_clt_connect(const char* address, const char* service)
{
    int r;
    struct addrinfo criteria;
    struct addrinfo* res;
    struct addrinfo choosen_addr;

    memset(&_socket, 0, sizeof(int));
    memset(&_socket_listen, 0, sizeof(int));
    memset(&choosen_addr, 0, sizeof(struct addrinfo));

    criteria = get_criteria();

    r = getaddrinfo(address, service, &criteria, &res);
    if(r != 0){
        fprintf(stderr, "Failed to get addrinfo: %s.\n", gai_strerror(r));
        return -1;
    }

    printf("Try to connect to %s:%s...", address, service);
    _socket = sock_connect(&res, &choosen_addr);
    if(_socket == -1) 
    {
        printf(" failed.\n");
        return -1;
    }

    printf(" done.\n");

    char addrinfo_string[100];
    print_addrinfo(choosen_addr, addrinfo_string, 100);
    printf("Connected to: %s.\n", addrinfo_string);

    freeaddrinfo(res);

    return 0;
}
int tcp_clt_disconnect() 
{
    close(_socket);
    memset(&_socket, 0, sizeof(int));

    printf("Disconnected from server.\n");

    return 0;
}

int tcp_open_server(const char* service)
{
    int r;
    struct addrinfo criteria;
    struct addrinfo* res;
    struct addrinfo choosen_addr;

    memset(&_socket_listen, 0, sizeof(int));
    memset(&_socket, 0, sizeof(int));
    memset(&choosen_addr, 0, sizeof(struct addrinfo));

    criteria = get_criteria();
    r = getaddrinfo(NULL, service, &criteria, &res);
    if(r != 0)
    {
        fprintf(stderr, "Failed to get addrinfo: %s.\n", gai_strerror(r));
        return -1;
    }

    _socket_listen = sock_bind(&res, &choosen_addr);
    if(_socket_listen == -1)
    {
        fprintf(stderr, "Failed to create socket.\n");
        return -1;
    }

    if(listen(_socket_listen, BACKLOG) == -1)
    {
        fprintf(stderr, "Failed to listen.\n");
        return -1;
    }

    char addrinfo_string[100];
    print_addrinfo(choosen_addr, addrinfo_string, 100);
    printf("Server bound to %s.\n", addrinfo_string);

    freeaddrinfo(res);

    return 0;
}
int tcp_close_server()
{
    close(_socket_listen);

    memset(&_socket_listen, 0, sizeof(int));

    return 0;
}
int tcp_accept_connection()
{
    socklen_t clt_l;
    struct sockaddr_storage clt_storage;

    memset(&_socket, 0, sizeof(int));

    _socket = accept(_socket_listen, (struct sockaddr*)&clt_storage, &clt_l);
    if(_socket == -1)
    {
        fprintf(stderr, "Failed to accept client.\n");
        return -1;
    }

    return 0;
}
int tcp_terminate_connection()
{
    close(_socket);
    memset(&_socket, 0, sizeof(int));

    return 0;
}

int tcp_send(void* buf, size_t size)
{
    return send(_socket, buf, size, 0);
}
int tcp_recv(void* buf, size_t max, size_t* k)
{
    if((*k = recv(_socket, buf, max, 0)) != -1)
        return 0;
    else
        return -1;
}

int sock_bind(struct addrinfo** res, struct addrinfo* s)
{
    struct addrinfo* resp;
    int sfd;

    resp = *res;
    while(resp != NULL){
        if((sfd = socket(resp->ai_family, resp->ai_socktype, resp->ai_protocol)) != -1){
            if((bind(sfd, resp->ai_addr, resp->ai_addrlen)) != -1){
                *s = *resp;
                return sfd;
            }
        }
        resp = resp->ai_next;
    }

    return -1;
}
int sock_connect(struct addrinfo** res, struct addrinfo* s) 
{
    int sfd;
    struct addrinfo* resp = *res;
    while(resp != NULL) 
    {
        if((sfd = socket(resp->ai_family, resp->ai_socktype, resp->ai_protocol)) != -1)
        {
            if((connect(sfd, (struct sockaddr*)resp->ai_addr, resp->ai_addrlen)) != -1) 
            {    
                *s = *resp;
                return sfd;
            }
        }

        resp = resp->ai_next;
    }

    return -1;
}
int print_addrinfo(struct addrinfo s, char* buf, size_t max)
{
    int r;
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

    r = getnameinfo(s.ai_addr, s.ai_addrlen, host, NI_MAXHOST,
        service, NI_MAXSERV, NI_NUMERICHOST);
    if(r)
    {
        fprintf(stderr, "Failed to get addrinfo: %s\n", gai_strerror(r));
        return r;
    }
    
    snprintf(buf, max, "%s.%s canonname: %s", host, service, s.ai_canonname);

    return 0;
}
struct addrinfo get_criteria(void)
{
    struct addrinfo criteria;

    memset(&criteria, 0, sizeof(struct addrinfo));
    criteria.ai_family = AF_INET;
    criteria.ai_socktype = SOCK_STREAM;
    //criteria.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
    criteria.ai_flags = AI_NUMERICSERV;
    criteria.ai_protocol = 0;
    criteria.ai_addr = NULL;
    criteria.ai_next = NULL;
    criteria.ai_canonname = NULL;

    return criteria;
}