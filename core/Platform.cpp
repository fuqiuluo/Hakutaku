#include "Platform.h"

namespace Hakutaku::Platform {
    bool rootPermit()  {
        return getpid() == 0;
    }

    inline int getPageSize() {
        return getpagesize();
    }

    Pointer getPageBegin(Pointer ptr) {
        int page_size = getPageSize();
        return (ptr / page_size) * page_size;
    }

    void stopProcess(Pid pid) {
        char cmd[64];
        sprintf(cmd, "kill -STOP %d", pid);
        std::system(cmd);
    }

    void recoverProcess(Pid pid) {
        char cmd[64];
        sprintf(cmd, "kill -CONT %d", pid);
        std::system(cmd);
    }

    void killProcess(Pid pid) {
        char cmd[64];
        sprintf(cmd, "kill %d", pid);
        std::system(cmd);
    }

}