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
//#include "cJSON.h"

#define PORT  8890
#define QUEUE_SIZE   5
#define BUFFER_SIZE 1024

//int parse_json_recv(const char* buf, int* id, char* ip, int* port);
void finish_with_error(MYSQL* con);
int get_value(MYSQL* con, const char* table, const char* field, int id, char* buf);
int set_value(MYSQL* con, const char* table, const char* field, int id, char* buf);

//传进来的sockfd，就是互相建立好连接之后的socket文件描述符
//通过这个sockfd，可以完成 [服务端]<--->[客户端] 互相收发数据
int str_echo(int sockfd,MYSQL* con)
{
//    
	int num_online = 0;
    int nbytes = 0;
    socklen_t len = 0;
    int id = 0;
    //char id[20] = {0};
    char ip[20] = { 0 };
    char online[20] = { 0 };
    int port = 0;
    int friend1 = 0, friend2 = 0, friend3 = 0;
//
    char buffer[BUFFER_SIZE];
    pid_t pid = getpid();
//    
            char buf_info[BUFFER_SIZE];
            char buf[BUFFER_SIZE];
            int state1=0,state2=0,state3=0;
            //int len = recv(sockfd, buffer_login, sizeof(buffer), 0);
            //if (len > 0)
            //{
            //    buffer_login[BUFFER_SIZE] = '\0';
            //    printf("User%s want to connect\n", buffer_login);
            //}

            //接收登录请求并使用CJSON解析提取用户ID和用电总量
            //len = recv(conn, buf, sizeof(buf), 0); ///接收
            //if (len > 0)
            //{
            //    buf[BUFFER_SIZE] = '\0';
            //    printf("收到连接请求%s\n", buf);
            //}
            //if (-1 == parse_json_recv(buf, &id, ip, &port)) {
            //    printf("解析用户连接请求失败！\n");
            //    return -1;
            //}
            len = recv(sockfd, buf, sizeof(buf), 0); ///接收
            if (len > 0)
            {
                buf[BUFFER_SIZE] = '\0';
                printf("收到id%s\n", buf);
            }
            id = atoi(buf);

            len = recv(sockfd, buf, sizeof(buf), 0); ///接收
            if (len > 0)
            {
                buf[BUFFER_SIZE] = '\0';
                printf("收到ip%s\n", buf);
            }
            strcpy(ip, buf);

            len = recv(sockfd, buf, sizeof(buf), 0); ///接收
            if (len > 0)
            {
                buf[BUFFER_SIZE] = '\0';
                printf("收到端口%s\n", buf);
            }
            port = atoi(buf);

            //更新用户数据
            sprintf(buf, "%s","yes");
            if (-1 != set_value(con, "user169", "online", id, buf))
                printf("[用户%d] 在线状态更新为:%s\n", id, buf);
            else {
                printf("[用户%d] 在现状态更新失败\n", id);
//                return -1;
            }
            strcpy(buf, ip);
            if (-1 != set_value(con, "user169", "ip", id, buf))
                printf("[用户%d] ip更新为:%s\n", id, ip);
            else {
                printf("[用户%d] ip更新失败\n", id);
                return -1;
            }
            sprintf(buf, "%d", port);
            if (-1 != set_value(con, "user169", "port", id, buf))
                printf("[用户%d] port更新为:%d\n", id, port);
            else {
                printf("[用户%d] port更新失败\n", id);
                return -1;
            }
            
            if (-1 != get_value(con, "user169", "friend1", id, buf)) {
                friend1 = atoi(buf);
                if (-1 != get_value(con, "user169", "online", friend1, buf)) {
                    if (strstr(buf,"yes")) {
                        num_online++;
                        state1=1;      
                    }
                    else {state1=0;}
                }
            }

            if (-1 != get_value(con, "user169", "friend2", id, buf)) {
                friend2 = atoi(buf);
                if (-1 != get_value(con, "user169", "online", friend2, buf)) {
                    if (strstr(buf, "yes")) {
                        num_online++;
			state2=1;
                    }
                    else {state2=0;}
                }
            }

            if (-1 != get_value(con, "user169", "friend3", id, buf)) {
                friend3 = atoi(buf);
                if (-1 != get_value(con, "user169", "online", friend3, buf)) {
                    if (strstr(buf, "yes")) {
                        num_online++;
                        state3=1;
                    }
                    else {state3=0;}
                }
            }
            sprintf(buf_info, "%d", num_online);
            send(sockfd, buf_info, sizeof(buf_info), 0);

            //查询用户数据
            if (-1 != get_value(con, "user169", "friend1", id, buf)) {
                friend1 = atoi(buf);
                if (-1 != get_value(con, "user169", "online", friend1, buf)) {
                    strcpy(online, buf);
                    if (strstr(online, "yes")) {
                        printf("friend1 is online   ");
                        if (-1 != get_value(con, "user169", "ip", friend1, buf)) {
                            strcpy(ip, buf);
                            printf("ip:%s   ", buf);
                        }
                        if (-1 != get_value(con, "user169", "port", friend1, buf)) {
                            port = atoi(buf);
                            printf("port:%d\n", port);
                        }
                        sprintf(buf_info, "好友%d在线  ip:%s  port:%d\n", friend1, ip, port);
                        send(sockfd, buf_info, sizeof(buf_info), 0);
                    }
                
                }
                else printf("查询好友1在线状态失败");
            }
            else printf("查询用户的好友1信息失败");


            if (-1 != get_value(con, "user169", "friend2", id, buf)) {
                friend2 = atoi(buf);
                if (-1 != get_value(con, "user169", "online", friend2, buf)) {
                    strcpy(online, buf);
                    if (strstr(online, "yes")) {
                        printf("friend2 is online   ");
                        if (-1 != get_value(con, "user169", "ip", friend2, buf)) {
                            strcpy(ip, buf);
                            printf("ip:%s   ", buf);
                        }
                        if (-1 != get_value(con, "user169", "port", friend2, buf)) {
                            port = atoi(buf);
                            printf("port:%d\n", port);
                        }
                        sprintf(buf_info, "好友%d在线  ip:%s  port:%d\n", friend2, ip, port);
                        send(sockfd, buf_info, sizeof(buf_info), 0);
                    }
                    
                }
                else printf("查询好友2在线状态失败");
            }
            else printf("查询用户的好友2信息失败");


            if (-1 != get_value(con, "user169", "friend3", id, buf)) {
                friend3 = atoi(buf);
                if (-1 != get_value(con, "user169", "online", friend3, buf)) {
                    strcpy(online, buf);
                    if (strstr(online, "yes")) {
                        printf("friend3 is online   ");
                        if (-1 != get_value(con, "user169", "ip", friend3, buf)) {
                            strcpy(ip, buf);
                            printf("ip:%s   ", buf);
                        }
                        if (-1 != get_value(con, "user169", "port", friend3, buf)) {
                            port = atoi(buf);
                            printf("port:%d\n", port);
                        }
                        sprintf(buf_info, "好友%d在线  ip:%s  port:%d\n", friend3, ip, port);
                        send(sockfd, buf_info, sizeof(buf_info), 0);
                    }
                    
                }
                else printf("查询好友3在线状态失败");
            }
            else printf("查询用户的好友3信息失败");
//查询上下线消息，发给客户端          
	pid_t spid=fork();
	if(spid==0){   
	    while (1)
	    {		
	    		int s1=state1;
	    		int s2=state2;
	    		int s3=state3;
		    if (-1 != get_value(con, "user169", "friend1", id, buf)) {
		        friend1 = atoi(buf);
		        if (-1 != get_value(con, "user169", "online", friend1, buf)) {
		            if (strstr(buf,"yes")) {
		            	state1=1;
		            }
		            else {state1=0;}
		        }
		    }
		    
		    if(s1!=state1) {
		    	if(state1==0){
		    		sprintf(buf_info, "好友%d下线  ip:%s  port:%d\n", friend1, ip, port);
		                send(sockfd, buf_info, sizeof(buf_info), 0);
		    	}
		    	else{
		    		sprintf(buf_info, "好友%d上线  ip:%s  port:%d\n", friend1, ip, port);
		                send(sockfd, buf_info, sizeof(buf_info), 0);
		    	}
		    }
/*
		    if (-1 != get_value(con, "user169", "friend2", id, buf)) {
		        friend2 = atoi(buf);
		        if (-1 != get_value(con, "user169", "online", friend2, buf)) {
		            if (strstr(buf, "yes")) {
		            	state2=1;
		            }
		            else {state2=0;}
		        }
		    }
		    
		    if(s2!=state2) {
		    	if(state2==0){
		    		sprintf(buf_info, "好友%d下线  ip:%s  port:%d\n", friend2, ip, port);
		                send(sockfd, buf_info, sizeof(buf_info), 0);
		    	}
		    	else{
		    		sprintf(buf_info, "好友%d上线  ip:%s  port:%d\n", friend2, ip, port);
		                send(sockfd, buf_info, sizeof(buf_info), 0);
		    	}
		    }

		    if (-1 != get_value(con, "user169", "friend3", id, buf)) {
		        friend3 = atoi(buf);
		        if (-1 != get_value(con, "user169", "online", friend3, buf)) {
		            if (strstr(buf, "yes")) {
		            	state3=1;
		            }
		            else{state3=0;}
		        }
		    }
		    
		    if(s3!=state3) {
		    	if(state3==0){
		    		sprintf(buf_info, "好友%d下线  ip:%s  port:%d\n", friend3, ip, port);
		                send(sockfd, buf_info, sizeof(buf_info), 0);
		    	}
		    	else{
		    		sprintf(buf_info, "好友%d上线  ip:%s  port:%d\n", friend3, ip, port);
		                send(sockfd, buf_info, sizeof(buf_info), 0);
		    	}
		    }
*/		    
		    sleep(1);
		}
		close(sockfd);
	}
//	    close(sockfd);
}

