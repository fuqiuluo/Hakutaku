#include <gtest/gtest.h>
#include "../core/Hakutaku.hpp"

TEST(MemoryOperate, SetMode) {
    std::string packageName = "com.example.app";
    pid_t pid = Hakutaku::getPid(packageName);
    auto process = Hakutaku::openProcess(pid);

    process->workMode = MODE_SYSCALL; // by syscall

    // process.workMode = MODE_MEM; // by mem file
    // This will happen open(proc/pid/mem)

    // process.workMode = MODE_DIRECT;
    // 直接读取内存，适用于注入当前程序进进程状态
    // Read the memory directly, suitable for injecting the current program into the process state
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}