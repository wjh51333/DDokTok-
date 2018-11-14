/* chat_client.c
 *   기능 : 서버에 접속한 후 키보드의 입력을 서버에 전달하고,
 *         서버로부터 오는 메시지를 화면에 출력한다.
 * 컴파일 : cc -o chat_client chat_client.c readline.c -lsocket -lnsl
 * 실행예 : chat_client 203.252.65.3 4001 사용자_ID
 */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/utime.h>

#define MAXLINE 1024
#define MAX_SOCK 512

char *escape = "Exit\n";

int s;    /* 서버와 연결된 소켓번호 */
typedef struct {
	char n[20];    /* 대화방에서 사용하는 이름 */
	int len;       /* 이름 크기 */
} name;

int main(int argc, char *argv[])
{
	char line[MAXLINE], sendLine[MAXLINE+1];
	char recvLine[MAXLINE];
	int n, pid, size;
	int nfds;
	name u_name;
	struct sockaddr_in server_addr;
	fd_set read_fds;

	if (argc < 4) {
		printf("실행방법 : %s 호스트 IP주소 호트번호 사용자이름 \n", argv[0]);
		return -1;
	}

	/* 채팅 참가자 이름 구조체 초기화 */
	sprintf(u_name.n, "[%s]", argv[3]);
	u_name.len = strlen(u_name.n);

	/* 소켓 생성 */
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Client : Cant' open stream socket.\n");
		return -1;
	}

	/* 채팅 서버의 소켓주소 구조체 server_addr 초기화 */
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));

	/* 연결 요청 */
	if (connect(s, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("Client : Can't connect to server.\n");
		return -1;
	}
	else
		printf("접속에 성공했습니다.\n");

	nfds = s + 1;
	FD_ZERO(&read_fds);

	while (1) {
		// select() 호출
		FD_SET(0, &read_fds);
		FD_SET(s, &read_fds);
		
		if (select(nfds, &read_fds, (fd_set*)0, (fd_set*)0, (struct timeval*)0) < 0) {
			printf("select error\n");
			return -1;
		}

		// 서버로부터 수신한 메시지 처리
		if (FD_ISSET(s, &read_fds)) {
			if ((size = recv(s, recvline, MAXLINE, 0)) > 0) {
				recvLine[size] = '\0';
				printf("%s \n", recvLine);
			}
		}

		// 키보드 입력 처리
		if (FD_ISSET(0, &read_fds)) {
			if (readline(0, sendLine, MAXLINE) > 0) {
				size = strlen(sendLine);
				sprintf(line, "%s %s", u_name.n, sendLine);

				if (send(s, line, size + u_name.len, 0) != (size + u_name.len))
					printf("Error : Written error on socket.\n");

				if (size == 5 && strncmp(sendLine, escape, 5) == 0) {
					printf("Goodbye.\n");
					close(s);
					return -1;
				}
			}
		}
	}
}
