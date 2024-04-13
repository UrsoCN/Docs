# AOSP Build System

介绍文档：
> <https://github.com/aggresss/PHDemo>  
> <https://blue-bird1.github.io/posts/aosp-3/>  
> <https://www.cnblogs.com/mr-raptor/archive/2012/06/07/2540359.html>  
> <https://zhuanlan.zhihu.com/p/448765932>

详细说明：

> [https://wladimir-tm4pda.github.io/porting/build_new_device.html](https://apc01.safelinks.protection.outlook.com/?url=https%3A%2F%2Fwladimir-tm4pda.github.io%2Fporting%2Fbuild_new_device.html&data=05%7C02%7Cxilong.zhang%40transsion.com%7Ce0d8969365ac44a4416908dc4e703932%7C2e8503a62d0143338e366ab7c8cd7ae2%7C0%7C0%7C638471492933268978%7CUnknown%7CTWFpbGZsb3d8eyJWIjoiMC4wLjAwMDAiLCJQIjoiV2luMzIiLCJBTiI6Ik1haWwiLCJXVCI6Mn0%3D%7C40000%7C%7C%7C&sdata=9B0XCY9uHsa3xeLjO%2BM%2BfFJu1QtKSXcWaxAGifKTcrQ%3D&reserved=0)  
> [https://source.android.com/docs/setup/create/new-device?hl=zh-cn](https://apc01.safelinks.protection.outlook.com/?url=https%3A%2F%2Fsource.android.com%2Fdocs%2Fsetup%2Fcreate%2Fnew-device%3Fhl%3Dzh-cn&data=05%7C02%7Cxilong.zhang%40transsion.com%7Ce0d8969365ac44a4416908dc4e703932%7C2e8503a62d0143338e366ab7c8cd7ae2%7C0%7C0%7C638471492933268978%7CUnknown%7CTWFpbGZsb3d8eyJWIjoiMC4wLjAwMDAiLCJQIjoiV2luMzIiLCJBTiI6Ik1haWwiLCJXVCI6Mn0%3D%7C40000%7C%7C%7C&sdata=GfibldkRLseIfgU%2BO7m%2B4JmUw2aNRk%2F2POpS%2B6BFv%2Bg%3D&reserved=0)  
> [Android系统开发入门-2.添加product](https://qiushao.net/2019/11/19/Android%E7%B3%BB%E7%BB%9F%E5%BC%80%E5%8F%91%E5%85%A5%E9%97%A8/2-%E6%B7%BB%E5%8A%A0product/index.html#:~:text=%E5%85%B6%E4%B8%AD%20PRODUCT_NAME%20%E8%B7%9F%20PRODUCT_DEVICE%20%E6%98%AF%E6%9C%80%E9%87%8D%E8%A6%81%E7%9A%84%E4%B8%A4%E4%B8%AA%E5%8F%98%E9%87%8F%EF%BC%8C%E9%9C%80%E8%A6%81%E6%88%91%E4%BB%AC%E8%87%AA%E5%B7%B1%E9%87%8D%E6%96%B0%E8%B5%8B%E5%80%BC%E3%80%82%20PRODUCT_NAME,%E8%A6%81%E4%BF%9D%E6%8C%81%E8%B7%9F%E9%85%8D%E7%BD%AE%E6%96%87%E4%BB%B6%E5%90%8D%E4%B8%80%E8%87%B4%E3%80%82%20%E7%BC%96%E8%AF%91%E8%BE%93%E5%87%BA%E7%9B%AE%E5%BD%95%E5%B0%B1%E6%98%AF%E7%94%B1%E8%BF%99%E4%B8%AA%E5%8F%98%E9%87%8F%E5%86%B3%E5%AE%9A%E7%9A%84%E3%80%82%20PRODUCT_DEVICE%20%E8%B7%9FBoardConfig.mk%E7%9B%B8%E5%85%B3%EF%BC%8C%E7%BC%96%E8%AF%91%E7%B3%BB%E7%BB%9F%E4%BC%9A%E6%A0%B9%E6%8D%AE%24%20%28PRODUCT_DEVICE%29%20%E6%9D%A5%E5%8A%A0%E8%BD%BD%E5%AF%B9%E5%BA%94%E7%9B%AE%E5%BD%95%E4%B8%8B%E7%9A%84BoardConfig.mk%E6%96%87%E4%BB%B6%E3%80%82)  
> <https://developer.aliyun.com/article/12880>

在AOSP中，选择一个Project并编译出产品软件可以使用：

`source build/envsetup.sh && lunch ${project_id} && make`

↓ ↓ ↓

| `source build/envsetup.sh`                                                         | `lunch ${project_id}`                                                                                                | `make`                                                                                                                                                                                                                                                                                                                                                            |
| ---------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 初始化编译环境                                                                            | 选择Project，设定相关prop                                                                                                   | 编译                                                                                                                                                                                                                                                                                                                                                                |
| 装载一些编译指令、编译目标<br><br>常用的有：`m`,`mm`,`mmm`,`lunch`,<br><br>`croot`,`jgrep`,`cgrep`…… | 设定环境变量以影响后续的编译产物<br><br>比如 `TARGET_PRODUCT=***`<br><br>`TARGET_BUILD_VARIANT=eng`<br><br>`TARGET_BUILD_TYPE=release` | 非递归的编译、打包<br><br>[Android编译系统-Android.mk篇](Android编译系统-Android.mk篇.md)<br><br>[Android编译系统-Android.bp篇](Android编译系统-Android.bp篇.md)<br><br>[Android 编译之make基础](https://www.jianshu.com/p/169397686d58)、[GNU make](https://www.gnu.org/software/make/manual/make.html#Target_002dspecific) |

第一步初始化编译环境的时候，还会加载`vendorsetup.sh`

```Bash
2033 # Execute the contents of any vendorsetup.sh files we can find.
2034 # Unless we find an allowed-vendorsetup_sh-files file, in which case we'll only
2035 # load those.
2036 #
2037 # This allows loading only approved vendorsetup.sh files
2038 function source_vendorsetup() {
2039     unset VENDOR_PYTHONPATH
2040     local T="$(gettop)"
2041     allowed=
2042     for f in $(cd "$T" && find -L device vendor product -maxdepth 4 -name 'allowed-vendorsetup_sh-files' 2>/dev/null | sort); do
2043         if [ -n "$allowed" ]; then
2044             echo "More than one 'allowed_vendorsetup_sh-files' file found, not including any vendorsetup.sh files:"
2045             echo "  $allowed"
2046             echo "  $f"
2047             return
2048         fi
2049         allowed="$T/$f"
2050     done
2051 
2052     allowed_files=
2053     [ -n "$allowed" ] && allowed_files=$(cat "$allowed")
2054     for dir in device vendor product; do
2055         for f in $(cd "$T" && test -d $dir && \
2056             find -L $dir -maxdepth 4 -name 'vendorsetup.sh' 2>/dev/null | sort); do
2057 
2058             if [[ -z "$allowed" || "$allowed_files" =~ $f ]]; then
2059                 echo "including $f"; . "$T/$f"
2060             else
2061                 echo "ignoring $f, not in $allowed"
2062             fi
2063         done
2064     done
2065 }
```

这里可以为项目编译框架添加其他脚本，比如说展锐平台的签名指令`cp_sign`、打包指令`makepac`

vendor/sprd/build/core/vendorsetup.sh

**新建一个****AOSP** **Project主要影响的就是第二步lunch过程**

第三步编译就如同一般的嵌入式开发中的交叉编译流程，依照`Android.mk`、`Android.bp`、`Makefile`里面定义好的编译规则逐步编译项目，详见上文表格中的文档。

## 1. 新建 product 配置

此处仍以 展锐平台 为例分析

通常来讲，配置一个项目需要单独创建一个配置目录`vendor/<company_name>/<products>` 或者 `device/<company-name>/<products>`，并新建一个项目配置文件`<first_product_name>.mk`

可参考：
device/linaro/dragonboard/rb5.mk

最少要包含如下内容

```Bash
  $(call inherit-product, $(SRC_TARGET_DIR)/product/generic.mk) # 用来定位后续的makefile
  #
  # Overrides
  PRODUCT_NAME := <first_product_name>            # 用来被lunch定位
  PRODUCT_DEVICE := <board_name>                  # 与BoardConfig.mk相关
                                                    # 编译系统会根据$(PRODUCT_DEVICE)来加载
                                                    # 对应目录下的BoardConfig.mk文件
```

其他常用变量：

| **变量**                      | **说明**                                                                                                                                                                            | **示例**                          |
| --------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------- |
| `PRODUCT_AAPT_CONFIG`       | 创建软件包时使用的 `aapt` 配置。                                                                                                                                                              |                                 |
| `PRODUCT_BRAND`             | 对软件进行自定义所针对的品牌（如果有），例如运营商。                                                                                                                                                        |                                 |
| `PRODUCT_CHARACTERISTICS`   | 用于允许向软件包中添加变体特定资源的 `aapt` 特性。                                                                                                                                                     | `tablet`、`nosdcard`             |
| `PRODUCT_COPY_FILES`        | 字词列表，如 `source_path:destination_path`。在构建相应产品时，应将源路径下的文件复制到目标路径。`config/makefile` 中定义了针对复制步骤的规则。                                                                                  |                                 |
| `PRODUCT_DEVICE`            | 工业设计的名称。这也是主板名称，构建系统会使用它来查找 `BoardConfig.mk`。                                                                                                                                     | `tuna`                          |
| `PRODUCT_LOCALES`           | 以空格分隔的列表，用于列出由双字母语言代码和双字母国家/地区代码组成的代码对，以便说明针对用户的一些设置，例如界面语言和时间、日期以及货币格式。`PRODUCT_LOCALES` 中列出的第一个语言区域会用作产品的默认语言区域。                                                                | `en_GB`、`de_DE`、`es_ES`、`fr_CA` |
| `PRODUCT_MANUFACTURER`      | 制造商的名称。                                                                                                                                                                           | `acme`                          |
| `PRODUCT_MODEL`             | 最终产品的最终用户可见名称。                                                                                                                                                                    |                                 |
| `PRODUCT_NAME`              | 总体产品的最终用户可见名称，将显示在设置 > 关于屏幕中。                                                                                                                                                     |                                 |
| `PRODUCT_OTA_PUBLIC_KEYS`   | 产品的无线下载 (OTA) 公钥列表。                                                                                                                                                               |                                 |
| `PRODUCT_PACKAGES`          | 将要安装的 APK 和模块列表。                                                                                                                                                                  |                                 |
| `PRODUCT_PACKAGE_OVERLAYS`  | 指明是使用默认资源还是添加任何产品特定叠加层。                                                                                                                                                           | `vendor/acme/overlay`           |
| `PRODUCT_SYSTEM_PROPERTIES` | 系统分区的系统属性分配（采用 `"key=value"` 格式）列表。其他分区的系统属性可通过 `PRODUCT_<PARTITION>_PROPERTIES` 设置，如供应商分区的 `PRODUCT_VENDOR_PROPERTIES`。支持的分区名称：`SYSTEM`、`VENDOR`、`ODM`、`SYSTEM_EXT` 和 `PRODUCT`。 |                                 |

## 2. 新建 AndroidProducts.mk

`AndroidProducts.mk`中，需要定义好`PRODUCT_MAKEFILES`和`COMMON_LUNCH_CHOICES`

注释很清晰了

```Bash
#
# This file should set PRODUCT_MAKEFILES to a list of product makefiles
# to expose to the build system.  LOCAL_DIR will already be set to
# the directory containing this file.
#
# This file may not rely on the value of any variable other than
# LOCAL_DIR; do not use any conditionals, and do not look up the
# value of any variable that isn't set in this file or in a file that
# it includes.
#

PRODUCT_MAKEFILES := \
    $(LOCAL_DIR)/<first_product_name>.mk \

COMMON_LUNCH_CHOICES := \
    <first_product_name>-userdebug \         # 加上这个之后就能在lunch的时候看到新增的选项了
```

我们在 `lunch` 流程可以看到 `<first_product_name>-userdebug` 这个选项，就是由这里的COMMON_LUNCH_CHOICES变量配置的。 `userdebug` 为 build type。有以下几种配置：

- eng : 对应到工程版。编译打包所有模块。同时ro.secure=0， ro.debuggable=1， ro.kernel.android.checkjni=1，表示adbd处于ROOT状态，所有调试开关打开
- userdebug : 对应到用户调试版。同时ro.debuggable=1，打开调试开关，但并没有放开ROOT权限
- user : 对应到用户版。同时ro.secure=1，ro.debuggable=0，关闭调试开关，关闭ROOT权限。最终发布到用户手上的版本，通常都是user版。
- user-root : ro.secure=1，ro.debuggable=0，关闭调试开关，打开ROOT权限。

## 3. 创建一个包含主板特定配置的 `BoardConfig.mk` Makefile

创建模拟器可以直接用

```Bash
include $(SRC_TARGET_DIR)/board/generic_x86_64/BoardConfig.mk
```

可参考：
device/linaro/dragonboard/rb5/BoardConfig.mk

## 4. 创建system properties

~~在~~ ~~`<board_name>`~~ ~~目录（~~ ~~`vendor/<company_name>/<board_name>`~~ ~~）中创建一个~~ ~~`system.prop`~~ ~~文件。~~

现在，配置properties一般使用的是

```Bash
PRODUCT_SYSTEM_PROPERTIES +=
PRODUCT_SYSTEM_EXT_PROPERTIES +=
PRODUCT_PRODUCT_PROPERTIES +=
PRODUCT_VENDOR_PROPERTIES +=
PRODUCT_ODM_PROPERTIES +=
```

这几个makefile变量

```Bash
56 # PRODUCT_<PARTITION>_PROPERTIES are lists of property assignments
57 # that go to <partition>/build.prop. Each property assignment is
58 # "key = value" with zero or more whitespace characters on either
59 # side of the '='.
60 _product_list_vars += \
61     PRODUCT_SYSTEM_PROPERTIES \
62     PRODUCT_SYSTEM_EXT_PROPERTIES \
63     PRODUCT_VENDOR_PROPERTIES \
64     PRODUCT_ODM_PROPERTIES \
65     PRODUCT_PRODUCT_PROPERTIES
```

具体规则可以看build/make/core/sysprop.mk

以下为MTK平台组件化编译内容(MSSI/TSSI)

组件化编译(Vendor Freeze)

TSSI分离编译：[内部信息_已编辑]

当时提交的分类：[内部信息_已编辑]

仓库位置：[内部信息_已编辑]

sys与vnd组件的相关模块：[内部信息_已编辑]
