#include <gtest/gtest.h>
#include "proc.h"
#include "process.h"
#include "reader.h"
#include "writer.h"
#include "searcher.h"

#include <iostream>

using namespace hak;

TEST(APP, GetPid) {
    std::string packageName = "bin.mt.plus";
    try {
        auto pid = hak::find_process(packageName);
        std::cout << "pid: " << pid << "\n";
        auto process = std::make_shared<hak::process>(pid);
        process->set_memory_mode(memory_mode::SYSCALL);
        auto maps = process->get_maps();
        auto searcher = hak::memory_searcher(process);
        std::string a = "16D;32F;123456D";
        searcher.searchNumber(a, hak::type_i32);

    } catch (std::exception& e) {
        std::cout << "error: " << e.what() << "\n";
    }
}

auto main() -> int {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}