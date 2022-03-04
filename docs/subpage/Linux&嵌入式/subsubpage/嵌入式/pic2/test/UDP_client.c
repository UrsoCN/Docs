#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include<netinet/in.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdlib.h>

void do_cli(FILE *fp,int sockfd,struct sockaddr *pservaddr,socklen_t servlen);

int main(int argc, char **argv)
{
	int sockfd;
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port=htons(atoi(argv[2]));//第3参数为端口
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);//IP
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	do_cli(stdin, sockfd, (struct sockaddr *) &servaddr,sizeof(servaddr));
	return 0;
}

void do_cli(FILE *fp,int sockfd,struct sockaddr *pservaddr,socklen_t servlen)
{
	int n;
	socklen_t len;
	char sendline[80], recvline[80+1];
	while(1)
	{ //消息收发，先往服务器发送，再接收服务器的反馈
		printf("Client Says:");
		fgets(sendline, 80, stdin);
		sendto(sockfd, sendline, strlen(sendline), 0,pservaddr, servlen);
		n=recvfrom(sockfd,recvline,80,0,pservaddr,&len);
		recvline[n]=0;
		printf("Server Reply:%s",recvline);
	}
}
