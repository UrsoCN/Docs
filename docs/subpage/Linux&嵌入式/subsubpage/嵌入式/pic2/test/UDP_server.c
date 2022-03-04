#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include<netinet/in.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<stdlib.h>

void do_echo(int sockfd, struct sockaddr *pcliaddr,socklen_t clilen);

int main(int argc, char* argv[])
{ //2个参数，第二个参数为端口号
	int sockfd;
	struct sockaddr_in servaddr, cliaddr;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(atoi(argv[1]));//指定端口号
	bind(sockfd, (struct sockaddr *) &servaddr,sizeof(servaddr));
	do_echo(sockfd, (struct sockaddr *) &cliaddr,sizeof(cliaddr));
	return 0;
}

void do_echo(int sockfd, struct sockaddr *pcliaddr,socklen_t clilen)
{
	int n;
	socklen_t len;
	char mesg[80];
	for(;;){
	//进行消息收发，先接收客户端数据，后向客户端发送
		len = clilen;
		n = recvfrom(sockfd, mesg, 80, 0, pcliaddr,&len);
		mesg[n] = 0;
		printf("Client Says: %s", mesg);
		printf("Server says:");
		fgets(mesg, 80, stdin);
		sendto(sockfd,mesg,strlen(mesg),0,pcliaddr,len);
	}
}
