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

#endif