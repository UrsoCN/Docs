# OpenGL配置

[https://www.bilibili.com/video/BV1Ni4y1o7Au](https://www.bilibili.com/video/BV1Ni4y1o7Au)

## 下载OpenGL Binary文件

## 在C++项目解决方案目录下创建Dependencies/GLFW文件夹

lib-vc2019文件夹下的*.dll和*dll.lib文件是可以删掉的

## 添加包含目录和添加链接库

- 右键VS项目-点击属性，点击C/C++-常规，在附加包含目录里添加相对路径`$(SolutionDir)Dependencies\GLFW\include`
- 点击链接器-常规，在附加库目录里添加相对路径`$(SolutionDir)Dependencies\GLFW\lib-vc2019`
- 点击链接器-输入，在附加依赖项里添加`glfw3.lib`，其余项其实是可以删掉的(已经被包含在默认的库中)

这个时候编辑器里波浪线应该就都消失了，但编译时仍然会出错

## 解决尝试链接找不到GL_CLEAR函数定义

- 点击链接器-输入，在附加依赖项里添加`;opengl32.lib`

## 解决无法解析的外部符号

- 尝试在google中搜相关函数，通过MSDN找到需要的库，方法同上

|2022年4月26日|
|---:|