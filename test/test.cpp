#include <gtest/gtest.h>
#include "../core/Hakutaku.hpp"

#include <iostream>

TEST(APP, GainMAPS) {
    std::string packageName = "bin.mt.plus";
    pid_t pid = Hakutaku::getPid(packageName);
    ASSERT_NE(pid, 0);
    printf("Pid: %d\n", pid);
    Hakutaku::Process process = Hakutaku::openProcess(pid);

    printf("Bit64: %d\n", process.is64Bit());

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

        Hakutaku::MemorySearcher searcher = process.getSearcher();
        searcher.searchNumber("10086D;0D;1D", RANGE_A);
        printf("ResultSize: %zu\n", searcher.size());

        searcher.filterNumber("10086D;1D;");
        printf("ResultSize: %zu\n", searcher.size());

        std::for_each(searcher.getResult().begin(), searcher.getResult().end(), [&](const std::set<Pointer> &ptrSet) {
            printf("==========> Group[%zu]\n", ptrSet.size());
            std::for_each(ptrSet.begin(), ptrSet.end(), [&](const auto &ptr) {
                int value = 0;
                process.read(ptr, &value, sizeof(int));
                printf("0x%04lx: %d\n", ptr, value);
            });
        });
    }
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}