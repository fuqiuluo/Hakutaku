#include <string>
#include <list>
#include <unistd.h>
#include <malloc.h>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <thread>
#include <linux/uinput.h>
#include <dirent.h>
#include <sys/syscall.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <iconv.h>
#include <sys/uio.h>

/*
 * 倘如你需要更多的帮助，你可以阅读
 * 1，https://github.com/bbgsm/ue4_cheat_engine/blob/main/MTools/GameTools.cpp
 *
 */

#define PLAT_PROC_DIR "/proc"

#ifdef HAKUTAKU_64BIT
typedef long int Pointer;
#else
typedef long Pointer;
#endif

#define RESULT_SUCCESS 0
#define RESULT_OFE (-1) // Open file error
#define RESULT_ADDR_NRA (-2) // Address not readable
#define RESULT_ADDR_NWA (-3) // Address not writeable
#define RESULT_UNKNOWN_WORK_MODE (-4)

#define RANGE_ALL 4094
#define RANGE_BAD 2 //
#define RANGE_V 4 // kgsl-3d0
#define RANGE_CA 8
#define RANGE_CB 16
#define RANGE_CD 32
#define RANGE_CH 64
#define RANGE_JH 128
#define RANGE_A 256
#define RANGE_XS 512
#define RANGE_S 1024
#define RANGE_AS 2048

#define MODE_DIRECT 0
#define MODE_MEM 1
#define MODE_SYSCALL 2

/* android api > 24
#if defined(__arm__)
int process_vm_readv_syscall = 376;
int process_vm_writev_syscall = 377;
#elif defined(__aarch64__)
int process_vm_readv_syscall = 270;
int process_vm_writev_syscall = 271;
#elif defined(__i386__)
int process_vm_readv_syscall = 347;
int process_vm_writev_syscall = 348;
#else
int process_vm_readv_syscall = 310;
int process_vm_writev_syscall = 311;
#endif
*/

typedef int Range;
typedef short WorkMode;

namespace Hakutaku {
    class Page {
    private:
        Pointer _start;
        Pointer _end;
        Page* _before;
        Page* _next;

    public:
        Page(Pointer start, Pointer end);

        Page(const Page &) = delete;
        Page &operator=(const Page &) = delete;

        [[nodiscard]] Pointer start() const;

        [[nodiscard]] Pointer end() const;

        [[nodiscard]] bool inside(Pointer pointer) const;

        void remove();

        friend class Maps;
    };

    class Address {
    public:
        explicit Address(Pointer addr);

        Address(Pointer addr, Page* page);

        Pointer addr;
        std::shared_ptr<Page> currentPage;
    };

    class Maps {
    private:
        Page *START;
        Page *END;

    public:
        Maps();
        ~Maps();
        Maps(const Maps &) = delete;
        Maps &operator=(const Maps &) = delete;

        void set(Page *start, Page *end);

        void append(Page *page);

        static void remove(Page *page);

        bool empty();

        Page *start();

        Page *end();

        // Slowly!
        size_t size();

        void clear();

        friend class Process;
    };

    class Process {
    private:
        pid_t pid;
        int memFd;

    public:
        WorkMode workMode;

        explicit Process(pid_t pid);
        ~Process();
        // no copy me, if i have a memFd -> it will be unnaturally released!
        Process(const Process &) = default;
        Process &operator=(const Process &) = delete;

        void stop() const;

        void recover() const;

        void kill() const;

        Pointer findModuleBase(const char *module_name, bool matchBss = false) const;

        [[nodiscard]] bool isMissingPage(Pointer addr) const;

        int getMaps(Maps &dstMap, Range range) const;

        int read(Pointer addr, void *data, size_t len);
        int write(Pointer addr, void *data, size_t len);

        /* it will lead to a strong bug!
        template<typename T>
        int read(Pointer addr, T *data) {
            return read(addr, data, sizeof(T));
        }

        template<typename T>
        int write(Pointer addr, T *data) {
            return write(addr, data, sizeof(T));
        } */
    };

