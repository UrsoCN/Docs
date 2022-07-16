# 在c++中使用现代OpenGL

OpenGL函数代码是由相应显卡厂家编写在驱动里的，我们想要调用OpenGL函数就需要访问这些驱动，取出函数，并且调用它们。我们实际上要做的取出函数，就是得到函数声明，再链接到对应函数上。所以说，我们需要访问驱动的动态链接库文件，然后检索库里面哪些函数的函数指针。

总的来说，直接调用OpenGL函数不是一个好的选择：

- 直接这样写没法跨平台：(在Windows平台下)访问显卡驱动并从中取出这些函数需要使用一些Win32接口调用，然后载入库，载入函数指针...这些只能在Windows上用。
- 如果有超过1000个函数之类的事情，我们就需要手动完成所有这些操作，为它们写代码

我们实际上在做的是使用`GLEW`库(OpenGL Extension Wrangler)(还有一个可用的库`GLAD`)，这个库基本上就是在头文件为你提供OpenGL接口规范，包括各种函数声明，符号声明和常量；这个库的实际实现就是进入EDI，在你使用的显卡驱动签名中查找对应的动态链接文件(*.dll)，载入所有函数指针

[https://sourceforge.net/projects/glew/](https://sourceforge.net/projects/glew/)

在使用`GLEW`需要注意的两点：

> First you need to create a valid OpenGL rendering context and call `glewInit()` to initialize the extension entry points. If `glewInit()` returns `GLEW_OK`, the initialization succeeded and you can use the available extensions as well as core OpenGL functionality.

接下来就是像加入`GLFW`包含目录和添加链接库类似的操作了

- 右键VS项目-点击属性，点击C/C++-常规，在附加包含目录里添加相对路径`$(SolutionDir)Dependencies\GLEW\include`
- 点击C/C++-预处理器，在预处理器定义里添加`GLEW_STATIC`
- 点击链接器-常规，在附加库目录里添加相对路径`$(SolutionDir)Dependencies\GLEW\lib\Release\Win32`
- 点击链接器-输入，在附加依赖项里添加`glew32s.lib`

需要注意的是：`$(SolutionDir)Dependencies\GLEW\lib\Release\Win32`下的两个`*.lib`文件都是静态链接库，如果需要链接dll，使用`glew32.lib`，`glew32s.lib`是整个静态库。

----

注：当“第二次”用“本地 Windows 调试器”运行程序报错`-1073741502 (0xc0000142)`时，很可能是由于没有设定“调试停止时自动关闭控制台”

可以点击工具-选项-搜索“控制台”-选中“调试停止时自动关闭控制台”
也可以打开任务管理器，找到`VsDebugConsole.exe`，手动结束进程来临时解决。

|2022年4月26日|
|---:|
