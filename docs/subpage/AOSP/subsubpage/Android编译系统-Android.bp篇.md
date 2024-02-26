# Android编译系统-Android.bp篇

> [Android.bp file introduction](https://laptrinhx.com/android-bp-file-introduction-662051062/)  
> [安卓编译系统中的Android.bp](https://note.qidong.name/2017/08/android-blueprint/)  
> [Android.bp Build Docs (android-8.0.0-r9)](https://note.qidong.name/demo/soong_build/)  
> [Android 10 根文件系统和编译系统(十八):Android.bp语法_cc_defaults-CSDN博客](https://blog.csdn.net/ldswfun/article/details/120730546)  
> [Android系统开发进阶-Android编译系统介绍](http://qiushao.net/2020/02/06/Android%E7%B3%BB%E7%BB%9F%E5%BC%80%E5%8F%91%E8%BF%9B%E9%98%B6/Android%E7%BC%96%E8%AF%91%E7%B3%BB%E7%BB%9F%E4%BB%8B%E7%BB%8D/)  
> [Android 编译之android.bp](https://www.jianshu.com/p/f69d1c381182)

AOSP中对soong编译系统的说明文档：`build/soong/README.md`。以下部分也会基于这份文档说明

Soong 是旧版基于 make 的 Android 构建系统的替代品。 它用 Android.bp 文件取代了 Android.mk 文件，后者是对要构建的模块的类似 JSON 的简单声明式描述。

可以查看 source.android.com 上的 [Simple Build Configuration](https://source.android.com/compatibility/tests/development/blueprints)，了解 Soong(blueprint)的测试。

## Android.bp file format

根据设计，Android.bp 文件非常简单。 文件中没有条件或控制流语句--任何复杂问题都由 Go 语言编写的构建逻辑来处理。 在可能的情况下，Android.bp 文件的语法和语义有意与 [Bazel BUILD files](https://www.bazel.io/versions/master/docs/be/overview.html) 相似。

### Modules

blueprint基本语法:
```
[module type] {
    name: "[name value]",                   // module name 必须存在且全局唯一
    [property1 name]："[property1 value]",
    [property2 name]："[property2 value]",
}
```

在已经使用Soong编译的项目中，使用`m soong_docs`可以生成`out/soong/docs/soong_build.html`，里面描述了所有的可用Modules及其Property。当前版本AOSP的[Soong list在线文档](https://ci.android.com/builds/latest/branches/aosp-build-tools/targets/linux/view/soong_build.html)

以下为cc_library的docs，内容包括**对该module的描述、可用的property列表、对可用property的描述**

> **cc_library**  
> cc_library creates both static and/or shared libraries for a device and/or host. By default, a cc_library has a single variant that targets the device. Specifying `host_supported: true` also creates a library that targets the host.  
> 
> name , srcs , exclude_srcs , defaults , host_supported , device_supported , afdo , aidl , allow_undefined_symbols , apex_available , arch , asflags , bazel_module , bootstrap ,   
> ...  
> visibility , vndk , whole_program_vtables , whole_static_libs , yacc  
> 
> **name** string, The name of the module. Must be unique across all modules.  
> **srcs** list of string, list of source files used to compile the C/C++ module. May be .c, .cpp, or .S files. srcs may reference the outputs of other modules that produce source files like genrule or filegroup using the syntax ":module".  
> **exclude_srcs** list of string, list of source files that should not be used to build the C/C++ module. This is most useful in the arch/multilib variants to remove non-common files  
> **defaults** list of string  
> ...


### File lists

File lists指的是 可以使用通配符匹配文件列表和输出路径的特殊property。

> 通配模式可以包含普通 Unix 通配符 *，例如 "\*.java"。 
> - Glob patterns can also contain a single ** wildcard as a path element, which will match zero or more path elements. For example, `java/**/*.java` will match `java/Main.java` and `java/com/android/Main.java` .  
> - Output path expansions take the format `:module` or `:module{.tag}` , where `module` is the name of a module that produces output files, and it expands to a list of those output files. With the optional `{.tag}` suffix, the module may produce a different list of outputs according to `tag` .  
> For example, a `droiddoc` module with the name "my-docs" would return its `.stubs.srcjar` output with "`:my-docs`" , and its `.doc.zip` file with "`:my-docs{.doc.zip}`" .  
> This is commonly used to reference `filegroup` modules, whose output files consist of their `srcs` 

比如说Bluetooth Apk的编译目标
```
android_app {
    name: "Bluetooth",
    defaults: ["bluetooth-module-sdk-version-defaults",
                "itel_btapp_defaults"],

    srcs: [
        "src/**/*.java",
        ":statslog-bluetooth-java-gen",
        ":bluetooth-proto-enums-java-gen",
    ],
    ...
    min_sdk_version: "Tiramisu",
    sdk_version: "module_current",
}
```

**srcs**中的"`src/**/*.java`",就是由通配符表示的src递归目录下的所有`.java`文件。"`:statslog-bluetooth-java-gen`"和"`:bluetooth-proto-enums-java-gen`"都是根据一些规则生成的代码文件，可以在当前/其他`Android.bp`中找到输出文件：`com/android/bluetooth/BluetoothStatsLog.java`以及"`:srcs_bluetooth_protos`"表示的`filegroup`

`:bluetooth-proto-enums-java-gen`的完整形式是`namespace:module name`

```
filegroup {
  name: "srcs_bluetooth_protos",
  srcs: [
      "a2dp/enums.proto",
      "enums.proto",
      "hci/enums.proto",
      "hfp/enums.proto",
      "smp/enums.proto",
  ],
}
```

### Variables

`Android.bp` 文件可以包含顶层 variable 赋值。

Variables的作用域包括其所声明文件的其余部分，以及任何子 Android.bp 文件。在变量被引用之前，除非使用 += 赋值对其进行附加，变量是不可变的。

比如：`subdirs`是一个文件级的顶层Variables，指定后会查找次级目录下的`Android.bp`。 类似于`Android.mk`中常用的`include $(call all-subdir-makefiles)`。

比如：`packages/modules/Bluetooth/Android.bp`的`subdirs`:
```
subdirs = [
    "apex",
    "binder",
    "build",
    "btif",
    "btcore",
    "common",
    "audio_a2dp_hw",
    "audio_bluetooth_hw",
    "audio_hal_interface",
    "audio_hearing_aid_hw",
    "gd",
    "hci",
    "utils",
    "device",
    "stack",
    "osi",
    "embdrv",
    "service",
    "include",
    "main",
    "bta",
    "vendor_libs",
    "test",
    "types",
    "udrv",
    "tools",
    "proto",
]
```


### Comments

Android.bp 文件可以包含 C 风格的多行 `/* */` 和 C++ 风格的单行 `//` 注释。

### Types

变量和属性都是强类型的，Variables是根据第一次赋值动态分配的，而Properties则是根据模块类型静态分配的。支持的类型有：

* Bool (`true` or `false`)
* Integers (`int`)
* Strings (`"string"`)
* Lists of strings (`["string1", "string2"]`)
* Maps (`{key1: "value1", key2: ["value2"]}`)

Maps可以是任何类型的值，包括嵌套映射。Lists和Maps的最后一个值后面可以有逗号。

Strings可以包含双引号，例如`"cat \"a b\""` 。

### Operators

String、String Lists和Maps可以使用 `+` 操作符进行追加。Integers可以使用 `+` 操作符求和。Appending a map produces the union of keys in both maps, appending the values of any keys that are present in both maps.(不好翻译自己看吧:)

### Defaults modules

Defaults modules可用于表示在多个模块中重复相同的属性。
例如：

```
cc_defaults {
    name: "gzip_defaults",
    shared_libs: ["libz"],
    stl: "none",
}

cc_binary {
    name: "gzip",
    defaults: ["gzip_defaults"],
    srcs: ["src/test/minigzip.c"],
}
```

### Packages

构建工作按Packages进行组织，每个软件包都是相关文件的集合，并以模块的形式说明了它们之间的依赖关系。

Packages被定义为包含一个名为 "Android.bp"文件的目录，该文件位于构建工作的顶层目录之下，其名称是相对于顶层目录的路径。软件包包括其目录中的所有文件，以及目录下的所有子目录，但本身包含`Android.bp`文件的子目录除外。

"Android.bp"Packages中的模块和包含的文件是模块的一部分。

例如，在下面的目录树中（其中 `.../android/` 是顶级 Android 目录），有两个包：`my/app` 和子包 `my/app/tests`。请注意，`my/app/data` 并不是一个Package，而是属于Package `my/app` 的一个目录。(即在这个语境下，只有包含Android.bp的目录才是一个Package)

```
    .../android/my/app/Android.bp
    .../android/my/app/app.cc
    .../android/my/app/data/input.txt
    .../android/my/app/tests/Android.bp
    .../android/my/app/tests/test.cc
```

这是基于Bazel构建工具的Packages的概念。

"Packages"模块类型允许指定软件包的相关信息。每个软件包只能指定一个 `package` 模块，如果同一软件包目录中有多个 `.bp` 文件，强烈建议在 `Android.bp` 文件中指定 `package` 模块（如果需要）。

与大多数模块类型不同，`package` 没有`name`属性。相反，名称会被设置为软件包的名称，例如，如果软件包位于 `top/intermediate/package` 中，那么软件包的名称就是 `//top/intermediate/package`。

比如，蓝牙中的顶层package：

```
package {
    default_applicable_licenses: ["packages_apps_Bluetooth_license"],
}
```

例如，以下内容将设置软件包中定义的所有模块的默认可见性，以及任何未设置自身默认可见性的子软件包（无论它们是否与`package`模块位于同一个`.bp`文件中）的默认可见性。

```
package {
    default_visibility: [":__subpackages"]
}
```

### Referencing Modules

一个Module可以通过name被其他module引用。

```
cc_binary {
    name: "app",
    shared_libs: ["libfoo"],
}
```

显然，只有当源代码树中只有一个 `libfoo` 模块时，这种方法才有效(也即module的name必须全局唯一)。对于较大的构建树，确保这种名称唯一性可能是个大问题。且我们可能还希望在多个相互排斥的子树（例如，实现不同的设备）中使用相同的名称，以便描述功能等同的模块。于是引入了Soong命名空间。

#### Namespaces

Android.bp 文件中的"soong_namespace {..}"定义了一个命名空间。例如，在

```
soong_namespace {
    ...
}
...
```

Android.bp里告知Soong，在package所在目录的子目录下的module name都是唯一的。而在该目录外可能会存在同名module，比如说在`device/google/bonito/Android.bp`定义soong_namespace，在`device/google/bonito/pixelstats`和`device/google/coral/pixelstats`里是可以存在同名module的。

命名空间的名称就是其目录的路径。因此，上例中命名空间的名称是 `device/google/bonito`。

隐式**global reference**与整个源代码树相对应。它的名称为空。

模块名称的**local reference**是包含它的最小命名空间。假设源代码树中有 `device/my` 和 `device/my/display` 命名空间。如果 `libfoo` 模块定义在 `device/my/display/lib/Android.bp` 中，则其名称空间为 `device/my/display`。

#### Name Resolution

module引用的形式决定了Soong如何定位模块。

对于"//_scope_:_name_"形式的**全局引用**，Soong会验证是否存在名为"_scope_"的命名空间，然后验证它是否包含"_name_"模块并使用它。Soong在解析 Android.bp 文件时，会在开头验证"_scope_"中只有一个"_name_"。

本地引用的形式为"_name_"，解析时需要在一个或多个命名空间中查找"_name_"模块。默认情况下，只在全局名称空间中查找"_name_"（换句话说，只考虑不属于明确定义的作用域的模块）。通过 `soong_namespaces` 的 `imports` 属性，可以指定从哪里查找模块。例如，"device/google/bonito/Android.bp "包含

```
soong_namespace {
    imports: [
        "hardware/google/interfaces",
        "hardware/google/pixel",
        "hardware/qcom/bootctrl",
    ],
}
```

对 `"libpixelstats"`的引用将解析到在`hardware/google/pixel/pixelstats/Android.bp`中定义的module，因为该module位于`hardware/google/pixel`命名空间中。

#### Referencing modules in makefiles

在我们逐步将 makefile 转换为 Android.bp 文件的同时，Android 构建是由 Android.bp 和 Android.mk 文件混合描述的，Android.mk 文件中定义的模块可以引用 Android.bp 文件中定义的模块。例如，Android.mk 文件中定义的二进制文件可能依赖于已转换的 Android.bp 文件中定义的库。

在 Android.bp 文件中定义的、属于全局命名空间的模块可以在 makefile 中引用，而无需额外工作。如果模块属于显式命名空间，则只有在命名空间的名称被添加到 PRODUCT_SOONG_NAMESPACES 变量的值之后，才能从 makefile 中引用该模块。

请注意，makefile 没有命名空间的概念，通过 PRODUCT_SOONG_NAMESPACES 公开具有相同模块的命名空间可能会导致 Make 失败。例如，同时公开 `device/google/bonito` 和 `device/google/coral` 命名空间会导致 Make 失败，因为它会看到 `pixelstats-vendor` 模块的两个目标。

### Visibility

module的 Visibility 属性控制模块是否能被其他包使用。基于 Bazel 可见性机制，模块总是对同一Package中声明的其他模块可见。

如果指定了 `visibility` 属性，则必须至少包含一条规则。

属性中的每条规则都必须采用以下形式之一：(不翻译了)

* `["//visibility:public"]`: Anyone can use this module.
* `["//visibility:private"]`: Only rules in the module's package (not its subpackages) can use this module.
* `["//visibility:override"]`: Discards any rules inherited from defaults or a creating module. Can only be used at the beginning of a list of visibility rules.
* `["//some/package:__pkg__", "//other/package:__pkg__"]`: Only modules in `some/package` and `other/package` (defined in `some/package/*.bp` and `other/package/*.bp`) have access to this module. Note that sub-packages do not have access to the rule; for example, `//some/package/foo:bar` or `//other/package/testing:bla` wouldn't have access. `__pkg__` is a special module and must be used verbatim. It represents all of the modules in the package.
* `["//project:__subpackages__", "//other:__subpackages__"]`: Only modules in packages `project` or `other` or in one of their sub-packages have access to this module. For example, `//project:rule`, `//project/library:lib` or `//other/testing/internal:munge` are allowed to depend on this rule (but not `//independent:evil`)
* `["//project"]`: This is shorthand for `["//project:__pkg__"]`
* `[":__subpackages__"]`: This is shorthand for `["//project:__subpackages__"]`
where `//project` is the module's package, e.g. using `[":__subpackages__"]` in `packages/apps/Settings/Android.bp` is equivalent to `//packages/apps/Settings:__subpackages__`.
* `["//visibility:legacy_public"]`: The default visibility, behaves as`//visibility:public` for now. It is an error if it is used in a module.

`//visibility:public` 和 `//visibility:private` 的可见性规则不能与任何其他可见性规范结合使用，但允许 `//visibility:public` 覆盖通过 `defaults` 属性导入的可见性规范。

`vendor/` 之外的包不能让自己对 `vendor/` 中的特定包可见，例如 `libcore` 中的模块不能声明它对 `vendor/google` 可见，而是必须使用 `//vendor:__subpackages__` 使自己对 `vendor/` 中的所有包可见。(也就是使用`//vendor:__subpackages__`申明当前package对vendor下的package可见)

OS部门就把`/vendor/mediatek/proprietary/packages/modules/Bluetooth/framework/Android.bp`中的`framework-bluetooth`设定为vendor可见以配合T-hub(这里的impl_library_visibility是用于library模块的Visibility 属性)

```
54 // post-jarjar version of framework-bluetooth
55 java_sdk_library {
56     name: "framework-bluetooth",
57     defaults: [
58         "framework-module-defaults",
59         "framework-bluetooth-defaults",
60     ],
61 
62     jarjar_rules: ":bluetooth-jarjar-rules",
63     installable: true,
64     optimize: {
65         enabled: false
66     },
67     hostdex: true, // for hiddenapi check
68 
69     impl_library_visibility: [
70         "//external/sl4a/Common",
71         "//frameworks/opt/wear",
72         "//packages/modules/Bluetooth/android/app/tests/unit",
73         "//packages/modules/Bluetooth/service",
74         "//packages/modules/Connectivity/nearby/tests/multidevices/clients/test_support/fastpair_provider",
75         "//packages/services/Car/car-builtin-lib",
76         ":__subpackages__",
77         "//vendor/mediatek/proprietary/packages/modules/Bluetooth/android/app/tests/unit",
78         "//vendor/mediatek/proprietary/packages/modules/Bluetooth/service",
79         // T-HUB Core[OS]: add by senquan.chen 20221205 start
80         "//vendor:__subpackages__",
81         // T-HUB Core[OS]: add by senquan.chen 20221205 end
82     ],
83 
84     apex_available: [
85         "com.android.bluetooth",
86     ],
87 
88     permitted_packages: [
89         "com.mediatek.bt",
90         "android.bluetooth",
91         // Created by jarjar rules.
92         "com.android.bluetooth.x",
93     ],
94     lint: {
95         strict_updatability_linting: true,
96     },
97 }
```

如果模块未指定`visibility`属性，则它使用模块包中`package`模块的`default_visibility`属性。

如果未为模块的包设置`default_visibility`属性，则它将使用其最近的上级包的`default_visibility`指定。

如果找不到`default_visibility`属性，则模块使用全局默认值`//visibility:legacy_public`。

`visibility`属性对默认模块没有影响，尽管它确实适用于使用它的任何非默认模块(defaults module)。要设置 defaults module的可见性，使用 defaults 模块上的`defaults_visibility`属性;不要与package模块上的`default_visibility`属性混淆。

一旦构建完全切换到 soong，可能会进行全局重构以将其更改为`//visibility:private`，此时所有当前未指定`default_visibility`属性的包都将更新为添加`default_visibility = [//visibility：legacy_public]`。然后，owner有责任将其替换为更合适的可见性。

### Formatter

Soong 包含一个用于 Android.bp 文件的规范格式化程序，类似于[gofmt](https://golang.org/cmd/gofmt/)。 要以递归方式重新格式化当前目录中的所有 Android.bp 文件，可以执行执行：`bpfmt -w .`

规范格式包括 4 个空格缩进、多元素List的每个元素后面的换行符，Lists和Maps中始终包含尾随逗号。

### Convert Android.mk files

Soong 包含一个工具，可以将 Android.mk 文件转换为 Android.bp 文件：`androidmk Android.mk > Android.bp`

该工具可转换变量、模块、注释和一些条件，但任何自定义 Makefile 规则、复杂条件或额外包含都必须手动转换。

#### Differences between Android.mk and Android.bp

* Android.mk 文件通常具有多个同名模块（例如，对于库的静态和共享版本，或者主机和设备版本）。Android.bp 文件要求每个模块都有唯一的名称，但单个模块可以构建为多个变体，例如添加`host_supported:true`。androidmk 转换器可能会生成多个相互冲突的模块，其中 `target: { android： { }, host： { } }` 块中存在差异的必须手动将其解析为单个模块。

### Conditionals

Soong 故意不支持 Android.bp 文件中的大多数条件编译(Conditionals)。我们建议从构建中删除大多数条件编译。有关如何删除条件编译的一些示例，请参阅"最佳实践"文档：(`build/soong/docs/best_practices.md#removing-conditionals`)。

Soong 原生支持的大多数Conditionals都转换为 map 属性。 构建模块时，将选择映射中的一个属性，并将其值附加到模块顶层具有相同名称的属性。

比如，为了支持特定于体系结构的代码：

```
cc_library {
    ...
    srcs: ["generic.cpp"],
    arch: {
        arm: {
            srcs: ["arm.cpp"],
        },
        x86: {
            srcs: ["x86.cpp"],
        },
    },
}
```

在构建 arm 模块时，将构建`generic.cpp`和`arm.cpp`源文件。在为 x86 构建时，将构建`generic.cpp`和`x86.cpp`源文件。

#### Soong Config Variables

在转换包含Conditionals的Vendor模块时，可以使用描述模块类型、变量和可能值的`soong_config_*`模块，通过 Soong 配置变量来支持简单的条件：

```
soong_config_module_type {
    name: "acme_cc_defaults",
    module_type: "cc_defaults",
    config_namespace: "acme",
    variables: ["board"],
    bool_variables: ["feature"],
    value_variables: ["width"],
    properties: ["cflags", "srcs"],
}

soong_config_string_variable {
    name: "board",
    values: ["soc_a", "soc_b", "soc_c"],
}
```

本例定义了一个新的`acme_cc_defaults`模块类型，它扩展了 `cc_defaults`模块类型，增加了三个基于变量 `board`, `feature` 和 `width` 的条件变量，可以影响属性 `cflags` 和 `srcs`。此外，每个条件变量将包含一个 `conditions_default` 属性，可在以下条件变量中影响 `cflags` 和 `srcs`：

* bool_variables（如 `feature`）：变量未指定或未设置为真值
* value_variables（如 `width`）：变量未指定
* variables（例如 `board`）：变量未指定或变量被设置为给定模块中未使用的字符串。

例如，在 `board` 中，如果 `board` 条件变量包含 `soc_a` 和 `conditions_default` 属性，当 board=soc_b 时，将使用 `conditions_default` 下的 `cflags` 和 `srcs` 值。要指定不修改 `soc_b` 的属性，可以设置 `soc_b： {},`.

变量值可通过产品的 "BoardConfig.mk "文件设置：(也就是说，仅有Android.bp仍然不能实现条件编译)
```
$(call soong_config_set,acme,board,soc_a)
$(call soong_config_set,acme,feature,true)
$(call soong_config_set,acme,width,200)
```

`acme_cc_defaults`模块类型可以在定义它的文件中定义后的任何地方使用，也可以导入到另一个文件中：
```
soong_config_module_type_import {
    from: "device/acme/Android.bp",
    module_types: ["acme_cc_defaults"],
}
```

它可以像其他模块类型一样被使用：

```
acme_cc_defaults {
    name: "acme_defaults",
    cflags: ["-DGENERIC"],
    soong_config_variables: {
        board: {
            soc_a: {
                cflags: ["-DSOC_A"],
            },
            soc_b: {
                cflags: ["-DSOC_B"],
            },
            conditions_default: {
                cflags: ["-DSOC_DEFAULT"],
            },
        },
        feature: {
            cflags: ["-DFEATURE"],
            conditions_default: {
                cflags: ["-DFEATURE_DEFAULT"],
            },
        },
        width: {
            cflags: ["-DWIDTH=%s"],
            conditions_default: {
                cflags: ["-DWIDTH=DEFAULT"],
            },
        },
    },
}

cc_library {
    name: "libacme_foo",
    defaults: ["acme_defaults"],
    srcs: ["*.cpp"],
}
```

使用上面的`BoardConfig.mk`代码段，`libacme_foo`将使用 `cflags: "-DGENERIC -DSOC_A -DFEATURE -DWIDTH=200"`.

或者使用 `DefaultBoardConfig.mk`：

```
SOONG_CONFIG_NAMESPACES += acme
SOONG_CONFIG_acme += \
    board \
    feature \
    width \

SOONG_CONFIG_acme_feature := false
```

那么 `libacme_foo` 将使用 `cflags: "-DGENERIC -DSOC_DEFAULT -DFEATURE_DEFAULT -DSIZE=DEFAULT"`。

`soong_config_module_type` 模块在用于封装默认模块（`cc_defaults`、`java_defaults` 等）时效果最佳，vendor的所有其他模块都可以使用正常的命名空间和可见性规则来引用这些模块。

## Build logic

构建逻辑是使用 [blueprint](http://godoc.org/github.com/google/blueprint) 框架用 Go 语言编写的。 构建逻辑接收使用反射法解析为 Go 结构的模块定义，并生成构建规则。 构建规则由 blueprint 收集并写入 [ninja](http://ninja-build.org) 构建文件。

## Environment Variables Config File

在startup过程中，Soong可以从预先指定的配置文件中加载环境变量。这些环境变量可用于控制编译行为。例如，这些变量可以决定是否在编译过程中使用remote-execution。

`ANDROID_BUILD_ENVIRONMENT_CONFIG_DIR`环境变量用于指定搜索配置文件的目录。`ANDROID_BUILD_ENVIRONMENT_CONFIG`变量决定要在配置目录中搜索的配置文件的名称。例如，以下编译命令将从 `build/soong` 目录中的 `example_config.json` 文件加载 `ENV_VAR_1` 和 `ENV_VAR_2` 环境变量。

```
ANDROID_BUILD_ENVIRONMENT_CONFIG_DIR=build/soong \
  ANDROID_BUILD_ENVIRONMENT_CONFIG=example_config \
  build/soong/soong_ui.bash
```

## Other documentation

* [Best Practices](docs/best_practices.md)
* [Build Performance](docs/perf.md)
* [Generating CLion Projects](docs/clion.md)
* [Generating YouCompleteMe/VSCode compile\_commands.json file](docs/compdb.md)
* Make-specific documentation: [build/make/README.md](https://android.googlesource.com/platform/build/+/master/README.md)

## Developing for Soong

不重要，不写了

## Contact

Email android-building@googlegroups.com (external) for any questions, or see [go/soong](http://go/soong) (internal).
