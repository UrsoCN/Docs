### 使用docsify应当注意的事情

#### 作为文档的.md文件名不应当有空格或某些特殊符号

当使用空格时，例如：

在`_sidebar.md`中使用`[DateTime in C#](./subpage/Csharp/subsubpage/DateTime in C#.md)`

由于docsify是根据路径作为url访问的，会使得网页中无法渲染为链接：(解决方法就是替换空格为下划线)

<img src="/1.png">

然而在改完之后页面显示404 - Not found了(灵光一现把#改成Sharp解决了)：

<img src="./2.png">



#### 导入图片应当使用HTML语法

直接使用MarkDown语法导入图片不能正常显示：

![1](E:/software/node/Docs/docs/subpage/diary/subsubpage/1.png)

解决方法是使用HTML语法导入`![1](E:/software/node/Docs/docs/subpage/diary/subsubpage/1.png)`

改为`<img src="/subpage/diary/subsubpage/1.png">`

然后添加了一个[图片缩放 - Zoom image](https://docsify.js.org/#/zh-cn/plugins?id=图片缩放-zoom-image)插件。