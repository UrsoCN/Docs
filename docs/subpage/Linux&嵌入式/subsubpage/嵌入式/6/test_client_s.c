#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
 
#define PORT  8890
#define BUFFER_SIZE 1024
 
int main(int argc, char **argv)
{
	//if(argc!=2)
	//{
	//    printf("usage: client IP \n");
	//    exit(0);
	//}
 
	//定义IPV4的TCP连接的套接字描述符
	int sock_cli = socket(AF_INET,SOCK_STREAM, 0);
	 
	//定义sockaddr_in
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);
	servaddr.sin_port = htons(PORT);  //服务器端口
	 
	//连接服务器，成功返回0，错误返回-1
	if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
	    perror("connect");
	    exit(1);
	}
	printf("connect server(IP:%s).\n",argv[1]);
 
	char sendbuf[BUFFER_SIZE];
	char recvbuf[BUFFER_SIZE];
	pid_t udp_pid=fork();    
	if(udp_pid==0)
	{
		printf("child process: %d created.\n", getpid());
		//close(sock_cli);
		int sockfd,n;
		pid_t upid;
		socklen_t len;
		char mesg[80];
		struct sockaddr_in servaddr, cliaddr;
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servaddr.sin_port = htons(atoi("8888"));//指定端口号
		//servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		//servaddr.sin_port = htons(atoi(argv[1]));//指定端口号
		bind(sockfd, (struct sockaddr *) &servaddr,sizeof(servaddr));
		printf("Server OK!\n");

//printf("222");
		//for(;;){
		//进行消息收发，先接收客户端数据，后向客户端发送
		len = sizeof(cliaddr);
		n = recvfrom(sockfd, mesg, 80, 0, (struct sockaddr *) &cliaddr,&len);
		mesg[n] = 0;
		printf("Client Says:%s", mesg);
		//printf("Server says:");
		//fgets(mesg, 80, stdin);
		//sendto(sockfd,mesg,strlen(mesg),0,(struct sockaddr *) &cliaddr,len);
		//}

		upid=fork();
		if(upid<0)
		{
		printf("error!\n");
		}

		while(1)
		{
			if(upid>0)
			{       
				fgets(mesg,80,stdin);
				sendto(sockfd,mesg,strlen(mesg),0,(struct sockaddr *) &cliaddr,len);
			}
			else if(upid==0)
			{
				printf("Client Says:");
				if(-1==(n = recvfrom(sockfd, mesg, 80, 0, (struct sockaddr *) &cliaddr,&len)))
				{
				printf("Read Error\n");
				exit(1);
				}       
				mesg[n] = 0;
				printf("%s",mesg);
			}
		}
		close(sockfd);
		exit(0);
		return 0;
	}
	else if(udp_pid>0)
	{
		//客户端将控制台输入的信息发送给服务器端，服务器原样返回信息
		while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
		{
		    send(sock_cli, sendbuf, strlen(sendbuf),0); ///发送
		    if(strcmp(sendbuf,"exit\n")==0)
		    {
			printf("client exited.\n");
			break;
		    }
		    printf("client receive:\n");
		    recv(sock_cli, recvbuf, sizeof(recvbuf),0); ///接收
		    fputs(recvbuf, stdout);

		    memset(sendbuf, 0, sizeof(sendbuf));
		    memset(recvbuf, 0, sizeof(recvbuf));
		}
		close(sock_cli);
		return 0;
	}
}

