#include "server.h" // 서버 관련 헤더 파일 포함

// 소켓을 닫는 함수
void closeSocket(int socket) {
    #ifdef _WIN32
    closesocket(socket); // Windows의 경우 closesocket 사용
    #else
    close(socket); // Unix 계열의 경우 close 사용
    #endif
}

/*
 * 클라이언트로부터 데이터를 수신하는 받아 buffer에 저장
 */ 
void receiveSocket(int connection_sock) {
    int received = recv(connection_sock, buffer, sizeof(buffer) - 1, 0); // 클라이언트로부터 데이터 수신
    if (received <= 0) {
        perror("recv failed"); // 수신 실패 시 에러 출력
        closeSocket(connection_sock); // 소켓 닫기
        return;
    }
    buffer[received] = '\0'; // 버퍼를 null로 종료하여 문자열로 만듦
}

/*
 * CURL에서 호출되는 데이터 쓰기 콜백 함수
 */ 
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb; // 실제 수신 데이터 크기
    char **buffer = (char **)userp; // 사용자 포인터를 char 포인터로 변환

    // 현재 버퍼의 크기와 수신할 데이터의 크기를 확인
    size_t current_length = strlen(*buffer);
    size_t new_size = current_length + realsize + 1; // +1 for null terminator

    // 버퍼 크기 늘리기
    char *new_buffer = realloc(*buffer, new_size); // 기존 버퍼 크기 재할당
    if (!new_buffer) {
        fprintf(stderr, "Failed to reallocate memory\n"); // 재할당 실패 시 에러 출력
        return 0; // 메모리 재할당 실패 시
    }
    *buffer = new_buffer; // 새로운 버퍼 포인터 할당

    // 데이터 복사
    strncat(*buffer, contents, realsize); // 수신된 데이터를 기존 버퍼에 붙여넣기
    return realsize; // 수신된 데이터 크기 반환
}

// HTTPS 요청을 처리하는 함수
void handle_https_request(int connection_sock, const char *url) {
    CURL *curl;
    CURLcode res;
    char *response = malloc(BUFFER_SIZE);
    if (!response) {
        perror("Failed to allocate memory");
        closeSocket(connection_sock);
        return;
    }
    memset(response, 0, BUFFER_SIZE);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            const char *error_msg = "HTTP/1.1 500 Internal Server Error\r\n"
                                     "Content-Type: text/plain\r\n"
                                     "Connection: close\r\n\r\n"
                                     "Internal Server Error";
            send(connection_sock, error_msg, strlen(error_msg), 0);
        } else {
            char *content_type = NULL;
            curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type);
            if (content_type && strstr(content_type, "ISO-8859-1")) {
                // ISO-8859-1로 받은 응답을 UTF-8로 변환
                char *converted_response = convert_encoding(response, "ISO-8859-1", "UTF-8");
                if (converted_response) {
                    const char *cors_headers = "HTTP/1.1 200 OK\r\n"
                                               "Access-Control-Allow-Origin: *\r\n"
                                               "Content-Type: text/html; charset=utf-8\r\n"
                                               "Connection: close\r\n\r\n";
                    send(connection_sock, cors_headers, strlen(cors_headers), 0);
                    send(connection_sock, converted_response, strlen(converted_response), 0);
                    free(converted_response);
                } else {
                    const char *error_msg = "HTTP/1.1 500 Internal Server Error\r\n"
                                             "Content-Type: text/plain\r\n"
                                             "Connection: close\r\n\r\n"
                                             "Encoding Conversion Error";
                    send(connection_sock, error_msg, strlen(error_msg), 0);
                }
            } else {
                // 이미 UTF-8로 되어 있을 경우, 변환 없이 전송
                const char *cors_headers = "HTTP/1.1 200 OK\r\n"
                                           "Access-Control-Allow-Origin: *\r\n"
                                           "Content-Type: text/html; charset=utf-8\r\n"
                                           "Connection: close\r\n\r\n";
                send(connection_sock, cors_headers, strlen(cors_headers), 0);
                send(connection_sock, response, strlen(response), 0);
            }
        }

        curl_easy_cleanup(curl);
    }

    free(response);
    closeSocket(connection_sock);
}

// 메인 함수
int main() {
    #ifdef _WIN32
        WSADATA wsaData; // Windows 소켓 데이터
        WSAStartup(MAKEWORD(2, 2), &wsaData); // Windows 소켓 초기화
    #endif

    int server_sock = socket(AF_INET, SOCK_STREAM, 0); // TCP 소켓 생성
    struct sockaddr_in server_address; // 서버 주소 구조체
    server_address.sin_family = AF_INET; // 주소 체계를 IPv4로 설정
    server_address.sin_port = htons(PORT); // 포트 설정
    server_address.sin_addr.s_addr = INADDR_ANY; // 모든 IP 주소 수신

    // 소켓에 주소 바인딩
    if (bind(server_sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("bind failed");
        return 1;
    }

    // 클라이언트 연결 요청 listen
    if (listen(server_sock, 5) < 0) {
        perror("listen failed");
        return 1;
    }
    
    printf("Listening on port %d\n", PORT); // listen 시작 메시지 출력
    
    while (1) {
        int connection_sock = accept(server_sock, NULL, NULL); // 클라이언트 연결 수락
        if (connection_sock < 0) {
            perror("accept failed");
            continue;
        }

        receiveSocket(connection_sock); // 클라이언트로부터 데이터 수신
        
        char method[10]; // HTTP 메서드를 저장할 배열
        sscanf(buffer, "%s", method); // 요청에서 메서드 추출
        printf("Method: %s\n", method);

        /*
         * 요청 URL에 따라 적절한 핸들러 호출
         */ 
        if (strstr(buffer, SEARCH_URL) != NULL) {
            handle_search(connection_sock); // 검색 요청 처리
        } else if (strstr(buffer, "/history") != NULL) {
            handle_history(connection_sock); // 검색 기록 요청 처리
        } else {
            handle_view(connection_sock); // 뷰 요청 처리
     }
    }

    close(server_sock); // 서버 소켓 닫기
    delete_search_history();
    #ifdef _WIN32
        WSACleanup(); // Windows 소켓 정리
    #endif
    return 0; // 프로그램 종료
}