int main(int argc, char* argv)
{
/*
    int num_online = 0;
    int nbytes = 0;
    socklen_t len = 0;
    int id = 0;
    //char id[20] = {0};
    char ip[20] = { 0 };
    char online[20] = { 0 };
    int port = 0;
    int friend1 = 0, friend2 = 0, friend3 = 0;
*/
    //初始化MYSQL变量并连接数据库
    MYSQL* con = mysql_init(NULL);
    if (con == NULL) {
        printf("mysql init fail!\n");
        fprintf(stderr, "%s\n", mysql_error(con));
        return -1;
    }
    //连接数据库
    if (NULL == mysql_real_connect(con,"localhost","root",NULL,"mydb",0,NULL,0)) {
        printf("mysql connect fail!\n");
        fprintf(stderr, "%s\n", mysql_error(con));
        mysql_close(con);
        return -1;
    }
    printf("数据库登录成功\n");

    //定义IPV4的TCP连接的套接字描述符
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //定义sockaddr_in
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // server_sockaddr.sin_port = htons(atoi(argv[1]));//指定端口号
    server_sockaddr.sin_port = htons(PORT);

    //bind成功返回0，出错返回-1
    if (bind(server_sockfd, (struct sockaddr*)&server_sockaddr, sizeof(server_sockaddr)) == -1)
    {
        perror("bind");
        exit(1);//1为异常退出
    }
    printf("bind success.\n");

    //listen成功返回0，出错返回-1，允许同时帧听的连接数为QUEUE_SIZE
    if (listen(server_sockfd, QUEUE_SIZE) == -1)
    {
        perror("listen");
        exit(1);
    }
    printf("listen success.\n");

    for (;;)
    {
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);
        //进程阻塞在accept上，成功返回非负描述字，出错返回-1
        int conn = accept(server_sockfd, (struct sockaddr*)&client_addr, &length);
        if (conn < 0)
        {
            perror("connect");
            exit(1);
        }
        printf("new client accepted.\n");

        pid_t childid;
        if (childid = fork() == 0)//子进程
        {
            printf("child process: %d created.\n", getpid());
            close(server_sockfd);//在子进程中关闭监听
//            
//            
            str_echo(conn,con);//处理监听的连接
            exit(0);
        }

    }

    printf("closed.\n");
    close(server_sockfd);
    return 0;
}

