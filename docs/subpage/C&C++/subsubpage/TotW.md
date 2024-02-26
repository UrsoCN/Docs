# C++ Tips of the Week

> <https://abseil.io/tips/>

## Tip of the Week #1: string_view

1. 什么是string_view?

在clang中查找string_view， 需要安装ag(一个代码查找工具) `apt-get install silversearcher-ag`

``` shell
$ whereis c++      
c++: /usr/bin/c++ /usr/include/c++ /usr/share/man/man1/c++.1.gz
$ cd /usr/include/c++
$ ls -liah        
总计 44K
32901182 drwxr-xr-x  3 root root 4.0K  6月 10 12:11 .
31981572 drwxr-xr-x 70 root root  20K 11月 26 00:42 ..
32901183 drwxr-xr-x 12 root root  16K  9月  5 21:13 11
32899276 lrwxrwxrwx  1 root root   32  1月 31  2023 v1 -> ../../lib/llvm-14/include/c++/v1
$ cp -r ./v1 ~/Dev/c++/
```

先用whereis找到系统中c++ include的位置 `/usr/include/c++`，再将clang的c++ include复制到其他位置(防止误操作)

``` shell
$ cd ~/Dev/c++/v1
$ ag string_view\; .
string_view
20:        class basic_string_view;
56:    typedef basic_string_view<char> string_view;
57:    typedef basic_string_view<char8_t> u8string_view; // C++20
58:    typedef basic_string_view<char16_t> u16string_view;
59:    typedef basic_string_view<char32_t> u32string_view;
60:    typedef basic_string_view<wchar_t> wstring_view;
226:    class _LIBCPP_TEMPLATE_VIS basic_string_view;
228:typedef basic_string_view<char>     string_view;
230:typedef basic_string_view<char8_t>  u8string_view;
232:typedef basic_string_view<char16_t> u16string_view;
233:typedef basic_string_view<char32_t> u32string_view;
235:typedef basic_string_view<wchar_t>  wstring_view;

__format/format_arg.h
165:    basic_string_view<char_type> __string_view;

__filesystem/path.h
417:  typedef basic_string_view<value_type> __string_view;

tags
2642:__string_view	__filesystem/path.h	/^  typedef basic_string_view<value_type> __string_view;$/;"	t	class:path	typeref:typename:basic_string_view<value_type>
2643:__string_view	__format/format_arg.h	/^    basic_string_view<char_type> __string_view;$/;"	m	union:basic_format_arg::__anonce9fee33010a	typeref:typename:basic_string_view<char_type>
$ vim string_view
```

其中~/.vimrc如下

``` text
syntax enable "自动检测语法
syntax on "自动语法高亮
set number "显示行号
set helplang=cn "中文help
set autoindent "换行后自动缩进
```

``` C++

```
