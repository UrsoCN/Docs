> **来源：**
>
> **https://www.cnblogs.com/ShaYeBlog/p/5592112.html**

# 浅谈config文件的使用

一、缘起


  最近做项目开始使用C#，因为以前一直使用的是C++，因此面向对象思想方面的知识还是比较全面的，反而是因没有经过完整、系统的.Net方面知识的系统学习，经常被一些在C#老鸟眼里几乎是常识的小知识点给绊倒。

 

  为什么这么说呢，因为我在网络上查找的资料的时候，经常大部分问题，都是能够找到或多或少的参考资料，但是这些小知识点却很少能够找到正确的解决方法，有也是只有提问，没有回到，那么这种情况出现，就只有2种解释：
1、这个方面的问题很难，难到没有人能够解决；
2、这个问题太简单，简单到稍微熟悉的人都不屑于回答，提问者也在一番思考后，轻松找到答案。（我比较倾向这个，呵呵，因此我也把这些小知识，叫做：容易被忽略的细节）

 

  然而，无论问题是否简单，既然我会被绊倒，耽搁时间，肯定也会有人被同样耽搁，因此我想把这些细节整理出来，还是具有一定意义的。

 

  于是，本系列文章开始...


二、问题描述


  除了正常情况下的config文件，使用ConfigurationManager加载，我们还可能会碰到一下这样的情况：
1、加载非当前应用程序yyy.exe默认的config文件的xxx.exe.config文件；（比如：与yyy.exe.config不在同一目录下 或者 文件名不同）
2、加载非应用程序的xxx.config文件；
3、让类库xxx.dll内的函数读取默认config文件的时候，读取的是xxx.dll同级目录下的xxx.dll.config文件，而不是加载xxx.dll的应用程序yyy.exe的默认应用程序配置文件：yyy.exe.config；
  以上三种情况，都不能直接使用ConfigurationManager来加载

 

三、解决过程


  让我们从最基础、最简单、最常见的config文件的加载来入手，解决上面三个问题：

 

step1：研究基础的config文件加载

  config文件，是给客户端应用程序保存配置信息用的，一般情况，一个应用程序只会有一个这样的文件，在编译之前，叫做App.config，每次使用Visual Studio编译，都会Copy到应用程序的生成目录，且Rename为：应用程序名.config。
  要读取config文件内的信息，只需要使用ConfigurationManager的相关函数和属性即可，因此我们来研究下ConfigurationManager，看看是否能找到解决问题的相关信息。
打开MSDN，找到这样一个方法：

OpenExeConfiguration 已重载。 将指定的客户端配置文件作为 Configuration 对象打开。

  OK，要找的就是这个，因为这个方法有一个重载方法是：

OpenExeConfiguration(String) 将指定的客户端配置文件作为 Configuration 对象打开。

 

step2：加载非当前应用程序默认的config文件

  于是，第一个问题的解决方案，似乎、应该、可能找到了，按照MSDN上的说明，若我们把要打开的xxx.exe.config的路径作为参数传入即可，代码如下：

  Configuration config = ConfigurationManager.OpenExeConfiguration("C:\\xxx.exe.config");
  DllInfo dllInfo = config.GetSection("DllInfo") as DllInfo;
  Console.WriteLine(dllInfo);

  但是，事情并没有这么顺利，这样是无法打开xxx.exe.config文件的，经过调试，发现：config的属性FilePath的值为："C:\\xxx.exe.config.config"，程序自己在传入的参数后增加了“.config”作为要打开的config文件的路径，这显然和我们之前从MSDN上所看到的不一样，不用说，我们被微软小小的耍了一把。这里要传入的参数，不应该是要打开的config的路径，而应该是这个config文件对应的应用程序的路径，也就是说上面的代码应该这样写：

  Configuration config = ConfigurationManager.OpenExeConfiguration("C:\\xxx.exe"); // 写的是应用程序的路径
  DllInfo dllInfo = config.GetSection("DllInfo") as DllInfo;
  Console.WriteLine(dllInfo);

  再次运行，呵呵，还是不行，提示错误：『加载配置文件时出错: 参数“exePath”无效。参数名: exePath』。显然我们有被耍了，这里要传入应用程序路径（exePath）没错，但是因为我们并没有在xxx.exe.config文件同目录下，加入xxx.exe文件，因此我们传入的exePath实际上是无效的，可见为了能够加载xxx.exe.config，我们弄一个xxx.exe文件放在一起。

  ok，运行，成功。

  **小结1：第一个问题的解决方案找到:**

使用ConfigurationManager.OpenExeConfiguration(string exePath)即可，同时注意2个小细节：
A：改方法需传入的是exePath，而不是configPath；
B：exePath必须是有效的，因此xxx.exe和xxx.exe.config应该成对出现，缺一不可。

 

step3：扩展step2的战果，找到加载xxx.config的方法

  step2已经找到了加载xxx.exe.config的方法，观察xxx.exe.config的名称，发现，若把xxx.exe看成YYY，显然xxx.exe.config = YYY.config，也就是说：xxx.exe.config是xxx.config中比较特殊的一种，他要求config文件的文件名最后4个字母必须是“.exe”。

  此时，大胆推测，使用ConfigurationManager.OpenExeConfiguration(string exePath)，应该可以解决问题。

  Configuration config = ConfigurationManager.OpenExeConfiguration("C:\\xxx"); // 记得要有xxx文件，否则这个路径就是无效的了。
  DllInfo dllInfo = config.GetSection("DllInfo") as DllInfo;
  Console.WriteLine(dllInfo);

  运行，HOHO,成功了。

  **小结2：第二个问题和第一个问题的解决方案一样。**

 

step4：扩展xxx.config解决问题3

  继续扩大战果，还是从文件名上来找思路，我们要加载的xxx.dll.config，其实也是xxx.config中稍微特殊的一种，显然也可以和step3那样处理。

  使用OpenExeConfiguration(string exePath)来解决问题三，在dll内，碰到需要读取config文件信息的时候，放弃使用ConfigurationManager的函数或属性直接获取，而改用OpenExeConfiguration(string exePath)加载config文件为一个Configuration对象的对应函数或属性即可。

  **小结3：第三个问题同样可以按照第一个问题的方案来做。**

 

四、额外的思考

  在应用程序yyy.exe中通过ConfigurationManager可以很方便的读取到yyy.exe.config文件中的信息，但在类库中使用ConfigruationManager读取的却不是自动编译生成的xxx.dll.cofig文件，而是引用类库的应用程序yyy.exe的yyy.exe.config文件。

 

  有没有什么办法，让类库中的ConfigurationManager读取的也是他默认的xxx.dll.config文件呢？

 

  其实，是可以的，不过这里涉及到了应用程序域（AppDomain）的概念， .Net上的应用程序都是运行在一个应用程序域（AppDomain）内的，在程序启动之初，都会默认启动一个AppDomain，查看MSDN可以看到AppDomain有一个属性：SetupInformation，这个属性保存的就是当前域的config文件路径；可惜，这个属性是只读的，所以我们默认AppDomain的config文件路径。

 

  因此，若想让类库能够直接使用ConfigurationManager来读取自己默认的config文件，就只能把类库放在一个新的AppDomain中执行，并且在创建AppDomain的时候指定他的SetupInformation为类库默认的config文件路径；AppDomain有一个用来创建新AppDomain的方法：CreateDomain(String, Evidence, AppDomainSetup)；只要把第三个参数的属性ConfigurationFile只想类库默认的config文件路径即可。