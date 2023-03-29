#include <gtest/gtest.h>
#include "../core/Hakutaku.hpp"

TEST(MemoryOperate, Read) {
    std::string packageName = "com.example.app";
    pid_t pid = Hakutaku::getPid(packageName);
    Hakutaku::Process process = Hakutaku::openProcess(pid);
    Pointer baseAddress = process.findModuleBase("libexample.so");
    // 读取
    int value;
    process.read(baseAddress + 0x1234, &value, sizeof(int));

}

TEST(MemoryOperate, Write) {
    std::string packageName = "com.example.app";
    pid_t pid = Hakutaku::getPid(packageName);
    Hakutaku::Process process = Hakutaku::openProcess(pid);
    Pointer baseAddress = process.findModuleBase("libexample.so");
    // 写入
    int write_data = 0x12345678;
    process.write(baseAddress + 0x1234, &write_data, sizeof(int));
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}