    namespace Touch {
        // Android KeyEvent模拟及KeyCode原生代码对照表
        // https://blog.csdn.net/u010871962/article/details/120657210
        void turnOnScreen();

        void touchHome();

        void touchBack();

        void touchMenu();

        void touchPower();

        void touchUp();

        void touchDown();

        void touch(int key);
    }

    namespace Utils {
        void hexDump(Process &process, Pointer addr, int lines);

        void sleep_s(long long sec);

        void sleep_ms(long long ms);

        void sleep_us(long long us);
    }

    namespace Platform {
        bool rootPermit();

        inline int getPageSize();

        Pointer getPageBegin(Pointer ptr);

        void stopProcess(pid_t pid);

        void recoverProcess(pid_t pid);

        void killProcess(pid_t pid);

        void reInotify() {
            system("echo 0 > /proc/sys/fs/inotify/max_user_watches");
        }

        std::string execCmd(const char *cmd);

        // Please make a page fault judgment before execution, otherwise there will be problems!
#if __ANDROID_API__ >= 23
        int readBySyscall(pid_t pid, Pointer addr, void *data, size_t len);

        int writeBySyscall(pid_t pid, Pointer addr, void *data, size_t len);
#endif

        int readByMem(int memFd, Pointer addr, void *data, size_t len);

        int writeByMem(int memFd, Pointer addr, void *data, size_t len);

        int readDirect(Pointer addr, void* data, size_t len);

        int writeDirect(Pointer addr, void* data, size_t len);

        Pointer findModuleBase(pid_t pid, const char *module_name, bool matchBss = false);
    }

    pid_t getPidByPidOf(std::string& packageName);

    /**
     * Get Pid by packageName(cmd_line)
     */
    pid_t getPid(std::string& packageName);

    inline Process openProcess(pid_t pid);
}

namespace Hakutaku::Touch {
    void touch(int key) {
        std::string cmd = "input keyevent " + std::to_string(key);
        system(cmd.c_str());
    }

    void turnOnScreen() {
        touch(224);
    }

    void touchHome() {
        touch(3);
    }

    void touchBack() {
        touch(4);
    }

    void touchMenu() {
        touch(82);
    }

    void touchPower() {
        touch(26);
    }

    void touchUp() {
        touch(19);
    }

    void touchDown() {
        touch(20);
    }
}

namespace Hakutaku::Utils {
    void hexDump(Process &process, Pointer addr, int lines) {
        printf("\n\t\t::::Hex Dump::::\n\n");
        char tmp[8];
        for(int i = 0;i < lines;i++) {
#if defined(__LP64__)
            printf("0x%04lx: ", addr + (i * 8));
#else
            printf("0x%04lx: ", addr + (i * 8));
#endif
            process.read(addr + (i * 8), &tmp, sizeof tmp);
            for(char j : tmp) {
#if defined(__LP64__)
                printf("%02hhx ", j);
#else
                printf("%02x ", j);
#endif
            }
            printf("  ");
            printf("\t| %s \t|", tmp);
            printf("\n");
        }
        printf("\n");
    }

    void sleep_s(long long int sec) {
        std::this_thread::sleep_for(std::chrono::seconds(sec));
    }

    void sleep_ms(long long int ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    void sleep_us(long long int us) {
        std::this_thread::sleep_for(std::chrono::microseconds(us));
    }
}

// #################################################
// #################### SOURCE #####################
// #################################################

// ############## PLATFORM ######################
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

    void stopProcess(pid_t pid) {
        char cmd[64];
        sprintf(cmd, "kill -STOP %d", pid);
        std::system(cmd);
    }

    void recoverProcess(pid_t pid) {
        char cmd[64];
        sprintf(cmd, "kill -CONT %d", pid);
        std::system(cmd);
    }

    void killProcess(pid_t pid) {
        char cmd[64];
        sprintf(cmd, "kill %d", pid);
        std::system(cmd);
    }

