# Hakutaku

Hakutaku名为白泽，寓意无所不知，

Hakutaku is a memory modification tool based on the Android SDK 24+.

一款内存修改器/按键精灵核心程序，支持安卓SDK24以上版本。

# 使用方法

- Process对象禁止复制，不然将导致/proc/[pid]/mem句柄提前释放，从而导致异常。

## 获取Pid

```c++
#include "core/Hakutaku.hpp"

int main() {
    // 通过遍历/proc
    pid_t pid = Hakutaku::getPid("com.example.app");
    // 通过执行pidof
    // pid_t pid = Hakutaku::getPidByPidOf("com.example.app");
    return 0;
}
```

## 挂起/恢复/杀死进程

```c++
int main() {
    pid_t pid = Hakutaku::getPid("com.example.app");
    Hakutaku::Process process = Hakutaku::openProcess(pid);
    process.stop();
    Hakutaku::Utils::sleep_ms(1000); // 暂停1秒
    process.recover();
    Hakutaku::Utils::sleep_ms(1000);
    process.kill();
    return 0;
}
```

## 获取ModuleBaseAddress

```c++
int main() {
    pid_t pid = Hakutaku::getPid("com.example.app");
    Hakutaku::Process process = Hakutaku::openProcess(pid);
    Pointer baseAddress = process.getModuleBaseAddress("libexample.so");
    // Pointer baseAddress = process.getModuleBaseAddress("libexample.so", true);
    // 如果第二个参数为true，则以[anon:.bss]段作为baseAddress
    return 0;
}
```

## 读取/写入内存

```c++
int main() {
    pid_t pid = Hakutaku::getPid("com.example.app");
    Hakutaku::Process process = Hakutaku::openProcess(pid);
    Pointer baseAddress = process.getModuleBaseAddress("libexample.so");
    // 读取
    int value;
    process.read(baseAddress + 0x1234, &value, sizeof(int));
    // 写入
    int write_data = 0x12345678;
    process.write(baseAddress + 0x1234, &write_data, sizeof(int));
    return 0;
}
```

## 获取指定内存范围Maps(Lite)

```c++
int main() {
    pid_t pid = Hakutaku::getPid("com.example.app");
    Hakutaku::Process process = Hakutaku::openProcess(pid);
    
    Hakutaku::Maps maps = Hakutaku::Maps();
    int result = process.getMapsLite(maps, RANGE_ALL); // 获取全内存
    if(result == RESULT_SUCCESS) {
        maps.clear(); //  清理map中上一次搜索的结果
    }
    int result = process.getMapsLite(RANGE_A | RANGE_CA | RANGE_CB); // 获取A,CA,CB内存
    
    
    // Lite模式不包含以下信息
    //
    //char perms[5]; // r-x
    //unsigned long inode; // inode
    //char name[512]; // 段名称
    //
    
    maps.clear(); //  清理map中上一次搜索的结果
    process.getMaps(maps, RANGE_ALL);
    Hakutaku::Utils::printMaps(maps); // 打印maps信息到控制台
    return 0;
}
```

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

## 判断当前内存是否缺页

```c++
int main() {
    pid_t pid = Hakutaku::getPid("com.example.app");
    Hakutaku::Process process = Hakutaku::openProcess(pid);
    Pointer address = 0x12345678;
    if(process.isMissingPage(address)) {
        printf("缺页\n");
    } else {
        printf("不缺页\n");
    }
    return 0;
}
```

## 设置当前读取/写入内存模式

```c++
int main() {
    pid_t pid = Hakutaku::getPid("com.example.app");
    Hakutaku::Process process = Hakutaku::openProcess(pid);
    process.workMode = MODE_SYSCALL; // by syscall
    // process.workMode = MODE_MEM; // by mem file
    // process.workMode = MODE_DIRECT; // 直接读取内存，适用于注入当前程序进进程状态
    return 0;
}
```

## ~~在指定内存页中进行搜索操作~~ （TODO）

```c++
int main() {
    pid_t pid = Hakutaku::getPid("com.example.app");
    Hakutaku::Process process = Hakutaku::openProcess(pid);
    Maps maps = process.getMapsLite(RANGE_A | RANGE_CA | RANGE_CB);
    // 搜索
    int search_data = 0x12345678;
    std::vector<Pointer> result = process.search(maps, &search_data, sizeof(int));
    // 写入
    int write_data = 0x87654321;
    for (Pointer pointer : result) {
        process.write(pointer, &write_data, sizeof(int));
    }
    return 0;
}
```

## 内存工具

```c++
int main() {
    // 以16进制形式打印(lines * 8)字节的内存
    Hakutaku::Utils::hexDump(process, address, lines);
    
    // 打印maps信息到控制台
    Hakutaku::Utils::printMaps(maps);
    
    // 懒得解释这个有什么用，下面是源码
    // system("echo 0 > /proc/sys/fs/inotify/max_user_watches");
    Hakutaku::Platform::reInotify();
    
    return 0;
}
```

## 触摸工具（TODO）

```c++
int main() {
    pid_t pid = Hakutaku::getPid("com.example.app");
    Hakutaku::Process process = Hakutaku::openProcess(pid);
    // 触摸Home
    Hakutaku::Touch::touchHome();
    // Todo...
}
```