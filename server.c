#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
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

#define PORT 8080
#define BUFFER_SIZE 50000

void handle_request(int client_sock) {
    char buffer[BUFFER_SIZE];
    int received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
        perror("recv failed");
#ifdef _WIN32
        closesocket(client_sock);
#else
        close(client_sock);
#endif
        return;
    }

    buffer[received] = '\0'; // null-terminate

    // Extract the search term from the request
    char *search_query = strstr(buffer, "search=");
    if (search_query) {
        search_query += strlen("search="); // Move past "search="
        char *end = strchr(search_query, ' ');
        if (end) *end = '\0'; // null-terminate the query

        // Prepare the request to Google
        char google_request[BUFFER_SIZE];
        snprintf(google_request, sizeof(google_request), 
                 "GET /search?q=%s HTTP/1.1\r\nHost: www.google.com\r\nConnection: close\r\n\r\n", 
                 search_query);
        
        // Create socket to connect to Google
        int google_sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in google_address;
        struct hostent *host = gethostbyname("www.google.com");
        
        google_address.sin_family = AF_INET;
        google_address.sin_port = htons(80);
        memcpy(&google_address.sin_addr.s_addr, host->h_addr, host->h_length);

        // Connect and send request to Google
        connect(google_sock, (struct sockaddr*)&google_address, sizeof(google_address));
        send(google_sock, google_request, strlen(google_request), 0);
        
        // Receive response from Google
        char response[BUFFER_SIZE];
        int total_received = 0;
        int rcvd_bytes;

        while ((rcvd_bytes = recv(google_sock, response + total_received, sizeof(response) - total_received - 1, 0)) > 0) {
            total_received += rcvd_bytes;
        }
        response[total_received] = '\0'; // null-terminate response

        // Add CORS headers
        const char *cors_headers = "HTTP/1.1 200 OK\r\n"
                                   "Access-Control-Allow-Origin: *\r\n"  // Allow all origins
                                   "Content-Type: text/html\r\n"        // Set content type to HTML
                                   "Connection: close\r\n\r\n";

        // Send CORS headers and response back to client
        send(client_sock, cors_headers, strlen(cors_headers), 0);
        send(client_sock, response, total_received, 0);
        
#ifdef _WIN32
        closesocket(google_sock); // 윈도우에서는 closesocket을 사용
#else
        close(google_sock); // POSIX에서는 close를 사용
#endif
    }

#ifdef _WIN32
    closesocket(client_sock); // 윈도우에서는 closesocket을 사용
#else
    close(client_sock); // POSIX에서는 close를 사용
#endif
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData); // 윈도우에서 소켓 초기화
#endif

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr*)&server_address, sizeof(server_address));
    listen(server_sock, 5);
    
    printf("Listening on port %d\n", PORT);
    
    while (1) {
        int client_sock = accept(server_sock, NULL, NULL);
        handle_request(client_sock);
    }

    close(server_sock); // POSIX에서는 close를 사용
#ifdef _WIN32
    WSACleanup(); // 윈도우에서 소켓 종료
#endif
    return 0;
}