    std::string execCmd(const char *cmd) {
        std::string out;
        FILE *pipe = popen(cmd, "r");
        if (!pipe) {
            return std::move(out);
        }
        char buf[4096] = {0};
        while (fgets(buf, sizeof buf, pipe)) {
            out.append(buf);
        }
        return std::move(out);
    }

    int readBySyscall(pid_t pid, Pointer addr, void *data, size_t len) {
        static iovec local;
        static iovec remote;
        local.iov_base = data;
        local.iov_len = len;
        remote.iov_base = (void*) addr;
        remote.iov_len = len;
        if (process_vm_readv(pid, &local, 1, &remote, 1, 0) != len) {
            return RESULT_ADDR_NRA;
        }
        return RESULT_SUCCESS;
    }

    int writeBySyscall(pid_t pid, Pointer addr, void *data, size_t len) {
        static iovec local;
        static iovec remote;
        local.iov_base = data;
        local.iov_len = len;
        remote.iov_base = (void*) addr;
        remote.iov_len = len;
        if (process_vm_writev(pid, &local, 1, &remote, 1, 0) != len) {
            return RESULT_ADDR_NWA;
        }
        return RESULT_ADDR_NWA;
    }

    int readByMem(int memFd, Pointer addr, void *data, size_t len) {
        pread64(memFd, data, len, addr);
        return RESULT_SUCCESS;
    }

    int writeByMem(int memFd, Pointer addr, void *data, size_t len) {
        pwrite64(memFd, data, len, addr);
        return RESULT_SUCCESS;
    }

    int readDirect(Pointer addr, void *data, size_t len) {
        memcpy(data, (void*)addr, len);
        return RESULT_SUCCESS;
    }

    int writeDirect(Pointer addr, void *data, size_t len) {
        memcpy((void*)addr, data, len);
        return RESULT_SUCCESS;
    }

    Pointer findModuleBase(pid_t pid, const char *module_name, bool matchBss) {
        long start_address = 0;
        char om[64], line[1024];
        bool LastIsSo = false;
        strcpy(om, module_name);
        std::string path = "/proc/" + std::to_string(pid) + "/maps";
        FILE *p = fopen(path.c_str(), "r");
        if (p) {
            while (fgets(line, sizeof(line), p)) {
                if (LastIsSo) {
                    if (strstr(line, "[anon:.bss]")) {
                        sscanf(line, "%lx-%*lx", &start_address); // NOLINT(cert-err34-c)
                        break;
                    } else {
                        LastIsSo = false;
                    }
                }
                if (strstr(line, module_name)) {
                    if (!matchBss) {
                        sscanf(line, "%lx-%*lx", &start_address); // NOLINT(cert-err34-c)
                        break;
                    } else {
                        LastIsSo = true;
                    }
                }
            }
            fclose(p);
        }
        return start_address;
    }
}

// ############## CORE ######################
namespace Hakutaku {
    void Process::stop() const {
        Platform::stopProcess(pid);
    }

    void Process::recover() const {
        Platform::recoverProcess(pid);
    }

    void Process::kill() const {
        Platform::killProcess(pid);
    }

    Pointer Page::start() const {
        return _start;
    }

    Pointer Page::end() const {
        return _end;
    }

    bool Page::inside(Pointer pointer) const {
        return pointer >= start() && pointer <= end();
    }

    void Page::remove() {
        // delete self
        _before->_next = _next;
    }

    Page::Page(Pointer start, Pointer end): _start(start), _end(end) {
        _next = nullptr;
        _before = nullptr;
    }

    Address::Address(Pointer addr): addr(addr) {
        currentPage = std::shared_ptr<Page>(nullptr);
    }

    Address::Address(Pointer addr, Page *page): addr(addr), currentPage(page) {}

    Maps::~Maps() {
        if (START != nullptr) {
            clear();
        }
    }

    void Maps::set(Page *start, Page *end) {
        if (START != nullptr) {
            clear();
        }
        START = start;
        END = end;
    }

