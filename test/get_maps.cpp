#include <gtest/gtest.h>
#include "proc.h"
#include "process.h"

#include <iostream>

using namespace hak;

TEST(APP, GetMapsByPid) {
    std::string packageName = "bin.mt.plus";
    try {
        auto pid = hak::find_process(packageName);
        std::cout << "pid: " << pid << "\n";
        auto maps = hak::get_maps(pid, hak::memory_range::A);
        size_t size = 0;
        do {
            std::string range_name;
            if (maps->range == memory_range::CA) {
                range_name = "CA";
            } else if (maps->range == memory_range::CD) {
                range_name = "CD";
            } else if (maps->range == memory_range::AS) {
                range_name = "AS";
            } else if (maps->range == memory_range::A) {
                range_name = "A";
            } else if (maps->range == memory_range::CH) {
                range_name = "CH";
            } else if (maps->range == memory_range::J) {
                range_name = "J";
            } else if (maps->range == memory_range::JH) {
                range_name = "JH";
            } else if (maps->range == memory_range::PS) {
                range_name = "PS";
            } else if (maps->range == memory_range::CB) {
                range_name = "CB";
            } else if (maps->range == memory_range::BAD) {
                range_name = "B";
            } else if (maps->range == memory_range::V) {
                range_name = "V";
            } else if (maps->range == memory_range::XA) {
                range_name = "XA";
            } else if (maps->range == memory_range::S) {
                range_name = "S";
            } else if (maps->range == memory_range::OTHER) {
                range_name = "O";
            } else if (maps->range == memory_range::XS) {
                range_name = "XS";
            }
            std::cout << "Maps-start: " << std::hex << maps->start() << ", inode: " << maps->inode << ", name: 【" << maps->module_name << "】，range: " << std::dec << range_name << "\n";
            size += maps->end() - maps->start();
        } while ((maps = maps->next()));
        std::cout << "size: " << (double) size / (1024 * 1024) << " Mb\n";
    } catch (std::exception& e) {
        std::cout << "error: " << e.what() << "\n";
    }
}

TEST(APP, GetMapsByProcess) {
    std::string packageName = "bin.mt.plus";
    try {
        auto pid = hak::find_process(packageName);
        std::cout << "pid: " << pid << "\n";
        auto process = std::make_shared<hak::process>(pid);
        auto maps = process->get_maps(hak::memory_range::A);
        size_t size = 0;
        do {
            std::string range_name;
            if (maps->range == memory_range::CA) {
                range_name = "CA";
            } else if (maps->range == memory_range::CD) {
                range_name = "CD";
            } else if (maps->range == memory_range::AS) {
                range_name = "AS";
            } else if (maps->range == memory_range::A) {
                range_name = "A";
            } else if (maps->range == memory_range::CH) {
                range_name = "CH";
            } else if (maps->range == memory_range::J) {
                range_name = "J";
            } else if (maps->range == memory_range::JH) {
                range_name = "JH";
            } else if (maps->range == memory_range::PS) {
                range_name = "PS";
            } else if (maps->range == memory_range::CB) {
                range_name = "CB";
            } else if (maps->range == memory_range::BAD) {
                range_name = "B";
            } else if (maps->range == memory_range::V) {
                range_name = "V";
            } else if (maps->range == memory_range::XA) {
                range_name = "XA";
            } else if (maps->range == memory_range::S) {
                range_name = "S";
            } else if (maps->range == memory_range::OTHER) {
                range_name = "O";
            } else if (maps->range == memory_range::XS) {
                range_name = "XS";
            }
            std::cout << "Maps-start: " << std::hex << maps->start() << ", inode: " << maps->inode << ", name: 【" << maps->module_name << "】，range: " << std::dec << range_name << "\n";
            size += maps->end() - maps->start();
        } while ((maps = maps->next()));
        std::cout << "size: " << (double) size / (1024 * 1024) << " Mb\n";
    } catch (std::exception& e) {
        std::cout << "error: " << e.what() << "\n";
    }
}

auto main() -> int {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}