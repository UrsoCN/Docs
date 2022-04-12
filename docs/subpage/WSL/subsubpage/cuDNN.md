# cuDNN

[NVIDIA CUDNN DOCUMENTATION](https://docs.nvidia.com/deeplearning/cudnn/install-guide/index.html)

网上大多是通过Tar文件安装的，这里记载一种通过deb文件安装的方法

下载 Debian 本地存储库安装包。在发出以下命令之前，您需要将 X.Y 和 8.x.x.x 替换为特定的 CUDA 和 cuDNN 版本。

## 安装

1. 导航到包含 cuDNN Debian 本地安装程序文件`cudnnpath`目录。

2. 启用本地存储库。或

``` bash
sudo dpkg -i cudnn-local-repo-${OS}-8.x.x.x_1.0-1_amd64.deb
```

3. 导入 CUDA GPG 密钥。

``` bash
sudo apt-key add /var/cudnn-local-repo-*/7fa2af80.pub
```

4. 刷新存储库元数据。

``` bash
sudo apt-get update
```

5. 安装运行时库。

``` bash
sudo apt-get install libcudnn8=8.x.x.x-1+cudaX.Y
```

6. 安装开发人员库。

``` bash
sudo apt-get install libcudnn8-dev=8.x.x.x-1+cudaX.Y
```

7. 安装代码示例和 cuDNN 库文档。

``` bash
sudo apt-get install libcudnn8-samples=8.x.x.x-1+cudaX.Y
```

## 验证在 Linux 上的安装

要验证 cuDNN 是否已安装并正常运行，请编译位于 Debian 文件中 /usr/src/cudnn_samples_v8目录中的 mnistCUDNN 样本。

1. 将 cuDNN 示例复制到可写路径。

``` bash
cp -r /usr/src/cudnn_samples_v8/ $HOME
```

2. 转到可写路径。

``` bash
cd  $HOME/cudnn_samples_v8/mnistCUDNN
```

3. 编译 mnistCUDNN 示例。

``` bash
make clean && make
```

4. 运行 mnistCUDNN 示例。

``` bash
./mnistCUDNN
```

5. 如果 cuDNN 已在 Linux 系统上正确安装并运行，您将看到类似于以下内容的消息：

Test passed!

## 注意事项

在验证安装时，执行 `make clean && make` 指令时，可能会遇到

```
fatal error: FreeImage.h: No such file or directory
```

[关于这个问题的讨论贴](https://forums.developer.nvidia.com/t/freeimage-is-not-set-up-correctly-please-ensure-freeimae-is-set-up-correctly/66950)

这是由于缺少了相应包，可以尝试执行下面命令安装。

``` bash
sudo apt-get install libfreeimage3 libfreeimage-dev
```

在国内网络安装时，可能会遇到如下问题：

```
Reading package lists... Done
Building dependency tree
Reading state information... Done
The following additional packages will be installed:
  libilmbase24 libjbig0 libjxr0 libopenexr24 libopenjp2-7 libraw19 libtiff5 libwebp6 libwebpmux3
The following NEW packages will be installed:
  libfreeimage-dev libfreeimage3 libilmbase24 libjbig0 libjxr0 libopenexr24 libopenjp2-7 libraw19 libtiff5 libwebp6
  libwebpmux3
0 upgraded, 11 newly installed, 0 to remove and 1 not upgraded.
Need to get 288 kB/1907 kB of archives.
After this operation, 8600 kB of additional disk space will be used.
Do you want to continue? [Y/n] y
Err:1 http://archive.ubuntu.com/ubuntu focal/universe amd64 libfreeimage3 amd64 3.18.0+ds2-1ubuntu3
  Connection failed [IP: 91.189.91.38 80]
Err:2 http://archive.ubuntu.com/ubuntu focal/universe amd64 libfreeimage-dev amd64 3.18.0+ds2-1ubuntu3
  Connection failed [IP: 185.125.190.39 80]
E: Failed to fetch http://archive.ubuntu.com/ubuntu/pool/universe/f/freeimage/libfreeimage3_3.18.0+ds2-1ubuntu3_amd64.deb  Connection failed [IP: 91.189.91.38 80]
E: Failed to fetch http://archive.ubuntu.com/ubuntu/pool/universe/f/freeimage/libfreeimage-dev_3.18.0+ds2-1ubuntu3_amd64.deb  Connection failed [IP: 185.125.190.39 80]
E: Unable to fetch some archives, maybe run apt-get update or try with --fix-missing?
```

可以通过换源/添加源解决：

[Ubuntu 20.04 Linux更换源教程](https://www.cnblogs.com/gaojia-hackerone/p/15202230.html)

