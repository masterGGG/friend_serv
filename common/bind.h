#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXLINE 4096

typedef struct {
	uint32_t pkg_len;
	uint32_t seq_num;
	uint16_t cmd_id;
	uint32_t status_code;
	uint32_t user_id;
}__attribute__((packed)) head_t;

void str_echo(int sockfd, int val)
{
	ssize_t n;
	char buf[MAXLINE];
	char send_buf[MAXLINE];
	memset(buf, 0, MAXLINE);
	memset(send_buf, 0, MAXLINE);

	head_t *head = (head_t *)send_buf;
	head->pkg_len = 18;
	head->seq_num = 0;
	head->cmd_id = 0x1000;
	head->status_code = 0;
	head->user_id = val;

	while((n = recv(sockfd, buf, MAXLINE,0)) > 0){
//		printf("recv msg from client: %s\n", buf);
		if( send(sockfd, send_buf, sizeof(head_t), 0) < 0){
			printf("send msg error: %s(errno: %d)\n", 
					strerror(errno), errno);
			 exit(0);
		}
		memset(buf, 0, MAXLINE);
	}

}

int start_service(int port, int val)
{
	int listenfd, connfd;
	socklen_t clilen;
	struct sockaddr_in servaddr, cliaddr;

	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("creat socket error: %s(errno: %d)\n", strerror(errno), errno);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;	//协议簇，这里选择了ipv4
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//对IPV4设置通配地址
	servaddr.sin_port = htons(port);//设置端口号666

	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
		printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
    		exit(0);       		
	}

	if( listen(listenfd, 10) == -1){ //socket可以排队的最大连接个数10个
		printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
		exit(0);
	}

	//printf("========waiting for client's request=========\n");
	while(1){
		//第二个参数为指向struct sockaddr *的指针，用于返回客户端的协议地址
		//第三个参数为协议地址的长度
		clilen = sizeof(cliaddr);
		if((connfd = accept(listenfd, 
				(struct sockaddr*)&cliaddr, &clilen)) == -1){
			printf("accept socket error: %s(errno: %d)",
						strerror(errno),errno);
			continue;
		}

		str_echo(connfd, val);
		close(connfd);
	}

	close(listenfd);

	return 0;
}
