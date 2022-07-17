# Hello, Cargo!

Cargo 是 Rust 的构建系统和包管理器。大多数 Rustacean 们使用 Cargo 来管理他们的 Rust 项目，因为它可以为你处理很多任务，比如构建代码、下载依赖库并编译这些库。（我们把代码所需要的库叫做 依赖（dependencies））。

- 可以使用 cargo new 创建项目。
- 可以使用 cargo build 构建项目。当项目最终准备好发布时，可以使用 cargo build --release 来优化编译项目。
- 可以使用 cargo run 一步构建并运行项目。
- 可以使用 cargo check 在不生成二进制文件的情况下构建项目来检查错误。
- 有别于将构建结果放在与源码相同的目录，Cargo 会将其放到 target/debug 目录。

|2022年7月15日|
|---:|
