#include <iostream>
#include "core/Hakutaku.hpp"

int main() {
    std::string packageName = "bin.mt.plus";
    Pid pid = Hakutaku::getPid(packageName);
    printf("Package: %s\n", packageName.c_str());
    if (pid == 0) {
        printf("Application not running!\n");
        return -1;
    } else {
        printf("Pid: %d\n", pid);
    }
    Hakutaku::Process process = Hakutaku::openProcess(pid);
    printf("Stop Process(Pid: %d) for 5sec.\n", pid);
    process.stop();
    sleep(5);
    process.recover();
    printf("Recover Process(Pid: %d).\n", pid);

    std::cout << "End Hakutaku!" << std::endl;
    return 0;
}
