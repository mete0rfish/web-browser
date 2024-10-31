#include "server.h"

void closeSocket(int socket) {
    #ifdef _WIN32
    closesocket(socket);
    #else
    close(socket);
    #endif
}

void receiveSocket(int connection_sock) {
    int received = recv(connection_sock, buffer, sizeof(buffer) - 1, 0); // 클라이언트로부터 데이터를 수신. 수신된 값은 received에 저장됨.

    if (received <= 0) { // 수신 여부 체크해서 실패하면 소켓 close
        perror("recv failed");
        closeSocket(connection_sock);
        return;
    }

    buffer[received] = '\0';
}

void handle_view(int connection_sock) {
    // 수신된 buffer에서 문자열 "view="를 탐색 후 위치 반환
    char *url_query = strstr(buffer, "view=");

    if (url_query) {
        /*
         전처리
         */
        url_query += strlen("view=");  // "view="문자열의 끝으로 포인터 이동
        char *end = strchr(url_query, ' ');  // 공백문자 위치 반환
        if (end) *end = '\0';

        // URL 쿼리 파라미터를 기반으로 HTTP 요청 작성
        char web_request[BUFFER_SIZE];
        snprintf(web_request, sizeof(web_request), 
                 "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", 
                 url_query);

        /*
         * 원격 서버에 REQUEST
         */
        int web_sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in web_address;
        struct hostent *host = gethostbyname(url_query);

        if (!host) {
            perror("Host not found");
            closeSocket(connection_sock);
            return;
        }

        web_address.sin_family = AF_INET;
        web_address.sin_port = htons(80);
        memcpy(&web_address.sin_addr.s_addr, host->h_addr, host->h_length);

        if (connect(web_sock, (struct sockaddr*)&web_address, sizeof(web_address)) < 0) {
            perror("Connection failed");
            closeSocket(web_sock);
            closeSocket(connection_sock);
            return;
        }

        if (send(web_sock, web_request, strlen(web_request), 0) < 0) {
            perror("Send failed");
            closeSocket(web_sock);
            closeSocket(connection_sock);
            return;
        }

        /*
         * 원격 서버의 RESPONSE
         * 동적 메모리 할당을 통해 응답 데이터를 받기 위한 배열을 준비
         */
        int response_size = BUFFER_SIZE;
        char *response = (char *)malloc(response_size);
        int total_received = 0;
        int rcvd_bytes;

        while ((rcvd_bytes = recv(web_sock, response + total_received, response_size - total_received - 1, 0)) > 0) {
            total_received += rcvd_bytes;

            // 메모리 크기가 초과되면 realloc으로 크기를 두 배로 늘림
            if (total_received >= response_size - 1) {
                response_size *= 2;
                response = (char *)realloc(response, response_size);
                if (!response) {
                    perror("Memory allocation failed");
                    closeSocket(web_sock);
                    closeSocket(connection_sock);
                    return;
                }
            }
        }
        response[total_received] = '\0';  // null 문자로 응답 데이터 종료

        /***** CORS 헤더를 추가하여 클라이언트에서의 CORS 에러 해결 *****/
        const char *cors_headers = "HTTP/1.1 200 OK\r\n"
                                   "Access-Control-Allow-Origin: *\r\n"
                                   "Content-Type: text/html\r\n"
                                   "Connection: close\r\n\r\n";

        send(connection_sock, cors_headers, strlen(cors_headers), 0);
        send(connection_sock, response, total_received, 0);

        free(response);
        closeSocket(web_sock);
    }
    closeSocket(connection_sock);
}

