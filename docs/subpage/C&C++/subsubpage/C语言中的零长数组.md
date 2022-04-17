# C语言中的零长数组

> **来源：https://blog.xwyam.tech:1729/index.php/archives/82/**

今早看到一位大佬的视频讲C语言中的零长数组，我也想以此为契机讲一讲自己对于零长数组的理解。以下内容都是我个人基于自己编码经历对零长数组的理解，大家仅作参考即可，如需在项目中使用，务必进行详尽自验。文中如有错误请专家大佬指正。

首先说明一下，零长数组只是我个人的称呼，是“长度为0的数组”的简称，这并不是一个标准中的概念。它和结构体的柔性数组成员的区别在文末有专门的小节说明。

## 零长数组的内存特征

零长数组通常都是用在结构体定义中。零长数组变量本身大小为0。用于结构体中时，它不会占用其他成员变量的内存空间。因此这种变量主要功能就是以一种简洁易懂的方法指定一个内存地址。

但需要注意的是，**零长数组是可以作为普通栈变量定义的**，此时它本身的大小也为0，但是在有些系统上它会挤占临近变量的内存空间的。考虑到零长数组几乎不被用于普通变量，因此本文不再赘述，有兴趣的同学可以自行探索。

一个最常见的带有零长数组的结构体定义和其内存分布如图：

``` C
struct info {
  int a1;
  int a2;
  int a3[0];
};
```

```
|<- struct info ->|
+--------+--------+
|   a1   |   a2   |
+--------+--------+
                  ^
                  a3
```

这个结构体长度依旧为8字节，而`a3`的地址则是在结构体地址范围之外了，即若结构体基地址为`0x1000`，则结构体占用内存的范围是`0x1000~0x1007`，`a1`、`a2`、`a3`的地址分别为`0x1000`、`0x1004`、`0x1008`。

当然，这个数组可以出现在结构体末尾，**也能出现在结构体中间**：

``` C
struct info {
  int a1;
  int a2[0];
  int a3;
};
```

```
|<- struct info ->|
+--------+--------+
|   a1   |   a3   |
+--------+--------+
         ^
         a2
```

此时，同样以结构体基地址`0x1000`为例，`a1`、`a2`、`a3`的地址分别为`0x1000`、`0x1004`、`0x1004`。可见此时`a2`与`a3`共用了同一个地址。

## 零长数组的典型使用场景

**零长数组最核心的功能，就是用一种很易懂的代码形式，去使用一个内存地址。**

比如现在有一个网络报文，它会有多层长度较确定的报文头和尾部变长的数据，当我想解析这些内容时，用零长数组就很方便：

``` C
char pkt_data[1500];
struct pkt {
  char mac_hdr[14];
  char ip_hdr[20];
  char udp_hdr[8];
  char l4_data[0];
};

struct pkt *p = (struct pkt *)pkt_data;
proc(p->mac_hdr, p->ip_hdr, p->udp_hdr, p->l4_data);
```

```
|<------------------------------------- pkt_data ---------------...----------------->|
+--------------+--------------------+--------+------------------...------------------+
|    mac_hdr   |       ip_hdr       | udp_hdr|                l4_data                |
+--------------+--------------------+--------+------------------...------------------+
```

当然，其实不用这个零长数组，也是可以实现同样的功能的，但是代码可读性就会大打折扣：

``` C
chst pkt_data[1500];
struct pkt {
  char mac_hdr[14];
  char ip_hdr[20];
  char udp_hdr[8];
};

struct pkt *p = (struct pkt *)pkt_data;
proc(p->mac_hdr, p->ip_hdr, p->udp_hdr, pkt_data + sizeof (struct pkt));
```

这里的`pkt_data + sizeof (struct pkt)`和上边例子中的`p->l4_data`作用是一样的，但是可读性和逻辑层次都要劣于零长数组的方式。

## 位于结构体中部的零长数组的使用场景

结构体中部的零长数组在实际应用中很少出现，因此很多人误以为零长数组只能用于结构体末尾，但是结构体中部的零长数组从技术上确实是可行的。不过需要先指明的是，**用于结构体中间的零长数组并不是一种优美的实现方式，条件允许的情况下应该通过重构或其他技术手段来实现需要的功能**。

一个具体的例子如下：当前有一系列数据，在不同场景下需要取用不同字段进行处理，场景及需要取用字段如下表所示：

|字段|场景1|场景2|场景3|
|---|---|---|---|
|a1|✓| | |
|a2|✓|✓| |
|a3|✓|✓|✓|
|a4| |✓|✓|
|a5| |✓|✓|

此时使用零长数组可以快速实现该功能：

