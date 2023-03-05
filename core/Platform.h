#include <unistd.h>
#include <cmath>
#include <string>

#define PLAT_PROC_DIR "/proc"

#ifdef HAKUTAKU_64BIT
typedef long long Pointer;
#else
typedef long Pointer;
#endif
typedef int Pid;

namespace Hakutaku::Platform {
    bool rootPermit();

    inline int getPageSize();

    Pointer getPageBegin(Pointer ptr);

    void stopProcess(Pid pid);

    void recoverProcess(Pid pid);

    void killProcess(Pid pid);
}

