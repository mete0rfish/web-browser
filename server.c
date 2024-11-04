#include "server.h" // 서버 관련 헤더 파일 포함

// 소켓을 닫는 함수
void closeSocket(int socket) {
    #ifdef _WIN32
    closesocket(socket); // Windows의 경우 closesocket 사용
    #else
    close(socket); // Unix 계열의 경우 close 사용
    #endif
}

//파일에다 검색기록 로그 남기기
void log_search_query(const char *query) {
    FILE *file = fopen("search_history", "a"); // 파일을 추가 모드로 엽니다.
    if (file == NULL) {
        perror("파일 열기 실패");
        return;
    }
    fprintf(file, "%s\n", query); // 쿼리를 파일에 작성합니다.
    fclose(file); // 파일을 닫습니다.
}

// 클라이언트로부터 데이터를 수신하는 함수
void receiveSocket(int connection_sock) {
    int received = recv(connection_sock, buffer, sizeof(buffer) - 1, 0); // 클라이언트로부터 데이터 수신
    if (received <= 0) {
        perror("recv failed"); // 수신 실패 시 에러 출력
        closeSocket(connection_sock); // 소켓 닫기
        return;
    }
    buffer[received] = '\0'; // 버퍼를 null로 종료하여 문자열로 만듦
}

// CURL에서 호출되는 데이터 쓰기 콜백 함수
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

// Google 응답으로 charset=ISO-8859-1 을 주기 때문에 UTF-8형식으로 변환
char* convert_encoding(const char* input, const char* from_enc, const char* to_enc) {
    iconv_t conv = iconv_open(to_enc, from_enc);
    if (conv == (iconv_t)-1) {
        perror("iconv_open");
        return NULL;
    }

    size_t in_bytes = strlen(input);
    size_t out_bytes = in_bytes * 4; // UTF-8로 변환할 경우 더 큰 버퍼 필요
    char* output = malloc(out_bytes);
    if (!output) {
        perror("malloc");
        iconv_close(conv);
        return NULL;
    }

    char* in_buf = (char*)input;
    char* out_buf = output;
    memset(out_buf, 0, out_bytes); // 출력 버퍼 초기화

    size_t result = iconv(conv, &in_buf, &in_bytes, &out_buf, &out_bytes);
    if (result == (size_t)-1) {
        perror("iconv");
        free(output);
        output = NULL;
    }

    iconv_close(conv);
    return output;
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

// "/view=" 요청을 처리하는 함수
void handle_view(int connection_sock) {
    char *url_query = strstr(buffer, VIEW_URL); // 버퍼에서 VIEW_URL 찾기
    if (url_query) {
        url_query += strlen(VIEW_URL); // URL 쿼리 시작점 설정
        char *end = strchr(url_query, ' '); // 공백 문자 찾기
        if (end) *end = '\0'; // 공백 문자로 문자열 종료

        // HTTPS 요청 처리
        char url[BUFFER_SIZE];
        snprintf(url, sizeof(url), "https://%s", url_query); // HTTPS URL 생성
        handle_https_request(connection_sock, url); // HTTPS 요청 처리 함수 호출
    } else {
        const char *response = "HTTP/1.1 400 Bad Request\r\n"
                               "Content-Type: text/plain\r\n"
                               "Connection: close\r\n\r\n"
                               "Bad Request"; // 잘못된 요청 응답 메시지
        send(connection_sock, response, strlen(response), 0); // 클라이언트에 응답 전송
        closeSocket(connection_sock); // 소켓 닫기
    }
}

// "/search=" 요청을 처리하는 함수
void handle_search(int connection_sock) {
    char *search_query = strstr(buffer, SEARCH_URL); // 버퍼에서 SEARCH_URL 찾기
    if (search_query) {
        search_query += strlen(SEARCH_URL); // 검색 쿼리 시작점 설정
        char *end = strchr(search_query, ' '); // 공백 문자 찾기
        if (end) *end = '\0'; // 공백 문자로 문자열 종료

        // 검색 기록을 로그에 남기기
        log_search_query(search_query); // 검색 쿼리를 파일에 기록

        // HTTPS 요청 처리
        char url[BUFFER_SIZE];
        snprintf(url, sizeof(url), "https://www.google.com/search?q=%s", search_query); // Google 검색 URL 생성
        handle_https_request(connection_sock, url); // HTTPS 요청 처리 함수 호출
    } else {
        const char *response = "HTTP/1.1 400 Bad Request\r\n"
                               "Content-Type: text/plain\r\n"
                               "Connection: close\r\n\r\n"
                               "Bad Request"; // 잘못된 요청 응답 메시지
        send(connection_sock, response, strlen(response), 0); // 클라이언트에 응답 전송
        closeSocket(connection_sock); // 소켓 닫기
    }
}

//검색기록 확인하는 함수
void handle_history(int connection_sock) {
    FILE *file = fopen("search_history", "r"); // 파일을 읽기 모드로 엽니다.
    if (file == NULL) {
        perror("파일 열기 실패");
        const char *error_msg = "HTTP/1.1 500 Internal Server Error\r\n"
                                "Content-Type: text/plain\r\n"
                                "Connection: close\r\n\r\n"
                                "Internal Server Error";
        send(connection_sock, error_msg, strlen(error_msg), 0);
        closeSocket(connection_sock);
        return;
    }

    // 파일 내용을 읽어와서 클라이언트에 전송
    const char *header = "HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/plain; charset=utf-8\r\n"
                         "Connection: close\r\n\r\n";
    send(connection_sock, header, strlen(header), 0);

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        send(connection_sock, line, strlen(line), 0); // 각 줄을 클라이언트에 전송
    }

    fclose(file); // 파일 닫기
    closeSocket(connection_sock); // 소켓 닫기
}

