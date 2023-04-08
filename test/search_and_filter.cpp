#include <gtest/gtest.h>
#include "../core/Hakutaku.hpp"

TEST(MemoryOperate, OnlySearchBaseValue) {
    std::string packageName = "com.example.app";
    pid_t pid = Hakutaku::getPid(packageName);
    Hakutaku::Process process = Hakutaku::openProcess(pid);

    // base type search
    Hakutaku::MemorySearcher searcher = process.getSearcher();
    int ret = searcher.search(1, RANGE_A);
    if(ret == RESULT_SUCCESS) {
        // Search succeeded
        int size = searcher.size(); // number of search results
        std::for_each(searcher.getResult().begin(), searcher.getResult().end(), [&](const std::set<Pointer> &ptrSet) {
            printf("==========> Group[%zu]\n", ptrSet.size());
            std::for_each(ptrSet.begin(), ptrSet.end(), [&](const auto &ptr) {
                printf("0x%04lx\n", ptr);
            });
            printf("\n");
        });
    } else {
        // search failed
        printf("搜索失败，错误码：%d\n", ret);
    }
}

TEST(MemoryOperate, SearchAndFilterBaseValue) {
    std::string packageName = "com.example.app";
    pid_t pid = Hakutaku::getPid(packageName);
    Hakutaku::Process process = Hakutaku::openProcess(pid);

    // based value search
    Hakutaku::MemorySearcher searcher = process.getSearcher();
    int ret = searcher.search(1, RANGE_A);
    if(ret == RESULT_SUCCESS) {
        // Search succeeded
        int size = searcher.size(); // number of search results
        std::for_each(searcher.getResult().begin(), searcher.getResult().end(), [&](const std::set<Pointer> &ptrSet) {
            printf("==========> Group[%zu]\n", ptrSet.size());
            std::for_each(ptrSet.begin(), ptrSet.end(), [&](const auto &ptr) {
                printf("0x%04lx\n", ptr);
            });
            printf("\n");
        });

        // to filter
        searcher.filter(2);
        int filter_size = searcher.size(); // number of filtered results
        std::for_each(searcher.getResult().begin(), searcher.getResult().end(), [&](const std::set<Pointer> &ptrSet) {
            printf("==========> Group[%zu]\n", ptrSet.size());
            std::for_each(ptrSet.begin(), ptrSet.end(), [&](const auto &ptr) {
                printf("0x%04lx\n", ptr);
            });
            printf("\n");
        });
    } else {
        // search failed
        printf("搜索失败，错误码：%d\n", ret);
    }
}

TEST(MemoryOperate, OnlySearchData) {
    std::string packageName = "com.example.app";
    pid_t pid = Hakutaku::getPid(packageName);
    Hakutaku::Process process = Hakutaku::openProcess(pid);

    // base type search
    Hakutaku::MemorySearcher searcher = process.getSearcher();
    const char* data = "abcdefg";
    int ret = searcher.search((void *) data, 7, RANGE_OTHER);
    if(ret == RESULT_SUCCESS) {
        // Search succeeded
        int size = searcher.size(); // number of search results
        std::for_each(searcher.getResult().begin(), searcher.getResult().end(), [&](const std::set<Pointer> &ptrSet) {
            printf("==========> Group[%zu]\n", ptrSet.size());
            std::for_each(ptrSet.begin(), ptrSet.end(), [&](const auto &ptr) {
                printf("0x%04lx\n", ptr);
            });
            printf("\n");
        });
    } else {
        // search failed
        printf("搜索失败，错误码：%d\n", ret);
    }
}


TEST(MemoryOperate, OnlySearchBaseValueWithSign) {
    std::string packageName = "com.example.app";
    pid_t pid = Hakutaku::getPid(packageName);
    Hakutaku::Process process = Hakutaku::openProcess(pid);

    // base type search
    Hakutaku::MemorySearcher searcher = process.getSearcher();

    // search value, not equal to 1
    int ret = searcher.search(1, RANGE_A, SIGN_NE);

    if(ret == RESULT_SUCCESS) {
        // Search succeeded
        int size = searcher.size(); // number of search results
        std::for_each(searcher.getResult().begin(), searcher.getResult().end(), [&](const std::set<Pointer> &ptrSet) {
            printf("==========> Group[%zu]\n", ptrSet.size());
            std::for_each(ptrSet.begin(), ptrSet.end(), [&](const auto &ptr) {
                printf("0x%04lx\n", ptr);
            });
            printf("\n");
        });
    } else {
        // search failed
        printf("搜索失败，错误码：%d\n", ret);
    }
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}