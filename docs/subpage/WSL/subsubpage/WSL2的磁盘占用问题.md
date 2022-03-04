WSL2的磁盘占用问题

> [https://www.cnblogs.com/enrio/p/14222648.html](https://www.cnblogs.com/enrio/p/14222648.html)  
> 因为 WSL2 使用了虚拟磁盘，意味着它可能只有 15GB 的数据，但是虚拟磁盘占用了 100GB 的空间。如果你往 WSL2 中放了大量的数据，然后就删掉，会发现WSL2的磁盘占用没有降下来，这就是虚拟磁盘造成的。

> [https://zhuanlan.zhihu.com/p/358528257](https://zhuanlan.zhihu.com/p/358528257)  
> WSL2使用虚拟硬盘(VHD)存储linux下的文件，随着Linux下文件越来越多，占用空间也会不断增长，不过有个最大限制256G。但是，在Linux中减少文件占用，WSL却没有相应的减少硬盘空间的占用。所以为了避免碰到256G的限制，或者硬盘空间告警，在删除掉linux下的文件后，我们需要手动释放这部分空间。

> [https://www.banyudu.com/posts/free-docker-space-in-wsl2.af131c?random=EW9jco](https://www.banyudu.com/posts/free-docker-space-in-wsl2.af131c?random=EW9jco)  
> 因为WSL2本质上来说是虚拟机，对于每个虚拟机，Windows会创建vhdx后缀的磁盘镜像文件，用于存储其内容，类似于vmdk、vdi，用过虚拟机的同学应该都不陌生。  
这种镜像文件的特点是支持自动扩容，但是一般不会自动缩容。因此一旦Docker镜像文件过多，引起镜像扩容，即使再使用docker system prune清理虚拟机中的镜像文件，也不会释放出已经占用的系统磁盘空间了。

附：[WSL备份与还原](http://www.xfy-learning.com/2020/05/30/WSL%E5%A4%87%E4%BB%BD%E4%B8%8E%E8%BF%98%E5%8E%9F/)