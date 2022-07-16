# guessing_game

## 根据输入打印输出

``` Rust
use std::io;

fn main() {
    println!("Guess the number!");

    println!("Please input your guess.");

    let mut guess = String::new();

    io::stdin()
        .read_line(&mut guess)
        .expect("Failed to read line");

    println!("You guessed: {guess}");
}
```

- 这里的`use`和`python`中的`import`以及`C/C++`中的`#include`作用类似，都是用来引入“库”。
- `println!`是一个宏，与函数不同，将在19章介绍。
- `let mut guess = String::new();`
  - 其中使用 let 语句来创建变量。
  - 其中`mut`代表mutable，可变的，即表示这是个可变的变量。
  - `::` 语法表明 `new` 是 `String` 类型的一个 关联函数（associated function）。关联函数是针对类型实现的，在这个例子中是 String，而不是 String 的某个特定实例。一些语言中把它称为 静态方法（static method）。
  - 总的来说，`let mut guess = String::new();` 这一行创建了一个可变变量，当前它绑定到一个新的 String 空实例上。
- `io::stdin().read_line(&mut guess).expect("Failed to read line");`
  - `stdin` 函数返回一个 `std::io::Stdin` 的实例，这代表终端标准输入句柄的类型。
  - `.read_line(&mut guess)`，调用 `read_line` 方法从标准输入句柄获取用户输入。我们还将 `&mut guess` 作为参数传递给 `read_line()` 函数，让其将用户输入储存到这个字符串中。
    - 其中`&` 表示这个参数是一个 引用（reference），它允许多处代码访问同一处数据，而无需在内存中多次拷贝。
  - `read_line` 会返回一个类型为 `Result` (一种枚举(enum)类型)的值。`Result` 的实例拥有 `expect` 方法。如果 `io::Result` 实例的值是 `Err`，`expect` 会导致程序崩溃，并显示当做参数传递给 `expect` 的信息。
- `println!("You guessed: {guess}");` 其中 `{}` 是预留在特定位置的占位符，这里的写法是字符串拼接变量的写法，于2021版本引入，经典的写法为`println!("You guessed: {}, guess");`

## 关于Creat

Rust 标准库中尚未包含随机数功能，所以我们需要添加一个外部的库，这里使用Creat来实现。

> 记住，crate 是一个 Rust 代码包。我们正在构建的项目是一个 二进制 crate，它生成一个可执行文件。 rand crate 是一个 库 crate，库 crate 可以包含任意能被其他程序使用的代码，但是不能自执行。
> Cargo 对外部 crate 的运用是其真正的亮点所在。在我们使用 rand 编写代码之前，需要修改 Cargo.toml 文件，引入一个 rand 依赖。现在打开这个文件并将下面这一行添加到 [dependencies] 片段标题之下。

