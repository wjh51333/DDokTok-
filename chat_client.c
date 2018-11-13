/*----------------------------------------------------------------------------------------- 
���ϸ� : chat_client.c 
�� �� : ������ ������ �� Ű������ �Է��� ������ �����ϰ�, 
�����κ��� ���� �޽����� ȭ�鿡 ����Ѵ�. 
������ : cc -o chat_client chat_client.c readline.c -lsocket -lnsl 
���࿹ : chat_client 203.252.65.3 4001 �����_ID 
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

int s; /* ������ ����� ���Ϲ�ȣ */ 
struct Name { 
char n[20]; /* ��ȭ�濡�� ����� �̸� */ 
int len;	/* �̸��� ũ�� */ 
} name; 

int main(int argc, char *argv[]) 
{ 
	char line[MAXLINE], sendline[MAXLINE+1]; 
	int n, pid, size; 
	struct sockaddr_in server_addr; 
	int nfds; 

	fd_set read_fds;
	if( argc < 4 ) { 
		printf("������ : %s ȣ��Ʈ IP�ּ� ��Ʈ��ȣ ������̸� \n", argv[0]); 
		return -1; 
	} 

	/* ä�� ������ �̸� ����ü �ʱ�ȭ */ 
	sprintf(name.n, "[%s]", argv[3]); 
	name.len = strlen(name.n); 

	/* ���� ���� */ 
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Client : Can't open stream socket.\n"); 
		return -1; 
	} 

	/* ä�� ������ �����ּ� ����ü server_addr �ʱ�ȭ */ 
	bzero((char *)&server_addr, sizeof(server_addr)); 
	server_addr.sin_family = AF_INET; 
	server_addr.sin_addr.s_addr = inet_addr(argv[1]); 
	server_addr.sin_port = htons(atoi(argv[2])); 
	/* �����û */ 
	if(connect(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) { 
		printf("Client : Can't connect to server.\n"); 
		return -1; 
	} 
	else { 
		printf("���ӿ� �����߽��ϴ�..\n"); 
	} 
	nfds = s + 1; 
	FD_ZERO(&read_fds); 
	while(1) { 
		/* -------------------------------------- selelct() ȣ�� ---------------------------------------/* 
		FD_SET(0, &read_fds); 
		FD_SET(s, &read_fds);
		if(select(nfds, &read_fds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0) { 
			printf("select error\n"); 
			return -1; 
		} 

		/*------------------------- �����κ��� ������ �޽��� ó�� -------------------------*/ 
		if (FD_ISSET(s, &read_fds)) { 
			char recvline[MAXLINE]; 
			int size; 
			if ((size = recv(s, recvline, MAXLINE, 0)) > 0) { 
				recvline[size] = '\0'; 
				printf("%s \n", recvline); 
			} 
		} 

		/* --------------------------------- Ű���� �Է� ó�� ----------------------------------*/ 
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
		} /* end of Ű���� �Է� ó�� */ 
	} /* end of while() */ 
}