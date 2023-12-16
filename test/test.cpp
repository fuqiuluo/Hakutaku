#include <gtest/gtest.h>
#include "proc.h"
#include "process.h"
#include "reader.h"
#include "writer.h"
#include "searcher.h"
#include "buffer_utils.h"
#include <unistd.h>
#include <sys/mman.h>

#include <iostream>

using namespace hak;

TEST(MEM, ABC) {

}

TEST(APP, GetPid) {
    std::string packageName = "bin.mt.plus";
    try {
        auto pid = hak::find_process(packageName);
        std::cout << "pid: " << pid << "\n";
        auto process = std::make_shared<hak::process>(pid);
        process->set_memory_mode(memory_mode::SYSCALL);
    } catch (std::exception& e) {
        std::cout << "error: " << e.what() << "\n";
    }
}

auto main() -> int {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}