#include <gtest/gtest.h>
#include "process.h"
#include "proc.h"
#include "proc_pagemap.h"

using namespace hak;

TEST(MemTrap, Main) {
    std::string packageName = "bin.mt.plus";
    try {
        auto pid = hak::find_process(packageName);
        std::cout << "pid: " << pid << "\n";
        auto process = std::make_shared<hak::process>(pid);
        process->set_memory_mode(memory_mode::SYSCALL);

        auto maps = process->get_maps();
        do {
            for (int i = 0; i < ((maps->end() - maps->start()) / getpagesize()); ++i) {
                auto entry = process->get_page_entry(maps->start() + i * getpagesize());
                std::cout << "present = " << entry.present << "\n";
                // 如果present为0代表缺页。
            }
        } while ((maps = maps->next()));
    } catch (std::exception& e) {
        std::cout << "error: " << e.what() << "\n";
    }
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}