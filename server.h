#ifndef SERVER_H
#define SERVER_H

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
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
#include <curl/curl.h>
#include <iconv.h>

#define PORT 8080
#define BUFFER_SIZE 50000
#define VIEW_URL "/view="
#define SEARCH_URL "/search="

char buffer[BUFFER_SIZE]; // 수신 데이터

#endif // SERVER_H
