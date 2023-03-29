#include <gtest/gtest.h>
#include "../core/Hakutaku.hpp"

TEST(MemTrap, Main) {
    std::string packageName = "bin.mt.plus";
    pid_t pid = Hakutaku::getPidByPidOf(packageName);
    auto process = Hakutaku::openProcess(pid);
    Pointer address = 0x12345678;
    if(process.isMissingPage(address)) {
        printf("missing page\n");
    } else {
        printf("not missing page\n");
    }
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}