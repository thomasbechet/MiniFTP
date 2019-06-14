#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include <stdlib.h>

int tcp_clt_connect(const char* address, const char* service);
int tcp_clt_disconnect();

int tcp_open_server();
int tcp_close_server();
int tcp_accept_connection();
int tcp_terminate_connection();

int tcp_send(void* buf, size_t size);
int tcp_recv(void* buf, size_t max, size_t* k);

#endif