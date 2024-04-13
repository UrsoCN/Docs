# Android编译系统-Android.mk篇

本篇主要介绍Android.mk，会附带一部分makefile内容，关于makefile的详细内容可以看[Android 编译之make基础](https://www.jianshu.com/p/169397686d58)、[GNU make
](https://www.gnu.org/software/make/manual/make.html#Target_002dspecific)

这里也有AOSP内的介绍`build/make/README.md`：

> This is the Makefile-based portion of the Android Build System.
> 
> For documentation on how to run a build, see `build/make/Usage.txt`
> 
> For a list of behavioral changes useful for Android.mk writers see `build/make/Changes.md`
> 
> For an outdated reference on Android.mk files, see
> `build/make/core/build-system.html`. Our Android.mk files look similar,
> but are entirely different from the Android.mk files used by the NDK build
> system. When searching for documentation elsewhere, ensure that it is for the
> platform build system -- most are not.
> 
> This Makefile-based system is in the process of being replaced with [Soong], a
> new build system written in Go. During the transition, all of these makefiles
> are read by [Kati], and generate a ninja file instead of being executed
> directly. That's combined with a ninja file read by Soong so that the build
> graph of the two systems can be combined and run as one.
> 
> [Kati]: https://github.com/google/kati
> [Soong]: https://android.googlesource.com/platform/build/soong/+/master

本篇内容主要是`build/make/core/build-system.html`和`build/make/Changes.md`的内容

`build/make/core/build-system.html`的前半部分作为Android Build System的设计目标及思想，建议各位阅读原文。

## How to add another component to the build - Android.mk templates

对于一个新库、一个新应用或一个新的可执行文件，每种常见类型的模块，templates 目录中都有相应的文件。通常只需复制其中之一，并填写您自己的值就足够了。一些更深奥的值不包含在模板中，而只是记录在这里，关于使用自定义工具生成文件的文档也是如此。
大多数情况下，只需在templates中查找 TODO 注释并按照它所说的进行操作即可。

- Apps

    Use the templates/apps file.
    This template is pretty self-explanitory. See the variables below for more details.

- Java Libraries

    Use the templates/java_library file.

    需要注意 LOCAL_MODULE 的值将成为编译产物 jar 文件的名称。（实际上现在，我们还没有制作 jar 文件，只是 .class 文件的目录，但该目录名是根据您输入LOCAL_MODULE的内容命名的）。此名称将是依赖于 java 库的模块中 LOCAL_JAVA_LIBRARIES 变量中的名称。

- C/C++ Executables

    Use the templates/executable file, or the templates/executable_host file.
    
    此模板有几个通常不需要的额外选项。请删除不需要的那些，并删除 TODO 注释。它使其余部分更易于阅读，如果以后再次需要它们，可以随时参考模板。

    默认情况下，在target上，它们内置于 `/system/bin` 中，在host上，它们内置于 `/host/bin` 中。这些可以通过设置LOCAL_MODULE_PATH来覆盖。

    有关更多信息，请参阅 _Putting targets elsewhere_。

- Shared Libraries

    Use the templates/shared_library file, or the templates/shared_library_host file.

- Static Libraries

    Use the templates/static_library file, or the templates/static_library_host file.

    请记住，在target上，我们使用共享库，在host上，我们使用静态库，因为可执行文件大小不是一个大问题，并且它简化了 SDK 中的分发。

## Using Custom Tools

如果你有一个为你生成源文件的工具，则可以让构建系统为其获取正确的依赖项。这里有几个例子。其中`$@`是“当前编译目标”的 make 内置变量。红色部分是您需要更改的部分。

需要在声明 LOCAL_PATH 和 LOCAL_MODULE 之后使用这个方法，由于`$(local-intermediates-dir)`和`$(local-host-intermediates-dir)`宏使用这些变量来确定文件的位置。

### Example 1

在这里，有一个生成的文件，称为`chartables.c`，它不依赖于任何东西。并由`$(HOST_OUT_EXECUTABLES)/dftables`的工具构建。请注意，在倒数第二行中，已在工具上创建依赖项。

```
intermediates:= $(local-intermediates-dir)
GEN := $(intermediates)/chartables.c
$(GEN): PRIVATE_CUSTOM_TOOL = $(HOST_OUT_EXECUTABLES)/dftables $@
$(GEN): $(HOST_OUT_EXECUTABLES)/dftables
        $(transform-generated-source)
LOCAL_GENERATED_SOURCES += $(GEN)
```

### Example 2

这里是一个假设的例子，我们使用 `cat` 来转换文件。假装它做了一些有用的事情。请注意，我们如何使用名为 PRIVATE_INPUT_FILE 的特定于目标的变量来存储输入文件的名称。

```
intermediates:= $(local-intermediates-dir)
GEN := $(intermediates)/file.c
$(GEN): PRIVATE_INPUT_FILE  $(LOCAL_PATH)/input.file
$(GEN): PRIVATE_CUSTOM_TOOL = cat $(PRIVATE_INPUT_FILE) > $@
$(GEN): $(LOCAL_PATH)/file.c
        $(transform-generated-source)
LOCAL_GENERATED_SOURCES += $(GEN)
```

### Example 3

如果有多个名称相似的文件，并且使用相同的工具，则可以将它们组合在一起。（这里 `*.lut.h` 文件是生成的文件，`*.cpp` 文件是输入文件）

```
intermediates:= $(local-intermediates-dir)
GEN := $(addprefix $(intermediates)/kjs/, \
            array_object.lut.h \
            bool_object.lut.h \
        )
$(GEN): PRIVATE_CUSTOM_TOOL = perl libs/WebKitLib/WebKit/JavaScriptCore/kjs/create_hash_table $< -i > $@
$(GEN): $(intermediates)/%.lut.h : $(LOCAL_PATH)/%.cpp
        $(transform-generated-source)
LOCAL_GENERATED_SOURCES += $(GEN)
```

## Platform specific conditionals

有时需要专门为不同的平台设置flag标志。下面列出了不同的构建系统定义的变量将设置为哪些值，以及一些示例。

对于设备构建，TARGET_OS是`linux`(我们使用的是 linux!)，TARGET_ARCH是`arm`。

对于模拟器构建，TARGET_OS和TARGET_ARCH设置为与平台上的HOST_OS和HOST_ARCH相同。TARGET_PRODUCT 是要为其构建的目标硬件/产品的名称。值`sim`用于模拟器。我们还没有考虑过此处将要发生的全部自定义范围，但可能还会在此处指定其他 UI 配置。

|HOST_OS|HOST_ARCH|HOST_BUILD_TYPE|
|:--:|:--:|:--:|
|linux|x86|release|
|darwin| |debug|
|(cygwin)|||

|TARGET_OS|TARGET_ARCH|TARGET_BUILD_TYPE|TARGET_PRODUCT|
|:--:|:--:|:--:|:--:|
|linux|arm|release|sim|
|darwin|x86|debug|dream|
|(cygwin)| | |sonner|

### Some Examples

```
ifeq ($(TARGET_BUILD_TYPE),release)
LOCAL_CFLAGS += -DNDEBUG=1
endif

# from libutils
ifeq ($(TARGET_OS),linux)
# Use the futex based mutex and condition variable
# implementation from android-arm because it's shared mem safe
LOCAL_SRC_FILES += futex_synchro.c
LOCAL_LDLIBS += -lrt -ldl
endif
```

## Putting modules elsewhere

如果你的模块放在某个地方，但需要让它们在其他地方构建，可以参阅这一部分。这样做的一个用途是将文件放在根文件系统上，而不是它们通常放在 /system 中的位置。将以下行添加到您的 Android.mk：

```
LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT_SBIN)
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_SBIN_UNSTRIPPED)
```

对于可执行文件和库，您还需要指定`LOCAL_UNSTRIPPED_PATH`位置，因为在目标构建中，我们会保留未剥离的可执行文件，以便 GDB 可以找到symbols。

在 config/envsetup.make (新版本AOSP为envsetup.sh) 中可以找到查找定义构建事物位置的所有变量。

仅供参考：如果要将可执行文件安装到 /sbin，您可能还希望在 Android.mk 中将 LOCAL_FORCE_STATIC_EXCUTABLE true，这将强制链接器仅接受静态库。

## Android.mk variables

这些是您在 Android.mk 文件中经常看到的变量，按字母顺序列出。

关于变量命名的说明：

- `LOCAL_` - 这些变量是按模块设置的。它们会被 `include $(CLEAR_VARS)` 清除，因此在include后，`LOCAL_`变量为空。在大多数模块中使用的大多数变量都是`LOCAL_`变量。
- `PRIVATE_` - 这些变量 make-target-specific 的变量。这意味着它们只能在该模块的命令中使用。这也意味着，他们不太可能被之后加入的模块修改。此 [make 文档链接](http://www.gnu.org/software/make/manual/make.html#Target_002dspecific)详细介绍了target-specific的变量。
- `INTERNAL_` - 这些变量对于构建系统的运行至关重要，因此不应该创建、修改这样命名的变量。
- `HOST_` 和 `TARGET_` - 它们包含特定于主机或目标构建的目录和定义。不要在生成文件中设置以 `HOST_` 或 `TARGET_` 开头的变量。
- `BUILD_` 和 `CLEAR_VARS` - 它们包含要包含的明确定义的模板生成文件的名称。比如`CLEAR_VARS`和`BUILD_HOST_PACKAGE`。
- 任何其他名称都是公平的，您可以在 Android.mk 中使用。但是，请记住，这是一个**非递归构建系统**，因此变量可能会被稍后包含的其他 Android.mk 更改，并且在执行规则/模块的命令时可能有所不同。

以下为一些常用的变量，重要的部分会翻译：

- LOCAL_ASSET_FILES
    In Android.mk files that include $(BUILD_PACKAGE) set this to the set of files you want built into your app. Usually: `LOCAL_ASSET_FILES += $(call find-subdir-assets)`
    This will probably change when we switch to ant for the apps' build system.
- LOCAL_CC
    If you want to use a different C compiler for this module, set `LOCAL_CC` to the path to the compiler. If `LOCAL_CC` is blank, the appropriate default compiler is used.
- LOCAL_CXX
    If you want to use a different C compiler for this module, set `LOCAL_CXX` to the path to the compiler. If `LOCAL_CXX` is blank, the appropriate default compiler is used.
- LOCAL_CFLAGS
    If you have additional flags to pass into the C or C compiler, add them here. For example: `LOCAL_CFLAGS += -DLIBUTILS_NATIVE=1`
- LOCAL_CPPFLAGS
    If you have additional flags to pass into only the C compiler, add them here. For example: `LOCAL_CPPFLAGS += -ffriend-injection`
    `LOCAL_CPPFLAGS` is guaranteed to be after `LOCAL_CFLAGS` on the compile line, so you can use it to override flags listed in `LOCAL_CFLAGS`.
- LOCAL_CPP_EXTENSION
    If your C files end in something other than ".cpp", you can specify the custom extension here. For example:`LOCAL_CPP_EXTENSION.cc`
    Note that all C files for a given module must have the same extension; it is not currently possible to mix different extensions.
- LOCAL_NO_DEFAULT_COMPILER_FLAGS
    Normally, the compile line for C and C files includes global include paths and global cflags. If `LOCAL_NO_DEFAULT_COMPILER_FLAGS` is non-empty, none of the default includes or flags will be used when compiling C and C files in this module. `LOCAL_C_INCLUDES`, `LOCAL_CFLAGS`, and `LOCAL_CPPFLAGS` will still be used in this case, as will any `DEBUG_CFLAGS` that are defined for the module.
- LOCAL_COPY_HEADERS (deprecated)
- LOCAL_COPY_HEADERS_TO (deprecated)
- LOCAL_C_INCLUDES
    C/C++ 附加include目录
    Additional directories to instruct the C/C compilers to look for header files in. These paths are rooted at the top of the tree. Use LOCAL_PATH if you have subdirectories of your own that you want in the include paths. For example:
    ```
    LOCAL_C_INCLUDES += extlibs/zlib-1.2.3
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/src
    ```
    You should not add subdirectories of include to LOCAL_C_INCLUDES, instead you should reference those files in the #include statement with their subdirectories. For example:
    ```
    #include <utils/KeyedVector.h>
    not #include <KeyedVector.h>
    ```
    There are some components that are doing this wrong, and should be cleaned up.
- LOCAL_MODULE_TAGS
    Set LOCAL_MODULE_TAGS to any number of whitespace-separated tags. If the tag list is empty or contains droid, the module will get installed as part of a make droid. Modules with the tag shell_$(TARGET_SHELL) will also be installed. Otherwise, it will only get installed by running make <your-module> or with the make all pseudotarget.
- LOCAL_REQUIRED_MODULES
    如果此模块安装了，还会安装它所需的所有模块。
    Set LOCAL_REQUIRED_MODULES to any number of whitespace-separated module names, like "libblah" or "Email". If this module is installed, all of the modules that it requires will be installed as well. This can be used to, e.g., ensure that necessary shared libraries or providers are installed when a given app is installed.
- LOCAL_FORCE_STATIC_EXECUTABLE
    强制静态链接
    If your executable should be linked statically, set LOCAL_FORCE_STATIC_EXECUTABLEtrue. There is a very short list of libraries that we have in static form (currently only libc). This is really only used for executables in /sbin on the root filesystem.
- LOCAL_GENERATED_SOURCES
    Files that you add to LOCAL_GENERATED_SOURCES will be automatically generated and then linked in when your module is built. See the Custom Tools template makefile for an example.
- LOCAL_JAVACFLAGS
    If you have additional flags to pass into the javac compiler, add them here. For example: `LOCAL_JAVACFLAGS += -Xlint:deprecation`
- LOCAL_JAVA_LIBRARIES
    添加javalib(*.jar)
    When linking Java apps and libraries, LOCAL_JAVA_LIBRARIES specifies which sets of java classes to include. Currently there are two of these: core and framework. In most cases, it will look like this: `LOCAL_JAVA_LIBRARIES := core framework`
    Note that setting LOCAL_JAVA_LIBRARIES is not necessary (and is not allowed) when building an APK with "include $(BUILD_PACKAGE)". The appropriate libraries will be included automatically.
- LOCAL_LDFLAGS
    链接库文件目录，这个和`LOCAL_LDLIBS`主要区别如下：(慎用)
    `LOCAL_LDFLAGS`出现在最终链接器命令行中的对象文件和库列表之前，这是要放置影响链接器行为的实际“标志”的地方。`LOCAL_LDLIBS`出现在最终链接命令行中的对象文件和库列表之后，这是您想要放置实际系统库依赖项的地方。之所以存在这种区别，是因为静态链接在Unix上的工作方式，即对象文件、静态库和共享库的顺序对于确定最终结果非常重要，有时您确实需要确保在另一个结果之前/之后出现一些内容。
    最后，建议遵循以下原则，即：
    - 将真正的链接器标志放入`LOCAL_LDFLAGS`
    - 将系统库依赖项放入`LOCAL_LDLIBS`中
    - 仅对系统库依赖项使用`LOCAL_LDLIBS`。如果您想指向另一个库，最好在`LOCAL_STATIC_LIBRARIES`和`LOCAL_SHARED_LIBRARIES`中列出它们(即使这意味着为它们定义一个PREBUILT_XXX模块)，因为这样可以让构建系统自动为您处理依赖关系和排序。
    You can pass additional flags to the linker by setting `LOCAL_LDFLAGS`. Keep in mind that the order of parameters is very important to ld, so test whatever you do on all platforms.
- LOCAL_LDLIBS
    LOCAL_LDLIBS allows you to specify additional libraries that are not part of the build for your executable or library. Specify the libraries you want in -lxxx format; they're passed directly to the link line. However, keep in mind that there will be no dependency generated for these libraries. It's most useful in simulator builds where you want to use a library preinstalled on the host. The linker (ld) is a particularly fussy beast, so it's sometimes necessary to pass other flags here if you're doing something sneaky. Some examples:
    ```
    LOCAL_LDLIBS += -lcurses -lpthread
    LOCAL_LDLIBS += -Wl,-z,origin
    ```
- LOCAL_NO_MANIFEST
    If your package doesn't have a manifest (AndroidManifest.xml), then set `LOCAL_NO_MANIFEST:=true`. The common resources package does this.
- LOCAL_PACKAGE_NAME
    LOCAL_PACKAGE_NAME is the name of an app. For example, Dialer, Contacts, etc. This will probably change or go away when we switch to an ant-based build system for the apps.
- LOCAL_PATH
    The directory your Android.mk file is in. You can set it by putting the following as the first line in your Android.mk: `LOCAL_PATH := $(my-dir)`
    The my-dir macro uses the MAKEFILE_LIST variable, so you must call it before you include any other makefiles. Also, consider that any subdirectories you inlcude might reset LOCAL_PATH, so do your own stuff before you include them. This also means that if you try to write several include lines that reference LOCAL_PATH, it won't work, because those included makefiles might reset LOCAL_PATH.
- LOCAL_POST_PROCESS_COMMAND
    For host executables, you can specify a command to run on the module after it's been linked. You might have to go through some contortions to get variables right because of early or late variable evaluation:
    ```
    module  $(HOST_OUT_EXECUTABLES)/$(LOCAL_MODULE)
    LOCAL_POST_PROCESS_COMMAND  /Developer/Tools/Rez -d DARWIN -t APPL\
        -d WXMAC -o $(module) Carbon.r
    ```
- LOCAL_PREBUILT_EXECUTABLES
    When including `$(BUILD_PREBUILT)` or `$(BUILD_HOST_PREBUILT)`, set these to executables that you want copied. They're located automatically into the right bin directory.
- LOCAL_PREBUILT_LIBS
    When including $(BUILD_PREBUILT) or $(BUILD_HOST_PREBUILT), set these to libraries that you want copied. They're located automatically into the right lib directory.
- LOCAL_SHARED_LIBRARIES
    These are the libraries you directly link against. You don't need to pass transitively included libraries. Specify the name without the suffix:
    ```
    LOCAL_SHARED_LIBRARIES  \
    libutils \
    libui \
    libaudio \
    libexpat \
    libsgl
    ```
- LOCAL_SRC_FILES
    The build system looks at `LOCAL_SRC_FILES` to know what source files to compile  `.cpp` `.c` `.y` `.l` `.java`. For lex and yacc files, it knows how to correctly do the intermediate `.h` and `.c/.cpp` files automatically. If the files are in a subdirectory of the one containing the `Android.mk`, prefix them with the directory name:
    ```
    LOCAL_SRC_FILES := \
        file1.cpp \
        dir/file2.cpp
    ```
- LOCAL_STATIC_LIBRARIES
    一般来讲，这里才是标识引用`lib`的最佳位置
    These are the static libraries that you want to include in your module. Mostly, we use shared libraries, but there are a couple of places, like executables in sbin and host executables where we use static libraries instead. 
    ```
    LOCAL_STATIC_LIBRARIES  \
    libutils \
    libtinyxml
    ```
- LOCAL_MODULE
    LOCAL_MODULE is the name of what's supposed to be generated from your Android.mk. For exmample, for libkjs, the LOCAL_MODULE is "libkjs" (the build system adds the appropriate suffix  .so .dylib .dll). For app modules, use LOCAL_PACKAGE_NAME instead of LOCAL_MODULE. We're planning on switching to ant for the apps, so this might become moot.
- LOCAL_MODULE_PATH
    Instructs the build system to put the module somewhere other than what's normal for its type. If you override this, make sure you also set LOCAL_UNSTRIPPED_PATH if it's an executable or a shared library so the unstripped binary has somewhere to go. An error will occur if you forget to.
    See Putting modules elsewhere for more.
- LOCAL_UNSTRIPPED_PATH
    Instructs the build system to put the unstripped version of the module somewhere other than what's normal for its type. Usually, you override this because you overrode LOCAL_MODULE_PATH for an executable or a shared library. If you overrode LOCAL_MODULE_PATH, but not LOCAL_UNSTRIPPED_PATH, an error will occur. 
    See Putting modules elsewhere for more.
- LOCAL_WHOLE_STATIC_LIBRARIES
    These are the static libraries that you want to include in your module without allowing the linker to remove dead code from them. This is mostly useful if you want to add a static library to a shared library and have the static library's content exposed from the shared library.
    ```
    LOCAL_WHOLE_STATIC_LIBRARIES := \
        libsqlite3_android
    ```
- LOCAL_YACCFLAGS
    Any flags to pass to invocations of yacc for your module. A known limitation here is that the flags will be the same for all invocations of YACC for your module. This can be fixed. If you ever need it to be, just ask. `LOCAL_YACCFLAGS  -p kjsyy`

## Implementation Details

除非要向构建系统添加新平台、新工具或添加新功能，否则不要修改 config 目录(新版AOSP为build/make)中的任何内容。一般来说，在你去这里闲逛之前，请咨询构建系统所有者（android-build-team）。也就是说，这里有一些关于引擎盖下发生的事情的说明。

### Environment Setup / buildspec.mk Versioning

为了方便用户在构建系统更改时、需要更改 buildspec.mk 或重新运行环境设置脚本时，它们在变量`BUILD_ENV_SEQUENCE_NUMBER`中包含版本号。如果此变量与生成系统预期的变量不匹配，则无法打印错误消息来解释所发生的情况。如果进行需要更新的更改，则需要更新两个位置，以便打印此消息。
- 在 config/envsetup.make 中，递增CORRECT_BUILD_ENV_SEQUENCE_NUMBER定义。
- 在 buildspec.mk.default 中，更新BUILD_ENV_SEQUENCE_DUMBER定义以匹配 config/envsetup.make 中的定义。

脚本会自动从生成系统获取值，不匹配时也会触发警告。

### Additional makefile variables

正常情况下不应当使用这些变量。在使用前咨询 android-build-team。这些主要用于解决其他问题或未完全正确完成的事情。

- LOCAL_ADDITIONAL_DEPENDENCIES
If your module needs to depend on anything else that isn't actually built in to it, you can add those make targets to `LOCAL_ADDITIONAL_DEPENDENCIES`. Usually this is a workaround for some other dependency that isn't created automatically.
- LOCAL_BUILT_MODULE
When a module is built, the module is created in an intermediate directory then copied to its final location. `LOCAL_BUILT_MODULE` is the full path to the intermediate file. See LOCAL_INSTALLED_MODULE for the path to the final installed location of the module.
- LOCAL_HOST
Set by the host_xxx.make includes to tell base_rules.make and the other includes that we're building for the host. Kenneth did this as part of openbinder, and I would like to clean it up so the rules, includes and definitions aren't duplicated for host and target.
- LOCAL_INSTALLED_MODULE
The fully qualified path name of the final location of the module. See `LOCAL_BUILT_MODULE` for the location of the intermediate file that the make rules should actually be constructing.
- LOCAL_REPLACE_VARS
Used in some stuff remaining from the openbinder for building scripts with particular values set,
- LOCAL_SCRIPTS
Used in some stuff remaining from the openbinder build system that we might find handy some day.
- LOCAL_MODULE_CLASS
Which kind of module this is. This variable is used to construct other variable names used to locate the modules. See base_rules.make and envsetup.make.
- LOCAL_MODULE_NAME
Set to the leaf name of the `LOCAL_BUILT_MODULE`. I'm not sure, but it looks like it's just used in the `WHO_AM_I` variable to identify in the pretty printing what's being built.
- LOCAL_MODULE_SUFFIX
The suffix that will be appended to `LOCAL_MODULE` to form `LOCAL_MODULE_NAME`. For example, `.so`, `.a`, `.dylib`.
- LOCAL_STRIP_MODULE
Calculated in base_rules.make to determine if this module should actually be stripped or not, based on whether `LOCAL_STRIPPABLE_MODULE` is set, and whether the combo is configured to ever strip modules. With Iliyan's stripping tool, this might change.
- LOCAL_STRIPPABLE_MODULE
Set by the include makefiles if that type of module is strippable. Executables and shared libraries are.
- LOCAL_SYSTEM_SHARED_LIBRARIES
Used while building the base libraries: libc, libm, libdl. Usually it should be set to "none," as it is in $(CLEAR_VARS). When building these libraries, it's set to the ones they link against. For example, libc, libstdc and libdl don't link against anything, and libm links against libc. Normally, when the value is none, these libraries are automatically linked in to executables and libraries, so you don't need to specify them manually.
