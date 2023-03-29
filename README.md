# Hakutaku

Language: [简体中文](README.md) | [English](README_en.md)

![GitHub release](https://img.shields.io/github/release/fuqiuluo/Hakutaku.svg)
![GitHub license](https://img.shields.io/github/license/fuqiuluo/HakutakU.svg)
![GitHub repo size](https://img.shields.io/github/repo-size/fuqiuluo/Hakutaku.svg)
![GitHub top language](https://img.shields.io/github/languages/top/fuqiuluo/Hakutaku.svg)

Hakutaku名为白泽，寓意无所不知。一款内存修改器/按键精灵核心程序，支持安卓SDK24以上版本。

Hakutaku is a memory modification tool based on the Android SDK 24+.

# 使用方法

#### 当前支持的内存范围

```c++
#define RANGE_ALL 8190 // 全内存
#define RANGE_BAD 2 // B内存
#define RANGE_V 4 // V内存
#define RANGE_CA 8 // CA内存
#define RANGE_CB 16 // CB内存
#define RANGE_CD 32 // CD内存
#define RANGE_CH 64 // CH内存
#define RANGE_JH 128 // JH内存
#define RANGE_A 256 // A内存
#define RANGE_XS 512 // XS内存
#define RANGE_S 1024 // S内存
#define RANGE_AS 2048 // AS内存
#define RANGE_OTHER 4096
```

#### 状态码/错误码
```c++
// 搜索返回code
#define RESULT_SUCCESS 0 // 成功
#define RESULT_OFE (-1) // 打开文件错误
#define RESULT_ADDR_NRA (-2) // 地址不可读
#define RESULT_ADDR_NWA (-3) // 地址不可写
#define RESULT_UNKNOWN_WORK_MODE (-4) // 未知内存读取/写入模式
#define RESULT_EMPTY_MAPS (-5) // maps为空
#define RESULT_NOT_FUNDAMENTAL (-6) // 非基础类型(使用无需提供size的模式搜索值时，只支持基础类型)
#define RESULT_EMPTY_RESULT (-7) // 搜索结果为空
// 以上代码不需要写进你自己的文件里面，只是给你看看
```

#### 搜索值支持的Sign
```c++
// 搜索值支持的Sign
#define SIGN_EQ 0 // 等于
#define SIGN_NE 1 // 不等于
#define SIGN_GT 2 // 大于
#define SIGN_GE 3 // 大于等于
#define SIGN_LT 4 // 小于
#define SIGN_LE 5 // 小于等于
// 以上代码不需要写进你自己的项目里面，只是给你看看
```

#### 使用注意

- Process对象禁止复制，不然将导致/proc/[pid]/mem句柄提前释放，从而导致异常。

### [获取Pid Demo](/test/pidof.cpp)

### [挂起/恢复/杀死进程 Demo](/test/stop_and_recover.cpp)

### [获取ModuleBaseAddress Demo](/test/get_module_base.cpp)

### [读取/写入内存 Demo](/test/read_and_write.cpp)

### [获取指定内存范围Maps](/test/get_maps.cpp)

### [判断当前内存是否缺页](/test/is_memory_trap.cpp)

### [设置当前读取/写入内存模式](/test/set_memory_mode.cpp)

### [单值搜索/过滤操作](/test/search_and_filter.cpp)

### [内存工具](/test/mem_tool.cpp)

### 触摸工具（TODO）