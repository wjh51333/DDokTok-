/* chat_server.c
 *   기능 : 채팅 참가자 관리, 채팅 메시지 수신 및 방송 
 * 컴파일 : cc -o chat_server chat_server.c readline.c -lsocket -lnsl
 * 실행예 : chat_server 4001
 */
#include 	<stdio.h>
#include 	<fcntl.h>
#include 	<stdlib.h>
#include 	<signal.h>
#include 	<sys/socket.h>
#include 	<sys/file.h>
#include 	<netinet/in.h>
#include	<string.h>

#define MAXLINE 	1024
#define MAX_SOCK 	512
#define MAXTEXT 	1000000

char *escapechar = "Exit\n";
char *whisp = "귓속말/";

int exitCheck(char *, char *, int);

struct User { 
	char name[100]; /* 대화방에서 사용할 이름 */ 
	int 	client_s; /* 채팅에 참가하는 클라이언트들의 소켓번호 리스트 */
};

struct User user[5];

int main(int argc, char *argv[])  {
	char 	rline[MAXLINE], my_msg[MAXLINE];
	char 	*start = "대화방에 오신 것 환영합니다!!\n";
	int 	i, j, n;
	int 	s, client_fd, clen;
	int	nfds;			/* 최대 소켓번호 +1 */
	fd_set	read_fds;	/* 읽기를 감지할 소켓번호 구조체 */
	int	num_chat = 0;		/* 채팅 참가자 수 */
	struct sockaddr_in 	client_addr, server_addr;
	char w_name[100];		/*귓속말 받을 사용자 이름*/
	char whis[MAXLINE];
	char save[MAXTEXT];   /* text backup */
	int fd;

	if(argc < 2)  {
		printf("실행방법 :%s 포트번호\n",argv[0]); 
		return -1;
	}

	printf("-------------대화방 서버 초기화 중-------------\n");

	/* 초기소켓 생성 */
	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)  {
		printf("Server: Can't open stream socket.");   
		return -1;
	}

	/* server_addr 구조체의 내용 세팅 */
	bzero((char *)&server_addr, sizeof(server_addr));  
	server_addr.sin_family = AF_INET;              
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(argv[1]));     

	if (bind(s,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
		printf("Server: Can't bind local address.\n");
		return -1;
	}

	/* 클라이언트로부터 연결요청을 기다림 */
	listen(s, 5);

	nfds = s + 1;		/* 최대 소켓번호 +1 */
	FD_ZERO(&read_fds);

	while(1) {
		// (최대 소켓번호 +1) 값을 갱신
		if((num_chat-1) >= 0)  nfds = user[num_chat-1].client_s + 1;

		//읽기 변화를 감지할 소켓번호를 fd_set 구조체에 지정
		FD_SET(s, &read_fds);

		for(i=0; i<num_chat; i++)  FD_SET(user[i].client_s, &read_fds);

		// select() 호출
		if (select(nfds, &read_fds, (fd_set *)0, (fd_set *)0,(struct timeval *)0) < 0) {
			printf("select error\n");
			return -1;
		}
		// 클라이언트 연결요청 처리
		if(FD_ISSET(s, &read_fds)) {
			clen = sizeof(client_addr);
			client_fd = accept(s, (struct sockaddr *)&client_addr, &clen);

			if(client_fd != -1)  {
				// 채팅 클라이언트 목록에 추가
				user[num_chat].client_s = client_fd;
				num_chat++;
				send(client_fd, start, strlen(start), 0);
				printf("%d번째 사용자 추가.\n",num_chat);
			}
		}

		// 문자열 초기화
		memset(rline, 0, sizeof(rline));

		// 임의의 클라이언트가 보낸 메시지를 모든 클라이언트에게 방송
		for(i = 0; i < num_chat; i++)  {
			if(FD_ISSET(user[i].client_s, &read_fds)) {
				if((n = recv(user[i].client_s, rline, MAXLINE,0))  > 0)  {
					rline[n] = '\0';

					// 종료문자 입력시 채팅 탈퇴 처리
					if (exitCheck(rline, escapechar, 5) == 1) {
						//shutdown(user[i].client_s, 2);
						printf("%s Exit\n", user[i].name);
						if(i != num_chat-1){
							user[i].client_s = user[num_chat-1].client_s;
							strcpy(user[i].name, user[num_chat-1].name);
						}
						num_chat--;
						continue;

					}

					// 귓속말 "귓속말/받을 사람/내용"
					if(strstr(rline, whisp) != NULL){
						strcat(save, rline);
						strcat(save, "\n");
						memset(whis, 0, sizeof(whis));
						strcpy(whis, rline);
						strcpy(w_name, strtok(whis, "/"));
						strcpy(w_name, strtok(NULL, "/"));
						printf("whisper user is %s\n", w_name);
						for(j=0;j<num_chat;j++){
							//printf("%d: %s\n", j, user[j].name);
							if(user[j].client_s && w_name != NULL && !strcmp(user[j].name, w_name)){
								send(user[j].client_s, rline, n, 0);
								printf("whisper: from %s to %s\n", user[i].name, w_name);
								break;
							}
							else if(j == num_chat-1){
								send(user[i].client_s, "whisper fail\n", n, 0);
							}
						}
					}

					// 이름 입력받았을 때
					if(strstr(rline, "name:")!=NULL){
						char* token = strtok(rline, ":");
						token = strtok(NULL, " ");
						strcpy(user[i].name, token);
						printf("%d번째 사용자 이름: %s, %d\n", i+1, user[i].name, user[i].client_s);
						sprintf(rline, "[%s] 입장", user[i].name);
					}

					// 대화내용 저장
					if (strstr(rline, "#save") != NULL) {
						if ((fd = open("./backup.txt", O_WRONLY | O_CREAT | O_APPEND, 0644)) == -1)
							perror("backup file open");
						write(fd, save, strlen);
						close(fd);
					}

					if(strstr(rline, whisp)==NULL && strstr(rline, "#save") == NULL) {
						for (j = 0; j < num_chat; j++)
							send(user[j].client_s, rline, n, 0);
						printf("%s\n", rline);
						strcat(save, rline);
						strcat(save, "\n");
					}
				}
			}
		}// for()종료
	}// while()종료
}


/* 종료문자 확인 함수 exitCheck()
 *  rline : 클라이언트가 전송한 문자열 포인터
 * escape : 종료문자 포인터
 *    len : 종료문자의 크기
 */
int exitCheck(char *rline, char *escape, int len)
{
	int i, max;
	char *tmp;

	max = strlen(rline);	
	tmp = rline;
	for(i = 0; i<max; i++) {
		if (*tmp == escapechar[0]) {
			if(strncmp(tmp, escapechar, len) == 0)
				return 1;
		} else 
			tmp++;
	} 
	return -1;
}


