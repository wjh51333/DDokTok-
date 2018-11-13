/*----------------------------------------------------------------------------------------------
 ���ϸ� : chat_server.c    
 ��  �� : ä�� ������ ����, ä�� �޽��� ���� �� ��� 
 ������ : cc -o chat_server chat_server.c readline.c -lsocket -lnsl
 ���࿹ : chat_server 4001
-----------------------------------------------------------------------------------------------*/
#include 	<stdio.h>
#include 	<fcntl.h>
#include 	<stdlib.h>
#include 	<signal.h>
#include 	<sys/socket.h>
#include 	<sys/file.h>
#include 	<netinet/in.h>

#define MAXLINE 	1024
#define MAX_SOCK 	512
 
char *escapechar = "exit\n";
int readline(int, char *, int);

int main(int argc, char *argv[])  {
   char 	rline[MAXLINE], my_msg[MAXLINE];
   char 	*start = "��ȭ�濡 ���Ű� ȯ���մϴ�...\n";
   int 	i, j, n;
   int 	s, client_fd, clilen;
   int	nfds;			/* �ִ� ���Ϲ�ȣ +1 */
   fd_set	read_fds;	/* �б⸦ ������ ���Ϲ�ȣ ����ü */
   int	num_chat = 0;		/* ä�� ������ �� */
   /* ä�ÿ� �����ϴ� Ŭ���̾�Ʈ���� ���Ϲ�ȣ ����Ʈ */
   int 	client_s[MAX_SOCK];
   struct sockaddr_in 	client_addr, server_addr;
   
   if(argc < 2)  {
      printf("������ :%s ��Ʈ��ȣ\n",argv[0]); 
      return -1;
   }
   
   printf("��ȭ�� ���� �ʱ�ȭ ��....\n");

   /* �ʱ���� ���� */
   if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)  {
      printf("Server: Can't open stream socket.");   
      return -1;
   }
   
   /* server_addr ����ü�� ���� ���� */
   bzero((char *)&server_addr, sizeof(server_addr));  
   server_addr.sin_family = AF_INET;              
   server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   server_addr.sin_port = htons(atoi(argv[1]));     
   
   if (bind(s,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
      printf("Server: Can't bind local address.\n");
      return -1;
   }
   
   /* Ŭ���̾�Ʈ�κ��� �����û�� ��ٸ� */
   listen(s, 5);

   nfds = s + 1;		/* �ִ� ���Ϲ�ȣ +1 */
   FD_ZERO(&read_fds);
   
   while(1) {
      /* (�ִ� ���Ϲ�ȣ +1) ���� ���� */
      if((num_chat-1) >= 0)  nfds = client_s[num_chat-1] + 1;

      /* �б� ��ȭ�� ������ ���Ϲ�ȣ�� fd_set ����ü�� ���� */
      FD_SET(s, &read_fds);
      for(i=0; i<num_chat; i++)  FD_SET(client_s[i], &read_fds);
      
	  /*--------------------------------------- select() ȣ�� ----------------------------------------- */
      if (select(nfds, &read_fds, (fd_set *)0, (fd_set *)0,(struct timeval *)0) < 0) {
	     printf("select error\n");
	     return -1;
      }
  	  /*------------------------------ Ŭ���̾�Ʈ �����û ó�� ------------------------------- */
      if(FD_ISSET(s, &read_fds)) {
	     clilen = sizeof(client_addr);
	     client_fd = accept(s, (struct sockaddr *)&client_addr, &clilen);

	     if(client_fd != -1)  {
	    	/* ä�� Ŭ���̾�Ʈ ��Ͽ� �߰� */
	    	client_s[num_chat] = client_fd; 
	    	num_chat++;
	    	send(client_fd, start, strlen(start), 0);
	    	printf("%d��° ����� �߰�.\n",num_chat);
	     }
      }
      
      /*------ ������ Ŭ���̾�Ʈ�� ���� �޽����� ��� Ŭ���̾�Ʈ���� ��� ----- */
      for(i = 0; i < num_chat; i++)  {
	    if(FD_ISSET(client_s[i], &read_fds)) {
	       if((n = recv(client_s[i], rline, MAXLINE,0))  > 0)  {
	         rline[n] = '\0';

	         /* ���Ṯ�� �Է½� ä�� Ż�� ó�� */
	         if (exitCheck(rline, escapechar, 5) == 1) {
		     shutdown(client_s[i], 2);
		     if(i != num_chat-1)     client_s[i] = client_s[num_chat-1];
		     num_chat--;
		     continue;
	       	 }

	          /* ��� ä�� �����ڿ��� �޽��� ��� */	
	          for (j = 0; j < num_chat; j++)  send(client_s[j], rline, n, 0);
	          printf("%s", rline);
	       }
	    }
      }
   }
}

/* ------------------------------- ���Ṯ�� Ȯ�� �Լ� ---------------------------- 
exitCheck()�� ������ �� ���� ���ڸ� �ʿ�� �Ѵ�
	rline: Ŭ���̾�Ʈ�� ������ ���ڿ� ������
	escapechar: ���Ṯ�� ������
	len: ���Ṯ���� ũ��
---------------------------------------------------------------------------------------------*/
int exitCheck(rline, escapechar, len)
  char	*rline;		/* Ŭ���̾�Ʈ�� ������ �޽��� */
  char	*escapechar;	/* ���Ṯ�� */
  int		len;
  {
     int	i, max;
     char	*tmp;
   
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