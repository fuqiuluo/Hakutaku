#include <gtest/gtest.h>

#include "process.h"
#include "proc.h"
#include "writer.h"
#include "reader.h"

using namespace hak;

TEST(MemoryOperate, Read) {
    std::string packageName = "bin.mt.plus";
    try {
        auto pid = hak::find_process(packageName);
        std::cout << "pid: " << pid << "\n";
        auto process = std::make_shared<hak::process>(pid);
        process->set_memory_mode(memory_mode::SYSCALL);
        auto reader = hak::memory_reader(process);

        pointer address = 0x1234567;
        i32 my_int = reader.read_i32(address);
    } catch (std::exception& e) {
        std::cout << "error: " << e.what() << "\n";
    }
}

TEST(MemoryOperate, Write) {
    std::string packageName = "bin.mt.plus";
    try {
        auto pid = hak::find_process(packageName);
        std::cout << "pid: " << pid << "\n";
        auto process = std::make_shared<hak::process>(pid);
        process->set_memory_mode(memory_mode::SYSCALL);
        auto writer = hak::memory_writer(process);

        pointer address = 0x1234567;
        writer.write_i32(address, 777);
    } catch (std::exception& e) {
        std::cout << "error: " << e.what() << "\n";
    }
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}