/*
//使用cJSON从接收到的数据中提取用户ID IP 端口
int parse_json_recv(const char* buf, int* id, char* ip, int* port) {
    cJSON* cjson_all = NULL;
    cJSON* cjson_id = NULL;
    cJSON* cjson_ip = NULL;
    cJSON* cjson_port = NULL;
    //解析整段JSO数据
    cjson_all = cJSON_Parse(buf);
    if (cjson_all == NULL)
        return -1;
    //根据键值提取用户ID
    cjson_id = cJSON_GetObjectItem(cjson_all, "id");
    *id = cjson_id->valueint;
    //根据键值提取用户IP
    cjson_ip = cJSON_GetObjectItem(cjson_all, "ip");
    strcpy(ip, cjson_id->valuestring);
    //根据键值提取用户端口
    cjson_port = cJSON_GetObjectItem(cjson_all, "port");
    *port = cjson_id->valueint;
    //释放cjson占用内存
    cJSON_Delete(cjson_all);
    return 0;
}
*/

void finish_with_error(MYSQL* con) {
    fprintf(stderr, "%s\n", mysql_error(con));
    mysql_close(con);
}


//从数据库中查询用户信息
int get_value(MYSQL* con, const char* table, const char* field, int id, char* buf)
{
    char sql[100];
    MYSQL_RES* result = NULL;
    MYSQL_ROW row;
    sprintf(sql, "select %s from %s where id=%11d;", field, table, id);
    //查询
    if (mysql_query(con, sql)) {
        finish_with_error(con);
        return -1;
    }
    //获取并存储查询结果
    result = mysql_store_result(con);
    if (NULL == result) {
        finish_with_error(con);
        return -1;
    }
    //根据行数查询数据
    if (row = mysql_fetch_row(result)) {
        if (row[0] != NULL) {
            strcpy(buf, row[0]);
        }
        else
            strcpy(buf, "NULL");
    }
    mysql_free_result(result);
    return 0;
}
//从数据库中更新用户信息
int set_value(MYSQL* con, const char* table, const char* field, int id, char* buf)
{
    char sql[100];
    sprintf(sql, "update %s set %s='%s' where id=%11d;", table, field, buf, id);
 //printf("%s", sql);
    //执行
    if (mysql_query(con, sql)) {
        finish_with_error(con);
        return -1;
    }
    return 0;
}
