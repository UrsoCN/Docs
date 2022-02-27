# 关于局部静态（Local Static）

> [【23】【Cherno C++】【中字】C++中的局部静态（Local Static）](bilibili.com/video/BV1Ny4y1D7EE/?spm_id_from=333.788)

> 在局部变量前加上“static”关键字，就成了静态局部变量。静态局部变量存放在内存的全局数据区。函数结束时，静态局部变量不会消失，每次该函数调用 时，也不会为其重新分配空间。它始终驻留在全局数据区，直到程序运行结束。静态局部变量的初始化与全局变量类似．如果不为其显式初始化，则C++自动为其 初始化为0。
静态局部变量与全局变量共享全局数据区，但静态局部变量只在定义它的函数中可见。静态局部变量与局部变量在存储位置上不同，使得其存在的时限也不同，导致对这两者操作 的运行结果也不同。

``` C++
#include <iostream>

// class Singleton //单例类，调用ta需要通过Get()
// {
// private:
// 	static Singleton* s_Instance;
// public:
// 	static Singleton& Get() { return *s_Instance; }

// 	void Hello() {}
// };

// Singleton* Singleton::s_Instance = nullptr;

class Singleton //与上面注释掉的一致，使用了Local static使得代码看起来更简洁
{
public:
	static Singleton &Get()
	{
		static Singleton instance;
		return instance;
	}

	void Hello() { std::cout << "hello " << std::endl; }
};

int main()
{
	Singleton::Get().Hello();
	std::cin.get();
}
```
