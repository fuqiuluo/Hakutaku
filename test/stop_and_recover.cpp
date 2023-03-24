#include <gtest/gtest.h>
#include "../core/Hakutaku.hpp"

TEST(APP, StopAndRecover) {
    std::string packageName = "bin.mt.plus";
    Pid pid = Hakutaku::getPid(packageName);
    ASSERT_NE(pid, 0);
    Hakutaku::Process process = Hakutaku::openProcess(pid);
    process.stop();
    sleep(5);
    process.recover();
}