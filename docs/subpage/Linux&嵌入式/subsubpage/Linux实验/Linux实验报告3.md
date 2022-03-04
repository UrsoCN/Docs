# Linux操作系统

<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [Linux操作系统](#linux操作系统)
  - [实验 3 Linux 多进程编](#实验-3-linux-多进程编)
    - [一、实验目的](#一实验目的)
      - [1.进一步学习 Linux 下的 C 语言编程。](#1进一步学习-linux-下的-c-语言编程)
      - [2.掌握 Linux 操作系统下进程的创建和结束操作。](#2掌握-linux-操作系统下进程的创建和结束操作)
    - [二、实验内容](#二实验内容)
      - [1.编写一个多进程程序](#1编写一个多进程程序)
      - [2.了解僵尸进程并通过一定手段解决。](#2了解僵尸进程并通过一定手段解决)
      - [3.了解守护进程并完成守护进程实验。](#3了解守护进程并完成守护进程实验)
    - [三、实验步骤](#三实验步骤)
      - [1.Linux 下多进程编程](#1linux-下多进程编程)
        - [(1)创建子进程](#1创建子进程)
        - [(2)僵尸进程与 waitpid](#2僵尸进程与-waitpid)
      - [2.守护进程](#2守护进程)
        - [(1)防止 gedit 被关闭](#1防止-gedit-被关闭)
        - [(2)守护进程(可在课后完成)](#2守护进程可在课后完成)

<!-- /code_chunk_output -->

>在正文部分，<font color = blue>蓝色的字</font>是**对问题的回答**或者**对下方图片的说明**。

<div STYLE="page-break-after: always;"></div>

## 实验 3 Linux 多进程编

### 一、实验目的

#### 1.进一步学习 Linux 下的 C 语言编程。

#### 2.掌握 Linux 操作系统下进程的创建和结束操作。

### 二、实验内容

#### 1.编写一个多进程程序

#### 2.了解僵尸进程并通过一定手段解决。

#### 3.了解守护进程并完成守护进程实验。

### 三、实验步骤

#### 1.Linux 下多进程编程

##### (1)创建子进程

&emsp;&emsp;编写多进程程序 <font color = green>myname</font>1.c(<font color = green>myname</font> 更改为自己的姓名拼音,下同),通过 makefile<font color = green>编译为 <font color = green>2016012345</font>(改为自己的学号)、运行。请将运行结果截图</font>。
&emsp;&emsp;该多进程程序需要完成如下功能:父进程循环 1000 次,每次循环执行如下操作:打印“My ID is <font color = green>2016012345</font>”(替换为自己的学号),然后利用 sleep 函数休眠 3 秒。
&emsp;&emsp;子进程利用 exec 函数调用 gedit 程序。
&emsp;&emsp;参考程序：

``` C
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
int main()
{
    pid_t pid;
    int i;
    pid = fork();
    //子进程使用一个exec函数打开gedit
    ...
    //父进程
    {
        printf("My ID is 2016012345. This is the parent process. the process ID is %d\n", getpid());
        for (i = 0; i < 1000; i++)
        {
            printf(" My ID is <font color = green>2016012345</font>. The Parent Process is running ... \n");
            sleep(3);
        }
    }
    exit(0);
}
```

<font color = blue>
下图为 编写zhangxilong1.c、makefile文件，并使用makefile编译为2018211169 的截图。
</font>

![source](./pic3/2020-12-16%2021-12-39屏幕截图.png)

<font color = blue>
下图为 运行2018211169 的截图。

可以看到在终端有文本输出（son的一行标志这子进程的运行），同时有一个gedit窗口被打开。其中，会每间隔三秒输出一句 My ID is 2018211169. The Parent Process is running ... 
</font>

![run](./pic3/2020-12-16%2021-13-35屏幕截图.png)

<font color = blue>zhangxilong1.c:</font>

``` C
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
int main(void)
{
    pid_t pid;
    int i;
    pid = fork();

    if (pid < 0) //错误
    {
        printf("error in fork...\n");
        exit(1);
    }
    if (pid == 0) //子进程使用一个exec函数打开gedit
    {
        printf("son\n");
        execlp("gedit", "exec_Test", NULL);//调用gedit
        exit(0);
    }
    if (pid > 0) //父进程
    {
        printf("My ID is 2018211169. This is the parent process. the process ID is %d\n", getpid());
        for (i = 0; i < 1000; i++)
        {
            printf(" My ID is 2018211169. The Parent Process is running ... \n");
            sleep(3);
        }
    }
    exit(0);
}
```

<font color = blue>makefile:</font>

``` makefile
2018211169:zhangxilong1.c
	gcc zhangxilong1.c -o 2018211169

clean:
	rm 2018211169
```

##### (2)僵尸进程与 waitpid

&emsp;&emsp;关闭(1)中打开的 gedit 窗口,上一程序将会出现僵尸进程。请打开另一终端,并在新终端使用 ps -u 命令查看,找到僵尸进程所在的行<font color = red>截图</font>(特别注意 STAT 列)。
&emsp;&emsp;在(1)的 源 码 基 础 上 加 上 waitpid 函 数 ( 提 示 : 在 合 适 的 位 置 使 用waitpid(pid,NULL,WNOHANG))来解决僵尸进程,得到新的程序 <font color = green>myname</font>2.c,通过makefile <font color = red>编译</font> <font color = green>myname</font>2.c,得到 <font color = green>2016012345-2 (改为自己的学号)</font>,运行,然后关闭 gedit,再次用 ps –u 命令确认无僵尸进程并<font color = green>截图</font>。实验报告中附上<font color = green>修改后的源码(文本形式,不要用图片)</font>。


<font color = blue>
下图为 在新终端中查看僵尸进程(被选中的白色框) 的截图。

可以看到在STAT列，该进程显示的是Z+，是后台的僵尸进程。
</font>

![zombie](./pic3/2020-12-15%2014-24-31屏幕截图.png)

<font color = blue>
下图为 编写zhangxilong2.c、修改makefile文件并使用make 2018211169-2编译 的截图。
</font>

![2018211169-2](./pic3/2020-12-16%2021-16-46屏幕截图.png)

<font color = blue>
下图为 运行2018211169-2 的截图。

前几个重复的输出的间隔为1秒（子进程），后面的重复输出间隔为3秒（父进程）。
</font>

![run2](./pic3/2020-12-15%2015-02-18屏幕截图.png)

<font color = blue>zhangxilong2.c:</font>

``` C
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h> 

int main(void)
{
    pid_t pid, pid_w;
    int i;
    pid = fork();

    if (pid < 0) //错误
    {
        printf("error in fork...\n");
        exit(1);
    }
    if (pid == 0) //子进程使用一个exec函数打开gedit
    {
        printf("son\n");
        execlp("gedit", "exec_Test.log", NULL); //调用gedit
        exit(0);
    }
    if (pid > 0) //父进程
    {
        do
        {
            pid_w = waitpid(pid, NULL, WNOHANG);
            if (pid_w == 0)
            {
                printf("Child process is running\n");
                sleep(1);
            }
        } while (pid_w == 0);
        if (pid_w == pid)
        {
            printf("My ID is 2018211169. This is the parent process. the process ID is %d\n", getpid());
            for (i = 0; i < 1000; i++)
            {
                printf(" My ID is 2018211169. The Parent Process is running ... \n");
                sleep(3);
            }
        }
        else
        {
            printf("Some error occurd.\n");
        }
    }
    exit(0);
}
```

#### 2.守护进程

##### (1)防止 gedit 被关闭

&emsp;&emsp;基于上述程序,进行修改,得到 <font color = green>myname</font>3.c,编译后得到 <font color = green>2016012345</font>-3,执行该程序。
&emsp;&emsp;新的程序需要达到以下目的:
&emsp;&emsp;手动关闭 gedit,父进程自动重新创建一个子进程,该子进程调用 exec 重新打开一个新的 gedit。完成该功能够请提交验收。此步骤<font color = red>无需截图</font>。
&emsp;&emsp;实验报告中附上<font color = green>修改后的源码(文本形式,不要用图片)</font>。

<font color = blue>
下图为 修改得到zhangxilong3.c，并使用make 2018211169-3编译 的截图。
</font>

![zhangxilong3](./pic3/2020-12-16%2021-24-45屏幕截图.png)

<font color = blue>
zhangxilong3.c:
</font>

``` C
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
int main(void)
{
    pid_t pid, pid_w;
    int i;
fork:
    pid = fork();

    if (pid < 0) //错误
    {
        printf("error in fork...\n");
        exit(1);
    }
    if (pid == 0) //子进程使用一个exec函数打开gedit
    {
        printf("son\n");
        execlp("gedit", "exec_Test", NULL); //调用gedit
        exit(0);
    }
    if (pid > 0) //父进程
    {
        do
        {
            pid_w = waitpid(pid, NULL, WNOHANG);
            if (pid_w == 0)
            {
                printf("Child process is running\n");
                sleep(1);
            }
        } while (pid_w == 0);
        if (pid_w == pid)
        {
            printf("My ID is 2018211169. This is the parent process. the process ID is %d\n", getpid());
            printf("Fork again...");
            goto fork;

            for (i = 0; i < 1000; i++)
            {
                printf(" My ID is 2018211169. The Parent Process is running ... \n");
                sleep(3);
            }
        }
        else
        {
            printf("Some error occurd.\n");
        }
    }
    exit(0);
}
```

##### (2)守护进程(可在课后完成)

&emsp;&emsp;上述程序中,如果将运行程序的终端关闭,父进程将退出,无法真正实现守护进程。尝试实现真正的守护进程——父进程创建子进程后退出,子进程再次创建子进程,并设置权限等参数,实现上一步骤中防止 gedit 关闭的功能。
&emsp;&emsp;请在课后完成该实验,并在实验报告中<font color = green>附上文本形式的源码</font>。

<font color = blue>
下图为 退出终端后（父进程结束） 的截图。

可以看到，即使关闭了父进程，gedit也会在被关闭之后重新启动。而我们能在任务管理器（或使用ps命令）中看到名为a.out的进程在后台运行。若要使gedit不再重复启动，可以使用kill 6349（PID）或者killall a.out关闭该进程。
</font>

![](./pic3/2020-12-16%2021-36-11屏幕截图.png)

<font color = blue>zhangxilong4.c:</font>

``` C
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
    int fd;
    char buf[] = "Daemon Test";
    
    pid_t pid;
    pid = fork();
    if (pid > 0) //父进程退出 重要！
        exit(0);
    if (setsid() == -1)
        exit(0);
    chdir("~"); //设置当前目录为用户目录
    umask(0);   //重设文件权限掩码，增加子进程灵活性
    int i;
    for (i = 0; i < getdtablesize(); ++i)
    {
        close(i); //关闭用不到的文件描述符
    }
    
    pid_t pid_w;

    fork:
    pid = fork();

    if (pid < 0) //错误
    {
        printf("error in fork...\n");
        exit(1);
    }
    if (pid == 0) //子进程使用一个exec函数打开gedit
    {
        printf("son\n");
        execlp("gedit", "exec_Test", NULL); //调用gedit
        exit(0);
    }
    if (pid > 0) //父进程
    {
        do
        {
            pid_w = waitpid(pid, NULL, WNOHANG);
            if (pid_w == 0)
            {
                printf("Child process is running\n");
                sleep(1);
            }
        } while (pid_w == 0);
        if (pid_w == pid)
        {
            printf("My ID is 2018211169. This is the parent process. the process ID is %d\n", getpid());
            printf("Fork again...");
            goto fork;

            for (i = 0; i < 1000; i++)
            {
                printf(" My ID is 2018211169. The Parent Process is running ... \n");
                sleep(3);
            }
        }
        else
        {
            printf("Some error occurd.\n");
        }
    }
    exit(0);

}
```
