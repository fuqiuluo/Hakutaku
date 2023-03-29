#include <gtest/gtest.h>
#include "../core/Hakutaku.hpp"

TEST(GetMaps, Lite) {
    std::string packageName = "com.example.app";
    pid_t pid = Hakutaku::getPid(packageName);
    Hakutaku::Process process = Hakutaku::openProcess(pid);

    Hakutaku::Maps maps = Hakutaku::Maps();
    int result = process.getMapsLite(maps, RANGE_ALL); // get full memory
    if(result == RESULT_SUCCESS) {
        maps.clear(); //  Clean up the results of the previous search in the map
    }

    int result2 = process.getMapsLite(maps, RANGE_A | RANGE_CA | RANGE_CB); // Get A, CA, CB memory
}

// Lite模式不包含以下信息
// Lite mode does not contain the following information
//char perms[5]; // r-x
//unsigned long inode; // inode
//char name[512]; // 段名称

TEST(GetMaps, Full) {
    std::string packageName = "com.example.app";
    pid_t pid = Hakutaku::getPid(packageName);
    Hakutaku::Process process = Hakutaku::openProcess(pid);

    Hakutaku::Maps maps = Hakutaku::Maps();
    process.getMaps(maps, RANGE_ALL);
    Hakutaku::Utils::printMaps(maps); // Print maps information to the console
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}