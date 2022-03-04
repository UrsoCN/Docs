# Linux操作系统

<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [Linux操作系统](#linux操作系统)
  - [实验2 Linux下C语言开发环境](#实验2-linux下c语言开发环境)
    - [一、实验目的](#一实验目的)
    - [二、实验内容](#二实验内容)
    - [三、实验步骤](#三实验步骤)
      - [1.Linux 下 C 语言开发流程](#1linux-下-c-语言开发流程)
      - [2.GDB基本命令的使用](#2gdb基本命令的使用)
      - [3.Make 工程管理器的使用 1。](#3make-工程管理器的使用-1)
      - [4.Make 工程管理器的使用 2。](#4make-工程管理器的使用-2)

<!-- /code_chunk_output -->

>在正文部分，<font color = blue>蓝色的字</font>是**对问题的回答**或者**对下方图片的说明**。

<div STYLE="page-break-after: always;"></div>

## 实验2 Linux下C语言开发环境

### 一、实验目的

1.熟悉 Linux 下的 C 语言开发环境。
2.掌握 vi、gcc、gdb、makefile 的使用。
3.掌握 Linux 下 C 语言程序设计流程。

### 二、实验内容

1.Linux 下 C 语言开发流程。
2.GCC 编译器的使用。
3.GDB 调试器的使用。
4.Make 工程管理器的使用。

### 三、实验步骤

#### 1.Linux 下 C 语言开发流程

(1) 启动终端,进入/tmp/<font color = green><font color = green>2016012345</font></font> 目录(<font color = green><font color = green>2016012345</font></font> 为你的学号,下同。若不存在该目录,请先创建)。

(2) 在/tmp/<font color = green><font color = green>2016012345</font></font> 目录创建“hello”文件夹并进入该目录,使用 vim 或者 vi编辑源程序,在终端中输入 vi hello.c,进入 vi,熟悉 vi 的三种模式之间的切换。
输入如下源代码,编辑完成后存盘。

``` C
#include<stdio.h>
int main()
{
    printf("Hello World! My Student ID is 2018211169 \n");
    return 0;
}
```

(3)使用 gcc 编译源代码,得到可执行程序 <font color = green><font color = green>2016012345</font></font>-1。
(4)在终端中执行 <font color = green><font color = green>2016012345</font></font>-1 程序并<font color = green>截图</font>(截图必须包含 shell 命令和得到的结果,下同)。

<font color = blue>
下图为 进入/tmp/2018211169目录，创建hello文件夹并进入该目录，用vim编辑hello.c源文件 的截图。
</font>

![vim](./pic2/2020-12-08%2014-22-27屏幕截图.png)

<font color = blue>
下图为 用vim编辑hello.c文件并保存、退出vim编辑器 的截图。
</font>

![hello.c](./pic2/2020-12-08%2014-17-32屏幕截图.png)

<font color = blue>
下图为 使用gcc编译源代码并生成可执行程序2018211169-1 的截图。
</font>

![2018211169-1](./pic2/2020-12-08%2014-25-05屏幕截图.png)

<font color = blue>
下图为 在终端执行2018211169-1程序 的截图。
</font>

![hello](./pic2/2020-12-08%2014-27-36屏幕截图.png)

#### 2.GDB基本命令的使用

&emsp;&emsp;在/tmp/<font color = green><font color = green>2016012345</font></font>/目录创建“gdb”文件夹并进入该目录。使用 vi 编辑源程序,
&emsp;&emsp;在终端中输入 vi test.c,输入如下源代码,编辑完成后存盘。此代码的功能为输出字符串string[]的倒序字符串,但结果没有输出显示,现通过调试的方式来解决程序中存在的问题。
&emsp;&emsp;程序源代码如下(注意:由于这里的代码是在 windows 输入的,若复制代码,可能会有些特殊字符导致出错,遇到这类问题,请将出错位置的回车删除后重新敲入回车):

``` C
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int display1 (char *string);
int display2 (char *string1);
int main ()
{
    char string[] = "<font color = green>2016012345</font>";//必须换成自己的学号
    display1 (string);
    display2 (string);
}
int display1 (char *string)
{
    printf ("Original string is %s \n", string);
}
int display2 (char *string1)
{
    char *string2;
    int size,i;
    size = strlen (string1);
    string2 = (char *) malloc (size + 1);
    for (i = 0; i < size; i++)
    string2[size - i] = string1[i];
    string2[size+1] = ' ';
    printf("The string afterward is %s\n",string2);
}
```

(1)用 gcc 编译:gcc -g test.c -o <font color = green>2016012345</font>-2。
(2)运行 <font color = green>2016012345</font>-2,输出为:
&emsp;&emsp;Original string is <font color = green>2016012345</font>
&emsp;&emsp;The string afterward is
&emsp;&emsp;可见,该程序没有能够倒序输出,需要调试找出问题并修改。<font color = red>请将下面的调试过程截图(截几张整体的图即可,无需每一步单独截图,但图的大小要适中,打印后必须能看清里面的字)并放入实验报告。</font>
(3)启动 Gdb 调试:gdb <font color = green>2016012345</font>-2
➢ 查看源代码:l
➢ 在 22 行(for 循环处)设置断点:b 22
➢ 在 25 行(printf 函数处)设置断点:b 25。
➢ 查看断点设置情况:info b
➢ 运行代码:r
➢ 单步运行代码:n
➢ 查看暂停点变量值:p i
➢ p string1[i]
➢ p string2[size - i]
➢ 继续单步运行代码数次,并使用命令查看,看看是否能<font color = red>找出程序的问题</font>
➢ 继续程序的运行:c
➢ 程序在 printf 前停止运行,依次查看 string2[0]、string2[1]...,看看 string2 是否被正确赋值。
➢ <font color = red>找出程序的问题</font>后(<font color = red>实验报告中需要说明这一问题</font>),退出 Gdb:q
➢ 重新编辑 test.c,修改找出的问题。
&emsp;&emsp;使用 gcc 重新编译,查看运行结果:./ <font color = green>2016012345</font>-2
&emsp;&emsp;Original string is <font color = green>2016012345</font>
&emsp;&emsp;The string afterward is <font color = green>5432106102</font>
若问题已修改,可输出正确结果。若输出还是不对,请继续调试。

<font color = blue>
下图为 编译并运行2018211169-2 的截图。

可以看到学号（原字符串）正常输出了，但其倒序字符串没有输出在终端。
</font>

![2018211169-2](./pic2/2020-12-08%2014-44-26屏幕截图.png)

<font color = blue>
下图为 启动GDB调试 的截图。
</font>

![GBD](./pic2/2020-12-08%2014-47-36屏幕截图.png)

<font color = blue>
下图为 在GDB中使用l命令查看源代码 的截图。
</font>

![l](./pic2/2020-12-08%2015-09-03屏幕截图.png)

<font color = blue>
下图为 在GDB中设置并查看断点 的截图。
</font>

![b](./pic2/2020-12-08%2015-09-49屏幕截图.png)

<font color = blue>
下图为 在GDB中设置断点后运行代码 的截图。
</font>

![r](./pic2/2020-12-08%2015-10-33屏幕截图.png)

<font color = blue>
下图为 单步运行后查看暂停点变量值 的截图。（这里使用的是p命令。但在后面，为了方便，使用的是display命令）
</font>

![n](./pic2/2020-12-08%2015-11-20屏幕截图.png)

<font color = blue>
下图为 断点1处的最后一次循环 的截图。

该程序的错误就是出自这里，这里错误地将string1的结尾的终止符赋值给了string2[0]，使得string2在开始时就结束了，其后面所赋的值没法被读取出来。
</font>

![string2[0]](./pic2/2020-12-08%2015-27-04屏幕截图.png)

<font color = blue>

就整体而言，string1中的10个字符被赋在了string2的[1~10]，而string2[0]被赋值为终止符。所以说，要将代码第23行改为：
>string2[size - i -1] = string1[i];
</font>

<font color = blue>
下图为 修正代码后运行2018211169-2 的截图。
</font>

![afterward](./pic2/2020-12-08%2021-36-06屏幕截图.png)

<font color = blue>
可以看到，string1成功被倒序输出。
</font>

#### 3.Make 工程管理器的使用 1。

&emsp;&emsp;(1)编写 Makefile 文件。
&emsp;&emsp;在/tmp/<font color = green>2016012345</font>/hello 目录下用 vi 编辑 makefile 文件(makefile 内容请自行完成),
利用该 makefile,将 hello.c 编译为 <font color = green>2016012345</font>-3 可执行文件。
&emsp;&emsp;(2)使用 make 编译项目。执行 make,用 ls -l 查看并记录所生成的文件,然后运行得到的 <font color = green>2016012345</font>-3 文件。将<font color = red>makefile 文件和 执行 make 命令的过程截图</font>

<font color = blue>
下图为 编写的 makefile 文件 的截图。
</font>

![makefile](./pic2/2020-12-08%2015-57-31屏幕截图.png)

<font color = blue>
下图为 执行make命令 的截图。
</font>

![make](./pic2/2020-12-08%2016-00-46屏幕截图.png)


#### 4.Make 工程管理器的使用 2。

&emsp;&emsp;(1) 将 /tmp/<font color = green>2016012345</font>/gdb 目 录 下 的 文 件 复 制 到 /tmp/<font color = green>2016012345</font>/gdb2 。 在/tmp/<font color = green>2016012345</font>/gdb2 中调整 gdb 实验的源码,将正确的 display1 函数和 display2 函数放入新的文件 display.c,main 函数放入 main.c。(若有必要,可新增 display.h 文件,将display1、display2 函数的申明放入.h 文件,并在 main.c 中引用 display.h)
&emsp;&emsp;(2)将 main.c、display.c 分别编译为 main.o、display.o(注意,此时的 gcc 要加入-c 参数,不进行链接)。
&emsp;&emsp;(3)将两个.o 文件链接为 <font color = green>2016012345</font>-4,执行该文件。请将上述两个步骤<font color = red>编译命令和编译结果截图</font>。
&emsp;&emsp;(4)写出完整的 makefile,通过 make clean 删除两个.o 文件和 <font color = green>2016012345</font>-5、通过make 得到可执行程序(得到的可执行文件为 <font color = green>2016012345</font>-5)。
&emsp;&emsp;对 <font color = green>makefile 的内容、make 执行的过程和结果</font><font color = red>截图</font>。

<font color = blue>
main.c:

``` C
#include <stdlib.h>
int display1 (char *string);
int display2 (char *string1);
int main ()
{
    char string[] = "2018211169";//必须换成自己的学号
    display1 (string);
    display2 (string);
}
```

display.c:

``` C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int display1 (char *string)
{
    printf ("Original string is %s \n", string);
}
int display2 (char *string1)
{
    char *string2;
    int size,i;
    size = strlen(string1);
    string2 = (char *)malloc(size + 1);
    for(i = 0; i < size; i++)
    	string2[size - i -1] = string1[i];
    printf("The string afterward is %s\n",string2);
}
```

</font>

<font color = blue>
下图是 将 main.c、display.c 分别编译为 main.o、display.o 的截图。
</font>

![-c](./pic2/2020-12-08%2022-00-49屏幕截图.png)

<font color = blue>
下图是 将两个.o 文件链接为 <font color = green>2016012345</font>-4,并执行该文件 的截图。
</font>

![gcc](./pic2/2020-12-08%2022-05-04屏幕截图.png)

makefile:
``` makefile
2018211169-5:main.o display.o
	gcc main.o display.o -o 2018211169-5
main.o:main.c
	gcc -c main.c
display.o:display.c
	gcc -c display.c

clean:
	rm *.o 2018211169-5
```

<font color = blue>
下图是 编写 makefile 文件并依次运行make clean、make命令 的截图。
</font>

![clean](./pic2/2020-12-08%2022-21-22屏幕截图.png)

<font color = blue>
可以看到make clean命令可以删除编译生成的*.o 和2018211169-5文件。make命令可以编译生成*.o 和2018211169-5文件。
</font>