``` C
struct data {
  int seg1_begin[0];
  int a1;
  int seg2_begin[0];
  int a2;
  int seg3_begin[0];
  int a3;
  int seg1_end[0];
  int a4;
  int a5;
  int seg2_end[0];
  int seg3_end[0];
};

struct data d;
proc(d.seg1_begin, d.seg1_end);
proc(d.seg2_begin, d.seg2_end);
proc(d.seg3_begin, d.seg3_end);
```

这里代码的复杂性是来源于需求的复杂性，结构体中间的零长数组将这个复杂性封装在结构体内部，这样可以保证调用者不易出错，同时易于排查编码错误。此处再次重申，**这并不是一个优美的实现方案，在条件允许的情况下，还是要对需求场景进行深入分析整合，然后对代码进行必要重构**。

## 零长数组和柔性数组成员

零长数组文中涉及的特性经实测，`C89`标准的编译器即可成员编译并正确运行。

`柔性数组成员(flexible array member)`则是`C99`标准中引入的特性，只能作为结构体最后一个成员，同时也不能作为普通变量使用。下面是标准中对它的说明：

> As a special case, the last element of a structure with more than one named member may have an incomplete array type; this is called a ﬂexible array member. In most situations, the ﬂexible array member is ignored. In particular, the size of the structure is as if the ﬂexible array member were omitted except that it may have more trailing padding than the omission would imply. However, when a . (or ->) operator has a left operand that is (a pointer to) a structure with a ﬂexible array member and the right operand names that member, it behaves as if that member were replaced with the longest array (with the same element type) that would not make the structure larger than the object being accessed; the offset of the array shall remain that of the ﬂexible array member, even if this would differ from that of the replacement array. If this array would have no elements, it behaves as if it had one element but the behavior is undeﬁned if any attempt is made to access that element or to generate a pointer one past it.

由此可见，这两者并不是同一个特性。我没有研究过对这两者编译的实现差异，但是我猜想柔性数组成员是零长数组增加了语法检查的语法糖。

## 关于零长数组或柔性数组成员与动态内存分配的联合使用

无论是结构体末尾的零长数组还有柔性数组成员，都是可以与动态内存分配联合使用的，这样使用有很多好处，网上也有很多示例。但是很多人说通过柔性数组的使用可以节约内存，减小内存碎片化，我认为这样的说法是不准确的。**“节约内存，减少内存碎片化”这样的好处是通过“一次分配全部内存”这样的操作获得的，柔性数组只是降低了这样做的代码复杂度**。

下面我实现两个“一次分配全部内存”的例子，都可以起到“节约内存，减少内存碎片化”的作用，只是一个利用零长数组简化代码复杂性，另一个强行实现。由代码示例可见，如果封装得当，两种实现仅在结构体定义和`MsgGetData`函数上有区别。

零长数组版：

``` C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct msg {
    int id;
    char data[0];
};

char *MsgGetData(struct msg *m)
{
    return m->data;
}

void MsgPrint(struct msg *m)
{
    printf("%d: %s\n", m->id, MsgGetData(m));
}

struct msg *MsgGen(int id, char *msg_data)
{
    int msg_len = strlen(msg_data);
    struct msg *m = (struct msg *)malloc(sizeof (struct msg) + msg_len + 1);
    m->id = id;
    memcpy(MsgGetData(m), msg_data, msg_len + 1);
    return m;
}

void MsgDestroy(struct msg *m)
{
    free(m);
}

struct msg *MsgUpdateData(struct msg *m, char *new_data)
{
    struct msg *nm = MsgGen(m->id, new_data);
    MsgDestroy(m);
    return nm;
}

int main()
{
    struct msg *m = MsgGen(5, "old msg");
    MsgPrint(m);
    m = MsgUpdateData(m, "update new msg data");
    MsgPrint(m);
    MsgDestroy(m);
    return 0;
}
```

强行实现版：

``` C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct msg {
    int id;
};

char *MsgGetData(struct msg *m)
{
    return (char *)((char *)m + sizeof (struct msg));
}

void MsgPrint(struct msg *m)
{
    printf("%d: %s\n", m->id, MsgGetData(m));
}

struct msg *MsgGen(int id, char *msg_data)
{
    int msg_len = strlen(msg_data);
    struct msg *m = (struct msg *)malloc(sizeof (struct msg) + msg_len + 1);
    m->id = id;
    memcpy(MsgGetData(m), msg_data, msg_len + 1);
    return m;
}

void MsgDestroy(struct msg *m)
{
    free(m);
}

struct msg *MsgUpdateData(struct msg *m, char *new_data)
{
    struct msg *nm = MsgGen(m->id, new_data);
    MsgDestroy(m);
    return nm;
}

int main()
{
    struct msg *m = MsgGen(5, "old msg");
    MsgPrint(m);
    m = MsgUpdateData(m, "update new msg data");
    MsgPrint(m);
    MsgDestroy(m);
    return 0;
}
```

|2022年4月17日|
|----:|
