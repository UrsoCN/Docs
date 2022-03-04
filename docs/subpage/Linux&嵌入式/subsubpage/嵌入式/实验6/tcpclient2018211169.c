#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

void *thread_listen(void * sockfd){
    char buffer[1024];
    int nbytes;

    while(1){
        if (-1 == (nbytes = recv(sockfd, buffer, 1024, 0)))
        { //接收客户端发来的数据
            printf("Read Error\n");
            exit(1);
        }
        buffer[nbytes] = '\0';
        //if("bye" == strtok(buffer," "))
        //    pthread_exit(NULL);
        printf("·received:\t%s\n", buffer);
    }
}

int main(int argc, char *argv[])
{ //参数判断略，共3个参数，命令名 ip 端口
    int sockfd;
    int sin_size;
    char input[1024];
    char buffer[1024];
    int ret;
    struct sockaddr_in server_addr;
    int nbytes;
    short portnumber = atoi(argv[2]);         //ascii转为int，得到端口号
    pthread_t listen_id;    //listen进程的id    
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //创建socket描述符sockfd
    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portnumber);                                              //字节序转换
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);                                      //IP
    if (-1 == connect(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr))) //发起连接
    {
        printf("Connect Error\n");
        exit(1);
    }

    ret = pthread_create(&listen_id,NULL,thread_listen,(void *)sockfd);
    //注意：sockfd这里是指针，要想传多个参数，可以定义一个结构体，把想传的参数包起来，传结构体的地址
    if(ret != 0){
        printf("Creat pthread error...\n");
        exit(1);
    }
/*--------------------------------------------------------------------------*/
    while (1)
    {
        fgets(input, 1000, stdin);             //输入字符，存入input
        printf("\n");
        send(sockfd, input, strlen(input), 0); //将input发给客户端
    }

    close(sockfd);
    exit(0);
}

