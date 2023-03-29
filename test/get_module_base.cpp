#include <gtest/gtest.h>
#include "../core/Hakutaku.hpp"

TEST(GetPid, Pidof) {
    std::string packageName = "com.example.app";
    pid_t pid = Hakutaku::getPid(packageName);
    Hakutaku::Process process = Hakutaku::openProcess(pid);
    Pointer baseAddress = process.findModuleBase("libexample.so");
    // Pointer baseAddress = process.findModuleBase("libexample.so", true);
    // 如果第二个参数为true，则以[anon:.bss]段作为baseAddress
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}