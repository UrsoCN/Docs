# Understanding lvalues and rvalues in C and C++ 理解C和C++中的左值和右值

> **[https://eli.thegreenplace.net/2011/12/15/understanding-lvalues-and-rvalues-in-c-and-c/](https://eli.thegreenplace.net/2011/12/15/understanding-lvalues-and-rvalues-in-c-and-c/)**
> **[https://blog.csdn.net/xuwqiang1994/article/details/79924310](https://blog.csdn.net/xuwqiang1994/article/details/79924310)**  

The terms lvalue and rvalue are not something one runs into often in C/C++ programming, but when one does, it's usually not immediately clear what they mean. The most common place to run into these terms are in compiler error & warning messages. For example, compiling the following with gcc:

术语lvalue和rvalue在 C/C++ 编程中并不经常遇到，但是当人们遇到时，通常不会立即清楚它们的含义。遇到这些术语最常见的地方是编译器错误和警告消息。例如，使用gcc编译以下内容：

``` C++
int foo() {return 2;}

int main()
{
    foo() = 2;

    return 0;
}
```

You get:

```
test.c: In function 'main':
test.c:8:5: error: lvalue required as left operand of assignment
```

True, this code is somewhat perverse and not something you'd write, but the error message mentions lvalue, which is not a term one usually finds in C/C++ tutorials. Another example is compiling this code with g++:

诚然，这段代码有点反常，不是你要写的东西，但错误消息提到lvalue，这不是 C/C++ 教程中常见的术语。另一个例子是用g++编译这段代码：

``` C++
int& foo()
{
    return 2;
}
```

Now the error is:

```
testcpp.cpp: In function 'int& foo()':
testcpp.cpp:5:12: error: invalid initialization of non-const reference
of type 'int&' from an rvalue of type 'int'
```

Here again, the error mentions some mysterious rvalue. So what do lvalue and rvalue mean in C and C++? This is what I intend to explore in this article.

在这里，错误再次提到了一些神秘的rvalue。那么左值和右值在 C 和 C++ 中是什么意思呢？这就是我打算在本文中探讨的内容。

## A simple definition

This section presents an intentionally simplified definition of lvalues and rvalues. The rest of the article will elaborate on this definition.

An lvalue (locator value) represents an object that occupies some identifiable location in memory (i.e. has an address).

rvalues are defined by exclusion, by saying that every expression is either an lvalue or an rvalue. Therefore, from the above definition of lvalue, an rvalue is an expression that does not represent an object occupying some identifiable location in memory.

本节介绍了一个有意简化的lvalues和rvalues定义。本文的其余部分将详细说明此定义。

左值（定位器值）表示占据内存中某个可识别位置（即具有地址）的对象。

右值是通过排除定义的，即每个表达式都是左值或右值。因此，从上述lvalue的定义来看，rvalue是一个表达式，它不代表占据内存中某个可识别位置的对象。

## Basic examples

The terms as defined above may appear vague, which is why it's important to see some simple examples right away.

Let's assume we have an integer variable defined and assigned to:

上面定义的术语可能看起来很模糊，这就是为什么立即查看一些简单示例很重要的原因。

假设我们定义了一个整数变量并将其分配给：

```C++
int var;
var = 4;
```

An assignment expects an lvalue as its left operand, and var is an lvalue, because it is an object with an identifiable memory location. On the other hand, the following are invalid:

赋值需要一个左值作为其左操作数，而var是一个左值，因为它是一个具有可识别内存位置的对象。另一方面，以下是无效的：

``` C++
4 = var;       // ERROR!
(var + 1) = 4; // ERROR!
```

Neither the constant 4, nor the expression var + 1 are lvalues (which makes them rvalues). They're not lvalues because both are temporary results of expressions, which don't have an identifiable memory location (i.e. they can just reside in some temporary register for the duration of the computation). Therefore, assigning to them makes no semantic sense - there's nowhere to assign to.

So it should now be clear what the error message in the first code snippet means. foo returns a temporary value which is an rvalue. Attempting to assign to it is an error, so when seeing foo() = 2; the compiler complains that it expected to see an lvalue on the left-hand-side of the assignment statement.

Not all assignments to results of function calls are invalid, however. For example, C++ references make this possible:

常量4和表达式var + 1都不是左值（这使它们成为右值）。它们不是左值，因为它们都是表达式的临时结果，它们没有可识别的内存位置（即它们可以在计算期间驻留在某个临时寄存器中）。因此，分配给它们没有语义意义 - 无处可分配。

所以现在应该清楚第一个代码片段中的错误消息是什么意思了。foo返回一个临时值，它是一个右值。试图分配给它是一个错误，所以当看到foo() = 2; 编译器抱怨它希望在赋值语句的左侧看到一个左值。

然而，并非所有对函数调用结果的赋值都是无效的。例如，C++ 引用使这成为可能：

``` C++
int globalvar = 20;

int& foo()
{
    return globalvar;
}

int main()
{
    foo() = 10;
    return 0;
}
```

Here foo returns a reference, which is an lvalue, so it can be assigned to. Actually, the ability of C++ to return lvalues from functions is important for implementing some overloaded operators. One common example is overloading the brackets operator [] in classes that implement some kind of lookup access. std::map does this:

这里foo返回一个引用，它是一个左值，因此可以分配给它。实际上，C++ 从函数返回左值的能力对于实现一些重载运算符很重要。一个常见的例子是在实现某种查找访问的类中重载方括号运算符[] 。std::map这样做：

``` C++
std::map<int, float> mymap;
mymap[10] = 5.6;
```

The assignment mymap[10] works because the non-const overload of std::map::operator[] returns a reference that can be assigned to.

赋值mymap[10]有效，因为std::map::operator[]的非常量重载返回一个可以赋值的引用。

## Modifiable lvalues

Initially when lvalues were defined for C, it literally meant "values suitable for left-hand-side of assignment". Later, however, when ISO C added the const keyword, this definition had to be refined. After all:

最初，当为 C 定义左值时，它的字面意思是“适合赋值左侧的值”。然而，后来，当 ISO C 添加了const关键字时，这个定义不得不被细化。毕竟：

``` C++
const int a = 10; // 'a' is an lvalue
a = 10;           // but it can't be assigned!
```

So a further refinement had to be added. Not all lvalues can be assigned to. Those that can are called modifiable lvalues. Formally, the C99 standard defines modifiable lvalues as:

因此，必须添加进一步的细化。并非所有左值都可以分配给。那些可以称为可修改的左值。形式上，C99 标准将可修改的左值定义为：

> [...] an lvalue that does not have array type, does not have an incomplete type, does not have a const-qualified type, and if it is a structure or union, does not have any member (including, recursively, any member or element of all contained aggregates or unions) with a const-qualified type.

## Conversions between lvalues and rvalues

Generally speaking, language constructs operating on object values require rvalues as arguments. For example, the binary addition operator '+' takes two rvalues as arguments and returns an rvalue:

一般来说，对对象值进行操作的语言结构需要右值作为参数。例如，二元加法运算符'+'将两个右值作为参数并返回一个右值：

``` C++
int a = 1;     // a is an lvalue
int b = 2;     // b is an lvalue
int c = a + b; // + needs rvalues, so a and b are converted to rvalues
               // and an rvalue is returned
```

As we've seen earlier, a and b are both lvalues. Therefore, in the third line, they undergo an implicit lvalue-to-rvalue conversion. All lvalues that aren't arrays, functions or of incomplete types can be converted thus to rvalues.

What about the other direction? Can rvalues be converted to lvalues? Of course not! This would violate the very nature of an lvalue according to its definition ^[rvalues can be assigned to lvalues explicitly. The lack of implicit conversion means that rvalues cannot be used in places where lvalues are expected.].

This doesn't mean that lvalues can't be produced from rvalues by more explicit means. For example, the unary '*' (dereference) operator takes an rvalue argument but produces an lvalue as a result. Consider this valid code:

正如我们之前看到的，a和b都是左值。因此，在第三行中，它们经历了隐式的左值到右值的转换。所有不是数组、函数或不完整类型的左值都可以转换为右值。

另一个方向呢？右值可以转换为左值吗？当然不是！根据其定义[1]，这将违反左值的本质。

这并不意味着不能通过更明确的方式从右值生成左值。例如，一元'*'（取消引用）运算符接受一个右值参数，但生成一个左值作为结果。考虑这个有效的代码：

``` C++
int arr[] = {1, 2};
int* p = &arr[0];
*(p + 1) = 10;   // OK: p + 1 is an rvalue, but *(p + 1) is an lvalue
```

Conversely, the unary address-of operator '&' takes an lvalue argument and produces an rvalue:

相反，一元地址运算符'&'接受一个左值参数并产生一个右值：

``` C++
int var = 10;
int* bad_addr = &(var + 1); // ERROR: lvalue required as unary '&' operand
int* addr = &var;           // OK: var is an lvalue
&var = 40;                  // ERROR: lvalue required as left operand
                            // of assignment
```

The ampersand plays another role in C++ - it allows to define reference types. These are called "lvalue references". Non-const lvalue references cannot be assigned rvalues, since that would require an invalid rvalue-to-lvalue conversion:

& 符号在 C++ 中扮演另一个角色——它允许定义引用类型。这些被称为“左值引用”。不能为非 const 左值引用分配右值，因为这需要无效的右值到左值转换：

``` C++
std::string& sref = std::string();  // ERROR: invalid initialization of
                                    // non-const reference of type
                                    // 'std::string&' from an rvalue of
                                    // type 'std::string'
```

Constant lvalue references can be assigned rvalues. Since they're constant, the value can't be modified through the reference and hence there's no problem of modifying an rvalue. This makes possible the very common C++ idiom of accepting values by constant references into functions, which avoids unnecessary copying and construction of temporary objects.

可以为常量左值引用分配右值。由于它们是常量，因此无法通过引用修改值，因此修改右值没有问题。这使得非常常见的 C++ 习惯用法成为可能，即通过对函数的常量引用来接受值，从而避免了不必要的复制和临时对象的构造。

## CV-qualified rvalues

If we read carefully the portion of the C++ standard discussing lvalue-to-rvalue conversions ^[That's section 4.1 in the new C++11 standard draft.], we notice it says:

如果我们仔细阅读 C++ 标准中讨论左值到右值转换[2]的部分，我们会注意到它说：

> An lvalue (3.10) of a non-function, non-array type T can be converted to an rvalue. [...] If T is a non-class type, the type of the rvalue is the cv-unqualified version of T. Otherwise, the type of the rvalue is T.

What is this "cv-unqualified" thing? CV-qualifier is a term used to describe const and volatile type qualifiers.

From section 3.9.3:

这是什么“cv不合格”的东西？CV 限定符是用于描述const和volatile类型限定符的术语。

从第 3.9.3 节开始：

> Each type which is a cv-unqualified complete or incomplete object type or is void (3.9) has three corresponding cv-qualified versions of its type: a const-qualified version, a volatile-qualified version, and a const-volatile-qualified version. [...] The cv-qualified or cv-unqualified versions of a type are distinct types; however, they shall have the same representation and alignment requirements (3.9)

But what has this got to do with rvalues? Well, in C, rvalues never have cv-qualified types. Only lvalues do. In C++, on the other hand, class rvalues can have cv-qualified types, but built-in types (like int) can't. Consider this example:

但这与右值有什么关系？好吧，在 C 中，右值永远不会有 cv 限定类型。只有左值可以。另一方面，在 C++ 中，类右值可以具有 cv 限定类型，但内置类型（如int）不能。考虑这个例子：

``` C++
#include <iostream>

class A {
public:
    void foo() const { std::cout << "A::foo() const\n"; }
    void foo() { std::cout << "A::foo()\n"; }
};

A bar() { return A(); }
const A cbar() { return A(); }


int main()
{
    bar().foo();  // calls foo
    cbar().foo(); // calls foo const
}
```

The second call in main actually calls the foo () const method of A, because the type returned by cbar is const A, which is distinct from A. This is exactly what's meant by the last sentence in the quote mentioned earlier. Note also that the return value from cbar is an rvalue. So this is an example of a cv-qualified rvalue in action.

main中的第二个调用实际上调用了A的foo () const方法，因为cbar返回的类型是const A，这与A不同。这正是前面提到的引文中最后一句话的意思。另请注意，来自cbar的返回值是一个右值。所以这是一个 cv 限定右值的例子。

## Rvalue references (C++11)

Rvalue references and the related concept of move semantics is one of the most powerful new features the C++11 standard introduces to the language. A full discussion of the feature is way beyond the scope of this humble article ^[You can find a lot of material on this topic by simply googling "rvalue references". Some resources I personally found useful: [this one](http://www.artima.com/cppsource/rvalue.html), [and this one](http://stackoverflow.com/questions/5481539/what-does-t-mean-in-c0x), [and especially this one](http://thbecker.net/articles/rvalue_references/section_01.html).], but I still want to provide a simple example, because I think it's a good place to demonstrate how an understanding of what lvalues and rvalues are aids our ability to reason about non-trivial language concepts.

I've just spent a good part of this article explaining that one of the main differences between lvalues and rvalues is that lvalues can be modified, and rvalues can't. Well, C++11 adds a crucial twist to this distinction, by allowing us to have references to rvalues and thus modify them, in some special circumstances.

As an example, consider a simplistic implementation of a dynamic "integer vector". I'm showing just the relevant methods here:

右值引用和移动语义的相关概念是 C++11 标准引入该语言的最强大的新特性之一。对该功能的完整讨论超出了这篇不起眼的文章[3]的范围，但我仍然想提供一个简单的例子，因为我认为这是一个很好的地方来展示对什么是左值和右值的理解如何帮助我们的能力推理非平凡的语言概念。

我刚刚花了这篇文章的大部分内容来解释左值和右值之间的主要区别之一是左值可以修改，而右值不能。好吧，C++11 为这种区别增加了一个关键的转折，允许我们在某些特殊情况下引用右值并因此修改它们。

例如，考虑动态“整数向量”的简单实现。我在这里只展示相关的方法：

``` C++
class Intvec
{
public:
    explicit Intvec(size_t num = 0)
        : m_size(num), m_data(new int[m_size])
    {
        log("constructor");
    }

    ~Intvec()
    {
        log("destructor");
        if (m_data) {
            delete[] m_data;
            m_data = 0;
        }
    }

    Intvec(const Intvec& other)
        : m_size(other.m_size), m_data(new int[m_size])
    {
        log("copy constructor");
        for (size_t i = 0; i < m_size; ++i)
            m_data[i] = other.m_data[i];
    }

    Intvec& operator=(const Intvec& other)
    {
        log("copy assignment operator");
        Intvec tmp(other);
        std::swap(m_size, tmp.m_size);
        std::swap(m_data, tmp.m_data);
        return *this;
    }
private:
    void log(const char* msg)
    {
        cout << "[" << this << "] " << msg << "\n";
    }

    size_t m_size;
    int* m_data;
};
```

So, we have the usual constructor, destructor, copy constructor and copy assignment operator ^[This a canonical implementation of a copy assignment operator, from the point of view of exception safety. By using the copy constructor and then the non-throwing std::swap, it makes sure that no intermediate state with uninitialized memory can arise if exceptions are thrown.] defined, all using a logging function to let us know when they're actually called.

Let's run some simple code, which copies the contents of v1 into v2:

因此，我们定义了通常的构造函数、析构函数、复制构造函数和复制赋值运算符[4]，它们都使用日志记录函数来让我们知道它们何时被实际调用。

让我们运行一些简单的代码，将v1的内容复制到v2中：

``` C++
Intvec v1(20);
Intvec v2;

cout << "assigning lvalue...\n";
v2 = v1;
cout << "ended assigning lvalue...\n";
```

What this prints is:

```
assigning lvalue...
[0x28fef8] copy assignment operator
[0x28fec8] copy constructor
[0x28fec8] destructor
ended assigning lvalue...
```

Makes sense - this faithfully represents what's going on inside operator=. But suppose that we want to assign some rvalue to v2:

有道理 - 这忠实地代表了operator=内部发生的事情。但是假设我们想为v2分配一些右值：

``` C++
cout << "assigning rvalue...\n";
v2 = Intvec(33);
cout << "ended assigning rvalue...\n";
```

Although here I just assign a freshly constructed vector, it's just a demonstration of a more general case where some temporary rvalue is being built and then assigned to v2 (this can happen for some function returning a vector, for example). What gets printed now is this:

虽然在这里我只是分配了一个新构建的向量，但这只是一个更一般的情况的演示，其中一些临时右值正在构建然后分配给v2（例如，这可能发生在某些返回向量的函数中）。现在打印的是这样的：

```
assigning rvalue...
[0x28ff08] constructor
[0x28fef8] copy assignment operator
[0x28fec8] copy constructor
[0x28fec8] destructor
[0x28ff08] destructor
ended assigning rvalue...
```

Ouch, this looks like a lot of work. In particular, it has one extra pair of constructor/destructor calls to create and then destroy the temporary object. And this is a shame, because inside the copy assignment operator, another temporary copy is being created and destroyed. That's extra work, for nothing.

Well, no more. C++11 gives us rvalue references with which we can implement "move semantics", and in particular a "move assignment operator" ^[So now you know why I was keeping referring to my operator= as "copy assignment operator". In C++11, the distinction becomes important.]. Let's add another operator= to Intvec:

哎呀，这看起来像很多工作。特别是，它有一对额外的构造函数/析构函数调用来创建然后销毁临时对象。这是一种耻辱，因为在复制赋值运算符内部，正在创建和销毁另一个临时副本。那是额外的工作，一无所获。

好吧，没有了。C++11 为我们提供了右值引用，我们可以使用它来实现“移动语义”，尤其是“移动赋值运算符” [5]。让我们添加另一个operator=到Intvec：

``` C++
Intvec& operator=(Intvec&& other)
{
    log("move assignment operator");
    std::swap(m_size, other.m_size);
    std::swap(m_data, other.m_data);
    return *this;
}
```

The && syntax is the new rvalue reference. It does exactly what it sounds it does - gives us a reference to an rvalue, which is going to be destroyed after the call. We can use this fact to just "steal" the internals of the rvalue - it won't need them anyway! This prints:

&&语法是新的右值引用。它完全按照它听起来的那样做——给了我们一个对右值的引用，它会在调用之后被销毁。我们可以使用这个事实来“窃取”右值的内部 - 无论如何它都不需要它们！这打印：

```
assigning rvalue...
[0x28ff08] constructor
[0x28fef8] move assignment operator
[0x28ff08] destructor
ended assigning rvalue...
```

What happens here is that our new move assignment operator is invoked since an rvalue gets assigned to v2. The constructor and destructor calls are still needed for the temporary object that's created by Intvec(33), but another temporary inside the assignment operator is no longer needed. The operator simply switches the rvalue's internal buffer with its own, arranging it so the rvalue's destructor will release our object's own buffer, which is no longer used. Neat.

I'll just mention once again that this example is only the tip of the iceberg on move semantics and rvalue references. As you can probably guess, it's a complex subject with a lot of special cases and gotchas to consider. My point here was to demonstrate a very interesting application of the difference between lvalues and rvalues in C++. The compiler obviously knows when some entity is an rvalue, and can arrange to invoke the correct constructor at compile time.

这里发生的是我们的新移动赋值运算符被调用，因为右值被分配给v2。Intvec(33)创建的临时对象仍然需要构造函数和析构函数调用，但不再需要赋值运算符中的另一个临时对象。操作符只是简单地将右值的内部缓冲区与它自己的缓冲区进行切换，这样右值的析构函数就会释放我们对象自己的缓冲区，该缓冲区不再被使用。整洁的。

我将再次提到这个例子只是移动语义和右值引用的冰山一角。您可能已经猜到，这是一个复杂的主题，需要考虑很多特殊情况和陷阱。我在这里的目的是展示一个非常有趣的应用程序，即 C++ 中左值和右值之间的差异。编译器显然知道某个实体何时是右值，并且可以安排在编译时调用正确的构造函数。

## Conclusion

One can write a lot of C++ code without being concerned with the issue of rvalues vs. lvalues, dismissing them as weird compiler jargon in certain error messages. However, as this article aimed to show, getting a better grasp of this topic can aid in a deeper understanding of certain C++ code constructs, and make parts of the C++ spec and discussions between language experts more intelligible.

Also, in the new C++ spec this topic becomes even more important, because C++11's introduction of rvalue references and move semantics. To really grok this new feature of the language, a solid understanding of what rvalues and lvalues are becomes crucial.

可以编写大量 C++ 代码而不用关心右值与左值的问题，在某些错误消息中将它们视为奇怪的编译器术语。然而，正如本文旨在展示的那样，更好地掌握这个主题有助于更深入地理解某些 C++ 代码结构，并使 C++ 规范的某些部分和语言专家之间的讨论更容易理解。

此外，在新的 C++ 规范中，这个主题变得更加重要，因为 C++11 引入了右值引用和移动语义。要真正了解该语言的这一新特性，对右值和左值的深刻理解变得至关重要。

|2022年4月20日|
|---:|
