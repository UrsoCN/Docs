---
created: 2026-06-07 21:38:44 +0800
modified: 2026-06-07 21:38:44 +0800
tags:
  - ai-summary
  - article
---
# reflect-hpp: C++14 静态反射与 JSON 序列化库

> **项目地址**: [archibate/reflect-hpp](https://github.com/archibate/reflect-hpp)  
> **授权**: 未明确标注（GitHub 公开仓库）  
> **语言**: C++14  
> **形态**: 单头文件（`reflect.hpp` ~500 行）+ 可选 JSON 扩展（`reflect_json.hpp`）

---

## 什么是 reflect-hpp？

reflect-hpp 是一个轻量级的 C++14 **静态反射（static reflection）** 库。它通过一个 `REFLECT(...)` 宏让开发者声明结构体的成员名称，进而实现**编译期的成员遍历**——包括成员变量、成员函数、静态变量和静态函数。

以此为基础设施，配套的 `reflect_json.hpp` 可以**零手动代码**将任意带有 `REFLECT` 声明的 C++ 结构体自动序列化/反序列化为 JSON。

---

## 核心用法

### 基本反射声明

```cpp
#include "reflect.hpp"

struct Address {
    std::string country;
    std::string province;
    std::string city;

    void show() {
        std::cout << "Country: " << country << '\n';
    }

    std::string to_str() const {
        return country + " " + province + " " + city;
    }

    static void test() {
        std::cout << "static function test\n";
    }

    REFLECT(country, province, city, show, to_str, test);
};

struct Student {
    std::string name;
    int age;
    Address addr;

    REFLECT(name, age, addr);
};
```

`REFLECT(...)` 宏自动扩充为 `foreach_member_ptr<T>()` 静态方法，使得反射**完全编译期确定**，**零运行时开销**。

### JSON 序列化

```cpp
#include "reflect_json.hpp"

Student stu = { .name = "Peng", .age = 23, .addr = { "China", "Shanghai", "Shanghai" } };

// 序列化 → JSON 字符串
std::string json = reflect_json::serialize(stu);
// → {"addr":{"city":"Shanghai","country":"China","province":"Shanghai"},"age":23,"name":"Peng"}

// 反序列化 ← JSON 字符串
auto stu2 = reflect_json::deserialize<Student>(json);
// stu2.name == "Peng", stu2.age == 23, stu2.addr.country == "China"
```

---

## 技术亮点

### 1. 编译期成员枚举（无运行时开销）

`REFLECT` 宏展开后生成的是一个**模板静态方法**，不是运行时元数据表：

```cpp
// 宏展开大致是：
template <class This, class Func>
static constexpr void foreach_member_ptr(Func &&func) {
    func("country", &This::country);
    func("province", &This::province);
    func("city", &This::city);
    func("show", &This::show);
    func("to_str", &This::to_str);
    func("test", &This::test);
}
```

调用者传一个 lambda，编译器可以**完全内联展开**——某种意义上等价于手动写了所有成员访问。对比运行期反射（如 Qt MOC、C# Reflection），这里有本质的性能优势。

### 2. SFINAE 智能分派：区分变量、函数、静态成员

```cpp
template <class T>
struct get_member_kind;

template <class T, class C>
struct get_member_kind<T C::*> {
    // T C::* 是成员变量指针
    static constexpr member_kind value = member_kind::member_variable;
};

template <class T, class C, class ...Ts>
struct get_member_kind<T (C::*)(Ts...)> {
    // T (C::*)(Ts...) 是成员函数指针
    static constexpr member_kind value = member_kind::member_function;
};

template <class T, class ...Ts>
struct get_member_kind<T (*)(Ts...)> {
    // T (*)(Ts...) 是静态函数指针
    static constexpr member_kind value = member_kind::static_function;
};
```

通过**部分模板特化**在编译期区分成员种类，覆盖了 **7 种成员函数 cv-ref 限定符**（含 `const &`、`&&` 等标准变体）。C++17 以上还额外支持 `noexcept` 变体——这是同类小型库中比较少见的覆化程度。

### 3. `_foreach_visitor` 模式：SFINAE 过滤非数据成员

```cpp
template <class T, class Func>
struct _foreach_visitor {
    T &&object;
    Func &&func;

    template <class U>
    constexpr std::enable_if_t<get_member_kind<U>::value == member_kind::member_variable>
    operator()(const char *name, U member) const {
        func(name, object.*member);
    }

    constexpr void operator()(...) const {}
};
```

`foreach_member(object, func)` 在遍历时**只对成员变量调用回调**，函数成员被静默跳过。这样设计非常务实：多数时候你只关心数据成员（序列化、比较、复制），不想被函数签名干扰。

### 4. 运行时名字查询

```cpp
// 按名字+类型获取成员引用（带类型安全检查）
int &age = reflect::get_member<int>(stu, "age");

// 检查是否有某个成员
bool has = reflect::has_member<Student>("addr");

// 检查成员种类
bool is_func = reflect::is_member_kind<Student>("show", member_kind::member_function);
```

`try_get_member<Type>(obj, name)` 同时检查名字和类型是否匹配，`get_member` 在此基础上抛异常——在需要非侵入式配置或脚本绑定的场景中很有用。

### 5. 外部反射支持（非侵入式）

```cpp
struct Point { int x, y; };

// 不需要修改 Point 类的定义
REFLECT_TYPE(Point, x, y);
```

对于无法修改源码的第三方类型，可以在命名空间级别通过 `REFLECT_TYPE` 宏注册反射信息。

### 6. 调用链的 composability

由于 `foreach_member_ptr` 是模板方法，反射系统可以叠加使用：

```cpp
reflect_json::serialize(stu)
    → objToJson(stu)
    → foreach_member(stu, [&](key, value) { root[key] = objToJson(value); })
    → 对 addr（嵌套 struct）递归调用 foreach_member
```

这种设计使得**嵌套结构体天然递归序列化**——无需额外声明。

### 7. 头文件极轻量

| 文件 | 行数 | 依赖 |
|------|------|------|
| `reflect.hpp` | ~500 行 | C++14 标准库 |
| `reflect_json.hpp` | ~150 行 | jsoncpp + reflect.hpp |

作为对比，nlohmann/json 约 20,000+ 行，magic_get（PFR）约 3,000+ 行。

---

## 与 nlohmann::json 的对比

| 维度 | reflect-hpp + jsoncpp | nlohmann::json |
|------|----------------------|----------------|
| **范式** | **C++ 类型优先**：定义 struct + REFLECT → 自动 JSON | **JSON 优先**：构建 JSON 对象，通过 `adl_serializer` 转为 C++ 类型 |
| **侵入性** | 需要在类内部加 `REFLECT(...)` 宏（或外部 `REFLECT_TYPE`） | 零侵入：通过 ADL 重载 `to_json`/`from_json` 或特化 `adl_serializer` |
| **JSON 能力** | 依赖 jsoncpp，仅基本 JSON 读写，无格式化控制 | 内置完整：JSON Pointer/Patch/SAX、BSON/CBON/MessagePack、Schema 校验 |
| **反射通用性** | **不限于 JSON**：`reflect::foreach_member` 可用于调试打印、GUI 生成、配置映射、ORM | 仅 JSON 序列化，无独立反射层 |
| **编译速度** | 极快（~500 行 + 简单宏展开） | 较慢（~20K 行模板元编程） |
| **C++ 标准** | C++14 起 | C++11 起 |
| **成员函数反射** | ✅ 支持（变量/函数/静态可区分） | ❌ 不涉及 |
| **运行时名字查询** | ✅ `get_member<T>(obj, "name")` | ❌ 需手动写映射表 |
| **STL 容器** | `special_traits` 特化 vector/map | 原生深度支持所有 STL 容器 |
| **社区成熟度** | ⭐ 34 star，2 年未更新，个人项目 | ⭐ 43k+ star，活跃维护 |
| **错误的用户体验** | 模板报错可读性一般 | 早有优化，报错相对清晰 |

### 核心差异一句话

> nlohmann::json 是做**JSON 的**——给你一套强大的 JSON 工具箱，然后通过 ADL 适配到 C++ 类型。  
> reflect-hpp 是做**反射的**——给你一套编译期成员遍历的机制，JSON 只是其中一个应用场景。

---

## 适用场景

### reflect-hpp 更适合：
- 有大量 POD 结构体需要序列化/反序列化的 C++ 项目（游戏配置、协议定义）
- 希望用**一套反射声明同时服务 JSON / 二进制 / GUI 显示 / 日志打印**的场景
- 对头文件大小敏感、不想引入 20K 行 JSON 库的嵌入式或边缘场景
- 需要在运行时按名字动态访问成员（配置热加载、脚本绑定）
- 构建时间敏感的项目（`reflect.hpp` 编译极快）

### nlohmann::json 更适合：
- JSON 是核心数据格式（API 通信、配置文件、日志导出）
- 需要 JSON Schema 校验、Patch 操作等高级功能
- 需要处理任意动态 JSON（反序列化时不预先知道类型）
- 社区支持度和长期维护有要求

---

## 局限

- **两年前停更**（最后 commit 为 "fix member_kind"），无人维护
- `reflect_json.hpp` 强依赖 jsoncpp，不是自包含的 JSON 实现
- 无 CMake 集成支持（需要手动 include）
- 不支持 `std::optional`、`std::variant`、`std::unique_ptr` 等现代 C++17 类型的自动处理
- 运行时名字查询通过线性遍历实现（成员较少时没问题，几百个成员可能影响性能）
- 无错误处理和边界校验（deserialize 时如果 JSON 字段缺失会赋默认值而非报错）

---

## 总结

reflect-hpp 以一个极简的宏加少量模板代码，在 C++14 下实现了真正意义上的**编译期静态反射**。它的技术价值不在于 JSON 序列化这个应用（那是锦上添花），而在于 `foreach_member_ptr` 这个基础设施——一套**零开销、类型安全、成员种类可区分**的编译期成员枚举机制。

在 C++26 标准化静态反射（`std::meta` 提案）到来之前，这种宏 + SFINAE 的巧用方案是务实的选择。对于已经使用 nlohmann::json 的项目，不值得为 JSON 序列化切换到 reflect-hpp；但对于希望在 C++ 中获得轻量反射能力的项目，reflect-hpp 是一个值得参考的经典范例。
