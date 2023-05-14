#include <gtest/gtest.h>
#include "../core/Hakutaku.hpp"

TEST(GetModuleBase, NoMatchBss) {
    std::string packageName = "com.example.app";
    pid_t pid = Hakutaku::getPid(packageName);
    std::shared_ptr<Hakutaku::Process> process = Hakutaku::openProcess(pid);
    Pointer baseAddress = process->findModuleBase("libexample.so");
}

TEST(GetModuleBase, MatchBss) {
    std::string packageName = "com.example.app";
    pid_t pid = Hakutaku::getPid(packageName);
    std::shared_ptr<Hakutaku::Process> process = Hakutaku::openProcess(pid);
    Pointer baseAddress = process->findModuleBase("libexample.so", true);
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}