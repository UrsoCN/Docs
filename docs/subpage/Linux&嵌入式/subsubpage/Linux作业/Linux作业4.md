# Linux操作系统

<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [Linux操作系统](#linux操作系统)
  - [作业4](#作业4)
    - [实现进程间通信](#实现进程间通信)
    - [作业要求:](#作业要求)
    - [过程和结果：](#过程和结果)
    - [附：源码](#附源码)

<!-- /code_chunk_output -->

>在正文部分，<font color = blue>蓝色的字</font>是**对问题的回答**或者**对下方图片的说明**。

<div STYLE="page-break-after: always;"></div>

## 作业4

### 实现进程间通信

&emsp;&emsp;实现如下功能:(可任选一种进程间通信方式)
&emsp;&emsp;程序 zhangsan1.c(zhangsan 为自己的姓名拼音,下同)实现冒泡算法排序( 参考课 程设计 3), 但不需要输入与输出信息,输入输出信息均由程序zhangsan2.c 提供。
&emsp;&emsp;程序 zhangsan2.c 输入一些数字,将这些数发送给第一个进程,由第一个进程排序后,将<font color = red>由小到大</font>排序后的结果返回程序 zhangsan2.c,然后打印排序后的结果。
&emsp;&emsp;分别使用 makefile 编译这两个程序,得到的可执行程序分别为 zhangsan1 和zhangsan2;运行这两个程序,验证程序的有效性。

### 作业要求:

&emsp;&emsp;1.附上完整代码和相应的 makefile,并说明使用了哪种进程间通信方式;
&emsp;&emsp;2.make 过程和结果截图。
&emsp;&emsp;3.两个程序运行过程的完整截图(两个程序的运行窗口必须出现在同一张图中),截图中需要有完整的数字输入和排序结果展示(输入的数字中,<font color = red>必须有一个数为自己学号的后 3 位</font>)。

### 过程和结果：

<font color = blue>
在这次作业中，我采用的是“共享内存”的进程间通信方式。
</font>

<font color = blue>下图为 使用makefile编译zhangxilong1和zhangxilong2执行文件 的截图。</font>

![make](./pic4/2020-12-22%2023-05-21屏幕截图.png)

<font color = blue>下图为 两个程序运行过程 的截图，169为我的学号后三位。
其中zhangxilong1执行时的空行是由于键入Enter使程序终止。</font>

![run](./pic4/2020-12-22%2023-07-53屏幕截图.png)

### 附：源码

<font color = blue>zhangxilong1.c:（分割线以上为获取共享内存，以下为将排序后的int数组写入另一块共享内存）</font>

``` C
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>

void bubble(int arr[],int len){
    int i, j, temp;
    for (i = 0; i < len - 1; i++)
        for (j = 0; j < len - 1 - i; j++)
            if (arr[j] > arr[j + 1]){
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
}

int main(int argc, char **argv){
    int shm_id, i;
    key_t key, key2;
    char *p_map;
    int *p_map2;
    char *token;
    int tmp[10] = {};   //初始化
    key = ftok(".", 1); //得到keyid(System V)这里ftok()的参数要一致
    if (key == -1)
        perror("ftok error...");
    shm_id = shmget(key, 4096, 0666); //获得共享内存区域
    if (shm_id == -1){
        perror("shmget error...");
        return -1;
    }
    p_map = (char *)shmat(shm_id, NULL, 0); //共享内存映射,attach

    token = strtok(p_map, " ");
    tmp[0] = atoi(token);

    for(i = 0;token != NULL;i++){
        //printf("%d\n",tmp[i]);    //这里会打印出共享内存中的数据，记得注释
        token = strtok(NULL, " ");  //再次调用时第一个参数需要改成NULL
        if(token!=NULL)
            tmp[i+1] = atoi(token);
    }

    if (shmdt(p_map) == -1)//撤销映射，detach
        perror("detach error...");

    bubble(tmp,i);//bubble排序
/*-----------------------------------------------------------------*/
    key2 = ftok(".", 2); //利用当前目录产生IPC
    if (key2 == -1){
        printf("ftok error...");
        return -1;
    }
    shm_id = shmget(key2, 4096, IPC_CREAT | 0666); //获得共享内存区域标识符ID,即shmid
    if (shm_id == -1){
        perror("shmget error...");
        return -1;
    }
    p_map2 = (int*)shmat(shm_id,NULL,0); //共享内存映射,attach

    for (i = 0; i < 10; i++){//写入共享内存
        *(p_map2 + i) = tmp[i]; //write to shared memery
    }
    getchar();

    if (shmdt(p_map2) == -1)//撤销映射，detach
        perror("detach error...");

    return 0;
}
```

<font color = blue>zhangxilong2.c:（分割线以上为写入共享内存，以下为读取排序后的共享内存）</font>

``` C
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>

int main(int argc, char **argv)
{
    key_t key;
    int shm_id,i = 0;
    char *p_map;
    int *p_map2;
    char buf[256] = "\0";
    char *token;

    key = ftok(".", 1); //利用当前目录产生IPC
    if (key == -1){
        printf("ftok error...");
        return -1;
    }
    shm_id = shmget(key, 4096, IPC_CREAT | 0666); //获得共享内存区域标识符ID,即shmid
    if (shm_id == -1){
        perror("shmget error...");
        return -1;
    }
    p_map = (char*)shmat(shm_id,NULL,0); //共享内存映射,attach

    fgets(p_map, 256, stdin);
    //*p_map = buf;   //写入共享内存

    if (shmdt(p_map) == -1)//撤消映射，detach
        perror("detach error...");

    printf("press anykey to sort...");
    getchar();
/*-----------------------------------------------------------------*/
    key = ftok(".", 2); //得到keyid(System V)
    if (key == -1)
        perror("ftok error...");
    shm_id = shmget(key, 4096, 0666); //获得共享内存区域
    if (shm_id == -1){
        perror("shmget error...");
        return -1;
    }
    p_map2 = (int *)shmat(shm_id, NULL, 0);

    while (*(p_map2 + i) != 0){
        printf("%d\t", *(p_map2 + i));
        i++;
    }
    if (shmdt(p_map2) == -1)
        perror("detach error...");

    return 0;
}
```

<font color = blue>makefile:</font>

``` C
zhangxilong1:zhangxilong1.c
	gcc zhangxilong1.c -o zhangxilong1
zhangxilong2:zhangxilong2.c
	gcc zhangxilong2.c -o zhangxilong2

clear:
	rm zhangxilong1 zhangxilong2
```