// client 요청 처리 함수
void handle_search(int connection_sock) {

    // 수신된 buffer에서 문자열 "search="를 탐색 후 위치 반환
    char *search_query = strstr(buffer, "search=");

    if (search_query) {
        /*
         전처리 
         */ 
        search_query += strlen("search="); // "search="문자열의 끝으로 포인터 이동
        char *end = strchr(search_query, ' '); // 공백문자 위치 반환
        if (end) *end = '\0';

        // search_query를 URL 쿼리 파라미터로 사용하여 검색 요청을 형성.
        char google_request[BUFFER_SIZE];
        snprintf(google_request, sizeof(google_request), 
                 "GET /search?q=%s HTTP/1.1\r\nHost: www.google.com\r\nConnection: close\r\n\r\n", 
                 search_query);

        /*
         * 구글에게 REQUEST 
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
         * 구글의 RESPONSE
         * 동적 메모리 할당을 통해 응답 데이터를 받기 위한 배열을 준비
         */
        int response_size = BUFFER_SIZE; // 초기 크기
        char *response = (char *)malloc(response_size); // 동적 메모리 할당
        int total_received = 0; // 총 수신된 바이트 수 저장 변수
        int rcvd_bytes; // 각 recv 호출 시 수신된 바이트 수 저장 변수

        while ((rcvd_bytes = recv(google_sock, response + total_received, response_size - total_received - 1, 0)) > 0) {
            total_received += rcvd_bytes;

            // 메모리 크기가 초과되면 realloc으로 크기를 두 배로 늘림
            if (total_received >= response_size - 1) {
                response_size *= 2;
                response = (char *)realloc(response, response_size);
                if (!response) { // 메모리 할당 실패 시
                    perror("Memory allocation failed");
                    closeSocket(google_sock);
                    closeSocket(connection_sock);
                    return;
                }
            }
        }
        response[total_received] = '\0'; // null 문자로 응답 데이터 종료

        /***** react(client)에서 포트5173을 사용하고, 지금 이 서버코드는 8080포트를 사용하기 때문에 cors에러 해결 *****/
        const char *cors_headers = "HTTP/1.1 200 OK\r\n"
                                   "Access-Control-Allow-Origin: *\r\n"
                                   "Content-Type: text/html\r\n"
                                   "Connection: close\r\n\r\n";

        send(connection_sock, cors_headers, strlen(cors_headers), 0); // CORS 헤더 전송
        send(connection_sock, response, total_received, 0); // 구글 응답 데이터 전송

        free(response); // 동적 메모리 해제
        closeSocket(google_sock); // 구글 소켓 닫기
    }
    closeSocket(connection_sock); // 클라이언트 소켓 닫기
}

int main() {
    #ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData); // 윈도우에서 소켓 초기화
    #endif

    int server_sock = socket(AF_INET, SOCK_STREAM, 0); // 서버소켓 생성
    struct sockaddr_in server_address; // 서버 주소 구조체
    server_address.sin_family = AF_INET; // IPv4
    server_address.sin_port = htons(PORT); // 8080포트 사용
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr*)&server_address, sizeof(server_address)); // 소켓에 주소 바인딩
    listen(server_sock, 5); // 클라이언트 요청 대기(소켓이 동시에 대기할 수 있는 연결 요청의 최대 수)
    
    printf("Listening on port %d\n", PORT);
    
    while (1) {
        // 서버는 계속해서 클라이언트의 요청을 대기한다.
        int connection_sock = accept(server_sock, NULL, NULL); // 클라이언트의 연결요청 수락
        char method[10], url[256];

        receiveSocket(connection_sock);

        sscanf(buffer, "%s", method);
        sscanf(buffer + strlen(method) + 1, "%s", url);
        printf("%s %s\n", method, url);

        if(strstr(buffer, SEARCH_URL) != NULL) {
            handle_search(connection_sock); // 요청 처리함수(위에 정의된 함수)
        }
        else {
            // TODO 웹 페이지 띄우기
            handle_view(connection_sock);
        }
    }

    close(server_sock); // POSIX에서는 close를 사용
    #ifdef _WIN32
        WSACleanup(); // 윈도우에서 소켓 종료
    #endif
    return 0;
}
