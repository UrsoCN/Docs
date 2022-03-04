# Linux操作系统

<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [Linux操作系统](#linux操作系统)
  - [作业3](#作业3)
    - [文件 IO 编程:](#文件-io-编程)
    - [作业要求:](#作业要求)
    - [附：源码](#附源码)

<!-- /code_chunk_output -->

>在正文部分，<font color = blue>蓝色的字</font>是**对问题的回答**或者**对下方图片的说明**。

<div STYLE="page-break-after: always;"></div>

## 作业3

### 文件 IO 编程:

&emsp;&emsp;编写 C 程序 <font color = green>2018211456</font>.c(其中 <font color = green>2018211456</font> 为自己的学号,下同),该程序的功能为:打开文件 <font color = green>2018211456</font>.txt (若不存在则创建该文件),向里面写入“hello,My name is zhangsan”(zhangsan 改为自己的姓名拼音)。

&emsp;&emsp;编写该程序相应的 makefile,生成的可执行程序为 <font color = green>2018211456</font>。

### 作业要求:

&emsp;&emsp;1.在作业末尾附上源码与 makefile 内容;&emsp;&emsp;[附：源码](#附源码)

<font color = blue>
下图为 编辑2018211169.c与makefile文件 的截图。
</font>

![gedit](./pic3/2020-12-11%2021-37-54屏幕截图.png)

&emsp;&emsp;2.使用 make 进行编译,将执行 make 的过程和结果截图;

<font color = blue>
下图为 使用make编译2018211169.c源文件 的截图。
</font>

![make](./pic3/2020-12-11%2021-40-57屏幕截图.png)

&emsp;&emsp;3.通过 cat 查看 <font color = green>2018211456</font>.txt 中的内容,截图展示 cat 命令的执行过程与执行结果

<font color = blue>
下图为 通过 cat 查看 <font color = green>2018211456</font>.txt 中的内容 的截图。

可以看到当前目录下是没有2018211169.txt文件的
</font>

![cat1](./pic3/2020-12-11%2021-46-29屏幕截图.png)

&emsp;&emsp;4.运行 <font color = green>2018211456</font>,并将运行的过程和结果截图。

<font color = blue>
下图为 运行2018211169 的截图。
</font>

![2018211169](./pic3/2020-12-11%2021-42-53屏幕截图.png)

&emsp;&emsp;5.再次通过 cat 查看 <font color = green>2018211456</font>.txt 中的内容,截图展示 cat 命令的执行过程与执行结果。

<font color = blue>
下图为 再次通过cat查看2018211169.txt中内容 的截图。

可以看到通过cat预览2018211169.txt中内容需要额外权限（这里通过sudo使用管理员权限执行）
</font>

![cat2](./pic3/2020-12-11%2021-44-59屏幕截图.png)

### 附：源码

2018211169.c:

``` C

#include <stdio.h>
#include <fcntl.h>  //声明调用open()时使用的flags常量
#include <sys/types.h>
#include <sys/stat.h>	//声明mode_t
#include <unistd.h>	//声明ssize_t


int main(void)
{
    int fd,size;
    char s[] = "hello, My name is zhangxilong";
    
    fd = open("2018211169.txt",O_RDWR|O_CREAT);//成功返回u文件描述符，出错返回-1
    if(-1 == fd){
        printf("Open or creat file named \"2018211169.txt\" failed. (sudo?)");
        return -1;
    }
    write(fd,s,sizeof(s));  //向该文件中写入字符串
    close(fd);  //关闭文件描述符
    return 0;
}

```

makefile:

``` makefile
2018211169:2018211169.c
	gcc 2018211169.c -o 2018211169

clean:
	rm 2018211169 2018211169.txt
```
