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
#include <mysql.h>
//#include <my_global.h>
#define UPORT 8888
#define PORT  8890
#define BUFFER_SIZE 1024

int main(int argc, char** argv)
{
    int num_online;
    //if(argc!=2)
    //{
    //    printf("usage: client IP \n");
    //    exit(0);
    //}

    //定义IPV4的TCP连接的套接字描述符
    int sock_cli = socket(AF_INET, SOCK_STREAM, 0);

    //定义sockaddr_in
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    //servaddr.sin_port = htons(argv[2]);  //服务器端口a
    //short tmp_port = atoi(argv[2]);
    servaddr.sin_port = htons(PORT);  //服务器端口a

    //连接服务器，成功返回0，错误返回-1
    if (connect(sock_cli, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect");
        exit(1);
    }
    printf("connect server(IP:%s).\n", argv[1]);

    char sendbuf[BUFFER_SIZE];
    char recvbuf[BUFFER_SIZE];

    //先发送登录请求
    //sprintf(sendbuf, "{\"id\":%s, \"ip\":\"%s\", \"port\":%s}", "2018215404", argv[1], argv[3]);
    //send(sock_cli, sendbuf, sizeof(sendbuf), 0);
    //printf("发送数据:%s\n", sendbuf);
    //memset(sendbuf, 0, sizeof(sendbuf));
//id
    sprintf(sendbuf, "%s", "2018211169");
    send(sock_cli, sendbuf, sizeof(sendbuf), 0);
    //printf("发送数据:%s\n", sendbuf);
    memset(sendbuf, 0, sizeof(sendbuf));
//ip
    sprintf(sendbuf, "%s",argv[1]);
    send(sock_cli, sendbuf, sizeof(sendbuf), 0);
    //printf("发送数据:%s\n", sendbuf);
    memset(sendbuf, 0, sizeof(sendbuf));
//port
    sprintf(sendbuf, "%s", argv[3]);
    send(sock_cli, sendbuf, sizeof(sendbuf), 0);
    //printf("发送数据:%s\n", sendbuf);
    memset(sendbuf, 0, sizeof(sendbuf));


    //接收服务器发过来的在线好友个数
    int len = recv(sock_cli, recvbuf, sizeof(recvbuf), 0); ///接收
    if (len > 0)
    {
        recvbuf[BUFFER_SIZE] = '\0';
        printf("%s\n", recvbuf);
    }
    num_online = atoi(recvbuf);
    memset(recvbuf, 0, sizeof(recvbuf));

    //接收服务器发的用户数据
    for (int i = 0; i < num_online; i++)
    {
        int len = recv(sock_cli, recvbuf, sizeof(recvbuf), 0); ///接收
        if (len > 0)
        {
            recvbuf[BUFFER_SIZE] = '\0';
            printf("%s\n", recvbuf);
        }
        memset(recvbuf, 0, sizeof(recvbuf));
    }
    
//进行udp通信客户端并接收tcp服务器数据
int udp_id;
printf("请输入你要进行通信的用户id");
scanf("%d",&udp_id);
//    	char sendbuf[BUFFER_SIZE];
//	char recvbuf[BUFFER_SIZE];
	pid_t udp_pid=fork();
    
	if(udp_pid==0)
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
//		servaddr.sin_port=htons(atoi(argv[4]));//第3参数为端口
		servaddr.sin_port=htons(UPORT);
		servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");//IP
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		printf("Connect OK!\n");

		//while(1)
		//{ //消息收发，先往服务器发送，再接收服务器的反馈
		printf("Start Talking ~\n");
//		printf("Client Says:");
		fgets(sendline, 80, stdin);
		sendto(sockfd, sendline, strlen(sendline), 0,(struct sockaddr *) &servaddr, sizeof(servaddr));
//		n=recvfrom(sockfd,recvline,80,0,(struct sockaddr *) &servaddr,&len);
//		recvline[n]=0;
//		printf("Server Says:%s",recvline);
		//} 

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
	    while (1)
	    {
		int len = recv(sock_cli, recvbuf, sizeof(recvbuf), 0); ///接收
		if (len > 0)
		{
		    recvbuf[BUFFER_SIZE] = '\0';
		    printf("%s---from server\n", recvbuf);
		}
		memset(recvbuf, 0, sizeof(recvbuf));
	    }
		close(sock_cli);
		return 0;
	}
/*
    //客户端将控制台输入的信息发送给服务器端，服务器原样返回信息
    while (1)
    {

//        printf("client receive:");
        int len = recv(sock_cli, recvbuf, sizeof(recvbuf), 0); ///接收
        if (len > 0)
        {
            recvbuf[BUFFER_SIZE] = '\0';
            printf("%s---from server\n", recvbuf);
        }
        memset(recvbuf, 0, sizeof(recvbuf));
    }

    close(sock_cli);
    return 0;
*/
}
