#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define SERVER_PORT 8888
#define SERVER_IP "127.0.0.1"
#define BUFF_LEN 1024

void modify_addr(){
    
}

void * thread_listen(void * sockfd){
    char buffer[BUFF_LEN];
    int nbytes;

    while(1){
        if (-1 == (nbytes = recv(sockfd, buffer, 1024, 0)))
        { //接收客户端发来的数据
            printf("Read Error\n");
            exit(1);
        }
        buffer[nbytes] = '\0';
        printf("·received:\t%s\n", buffer);
    }
}

int main(){
    //变量声明
    pthread_t listen_id;    //listen进程的id    
    int sockfd;

    struct sockaddr_in server_addr;
    struct sockaddr_in other_client_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //创建socket描述符sockfd,本地socket
    bzero(&server_addr, sizeof(struct sockaddr_in));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);      //字节序转换
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);//IP
    if (-1 == connect(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr))) //发起连接
    {
        printf("Connect Error\n");
        exit(1);
    }else{
        printf("Connected.\n");
    }

    int ret = pthread_create(&listen_id,NULL,thread_listen,(void *)sockfd);
    //注意：sockfd这里是指针，要想传多个参数，可以定义一个结构体，把想传的参数包起来，传结构体的地址
    if(ret != 0){
        printf("Creat pthread error...\n");
        exit(1);
    }

}