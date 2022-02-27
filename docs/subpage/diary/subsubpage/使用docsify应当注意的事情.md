### 使用docsify应当注意的事情

#### 作为文档的.md文件名不应当有空格或某些特殊符号

当使用空格时，例如：

在`_sidebar.md`中使用`[DateTime in C#](./subpage/Csharp/subsubpage/DateTime in C#.md)`

由于docsify是根据路径作为url访问的，会使得网页中无法渲染为链接：(解决方法就是替换空格为下划线)

![1.png](https://raw.githubusercontent.com/UrsoCN/Docs/master/docs/subpage/diary/subsubpage/%E4%BD%BF%E7%94%A8docsify%E5%BA%94%E5%BD%93%E6%B3%A8%E6%84%8F%E7%9A%84%E4%BA%8B%E6%83%85/1.png)

然而在改完之后页面显示404 - Not found了(灵光一现把#改成Sharp解决了)：

![2.png](https://raw.githubusercontent.com/UrsoCN/Docs/master/docs/subpage/diary/subsubpage/%E4%BD%BF%E7%94%A8docsify%E5%BA%94%E5%BD%93%E6%B3%A8%E6%84%8F%E7%9A%84%E4%BA%8B%E6%83%85/1.png)

#### 导入图片

要使得部署在github上的静态网页能够正常显示图片，需要将GitHub当成图床，键入合适的链接，这里的第一张图链是`https://raw.githubusercontent.com/UrsoCN/Docs/master/docs/subpage/diary/subsubpage/1.png`

然后添加了一个[图片缩放 - Zoom image](https://docsify.js.org/#/zh-cn/plugins?id=图片缩放-zoom-image)插件。
