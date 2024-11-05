#include "server.h"

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