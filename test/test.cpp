#include <gtest/gtest.h>
#include "../core/Hakutaku.hpp"

#include <iostream>

TEST(APP, GainMAPS) {
    std::string packageName = "bin.mt.plus";
    pid_t pid = Hakutaku::getPid(packageName);
    ASSERT_NE(pid, 0);
    Hakutaku::Process process = Hakutaku::openProcess(pid);
    process.workMode = MODE_SYSCALL;
    Hakutaku::Maps maps = Hakutaku::Maps();
    int result = process.getMaps(maps, RANGE_ALL);
    ASSERT_EQ(result, 0);
    printf("Maps Size: %zu\n", maps.size());
    if (!maps.empty()) {
        printf("Maps Start: %ld\n", maps.start()->start());
        printf("Maps End: %zu\n", maps.end()->end());

        Pointer start = maps.start()->start();
        int value;
        process.read(start, &value, sizeof(int));
        printf("value: %d\n", value);

        printf("/system/lib64/libnetdutils.so Base: %ld\n", process.findModuleBase("/system/lib64/libnetdutils.so"));
    }
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}