//프로그램 종료시 검색기록이 존재하는 텍스트 파일 내용 삭제
void delete_search_history() {
    if (remove("search_history") == 0) {
        printf("검색 기록 파일이 삭제되었습니다.\n");
    } else {
        perror("파일 삭제 실패");
    }
}


// 메인 함수
int main() {
    #ifdef _WIN32
        WSADATA wsaData; // Windows 소켓 데이터
        WSAStartup(MAKEWORD(2, 2), &wsaData); // Windows 소켓 초기화
    #endif

    int server_sock = socket(AF_INET, SOCK_STREAM, 0); // TCP 소켓 생성
    struct sockaddr_in server_address; // 서버 주소 구조체
    server_address.sin_family = AF_INET; // IPv4
    server_address.sin_port = htons(PORT); // 포트 설정
    server_address.sin_addr.s_addr = INADDR_ANY; // 모든 IP 주소 수신

    // 소켓에 주소 바인딩
    if (bind(server_sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("bind failed"); // 바인딩 실패 시 에러 출력
        return 1;
    }

    // 클라이언트 연결 요청 리슨
    if (listen(server_sock, 5) < 0) {
        perror("listen failed"); // 리슨 실패 시 에러 출력
        return 1;
    }
    
    printf("Listening on port %d\n", PORT); // 리슨 시작 메시지 출력
    
    while (1) { // 무한 루프
        int connection_sock = accept(server_sock, NULL, NULL); // 클라이언트 연결 수락
        if (connection_sock < 0) {
            perror("accept failed"); // 연결 수락 실패 시 에러 출력
            continue; // 다음 요청으로 진행
        }

        receiveSocket(connection_sock); // 클라이언트로부터 데이터 수신
        
        char method[10]; // HTTP 메서드를 저장할 배열
        sscanf(buffer, "%s", method); // 요청에서 메서드 추출
        printf("Method: %s\n", method); // 메서드 출력

        // 요청 URL에 따라 적절한 핸들러 호출
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