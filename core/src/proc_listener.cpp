#include "proc_listener.h"
#include <unistd.h>
#include <sys/socket.h>
#include "linux/cn_proc.h"
#include "linux/netlink.h"
#include "linux/connector.h"

static volatile bool is_init_listener = false;
static volatile bool stopped = false;

static std::function<void()> event_fork = nullptr;

/*
 * Only For Kernel Module
 * Only For Kernel Module
 * Only For Kernel Module
 * Only For Kernel Module
 * Only For Kernel Module
 * Only For Kernel Module
 * 对于存活寿命小于1s的短进程，很多时候无法获取进程名称，因为proc文件系统的该进程pid文件夹一创建就关闭了。而且，在频繁创建短进程的场景下，会先收到进程退出事件才收到进程创建事件，需要额外做一个筛选列表
 */

static auto nl_connect() -> int  {
    int _rc;
    int nl_sock;
    struct sockaddr_nl sa_nl;

    nl_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
    if (nl_sock == -1) {
        printf("Can't open netlink socket");
        return -1;
    }

    sa_nl.nl_family = AF_NETLINK;
    sa_nl.nl_groups = CN_IDX_PROC;
    sa_nl.nl_pid = getpid();

    _rc = bind(nl_sock, (struct sockaddr *)&sa_nl, sizeof(sa_nl));
    if (_rc == -1) {
        printf( "Can't bind netlink socket");
        close(nl_sock);
        return -1;
    }

    return nl_sock;
}

static auto set_proc_ev_listen(int nl_sock) -> int  {
    int _rc;
    struct __attribute__ ((aligned(NLMSG_ALIGNTO))) {
        struct nlmsghdr nl_hdr;
        struct __attribute__ ((__packed__)) {
            struct cn_msg cn_msg;
            enum proc_cn_mcast_op cn_mcast;
        };
    } nlcn_msg;

    memset(&nlcn_msg, 0, sizeof(nlcn_msg));
    nlcn_msg.nl_hdr.nlmsg_len = sizeof(nlcn_msg);
    nlcn_msg.nl_hdr.nlmsg_pid = getpid();
    nlcn_msg.nl_hdr.nlmsg_type = NLMSG_DONE;

    nlcn_msg.cn_msg.id.idx = CN_IDX_PROC;
    nlcn_msg.cn_msg.id.val = CN_VAL_PROC;
    nlcn_msg.cn_msg.len = sizeof(enum proc_cn_mcast_op);

    nlcn_msg.cn_mcast = PROC_CN_MCAST_LISTEN;

    _rc = send(nl_sock, &nlcn_msg, sizeof(nlcn_msg), 0); // NOLINT(*-narrowing-conversions)
    if (_rc == -1) {
        printf("Can't register to netlink");
        return -1;
    }

    return 0;
}

static auto handle_proc_ev(int nl_sock) -> int {
    int _rc;
    struct __attribute__ ((aligned(NLMSG_ALIGNTO))) {
        struct nlmsghdr nl_hdr;
        struct __attribute__ ((__packed__)) {
            struct cn_msg cn_msg;
            struct proc_event proc_ev;
        };
    } nlcn_msg;
    while (!stopped) {
        _rc = recv(nl_sock, &nlcn_msg, sizeof(nlcn_msg), 0); // NOLINT(*-narrowing-conversions)
        if (_rc == 0) {
            printf("netlink no");
            return 0;
        }
        if (_rc == -1) {
            if (errno == EINTR) {
                continue;
            }
            printf("netlink recv");
            return -1;
        }
        switch (nlcn_msg.proc_ev.what) {
            case proc_event::what::PROC_EVENT_NONE:
                printf("set mcast listen ok\n");
                break;
            case proc_event::what::PROC_EVENT_FORK:
                printf("fork: parent tid=%d pid=%d -> child tid=%d pid=%d\n",
                       nlcn_msg.proc_ev.event_data.fork.parent_pid,
                       nlcn_msg.proc_ev.event_data.fork.parent_tgid,
                       nlcn_msg.proc_ev.event_data.fork.child_pid,
                       nlcn_msg.proc_ev.event_data.fork.child_tgid);
                break;
            case proc_event::what::PROC_EVENT_EXEC:
                printf("exec: tid=%d pid=%d\n",
                       nlcn_msg.proc_ev.event_data.exec.process_pid,
                       nlcn_msg.proc_ev.event_data.exec.process_tgid);
                break;
            case proc_event::what::PROC_EVENT_UID:
                printf("uid change: tid=%d pid=%d from %d to %d\n",
                       nlcn_msg.proc_ev.event_data.id.process_pid,
                       nlcn_msg.proc_ev.event_data.id.process_tgid,
                       nlcn_msg.proc_ev.event_data.id.r.ruid,
                       nlcn_msg.proc_ev.event_data.id.e.euid);
                break;
            case proc_event::what::PROC_EVENT_GID:
                printf("gid change: tid=%d pid=%d from %d to %d\n",
                       nlcn_msg.proc_ev.event_data.id.process_pid,
                       nlcn_msg.proc_ev.event_data.id.process_tgid,
                       nlcn_msg.proc_ev.event_data.id.r.rgid,
                       nlcn_msg.proc_ev.event_data.id.e.egid);
                break;
            case proc_event::what::PROC_EVENT_EXIT:
                printf("exit: tid=%d pid=%d exit_code=%d\n",
                       nlcn_msg.proc_ev.event_data.exit.process_pid,
                       nlcn_msg.proc_ev.event_data.exit.process_tgid,
                       nlcn_msg.proc_ev.event_data.exit.exit_code);
                break;
            default:
                printf("unhandled proc event\n");
                break;
        }
    }

    return 0;
}

void init_listener() {
    if (!is_init_listener) {
        int nl_sock = nl_connect();
        if (nl_sock == -1) {
            return;
        }
        int _rc = set_proc_ev_listen(nl_sock);
        if (_rc == -1) {
            return;
        }
        atexit([]() {
            stopped = true;
        });
        _rc = handle_proc_ev(nl_sock);
        if (_rc == -1) {
            return;
        }
        is_init_listener = true;
    }
}

void hak::listen_proc_fork_ev(std::function<void()> _ev) {
    init_listener();

}
