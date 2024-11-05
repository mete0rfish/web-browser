#include "server.h"

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
