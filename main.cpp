#include <iostream>
#include "core/Platform.h"
#include "core/Hakutaku.h"

int main() {
    std::string packageName = "com.tencent.mobileqq";
    Pid pid = Hakutaku::getPid(packageName);
    printf("Package: %s\n", packageName.c_str());
    if (pid == 0) {
        printf("Application not running!\n");
        return -1;
    } else {
        printf("Pid: %d\n", pid);
    }
    printf("Stop Process(Pid: %d) for 5sec.\n", pid);
    Hakutaku::Platform::stopProcess(pid);
    sleep(5);
    printf("Recover Process(Pid: %d).\n", pid);
    Hakutaku::Platform::recoverProcess(pid);

    std::cout << "End Hakutaku!" << std::endl;
    return 0;
}
