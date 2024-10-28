#include "server.h"

// react(client)에서 검색요청을 하면 8080포트로 요청을 보낸다.
#define PORT 8080 
#define BUFFER_SIZE 50000

void closeSocket(int socket) {
    #ifdef _WIN32
    closesocket(socket);
    #else
    close(socket);
    #endif
}

// client 요청 처리 함수
void search_handler(int connection_sock) {

    char buffer[BUFFER_SIZE]; // 수신 데이터
    int received = recv(connection_sock, buffer, sizeof(buffer) - 1, 0); // 클라이언트로부터 데이터를 수신. 수신된 값은 received에 저장됨.
    
    if (received <= 0) { // 수신 여부 체크해서 실패하면 소켓 close
        perror("recv failed");
        closeSocket(connection_sock);
        return;
    }

    buffer[received] = '\0';

    char *search_query = strstr(buffer, "search="); // 수신된 buffer에서 문자열 "search="를 탐색 후 위치 반환

    if (search_query) {
        /*
         전처리 
         */ 
        search_query += strlen("search="); // "search="문자열의 끝으로 포인터 이동
        char *end = strchr(search_query, ' '); // 공백문자 위치 반환
        if (end) *end = '\0';

        char google_request[BUFFER_SIZE];
        snprintf(google_request, sizeof(google_request), 
                 "GET /search?q=%s HTTP/1.1\r\nHost: www.google.com\r\nConnection: close\r\n\r\n", 
                 search_query); 
        // search_query를 URL 쿼리 파라미터로 사용하여 검색 요청을 형성.
        
        /*
         *구글에게 REQUEST 
         */ 
        int google_sock = socket(AF_INET, SOCK_STREAM, 0); // 구글에 연결할 소켓 생성(IPv4), SOCK_STREAM : TCP사용하겠다.
        struct sockaddr_in google_address; // google 주소 구조체
        struct hostent *host = gethostbyname("www.google.com"); // 도메인이름 => ip주소로 변환
        
        google_address.sin_family = AF_INET;
        google_address.sin_port = htons(80);
        memcpy(&google_address.sin_addr.s_addr, host->h_addr, host->h_length);

        connect(google_sock, (struct sockaddr*)&google_address, sizeof(google_address));
        send(google_sock, google_request, strlen(google_request), 0);
        
        /*
         *  구글의 RESPONSE
         */
        char response[BUFFER_SIZE];
        int total_received = 0;
        int rcvd_bytes;

        while ((rcvd_bytes = recv(google_sock, response + total_received, sizeof(response) - total_received - 1, 0)) > 0) {
            total_received += rcvd_bytes;
        }
        response[total_received] = '\0';

        /***** react(client)에서 포트5173을 사용하고, 지금 이 서버코드는 8080포트를 사용하기 때문에 cors에러 해결 *****/
        const char *cors_headers = "HTTP/1.1 200 OK\r\n"
                                   "Access-Control-Allow-Origin: *\r\n"
                                   "Content-Type: text/html\r\n"
                                   "Connection: close\r\n\r\n";

        send(connection_sock, cors_headers, strlen(cors_headers), 0);
        send(connection_sock, response, total_received, 0);

        closeSocket(google_sock);
    }
    closeSocket(connection_sock);
}

int main() {
    #ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData); // 윈도우에서 소켓 초기화
    #endif

    int server_sock = socket(AF_INET, SOCK_STREAM, 0); // 서버소켓 생성
    struct sockaddr_in server_address; // 서버 구조 구조체
    server_address.sin_family = AF_INET; // IPv4
    server_address.sin_port = htons(PORT); // 8080포트 사용
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr*)&server_address, sizeof(server_address)); // 소켓에 주소 바인딩
    listen(server_sock, 5); // 클라이언트 요청 대기(소켓이 동시에 대기할 수 있는 연결 요청의 최대 수)
    
    printf("Listening on port %d\n", PORT);
    
    while (1) {
        // 서버는 계속해서 클라이언트의 요청을 대기한다.
        int connection_sock = accept(server_sock, NULL, NULL); // 클라이언트의 연결요청 수락
        search_handler(connection_sock); // 요청 처리함수(위에 정의된 함수)
    }

    close(server_sock); // POSIX에서는 close를 사용
    #ifdef _WIN32
        WSACleanup(); // 윈도우에서 소켓 종료
    #endif
    return 0;
}
