# 记载一次安装miniconda以及jupyter notebook

> 参考：
> <https://zhuanlan.zhihu.com/p/339695941>  
> <https://www.cnblogs.com/xl717/p/12163143.html>  
> <https://www.jianshu.com/p/edaa744ea47d>

## 安装miniconda

> 运行在WSL上的Ubuntu 20.04

### 下载Miniconda3-latest-Linux-x86_64.sh

可以在annaconda官网(anaconda.org)下载，不过在国内，更建议使用各大学、组织搭建的软件源

清华大学: <https://mirrors.tuna.tsinghua.edu.cn/help/anaconda/>  
北京外国语大学: <https://mirrors.bfsu.edu.cn/help/anaconda/>  
南京邮电大学: <https://mirrors.njupt.edu.cn/>  
南京大学: <http://mirrors.nju.edu.cn/>  
重庆邮电大学: <http://mirror.cqupt.edu.cn/>  
上海交通大学: <https://mirror.sjtu.edu.cn/>

这里建议使用北京外国语大学的软件源，这同样是清华大学维护的且速度够快(使用的人少)。

### 安装Miniconda

注意：应当定位到刚刚下载的.sh文件的目录

``` bash
bash Miniconda3-latest-Linux-x86_64.sh #运行
```

注意：当不能正常运行时，执行下方代码

``` bash
chmod +x Miniconda3-latest-Linux-x86_64.sh #给执行权限
```

注意：以下描述是对于 **conda 4.10.3** 而言的，之后的版本可能会有些许区别，注意阅读提示语句。

运行之后，会在终端让你阅读一些注意事项，可以长按回车键略过。随后键入yes开始安装。
当出现 **Do you wish the installer to initialize Miniconda3 by running conda init?** 的提示语句时，需要注意：

1. 此时选择yes能更方便地使用Miniconda，但是可能会污染原本系统的环境，详见[链接](https://mp.weixin.qq.com/s?__biz=MzAxMDkxODM1Ng==&mid=2247486380&idx=1&sn=9329fcd0a60ac5488607d359d6c28134&chksm=9b484b17ac3fc20153d25cbdefe5017c7aa9080d13b5473a05f79808244e848b0a45d2a6a735&scene=21#wechat_redirect)
2. 选择no时，可以手动编辑Path。详见：<https://blog.csdn.net/qq_41126685/article/details/105525408>以及<https://www.cnblogs.com/xl717/p/12163143.html>

在选择no的情况下正常启动conda：
进入 **/home/用户名/miniconda3/bin** 目录
执行：```. ./activate``` 会默认进入base环境，此时命令行最先面会有(base)
退出指令为：```conda deactivate```
可以通过如：```conda create --name env_name python=3.8```创建一个环境，进入此环境使用：```. ./activate env_name```

之后就可以在这个环境内安装各类python库了，一下是关于安装Jupyter Notebook

## 安装Jupyter Notebook

在conda环境内，执行```conda install jupyter```以安装。键入```jupyter notebook```以开启服务。

需要注意的是，在WSL下，可能无法正常自动跳转到浏览器打开Jupyter Notebook，终端阻塞。

通过ctrl+C关闭此进程，并在Ubuntu的命令端输入```jupyter notebook --generate-config```会生成一个配置文件，根据提示目录找到该文件，并搜索```# c.NotebookApp.notebook_dir = ''```这一行代码，在下方加上：

``` bash
import webbrowser
webbrowser.register('chrome',None,webbrowser.GenericBrowser(u'C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe'))
c.NotebookApp.browser = 'chrome'
```

并保存。

注意：浏览器位置可以通过右键浏览器图标，选择 **属性** 查看，路径中分隔符必须是双斜杠 **\\\\**。

此时，可以通过键入```jupyter notebook```以开启服务，回车后会出现一串地址，将其复制到浏览器或者按住ctrl用鼠标左键点击链接打开。
此时打开的目录是终端开启服务时的目录，可以编辑上文中```# c.NotebookApp.notebook_dir = ''```这一行代码，在单引号之中加上想要打开的默认位置，如：```# c.NotebookApp.notebook_dir = '/home/'```。

> 技巧及注意点：可以通过 explorer.exe ./ 来调用windows资源管理器来打开当前WSL目录，(win11)也可以在资源管理器侧边栏找到WSL(\\\wsl.localhost\Ubuntu-20.04\)
