#ifndef HAK_PROC_LISTENER_H
#define HAK_PROC_LISTENER_H

#include "types.h"
#include <functional>

namespace hak {

    /*
     * 暂时搁置内核模块开发计划
     *
     * Putting kernel module development plans on hold for now
     */

    void listen_proc_fork_ev(std::function<void()> _ev);
}

#endif //HAK_PROC_LISTENER_H
