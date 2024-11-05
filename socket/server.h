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

void closeSocket(int socket);
void log_search_query(const char *query);
void receiveSocket(int connection_sock);
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
char* convert_encoding(const char* input, const char* from_enc, const char* to_enc);
void handle_https_request(int connection_sock, const char *url);
void handle_view(int connection_sock);
void handle_search(int connection_sock);
void handle_history(int connection_sock);
void delete_search_history();

#endif // SERVER_H
