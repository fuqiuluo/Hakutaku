#include <gtest/gtest.h>
#include "../core/Hakutaku.hpp"

TEST(ProcessTools, Is64Bit) {
    std::string packageName = "com.example.app";
    pid_t pid = Hakutaku::getPid(packageName);
    auto process = Hakutaku::openProcess(pid);

    bool is64 = process->is64Bit();
}

TEST(MemoryTools, PrintMaps) {
    std::string packageName = "com.example.app";
    pid_t pid = Hakutaku::getPid(packageName);
    auto process = Hakutaku::openProcess(pid);

    Hakutaku::Maps maps = Hakutaku::Maps();
    int result = process->getMaps(maps, RANGE_ALL);
    if(result == RESULT_SUCCESS) {
        Hakutaku::Utils::printMaps(maps);
    }
}

TEST(MemoryTools, HexDump) {
    std::string packageName = "com.example.app";
    pid_t pid = Hakutaku::getPid(packageName);
    auto process = Hakutaku::openProcess(pid);

    Pointer baseAddress = process->findModuleBase("libexample.so");
    Hakutaku::Utils::hexDump(process, baseAddress, 16);
    // 将打印16行的（16*8字节）的hex信息到控制台
    // Will print 16 lines (168 bytes) of hex information to the console
}

TEST(MemoryTools, DumpMem) {
    std::string packageName = "com.example.app";
    pid_t pid = Hakutaku::getPid(packageName);
    auto process = Hakutaku::openProcess(pid);

    Pointer baseAddress = process->findModuleBase("libexample.so");
    Hakutaku::Utils::dumpMemory(process, baseAddress, 1024, [](char buf[1024 * 4], size_t currBufSize) {
        // ...
    });
}

TEST(MemorySafe, PassInotify) {
    // 懒得解释这个有什么用，下面是源码
    // system("echo 0 > /proc/sys/fs/inotify/max_user_watches");
    Hakutaku::Platform::reInotify();
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}