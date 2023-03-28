#include <gtest/gtest.h>
#include "../core/Hakutaku.hpp"

#include <iostream>

TEST(MEM, mem) {
    char* tmp = new char[64]{};
    memset(tmp, 1, 64);
    char* a = tmp + 1;
    printf("v: %d\n", *a);
    int* b = reinterpret_cast<int *>(a);
    printf("v: %d\n", *b);
    delete[] tmp;
}

TEST(APP, GainMAPS) {
    std::string packageName = "bin.mt.plus";
    pid_t pid = Hakutaku::getPid(packageName);
    ASSERT_NE(pid, 0);
    printf("Pid: %d\n", pid);
    Hakutaku::Process process = Hakutaku::openProcess(pid);

    process.workMode = MODE_SYSCALL;
    Hakutaku::Maps maps = Hakutaku::Maps();
    int result = process.getMapsLite(maps, RANGE_ALL);
    ASSERT_EQ(result, 0);
    printf("Maps Size: %zu\n", maps.size());
    if (!maps.empty()) {
        printf("Maps Start: 0x%04lx\n", maps.start()->start());
        printf("Maps End: 0x%04lx\n", maps.end()->end());

        Pointer start = process.findModuleBase("/apex/com.android.runtime/lib/bionic/libc.so");
        printf("/apex/com.android.runtime/lib/bionic/libc.so Base: 0x%04lx\n", start);

        maps.clear();
        ASSERT_EQ(process.getMaps(maps, RANGE_A), 0);
        Hakutaku::Utils::printMaps(maps);

        Hakutaku::MemoryTools memoryTools = process.getMemoryTools();
        int value = 1;
        int ret = memoryTools.search(&value, 4, RANGE_A);
        printf("%d, %d\n", ret, memoryTools.empty());
        std::for_each(memoryTools.getResult().begin(), memoryTools.getResult().end(), [&](const auto &ptr) {
            printf("0x%04lx\n", ptr);
        });

        ret = memoryTools.search(&value, 4, RANGE_A);
        printf("%d, %d\n", ret == 0, !memoryTools.empty());
        std::for_each(memoryTools.getResult().begin(), memoryTools.getResult().end(), [&](const auto &ptr) {
            printf("0x%04lx\n", ptr);
        });
    }
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}