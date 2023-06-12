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

        auto maps = process->get_maps();
        do {
            std::cout << std::hex << "address = " << maps->start() << ", missing = " << process->is_missing_page(maps->start()) << ", name = " << maps->module_name << "\n";
        } while ((maps = maps->next()));
    } catch (std::exception& e) {
        std::cout << "error: " << e.what() << "\n";
    }
}

auto main() -> int {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}