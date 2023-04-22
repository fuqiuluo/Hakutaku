#include <gtest/gtest.h>
#include "../core/Hakutaku.hpp"

TEST(MemoryOperate, FuzzySearch) {
    std::string packageName = "com.example.app";
    pid_t pid = Hakutaku::getPid(packageName);
    Hakutaku::Process process = Hakutaku::openProcess(pid);

    Hakutaku::FuzzySearcher searcher = process.getFuzzySearcher();
    int ret = searcher.dump(RANGE_A, Hakutaku::Int);
    if(ret == RESULT_SUCCESS) {
        searcher.changed(); // 改变了
        searcher.increased(); // 增加了
        searcher.decreased(); // 减少了
        searcher.unchanged(); // 未改变

        auto size = searcher.getResultSize();
        auto result = searcher.getResult();
        std::for_each(result.begin(), result.end(), [&](const auto &item) {
            Pointer addr = item.first;
            Hakutaku::BasicValue value = item.second;
            std::printf("addr: %ld, %d\n", addr, value.i32);
        });

        searcher.clearResult();
        // You don't need to clear it, because it is in the stack area, and it will be released by itself when it leaves the scope
        // 你可以不清空，因为他在栈区，离开作用域他会自己释放
    }
}