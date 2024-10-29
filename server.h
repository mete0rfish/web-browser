#ifndef SERVER_H
# define SERVER_H

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h> 
    // 그 외 운영체제의 경우
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// react(client)에서 검색요청을 하면 8080포트로 요청을 보낸다.
#define PORT 8080 
#define BUFFER_SIZE 50000
#define SEARCH_URL "/search="

char buffer[BUFFER_SIZE]; // 수신 데이터

#endif