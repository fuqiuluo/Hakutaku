#include <gtest/gtest.h>
#include "../core/Hakutaku.hpp"

TEST(GetPid, Pidof) {
    std::string packageName = "bin.mt.plus";
    Pid pid = Hakutaku::getPidByPidOf(packageName);
    ASSERT_NE(pid, 0);
}

TEST(GetPid, File) {
    std::string packageName = "bin.mt.plus";
    Pid pid = Hakutaku::getPid(packageName);
    ASSERT_NE(pid, 0);
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}