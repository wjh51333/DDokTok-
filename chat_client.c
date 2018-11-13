/*----------------------------------------------------------------------------------------- 
파일명 : chat_client.c 
기 능 : 서버에 접속한 후 키보드의 입력을 서버에 전달하고, 
서버로부터 오는 메시지를 화면에 출력한다. 
컴파일 : cc -o chat_client chat_client.c readline.c -lsocket -lnsl 
실행예 : chat_client 203.252.65.3 4001 사용자_ID 
-------------------------------------------------------------------------------------------*/ 
#include <stdio.h> 
#include <fcntl.h> 
#include <stdlib.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> 

#define MAXLINE 1024 
#define MAX_SOCK 512 
char *escapechar = "exit\n"; 
int readline(int, char *, int); 

int s; /* 서버와 연결된 소켓번호 */ 
struct Name { 
char n[20]; /* 대화방에서 사용할 이름 */ 
int len;	/* 이름의 크기 */ 
} name; 

int main(int argc, char *argv[]) 
{ 
	char line[MAXLINE], sendline[MAXLINE+1]; 
	int n, pid, size; 
	struct sockaddr_in server_addr; 
	int nfds; 

	fd_set read_fds;
	if( argc < 4 ) { 
		printf("실행방법 : %s 호스트 IP주소 포트번호 사용자이름 \n", argv[0]); 
		return -1; 
	} 

	/* 채팅 참가자 이름 구조체 초기화 */ 
	sprintf(name.n, "[%s]", argv[3]); 
	name.len = strlen(name.n); 

	/* 소켓 생성 */ 
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Client : Can't open stream socket.\n"); 
		return -1; 
	} 

	/* 채팅 서버의 소켓주소 구조체 server_addr 초기화 */ 
	bzero((char *)&server_addr, sizeof(server_addr)); 
	server_addr.sin_family = AF_INET; 
	server_addr.sin_addr.s_addr = inet_addr(argv[1]); 
	server_addr.sin_port = htons(atoi(argv[2])); 
	/* 연결요청 */ 
	if(connect(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) { 
		printf("Client : Can't connect to server.\n"); 
		return -1; 
	} 
	else { 
		printf("접속에 성공했습니다..\n"); 
	} 
	nfds = s + 1; 
	FD_ZERO(&read_fds); 
	while(1) { 
		/* -------------------------------------- selelct() 호출 ---------------------------------------/* 
		FD_SET(0, &read_fds); 
		FD_SET(s, &read_fds);
		if(select(nfds, &read_fds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0) { 
			printf("select error\n"); 
			return -1; 
		} 

		/*------------------------- 서버로부터 수신한 메시지 처리 -------------------------*/ 
		if (FD_ISSET(s, &read_fds)) { 
			char recvline[MAXLINE]; 
			int size; 
			if ((size = recv(s, recvline, MAXLINE, 0)) > 0) { 
				recvline[size] = '\0'; 
				printf("%s \n", recvline); 
			} 
		} 

		/* --------------------------------- 키보드 입력 처리 ----------------------------------*/ 
		if (FD_ISSET(0, &read_fds)) { 
			if (readline(0, sendline, MAXLINE) > 0) { 
				size = strlen(sendline); 
				sprintf(line, "%s %s", name.n, sendline); 
				if (send(s, line, size + name.len, 0) != (size+name.len)) 
					printf("Error : Written error on socket.\n"); 
				if (size == 5 && strncmp(sendline, escapechar, 5) == 0) { 
					printf("Good bye.\n"); 
					close(s); 
					return -1; 
				} 
			} 
		} /* end of 키보드 입력 처리 */ 
	} /* end of while() */ 
}