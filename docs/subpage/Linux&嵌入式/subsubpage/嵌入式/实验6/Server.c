/* TODO:
    server端需要实现通过TCP将Mysql数据库中的数据共享给client
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#include <mysql.h>
#include <my_global.h>

void *thread_listen(void * new_fd){
    char buffer[1024];
    int nbytes;

    while(1){
        if (-1 == (nbytes = recv(new_fd, buffer, 1024, 0)))
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

void set(){ //修改数据

}

void get(){ //获取数据

}

int main(int argc, char *argv[])
{
    //声明Mysql相关的变量
    MYSQL *con = mysql_init(NULL);
    MYSQL_ROW row;
    MYSQL_RES *result;
    //连接Mysql
    if(NULL == con){
        fprintf(stderr,"%s\n",mysql_error(con));
        return -1;
    }
    if(NULL == mysql_real_connect(con,"localhost","root",NULL,"mydb",0,NULL,0)){
        fprintf(stderr,"%s\n",mysql_error(con));
        mysql_close(con);
        return -1;
    }
/*--------------------------------------------------------------------------*/
    //声明TCP相关的变量
    int sockfd, new_fd; //注意，这里有两个socket描述符
    struct sockaddr_in server_addr, client_addr;    //分别存放服务器和客户端的地址、端口等信息
    int sin_size;
    int ret;
    char input[1024];
    short portnumber = atoi(argv[1]);   //端口号，需要制定参数
    pthread_t listen_id;    //listen进程的id
    if (1 == (sockfd = socket(AF_INET, SOCK_STREAM, 0)))
    { //创建socket
        printf("Socket error\n");
        exit(1);
    }
    bzero(&server_addr, sizeof(struct sockaddr_in)); //清空server_addr
    server_addr.sin_family = AF_INET;                //服务器使用的协议
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //服务器IP
    server_addr.sin_port = htons(portnumber);        //服务器端口号
    bind(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr));   //绑定sockfd
    listen(sockfd, 5);  //设定为最多监听5个client

    bzero(&client_addr, sizeof(struct sockaddr_in)); //先清空客户端信息，防止出错
    sin_size = sizeof(client_addr);
    //接受连接，将客户端信息存入client_addr，得到new_fd
    //！！！sin_size
    new_fd = accept(sockfd, (struct sockaddr *)(&client_addr), &sin_size);
    ret = pthread_create(&listen_id,NULL,thread_listen,(void *)new_fd);
    if(ret != 0){
        printf("Creat pthread error...\n");
        exit(1);
    }

/*--------------------------------------------------------------------------*/
//     int sockfd, new_fd; //注意，这里有两个socket描述符
//     //分别存放服务器和客户端的地址、端口等信息
//     struct sockaddr_in server_addr, client_addr;
//     int sin_size;
//     int ret;
//     char input[1024];
//     short portnumber = atoi(argv[1]);
//     pthread_t listen_id;    //listen进程的id
//     if (1 == (sockfd = socket(AF_INET, SOCK_STREAM, 0)))
//     { //创建socket
//         printf("Socket error\n");
//         exit(1);
//     }
//     bzero(&server_addr, sizeof(struct sockaddr_in));
//     server_addr.sin_family = AF_INET;                //服务器使用的协议
//     server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //服务器IP
//     server_addr.sin_port = htons(portnumber);        //服务器端口号
//     //绑定sockfd
//     bind(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr));
//     listen(sockfd, 5);                               //在sockfd上侦听，最多允许5个连接
//     bzero(&client_addr, sizeof(struct sockaddr_in)); //先清空客户端信息，防止出错
//     sin_size = sizeof(client_addr);
//     //接受连接，将客户端信息存入client_addr，得到new_fd
//     new_fd = accept(sockfd, (struct sockaddr *)(&client_addr), &sin_size);
//     ret = pthread_create(&listen_id,NULL,thread_listen,(void *)new_fd);
//     if(ret != 0){
//         printf("Creat pthread error...\n");
//         exit(1);
//     }

// /*--------------------------------------------------------------------------*/
//     while (1)
//     {
//         fgets(input, 1000, stdin);             //输入字符，存入input
//         printf("\n");
//         send(new_fd, input, strlen(input), 0); //将input发给客户端
//     }

//     close(new_fd); //关闭socket描述符
//     close(sockfd); //关闭socket描述符
//     exit(0);
}
