# Linux操作系统

<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [Linux操作系统](#linux操作系统)
  - [作业2](#作业2)
    - [1、写一个冒泡排序的 C(或 C++)程序,输入一些数字(若有必要,可以在输入数字之前输入数字的数量),输出由小到大排序后的结果;排序函数放在bubble.c 文件中,主函数放在 zhangsan.c 中(文件名改为自己的姓名);](#1写一个冒泡排序的-c或-c程序输入一些数字若有必要可以在输入数字之前输入数字的数量输出由小到大排序后的结果排序函数放在bubblec-文件中主函数放在-zhangsanc-中文件名改为自己的姓名)
    - [2、对源代码分别进行编译、链接,要求把编译过程(可与汇编过程一起完成)、链接过程分开处理,得到的可执行文件必须以学号命名;](#2对源代码分别进行编译链接要求把编译过程可与汇编过程一起完成链接过程分开处理得到的可执行文件必须以学号命名)
    - [3、运行可执行程序,验证运行结果是否正确;](#3运行可执行程序验证运行结果是否正确)
    - [4、写出相应的 makefile,要求将链接过程与编译汇编分开,添加伪目标 clean,使得输入 make clean 可以删除.o 文件和最终的可执行文件,输入 make 可编译得到可执行程序;](#4写出相应的-makefile要求将链接过程与编译汇编分开添加伪目标-clean使得输入-make-clean-可以删除o-文件和最终的可执行文件输入-make-可编译得到可执行程序)
    - [5、分别输入 make clean、make,验证 makefile 的有效性。](#5分别输入-make-cleanmake验证-makefile-的有效性)
  - [作业要求:](#作业要求)
    - [1、作业末尾附上程序源码、makefile;](#1作业末尾附上程序源码makefile)
    - [2、编译、链接、运行过程的完整截图,包括输入的数字(输入的数字中,必须有一个数为自己学号的后 3 位)和所显示的排序结果;](#2编译链接运行过程的完整截图包括输入的数字输入的数字中必须有一个数为自己学号的后-3-位和所显示的排序结果)
    - [3、执行 make clean、make 命令的完整截图,截图中需要包含输入的命令以及命令执行后显示的全部结果。](#3执行-make-cleanmake-命令的完整截图截图中需要包含输入的命令以及命令执行后显示的全部结果)
  - [附：源码](#附源码)

<!-- /code_chunk_output -->

>在正文部分，<font color = blue>蓝色的字</font>是**对问题的回答**或者**对下方图片的说明**。

<div STYLE="page-break-after: always;"></div>

## 作业2

### 1、写一个冒泡排序的 C(或 C++)程序,输入一些数字(若有必要,可以在输入数字之前输入数字的数量),输出由小到大排序后的结果;排序函数放在bubble.c 文件中,主函数放在 zhangsan.c 中(文件名改为自己的姓名);

<font color = blue>
下图为 编写的.c文件及其目录 的截图，程序源码放在末尾。
</font>

![ls](./pic2/2020-12-03%2017-41-35屏幕截图.png)

### 2、对源代码分别进行编译、链接,要求把编译过程(可与汇编过程一起完成)、链接过程分开处理,得到的可执行文件必须以学号命名;

<font color = blue>
下图为 编译、链接.c文件，并得到可执行文件2018211169 的截图。
</font>

![gcc](./pic2/2020-12-04%2021-44-48屏幕截图.png)

### 3、运行可执行程序,验证运行结果是否正确;

<font color = blue>
下图为 运行zhangxilong 的截图。

输入一组数[1  8 4 3 6 2 11  0]（用tab分隔开，其中最后的0是作为输入结束的标志符）。
输出[1  2 3 4 6 8 11]，成功将输入的一组数排序后输出。
</font>

![zhangxilong](./pic2/2020-12-04%2021-45-58屏幕截图.png)

### 4、写出相应的 makefile,要求将链接过程与编译汇编分开,添加伪目标 clean,使得输入 make clean 可以删除.o 文件和最终的可执行文件,输入 make 可编译得到可执行程序;

<font color = blue>
下图为 编写的makefile文件 的截图。
</font>

![makefile](./pic2/2020-12-04%2021-57-59屏幕截图.png)

### 5、分别输入 make clean、make,验证 makefile 的有效性。

<font color = blue>
下图为 编写makefile文件并测试make clean和make命令效果 的截图。

可以看到，make clean即rm *.o zhangxilong可以清理gcc生成的文件；make又可以将源文件重新编译、链接成.o和可执行文件。
</font>

![makefile2](./pic2/2020-12-04%2021-59-42屏幕截图.png)

## 作业要求:

### 1、作业末尾附上程序源码、makefile;

### 2、编译、链接、运行过程的完整截图,包括输入的数字(输入的数字中,必须有一个数为自己学号的后 3 位)和所显示的排序结果;

### 3、执行 make clean、make 命令的完整截图,截图中需要包含输入的命令以及命令执行后显示的全部结果。

## 附：源码

bubble.c:

``` C
void bubble(int arr[],int len)
{
    int i, j, temp;
    for (i = 0; i < len - 1; i++)
        for (j = 0; j < len - 1 - i; j++)
            if (arr[j] > arr[j + 1])
            {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
}
```

zhangxilong.c:

``` C
#include <stdio.h>

void bubble(int arr[],int len);

int main()
{
    int arr[10];
    int ele;
    int i = 0;
    printf("Please enter some number...(end with \'0\')\n");
    scanf("%d", &ele);
    while (ele)
    {
        arr[i] = ele;
        i++;
        scanf("%d", &ele);
    }

    bubble(arr, i);

    for (int ii = 0; ii < i; ii++)
    {
        printf("%d\t", arr[ii]);
    }

    return 0;
}
```

makefile:

``` makefile
2018211169:zhangxilong.o bubble.o
	gcc zhangxilong.o bubble.o -o 2018211169
zhangxilong.o:zhangxilong.c
	gcc -c zhangxilong.c -o zhangxilong.o
bubble.o:bubble.c
	gcc -c bubble.c -o bubble.o

clean:
	rm *.o 2018211169
```
