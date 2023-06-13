#include <gtest/gtest.h>
#include "proc.h"
#include "process.h"
#include "reader.h"
#include "writer.h"
#include "searcher.h"
#include <unistd.h>
#include <sys/mman.h>

#include <iostream>

using namespace hak;

TEST(APP, GetPid) {
    std::string packageName = "bin.mt.plus";
    try {
        auto pid = hak::find_process(packageName);
        std::cout << "pid: " << pid << "\n";
        auto process = std::make_shared<hak::process>(pid);
        process->set_memory_mode(memory_mode::SYSCALL);
        auto searcher = hak::memory_searcher(process);
        auto reader = hak::memory_reader(process);
        searcher.set_memory_range(memory_range::A);
        auto size = searcher.searchNumber("1D;0D;1D", type_i32);
        std::cout << "search result size: " << size << "\n";

    } catch (std::exception& e) {
        std::cout << "error: " << e.what() << "\n";
    }
}

auto main() -> int {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}