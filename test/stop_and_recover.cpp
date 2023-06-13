#include <gtest/gtest.h>
#include "../core/Hakutaku.hpp"

TEST(APP, StopAndRecover) {
    std::string packageName = "bin.mt.plus";
    pid_t pid = Hakutaku::getPid(packageName);
    ASSERT_NE(pid, 0);
    auto process = Hakutaku::openProcess(pid);
    process->stop();
    sleep(5);
    process->recover();
}