在这里，我们引入`rand = "0.8.3"`，需要注意的是，这里的`0.8.3`是[语义化版本(Semantic Versioning)](https://semver.org/lang/zh-CN/)，这是一种定义版本号的标准。0.8.3 事实上是 `^0.8.3` 的简写，它表示任何至少是 `0.8.3` 但小于 `0.9.0` 的版本。保存Cargo.toml后重新进行编译会自动进行外部库的下载、编译。

### 关于Cargo.lock

> Cargo 有一个确保任何人在任何时候重新构建代码，都会产生相同的结果的机制：Cargo 只会使用你指定的依赖版本，除非你又手动指定了别的。例如，如果下周 rand crate 的 0.8.4 版本出来了，它修复了一个重要的 bug，同时也含有一个会破坏代码运行的缺陷。为了处理这个问题，Rust在你第一次运行 cargo build 时建立了 Cargo.lock 文件，我们现在可以在guessing_game 目录找到它。
> 当第一次构建项目时，Cargo 计算出所有符合要求的依赖版本并写入 Cargo.lock 文件。当将来构建项目时，Cargo 会发现 Cargo.lock 已存在并使用其中指定的版本，而不是再次计算所有的版本。这使得你拥有了一个自动化的可重现的构建。换句话说，项目会持续使用 0.8.3 直到你显式升级，多亏有了 Cargo.lock 文件。由于 Cargo.lock 文件对于“可重复构建”非常重要，因此它通常会和项目中的其余代码一样纳入到版本控制系统中。

### 手动更新creat/Cargo.lock

> 当你 确实 需要升级 crate 时，Cargo 提供了这样一个命令，update，它会忽略 Cargo.lock 文件，并计算出所有符合 Cargo.toml 声明的最新版本。Cargo 接下来会把这些版本写入 Cargo.lock 文件。
> 不过，Cargo 默认只会寻找大于 0.8.3 而小于 0.9.0 的版本。如果想要使用 0.9.0 版本的 rand 或是任何 0.9.x 系列的版本，则必须更新 Cargo.toml 文件。

## 添加更多功能

### 生成随机数字

``` Rust
use std::io;
use rand::Rng;

fn main() {
    println!("Guess the number!");

    let secret_number = rand::thread_rng().gen_range(1..=100);

    println!("The secret number is: {secret_number}");

    println!("Please input your guess.");

    let mut guess = String::new();

    io::stdin()
        .read_line(&mut guess)
        .expect("Failed to read line");

    println!("You guessed: {guess}");
}
```

- `use rand::Rng;` 其中 `Rng`(Random Number Generator?)是一个 `trait`(特性)，它定义了随机数生成器应实现的方法，想使用这些方法的话，此 `trait` 必须在作用域中。
- `let secret_number = rand::thread_rng().gen_range(1..=100);` 调用 `rand::thread_rng`的`gen_range`函数提供的实际使用的随机数生成器。`gen_range` 方法会获取一个范围表达式（range expression）作为参数，并生成一个在此范围(包含上下限，`gen_range(1..100)`只包含下限)之间的随机数。

> 注意：你不可能凭空就知道应该 use 哪个 trait 以及该从 crate 中调用哪个方法，因此每个crate 有使用说明文档。Cargo 有一个很棒的功能是：运行 cargo doc --open 命令来构建所有本地依赖提供的文档，并在浏览器中打开。例如，假设你对 rand crate 中的其他功能感兴趣，你可以运行 cargo doc --open 并点击左侧导航栏中的 rand。

### 比较猜测的数字和秘密数字

``` Rust
use rand::Rng;
use std::cmp::Ordering;
use std::io;

fn main() {
    println!("Guess the number!");

    let secret_number = rand::thread_rng().gen_range(1..=100);

    println!("The secret number is: {secret_number}");

    println!("Please input your guess.");

    // --snip--

    let mut guess = String::new();

    io::stdin()
        .read_line(&mut guess)
        .expect("Failed to read line");

    let guess: u32 = guess.trim().parse().expect("Please type a number!");

    println!("You guessed: {guess}");

    match guess.cmp(&secret_number) {
        Ordering::Less => println!("Too small!"),
        Ordering::Greater => println!("Too big!"),
        Ordering::Equal => println!("You win!"),
    }
}
```

- [对std::cmp::Ordering;及其方法的说明](https://kaisery.github.io/trpl-zh-cn/ch02-00-guessing-game-tutorial.html#%E6%AF%94%E8%BE%83%E7%8C%9C%E6%B5%8B%E7%9A%84%E6%95%B0%E5%AD%97%E5%92%8C%E7%A7%98%E5%AF%86%E6%95%B0%E5%AD%97)
- 这里使用一个 `match` 表达式，根据对 `guess` 和 `secret_number` 调用 `cmp` 返回的 `Ordering` 成员来决定接下来做什么。
- 一个 `match` 表达式由 分支（arms） 构成。一个分支包含一个 模式（pattern）和表达式开头的值与分支模式相匹配时应该执行的代码。Rust 获取提供给 match 的值并挨个检查每个分支的模式。match 结构和模式是 Rust 中强大的功能，它体现了代码可能遇到的多种情形，并帮助你确保没有遗漏处理。(switch case?)
- `let guess: u32 = guess.trim().parse().expect("Please type a number!");`
  - Rust 有一个静态强类型系统，同时也有类型推断。由于`guess`是`String`类型而`secret_number`是数字类型(Rust中默认是`i32`类型)，所以我们需要显式地对guess的类型进行转换。
  - 这里 Rust 允许用一个新值来 隐藏 （shadow） 之前的同名变量(guess) 之前的值。这个功能常用在需要转换值类型之类的场景。它允许我们复用 guess 变量的名字，而不是被迫创建两个不同变量。
  - 字符串的 parse 方法 将字符串转换成其他类型。这里用它来把字符串转换为数值。我们需要告诉 Rust 具体的数字类型，这里通过 let guess: u32 指定。guess 后面的冒号（:）告诉 Rust 我们指定了变量的类型。

### 添加循环

``` Rust
    loop {
        println!("Please input your guess.");

        let mut guess = String::new();

        io::stdin().read_line(&mut guess).expect("Failed to read line");

        let guess: u32 = guess.trim().parse().expect("Please type a number!");

        println!("You guessed: {guess}");

        match guess.cmp(&secret_number) {
            Ordering::Less => println!("Too small!"),
            Ordering::Greater => println!("Too big!"),
            Ordering::Equal => println!("You win!"),
        }
    }
```

- 用户可以通过`crtl + c`终止程序，也可以通过输入**非数字**跳出循环(抛出异常)，然而，这并不理想，我们想要当猜测正确的数字时游戏停止。

``` Rust
            Ordering::Equal => {
                println!("You win!");
                break;
            }
```

- 可以通过在 You win! 之后增加一行 `break`让用户在猜对了神秘数字后退出循环。

``` Rust
        let guess: u32 = match guess.trim().parse() {
            Ok(num) => num,
            Err(_) => continue,
        };
```

- 为了进一步改善游戏性，不要在用户输入非数字时崩溃，需要忽略非数字，让用户可以继续猜测。将 `expect` 调用换成 `match` 语句，以从遇到错误就崩溃转换为处理错误。

---

最终程序：(加入了判断debug模式的语句)

``` Rust
use rand::Rng;
use std::cmp::Ordering;
use std::io;

fn main() {
    println!("Guess the number!");

    let secret_number = rand::thread_rng().gen_range(1..=100);

    if cfg!(debug_assertions) {
        println!("The secret number is: {secret_number}");
    }

    loop {
        println!("Please input your guess.");

        let mut guess = String::new();

        io::stdin()
            .read_line(&mut guess)
            .expect("Failed to read line");

        // let guess = guess.trim().parse::<u32>().expect("Please type a number!");
        // let guess: u32 = guess.trim().parse().expect("Please type a number!");
        let guess: u32 = match guess.trim().parse() {
            Ok(num) => num,
            Err(_) => continue,
        };

        println!("You guessed: {guess}");

        match guess.cmp(&secret_number) {
            Ordering::Less => println!("Too small!"),
            Ordering::Greater => println!("Too big!"),
            Ordering::Equal => {
                println!("You win!");
                break;
            }
        }
    }
}
```
