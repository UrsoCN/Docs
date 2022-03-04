#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <my_global.h>

// struct user
// {
//     int id;
//     char* nickname;
//     int friend[3];
//     int online;
//     char* ip;
//     int port;
// };

int main(int argc,char **argv){
    MYSQL *con = mysql_init(NULL);
    MYSQL_ROW row;
    MYSQL_RES *result;
    if(NULL == con){
        fprintf(stderr,"%s\n",mysql_error(con));
        return -1;
    }
    if(NULL == mysql_real_connect(con,"localhost","root",NULL,"mydb",0,NULL,0)){
        fprintf(stderr,"%s\n",mysql_error(con));
        mysql_close(con);
        return -1;
    }
    /*if(mysql_query(con,"CREATE DATABASE what")){
        fprintf(stderr,"%s\n",mysql_error(con));
        mysql_close(con);
        return -1;
    }*/

    //fprintf(stdout,"%s\n",mysql_get_server_info(con));
    //fprintf(stdout,"%s\n",mysql_get_host_info(con));

    //struct user user169;

    printf("请选择需要：\n");
    printf("  1)查询\n");
    printf("  2)更新\n");

    int tmp,tmp2;
    char username[20];
    char sql[100];

    int id1,id2,id3;
    char online[5];
    char ip[20];
    int port;


    scanf("%d",&tmp);
    switch (tmp)
    {
    case 1:
        printf("  请输入要查询的用户名：\n");
        scanf("%s", username);
        printf("  需要查询该用户的什么信息？\n");
        printf("    1)好友列表\n");
        printf("    2)是否在线\n");
        printf("    3)ip与端口\n");

        scanf("%d",&tmp2);
        
        switch (tmp2)
        {
        case 1:
            sprintf(sql,"select * from user169 where nickname = '%s'",username);
            if (mysql_query(con,sql)){
                fprintf(stderr, "%s\n", mysql_error(con));
                mysql_close(con);
                return -1;
            }
            result = mysql_store_result(con);
            row = mysql_fetch_row(result);
            printf("%s\t%s\t%s\n",row[3],row[4],row[5]);

            break;
        case 2:
            sprintf(sql,"select * from user169 where nickname = '%s'",username);
            if (mysql_query(con,sql)){
                fprintf(stderr, "%s\n", mysql_error(con));
                mysql_close(con);
                return -1;
            }
            result = mysql_store_result(con);
            row = mysql_fetch_row(result);
            printf("%s\n",row[6]);

            break;
        case 3:
            sprintf(sql,"select * from user169 where nickname = '%s'",username);
            if (mysql_query(con,sql)){
                fprintf(stderr, "%s\n", mysql_error(con));
                mysql_close(con);
                return -1;
            }
            result = mysql_store_result(con);
            row = mysql_fetch_row(result);
            printf("%s:%s\n",row[7],row[8]);

            break;
        default:
            break;
        }
        

        break;
    case 2:
        printf("  请输入要更新的用户名：\n");
        scanf("%s", username);
        printf("  需要更新该用户的什么信息？\n");
        printf("    1)好友列表\n");
        printf("    2)是否在线\n");
        printf("    3)ip与端口\n");

        scanf("%d",&tmp2);
        switch (tmp2)
        {
        case 1:
            printf("    输入好友信息（以空格分开）：\n");
            scanf("%d%d%d",&id1,&id2,&id3);
            sprintf(sql,"update user169 set friend1 = %d where nickname = '%s'",id1,username);
            if (mysql_query(con,sql)){
                fprintf(stderr, "%s\n", mysql_error(con));
                mysql_close(con);
                return -1;
            }

            sprintf(sql,"update user169 set friend2 = %d where nickname = '%s'",id2,username);
            if (mysql_query(con,sql)){
                fprintf(stderr, "%s\n", mysql_error(con));
                mysql_close(con);
                return -1;
            }

            sprintf(sql,"update user169 set friend3 = %d where nickname = '%s'",id3,username);
            if (mysql_query(con,sql)){
                fprintf(stderr, "%s\n", mysql_error(con));
                mysql_close(con);
                return -1;
            }

            break;
        case 2:
            printf("    输入在线信息(yes/no)：\n");
            scanf("%s",online);
            sprintf(sql,"update user169 set online = '%s' where nickname = '%s'",online,username);
            if (mysql_query(con,sql)){
                fprintf(stderr, "%s\n", mysql_error(con));
                mysql_close(con);
                return -1;
            }

            break;
        case 3:
            printf("    输入ip：\n");
            scanf("%s",ip);
            printf("    输入端口：\n");
            scanf("%d",&port);
            sprintf(sql,"update user169 set ip = '%s' where nickname = '%s'",ip,username);
            if (mysql_query(con,sql)){
                fprintf(stderr, "%s\n", mysql_error(con));
                mysql_close(con);
                return -1;
            }

            sprintf(sql,"update user169 set port = %d where nickname = '%s'",port,username);
            if (mysql_query(con,sql)){
                fprintf(stderr, "%s\n", mysql_error(con));
                mysql_close(con);
                return -1;
            }

            break;
        default:
            break;
        }

    default:
        break;
    }

    mysql_close(con);
    return 0;
}
