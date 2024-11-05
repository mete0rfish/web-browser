#include "server.h"

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

/*
 * Google 응답으로 charset=ISO-8859-1 을 주기 때문에 UTF-8형식으로 변환
 */ 
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