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




	pid_t udp_pid;   
	udp_pid=fork(); 
	if(0 == udp_pid)
	{
		printf("child process: %d created.\n", getpid());
		//close(sock_cli);
		int sockfd,n;
		pid_t upid;
		socklen_t len;
		char sendline[80], recvline[80+1];
		struct sockaddr_in servaddr;
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port=htons(atoi("8888"));//第3参数为端口
		servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");//IP

		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		printf("Connect OK!\n");
		printf("Start Talking ~\n");
		fgets(sendline, 80, stdin);
		sendto(sockfd, sendline, strlen(sendline), 0,(struct sockaddr *) &servaddr, sizeof(servaddr));

		upid=fork();
		if(upid<0)	printf("error!\n");

		while(1)
		{
			if(upid==0)
			{
				printf("Server Says:");                  
				if(-1==(n=recvfrom(sockfd,recvline,80,0,(struct sockaddr *) &servaddr,&len)))
				{
					printf("Read Error\n");
					exit(1);
				}
		  
				recvline[n]=0;
				printf("%s",recvline);
			}
			else if(upid>0)
			{	
				fgets(sendline, 80, stdin);
				sendto(sockfd, sendline, strlen(sendline), 0,(struct sockaddr *) &servaddr, sizeof(servaddr));
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




//子进程

//父进程保持与服务器的TCP连接