    void Maps::append(Page *page) {
        if (page == nullptr)
            return;
        if (END != nullptr) {
            END->_next = page;
            END = page;
        } else if (empty()) {
            START = page;
            END = page;
        } else if (START != nullptr) {
            Page* last = START;
            while (last != nullptr) {
                if (last->_next == nullptr)
                    break;
                last = last->_next;
            }
            END = last;
            END->_next = page;
            END = page;
        }
    }

    void Maps::remove(Page *page) {
        page->remove();
    }

    bool Maps::empty() {
        return size() == 0;
    }

    void Maps::clear() {
        Page* tmp;
        Page* curr = START;
        while (curr != nullptr) {
            tmp = curr->_next;
            delete curr;
            curr = tmp;
        }
        START = nullptr;
    }

    size_t Maps::size() {
        size_t size = 0;
        Page* curr = START;
        while (curr != nullptr) {
            size++;
            curr = curr->_next;
        }
        return size;
    }

    Page *Maps::start() {
        return START;
    }

    Page *Maps::end() {
        return END;
    }

    Maps::Maps() {
        START = nullptr;
        END = nullptr;
    }

    inline bool isMatch(char *buff, Range range) {
        if (range == RANGE_ALL)
            return true;
        if ((range & RANGE_A) == RANGE_A &&
            (strstr(buff, "rw") != nullptr && strlen(buff) < 46))
            return true;
        if ((range & RANGE_BAD) == RANGE_BAD && // 部分修改器认为BAD内存为kgsl-3d0
            (strstr(buff, "rw") != nullptr && strstr(buff,"/system/fonts") != nullptr))
            return true;
        if ((range & RANGE_V) == RANGE_V &&
            (strstr(buff, "rw") != nullptr && strstr(buff,"/dev/kgsl-3d0") != nullptr))
            return true;
        if ((range & RANGE_CA) == RANGE_CA &&
            (strstr(buff, "rw") != nullptr && strstr(buff,"[anon:libc_malloc]") != nullptr))
            return true;
        if ((range & RANGE_CB) == RANGE_CB &&
            (strstr(buff, "rw") != nullptr && strstr(buff,"[anon:.bss]") != nullptr))
            return true;
        if ((range & RANGE_CD) == RANGE_CD &&
            (strstr(buff, "rw") != nullptr && strstr(buff,"/data/") != nullptr))
            return true;
        if ((range & RANGE_CH) == RANGE_CH &&
            (strstr(buff, "rw") != nullptr && strstr(buff,"[heap]") != nullptr))
            return true;
        if ((range & RANGE_AS) == RANGE_AS &&
            (strstr(buff, "rw") != nullptr && strstr(buff,"/dev/ashmem/") != nullptr && !strstr(buff,"dalvik")))
            return true;
        if ((range & RANGE_JH) == RANGE_JH &&
            (strstr(buff, "rw") != nullptr && (strstr(buff,"/dev/ashmem/") != nullptr || strstr(buff, "anon:dalvik-main") != nullptr) ))
            return true;
        if ((range & RANGE_XS) == RANGE_XS &&
            (strstr(buff, "rw") != nullptr && strstr(buff,"/system") != nullptr))
            return true;
        if ((range & RANGE_S) == RANGE_S &&
            (strstr(buff, "rw") != nullptr && strstr(buff,"[stack]") != nullptr))
            return true;
        return false;
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err34-c"
    int Process::getMaps(Maps &dstMap, Range range) const {
        char tmp[64], buff[256];
        sprintf(tmp, "/proc/%d/maps", pid);
        FILE *fp = fopen(tmp, "r");
        if (fp == nullptr) {
            return RESULT_OFE;
        }
        while (!feof(fp)) {
            fgets(buff, sizeof buff, fp);
            if (isMatch(buff, range)) {
                Pointer start = 0;
                Pointer end = 0;
                sscanf(buff, "%lx-%lx", &start, &end);
                dstMap.append(new Page(start, end));
            }
        }
        fclose(fp);
        return 0;
    }
#pragma clang diagnostic pop

    bool Process::isMissingPage(Pointer addr) const {
        if (workMode == MODE_DIRECT) {
            auto pagesize = Platform::getPageSize();
            unsigned char vec = 0;
            mincore((void*)(addr & (~(pagesize - 1))), pagesize, &vec);
            return vec != 1;
        } else {
            Pointer vir_index = addr / Platform::getPageSize();
            Pointer file_offset = vir_index * 8;
            Pointer item_bit = 0;
            struct iovec iov{};
            iov.iov_base = &item_bit;
            iov.iov_len = 8;
            preadv(pid, &iov, 1, file_offset);
            if(item_bit & (uint64_t) 1 << 63){
                return false;
            }
            return true;
        }
    }

    Process::Process(pid_t pid) : pid(pid) {
        workMode = MODE_SYSCALL;
        memFd = 0;
    }

    Process::~Process() {
        if (memFd != 0) {
            close(memFd);
        }
    }

    int Process::read(Pointer addr, void *data, size_t len) {
        switch (workMode) {
            case MODE_DIRECT:
                return Platform::readDirect(addr, data, len);
            case MODE_SYSCALL:
                return Platform::readBySyscall(pid, addr, data, len);
            case MODE_MEM: {
                if (memFd == 0) {
                    char tmp[64];
                    sprintf(tmp, "/proc/%d/mem", pid);
                    memFd = open(tmp, 00000002);
                }
                return Platform::readByMem(memFd, addr, data, len);
            }
        }
        return RESULT_UNKNOWN_WORK_MODE;
    }

    int Process::write(Pointer addr, void *data, size_t len) {
        switch (workMode) {
            case MODE_DIRECT:
                return Platform::writeDirect(addr, data, len);
            case MODE_SYSCALL:
                return Platform::writeBySyscall(pid, addr, data, len);
            case MODE_MEM: {
                if (memFd == 0) {
                    char tmp[64];
                    sprintf(tmp, "/proc/%d/mem", pid);
                    memFd = open(tmp, 00000002);
                }
                return Platform::writeByMem(memFd, addr, data, len);
            }
        }
        return RESULT_UNKNOWN_WORK_MODE;
    }

    Pointer Process::findModuleBase(const char *module_name, bool matchBss) const {
        return Platform::findModuleBase(pid, module_name, matchBss);
    }

    pid_t getPidByPidOf(std::string& packageName) {
        std::string cmd = "pidof ";
        cmd += packageName;
        auto result = Platform::execCmd(cmd.c_str());
        return (int) std::strtol(result.c_str(), nullptr, 0);
    }

    pid_t getPid(std::string& packageName) {
        auto proc_dir = opendir(PLAT_PROC_DIR);
        if (proc_dir == nullptr) {
            return 0;
        }
        FILE* fp;
        //char tmp[512]; too slow!
        std::string file_path;
        dirent* pid_file;
        char cmd_line[128];

        while ((pid_file = readdir(proc_dir))) {
            if (pid_file->d_type != DT_DIR ||
                strcmp(pid_file->d_name, ".") == 0 ||
                strcmp(pid_file->d_name, "..") == 0)
                continue;

            //sprintf(tmp, "/proc/%s/cmdline", pid_file->d_name);
            file_path = "/proc/";
            file_path += pid_file->d_name;
            file_path += "/cmdline";
            fp = fopen(file_path.c_str(), "r");
            if (fp != nullptr) {
                std::fgets(cmd_line, sizeof(cmd_line), fp);
                std::fclose(fp);
                if (packageName == cmd_line) {
                    closedir(proc_dir);
                    return (int) std::strtol(pid_file->d_name, nullptr, 0);
                }
            }
        }
        closedir(proc_dir);
        return 0;
    }

    inline Process openProcess(pid_t pid) {
        auto process = Process(pid);
        return process;
    }
}