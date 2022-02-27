# 记录一次安装GPU版pytorch

大半夜给我气坏了

## 在WSL上安装CUDA

> <https://zhuanlan.zhihu.com/p/149848405>  
> <https://zhuanlan.zhihu.com/p/350399229>

1. 安装cuda on wsl驱动
   这个我之前安装过了，请找其他教程/记录
2. 换源安装cuda-toolkit
   注意：这里的链接要根据你的系统版本自己替换一下，我的是Ubuntu20.04，如果是18.04需要把2004换成1804.

   ``` bash
   sudo apt-key adv --fetch-keys http://mirrors.aliyun.com/nvidia-cuda/ubuntu2004/x86_64/7fa2af80.pub
   sudo sh -c 'echo "deb http://mirrors.aliyun.com/nvidia-cuda/ubuntu2004/x86_64 /" > /etc/apt/sources.list.d/cuda.list'
   sudo apt-get update
   sudo apt-get install -y cuda-toolkit-11-0
   ```

3. 设置cuda环境变量
    编辑```~/.bashrc```文件，在末尾添加

    ``` bash
    export CUDA_HOME=/usr/local/cuda
    export PATH=$PATH:$CUDA_HOME/bin
    export LD_LIBRARY_PATH=/usr/local/cuda-10.1/lib64${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}
    ```

    如果提示缺少相应的依赖库，直接执行如下代码自动安装相应的依赖库

    ``` bash
    sudo apt-get install freeglut3-dev build-essential libx11-dev libxmu-dev libxi-dev libgl1-mesa-glx libglu1-mesa libglu1-mesa-dev
    ```

    查看cuda是否安装成功：```nvcc -V```
    正常则应当显示：

    ``` bash
    nvcc: NVIDIA (R) Cuda compiler driver
    Copyright (c) 2005-2020 NVIDIA Corporation
    Built on Wed_Jul_22_19:09:09_PDT_2020
    Cuda compilation tools, release 11.0, V11.0.221
    Build cuda_11.0_bu.TC445_37.28845127_0
    ```

4. 用自带的测试样例验证cuda是否安装成功

    ``` bash
    cd /usr/local/cuda/samples/4_Finance/BlackScholes
    sudo make
    ./BlackScholes
    #好像是11.0可以这样
    ```

## 重新安装GPU版本的pytorch

><https://blog.csdn.net/god_roading/article/details/117717103>
<https://blog.csdn.net/iteapoy/article/details/105184367>

在安装完cuda并且验证完成之后，我发现还并不能正常在pytorch里使用GPU，torch.cuda.is_available()显示false

折腾了很久之后才知道之前没有安装CUDA时安装的pytorch并不会在安装cuda-toolkit之后更新为GPU版

判断方法：在python环境下执行

```python
import torch
torch.version.cuda
```

如果确实是安装了gpu版本的pytorch，上面这个应该会显示cuda的版本，如果是cpu，不会返回什么。

所以只好卸载pytorch之后重新安装：
```conda uninstall pytorch```

打开pytorch官网
><https://pytorch.org/get-started/locally/>

选择自己设备配置，复制指令并执行

```conda install pytorch torchvision torchaudio cudatoolkit=11.3 -c pytorch```

<https://crushonu.top/conda%E4%B8%B4%E6%97%B6%E6%8D%A2%E6%BA%90-conda%E5%AE%89%E8%A3%85pytorch-10/#conda%E4%B8%B4%E6%97%B6%E6%8D%A2%E6%BA%90%E5%AE%89%E8%A3%85pytorch>

1. 需要使用国内源时，把最后的```-c pytorch```去掉，就会使用你添加的源了

2. 当出现以下错误时，说明你添加的源中没有某个需要的包，可以使用```-c 某个链接```替换原来的```-c pytorch```这条指令是临时使用```-c```后面的源，其他保持默认
   >Solving environment: failed with initial frozen solve. Retrying with flexible solve.
   >PackagesNotFoundError: The following packages are not available from current channels:    - torchaudio

3. conda换源 <https://zhuanlan.zhihu.com/p/87123943>
