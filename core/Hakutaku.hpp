#include <string>
#include <list>
#include <forward_list>
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
#define RESULT_EMPTY_MAPS (-5)
#define RESULT_NOT_FUNDAMENTAL (-6)
#define RESULT_EMPTY_RESULT (-7)
#define RESULT_INVALID_ARGUMENT (-8)
#define RESULT_OUT_RANGE (-9)

#define RANGE_ALL 8190
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
#define RANGE_OTHER 4096

#define MODE_DIRECT 0
#define MODE_MEM 1
#define MODE_SYSCALL 2

#define SIGN_EQ 0 // 等于
#define SIGN_NE 1 // 不等于
#define SIGN_GT 2 // 大于
#define SIGN_GE 3 // 大于等于
#define SIGN_LT 4 // 小于
#define SIGN_LE 5 // 小于等于

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
        char perms[5]{}; // r-x
        unsigned long inode = 0; // inode
        std::string name; // 段名称

        Page(Pointer start, Pointer end);

        Page(const Page &) = delete;
        Page &operator=(const Page &) = delete;

        [[nodiscard]] Pointer start() const;

        [[nodiscard]] Pointer end() const;

        [[nodiscard]] bool inside(Pointer pointer) const;

        Page *next();

        Page *before();

        void remove();

        friend class Maps;
        friend class Process;
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

    class MemorySearcher;

    class Process {
    private:
        pid_t pid;
        int memHandle;
        int pagemapHandle;

    public:
        WorkMode workMode;

        explicit Process(pid_t pid);
        ~Process();
        // no copy me, if i have a memHandle -> it will be unnaturally released!
        Process(const Process &) = default;
        Process &operator=(const Process &) = delete;

        void stop() const;

        void recover() const;

        void kill() const;

        Pointer findModuleBase(const char *module_name, bool matchBss = false) const;

        [[nodiscard]] bool isMissingPage(Pointer addr);

        int getMapsLite(Maps &dstMap, Range range) const;
        int getMaps(Maps &dstMap, Range range) const;

        int read(Pointer addr, void *data, size_t len);
        int write(Pointer addr, void *data, size_t len);

        MemorySearcher getSearcher();
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

    class MemorySearcher {
    private:
        enum ValueType: std::int8_t {
            Byte,
            Short,
            Int,
            Long,
            Float,
            Double,
            UByte,
            UShort,
            UInt,
            ULong,
            Unknown
        };

        class Value {
        public:
            union {
                std::int8_t i8;
                std::int16_t i16;
                std::int32_t i32;
                std::int64_t i64;
                std::uint8_t u8;
                std::uint16_t u16;
                std::uint32_t u32;
                std::uint64_t u64;
                float f;
                double d;
            } value{};

            ValueType type = ValueType::Int;
        };

    private:
        Process *process;
        std::forward_list<Pointer> result;
        size_t resultSize;

        explicit MemorySearcher(Process *process);

        int search(Page *start, size_t size, const std::function<bool(void*)>& matcher);

        int filter(size_t size, const std::function<bool(void*)>& matcher);
    public:
        /*
         * 自动清空上一次搜索的结果，并重新获取Maps重新搜索
         */
        int search(void* data, size_t size, Range range = RANGE_ALL, int sign = SIGN_EQ); // Originally search

        template<typename T>
        int search(T data, Range range = RANGE_ALL, int sign = SIGN_EQ);

        int searchNumber(const char *expr, Range range = RANGE_ALL, int sign = SIGN_EQ);

        /*
         * 使用上一次搜索的结果进行过滤
         */
        int filter(void* data, size_t size, int sign = SIGN_EQ);

        template<typename T>
        int filter(T data, int sign = SIGN_EQ);

        /*
         * 清空上一次搜索的结果
         */
        void clearResult();

        std::forward_list<Pointer>& getResult();

        bool empty();

        [[nodiscard]] size_t getSize() const;

        friend class Process;
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

        void printMaps(Maps& map);

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

    void printMaps(Maps& map) {
        printf("\n::::Maps::::\nName \t Inode \t Perms \t Name\n");
        Page* current = map.start();
        while (current != nullptr) {
            printf("PAGES[0x%04lx-0x%04lx] \t%lu\t %s\t %s\n", current->start(), current->end(), current->inode,current->perms,current->name.c_str());
            current = current->next();
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
        std::string cmd = "kill -STOP " + std::to_string(pid);
        std::system(cmd.c_str());
    }

    void recoverProcess(pid_t pid) {
        std::string cmd = "kill -CONT " + std::to_string(pid);
        std::system(cmd.c_str());
    }

    void killProcess(pid_t pid) {
        std::string cmd = "kill " + std::to_string(pid);
        std::system(cmd.c_str());
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

    Page *Page::before() {
        return _before;
    }

    Page *Page::next() {
        return _next;
    }

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
        END = nullptr;
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

    inline bool isMatch(char *buff, Range range) { // NOLINT(misc-no-recursion)
        if (range == RANGE_ALL)
            return true;
        if (strstr(buff, "rw") == nullptr) return false;
        if ((range & RANGE_A) == RANGE_A &&strlen(buff) < 46) return true;
        if ((range & RANGE_BAD) == RANGE_BAD && strstr(buff, "/system/fonts") != nullptr) // 部分修改器认为BAD内存为kgsl-3d0
            return true;
        if ((range & RANGE_V) == RANGE_V && strstr(buff, "/dev/kgsl-3d0") != nullptr)
            return true;
        if ((range & RANGE_CA) == RANGE_CA && strstr(buff, "[anon:libc_malloc]") != nullptr)
            return true;
        if ((range & RANGE_CB) == RANGE_CB && strstr(buff, "[anon:.bss]") != nullptr)
            return true;
        if ((range & RANGE_CD) == RANGE_CD && strstr(buff, "/data/") != nullptr)
            return true;
        if ((range & RANGE_CH) == RANGE_CH && strstr(buff, "[heap]") != nullptr)
            return true;
        if ((range & RANGE_AS) == RANGE_AS && strstr(buff, "/dev/ashmem") != nullptr && !strstr(buff, "dalvik"))
            return true;
        if ((range & RANGE_JH) == RANGE_JH && (strstr(buff, "/dev/ashmem") != nullptr || strstr(buff, "anon:dalvik") != nullptr))
            return true;
        if ((range & RANGE_XS) == RANGE_XS && strstr(buff, "/system") != nullptr) return true;
        if ((range & RANGE_S) == RANGE_S && (strstr(buff, "[stack]") != nullptr || strstr(buff, "[stack_and_tls") != nullptr )) return true;

        if ((range & RANGE_OTHER) == RANGE_OTHER && !isMatch(buff, 4094)) return true;

        return false;
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err34-c"
    int Process::getMapsLite(Maps &dstMap, Range range) const {
        std::string path = "/proc/" + std::to_string(pid) + "/maps";
        FILE *fp = fopen(path.c_str(), "r");
        if (fp == nullptr) {
            return RESULT_OFE;
        }
        char buff[256];
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
        return RESULT_SUCCESS;
    }

    int Process::getMaps(Maps &dstMap, Range range) const {
        std::string path = "/proc/" + std::to_string(pid) + "/maps";
        FILE *fp = fopen(path.c_str(), "r");
        if (fp == nullptr) {
            return RESULT_OFE;
        }
        char buff[256], tmp[512];
        while (!feof(fp)) {
            fgets(buff, sizeof buff, fp);
            if (isMatch(buff, range)) {
                Page *page = new Page(0, 0);
                uintptr_t useless = 0;
                sscanf(buff, "%lx-%lx %4s %lx %lx:%lx %lu %s", &page->_start, &page->_end, page->perms, &useless, &useless, &useless, &page->inode, tmp);
                if (page->_start > 0 && page->_end > 0 && page->_start < page->_end) {
                    page->name = std::string(tmp, strlen(tmp));
                    dstMap.append(page);
                } else {
                    delete page;
                }
            }
        }
        fclose(fp);
        return RESULT_SUCCESS;
    }
#pragma clang diagnostic pop

    bool Process::isMissingPage(Pointer addr) {
        if (workMode == MODE_DIRECT) {
            auto pagesize = Platform::getPageSize();
            unsigned char vec = 0;
            mincore((void*)(addr & (~(pagesize - 1))), pagesize, &vec);
            return vec != 1;
        } else {
            if (pagemapHandle == 0) {
                std::string path = "/proc/" + std::to_string(pid) + "/task/" + std::to_string(pid) + "/pagemap";
                pagemapHandle = open(path.c_str(), O_RDONLY);
            }
            Pointer file_offset = (addr / Platform::getPageSize()) * 8;
            Pointer item_bit = 0;
            struct iovec iov{};
            iov.iov_base = &item_bit;
            iov.iov_len = 8;
            preadv(pagemapHandle, &iov, 1, file_offset);
            if(item_bit & (uint64_t) 1 << 63){
                return false;
            }
            return true;
        }
    }

    Process::Process(pid_t pid) : pid(pid) {
        workMode = MODE_SYSCALL;
        memHandle = 0;
        pagemapHandle = 0;
    }

    Process::~Process() {
        if (memHandle != 0) {
            close(memHandle);
        }
        if (pagemapHandle != 0) {
            close(pagemapHandle);
        }
    }

    int Process::read(Pointer addr, void *data, size_t len) {
        switch (workMode) {
            case MODE_DIRECT:
                return Platform::readDirect(addr, data, len);
            case MODE_SYSCALL:
                return Platform::readBySyscall(pid, addr, data, len);
            case MODE_MEM: {
                if (memHandle == 0) {
                    std::string path = "/proc/" + std::to_string(pid) + "/mem";
                    memHandle = open(path.c_str(), 00000002);
                }
                return Platform::readByMem(memHandle, addr, data, len);
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
                if (memHandle == 0) {
                    std::string path = "/proc/" + std::to_string(pid) + "/mem";
                    memHandle = open(path.c_str(), 00000002);
                }
                return Platform::writeByMem(memHandle, addr, data, len);
            }
        }
        return RESULT_UNKNOWN_WORK_MODE;
    }

    Pointer Process::findModuleBase(const char *module_name, bool matchBss) const {
        return Platform::findModuleBase(pid, module_name, matchBss);
    }

    MemorySearcher Process::getSearcher() {
        return MemorySearcher(this);
    }

    MemorySearcher::MemorySearcher(Process *process): process(process) {
        resultSize = 0;
    }

    int MemorySearcher::search(void *data, size_t size, Range range, int sign) {
        if (!result.empty())
            clearResult();
        Maps map = Maps();
        int ret = process->getMaps(map, range);
        if (ret != RESULT_SUCCESS)
            return ret;
        if (map.empty())
            return RESULT_EMPTY_MAPS;
        return search(map.start(), size, [&](void *tmp) {
            if (sign == SIGN_EQ) {
                return memcmp(data, tmp, size) == 0;
            } else if (sign == SIGN_NE) {
                return memcmp(data, tmp, size) != 0;
            } else if (sign == SIGN_GT) {
                return memcmp(data, tmp, size) > 0;
            } else if (sign == SIGN_GE) {
                return memcmp(data, tmp, size) >= 0;
            } else if (sign == SIGN_LT) {
                return memcmp(data, tmp, size) < 0;
            } else if (sign == SIGN_LE) {
                return memcmp(data, tmp, size) <= 0;
            }
            return memcmp(data, tmp, size) == 0;
        });
    }

    template<typename T>
    int MemorySearcher::search(T data, Range range, int sign) {
        if (!std::is_fundamental<T>::value) { // 不支持输入非基础类型
            return RESULT_NOT_FUNDAMENTAL;
        }
        return search(&data, sizeof(T), range, sign);
    }

    int MemorySearcher::search(Page *start, size_t size, const std::function<bool(void *)>& matcher) {
        if (start == nullptr)
            return RESULT_ADDR_NRA;
        char tmp[size];
        Page* currPage = start;
        while (currPage != nullptr) {
            auto st = currPage->start();
#if !IGNORE_MISSING_PAGE
            if (process->isMissingPage(st)) {
                //printf("missing page\n");
                goto nextPage;
            }
#endif
#if SUPPORT_UNALIGNED_MEMORY
            for (int i = 0; i < (currPage->end() - st); ++i) {
                Pointer addr = st + i;
                process->read(addr, tmp, size);
                if(matcher(tmp))
                    results.push_front(addr);
            }
#else
            for (int i = 0; i < (currPage->end() - st) / size; ++i) {
                Pointer addr = st + i * static_cast<int>(size);
                //printf("addr: 0x%04lx\n", addr);
                process->read(addr, tmp, size);
                if(matcher(tmp)) {
                    //printf("yes!\n");
                    result.push_front(addr);
                    resultSize++;
                }
            }
#endif
            nextPage:
            currPage = currPage->next();
        }

        return RESULT_SUCCESS;
    }

    bool MemorySearcher::empty() {
        return result.empty();
    }

    std::forward_list<Pointer> &MemorySearcher::getResult() {
        return result;
    }

    void MemorySearcher::clearResult() {
        result.clear();
    }

    int MemorySearcher::filter(void *data, size_t size, int sign) {
        return filter(size, [&](void *tmp) {
            if (sign == SIGN_EQ) {
                return memcmp(data, tmp, size) == 0;
            } else if (sign == SIGN_NE) {
                return memcmp(data, tmp, size) != 0;
            } else if (sign == SIGN_GT) {
                return memcmp(data, tmp, size) > 0;
            } else if (sign == SIGN_GE) {
                return memcmp(data, tmp, size) >= 0;
            } else if (sign == SIGN_LT) {
                return memcmp(data, tmp, size) < 0;
            } else if (sign == SIGN_LE) {
                return memcmp(data, tmp, size) <= 0;
            }
            return memcmp(data, tmp, size) == 0;
        });
    }

    int MemorySearcher::filter(size_t size, const std::function<bool(void *)> &matcher) {
        if (result.empty())
            return RESULT_EMPTY_RESULT;
        char tmp[size];
        std::for_each(result.begin(), result.end(), [&](const Pointer &ptr) {
            process->read(ptr, tmp, size);
            if (!matcher(tmp)) {
                result.remove(ptr);
                resultSize--;
            }
        });
        return RESULT_SUCCESS;
    }

    size_t MemorySearcher::getSize() const {
        return resultSize;
    }

    int MemorySearcher::searchNumber(const char *expr, Range range, int sign) {
        if (!result.empty())
            clearResult();

        int result_code = RESULT_SUCCESS;
        std::vector<Value> values;
        unsigned int step = 256;

        {
            std::string cache; // 缓冲区
            ValueType cache_type = Unknown;
            bool is_unsigned = false; // 无符号
            bool is_inputting_value = false; // 正在输入值
            bool is_determined_type = false; // 已决断类型
            bool is_inputting_step = false; // 正在输入步长

            // 类型决断
            auto determineType = [](bool is_unsigned, char type) {
                switch (type) {
                    case 'i':
                    case 'I': {
                        if (is_unsigned) {
                            return ValueType::UInt;
                        } else {
                            return ValueType::Int;
                        }
                    }
                    case 'c':
                    case 'C':
                    case 'b':
                    case 'B': {
                        if (is_unsigned) {
                            return ValueType::UByte;
                        } else {
                            return ValueType::Byte;
                        }
                    }
                    case 's':
                    case 'S': {
                        if (is_unsigned) {
                            return ValueType::UShort;
                        } else {
                            return ValueType::Short;
                        }
                    }
                    case 'l':
                    case 'L': {
                        if (is_unsigned) {
                            return ValueType::ULong;
                        } else {
                            return ValueType::Long;
                        }
                    }
                    case 'f':
                    case 'F': {
                        //printf("Floart\n");
                        return ValueType::Float;
                    }
                    case 'd':
                    case 'D': {
                        return ValueType::Double;
                    }
                    default: {
                        return ValueType::Unknown;
                    }
                }
            };
            // 数据解析
            auto parseValue = [&](bool reload = false) {
                Value value;
                switch (cache_type) {
                    case Byte:
                        value.value.i8 = (std::int8_t) std::stoi(cache);
                        break;
                    case Short:
                        value.value.i16 = (std::int16_t) std::stoi(cache);
                        break;
                    case Int: {
                        auto temp = std::stol(cache);
                        if (temp > 0x7fffffff) {
                            value.value.u32 = temp;
                            cache_type = UInt;
                        } else {
                            value.value.i32 = (std::int32_t) temp;
                        }
                        break;
                    }
                    case Long:
                        value.value.i64 = (std::int64_t) std::stol(cache);
                        break;
                    case Float:
                        value.value.f = std::stof(cache);
                        break;
                    case Double:
                        value.value.d = std::stod(cache);
                        break;
                    case UByte:
                        value.value.u8 = (std::uint8_t) std::stoi(cache);
                        break;
                    case UShort:
                        value.value.u16 = (std::uint16_t) std::stoi(cache);
                        break;
                    case UInt:
                        value.value.u32 = (std::uint32_t) std::stoi(cache);
                        break;
                    case ULong:
                        value.value.u64 = (std::uint64_t) std::stoul(cache);
                        break;
                    case Unknown:
                        throw std::runtime_error("This type is prohibited from being recognized.");
                }
                value.type = cache_type;
                values.push_back(value);
                cache.clear();
                if (reload) {
                    // 清空标记
                    is_unsigned = false;
                    is_determined_type = false;
                    is_inputting_value = false;
                    cache_type = Unknown;
                }
            };
            // 期盼截断
            auto expectNext = [&expr](int& index, char expect) {
                index++;
                char curr = expr[index];
                return expect == curr;
            };

            size_t s = strlen(expr);
            if (s == 0) return RESULT_SUCCESS;

            // right:
            // 1F
            // 1FF (equals 1F)
            // 1FI (equals 1I)
            // 1f;2F
            // 1F;;2F
            // 1f;2F::3
            // 1f;2F;::3

            // error:
            // 1F2F
            // 1F;2F;::4D

            for (int i = 0; i < s; ++i) {
                char tmp = expr[i];
                ValueType tryDetermineType = determineType(is_unsigned, tmp);
                if (tryDetermineType == Unknown) {
                    if (is_determined_type && tmp != ';') {
                        if (tmp == ':' && expectNext(i, ':')) {
                            parseValue(true);
                            is_inputting_step = true;
                        } else {
                            // 搜索规范，一旦决断了类型必须以';'结尾，除非声明步长
                            throw std::runtime_error("Expected ';' ended up as a value but not found.");
                        }
                    } else if (tmp == 'u' || tmp == 'U') {
                        is_unsigned = true; // 决断为无符号数字
                    } else if(tmp == ':' && expectNext(i, ':')) {
                        is_inputting_step = true;
                    } else if (tmp == ';' && !cache.empty()) {
                        if (is_inputting_step)
                            throw std::runtime_error("Input ';' not expected when entering step.");
                        parseValue(true);
                    } else {
                        if (!is_inputting_step)
                            is_inputting_value = true;
                        cache.append(expr, i, 1);
                    }
                    continue;
                } else {
                    if (is_inputting_step)
                        throw std::runtime_error("Declared type not expected on input step.");
                    cache_type = tryDetermineType;
                    is_determined_type = true;
                }
            } // for (int i = 0; i < s; ++i)
            if (!cache.empty()) { // 处理尾值
                if (is_inputting_value) {
                    parseValue();
                } else if (is_inputting_step) {
                    step = (std::uint32_t) std::stoul(cache);
                }
            }
        }

        Maps map = Maps();
        process->getMapsLite(map, range);
        if (map.empty())
            return RESULT_EMPTY_MAPS;

        // 决断值长度
        auto determineSize = [](ValueType type) {
            switch (type) {
                case Byte:
                    return sizeof(char);
                case Short:
                    return sizeof(short);
                case Int:
                    return sizeof(int);
                case Long:
                    return sizeof(long);
                case Float:
                    return sizeof(float);
                case Double:
                    return sizeof(double);
                case UByte:
                    return sizeof(unsigned char);
                case UShort:
                    return sizeof(unsigned short);
                case UInt:
                    return sizeof(unsigned int);
                case ULong:
                    return sizeof(unsigned long);
                default:
                    throw std::runtime_error("Unknown type.");
            }
        };

        Page *currPage = map.start();
        auto tv = values.begin();
        size_t valueSize = determineSize(tv->type);
        char temp[valueSize];

        // 决断周围特征值
        std::function<bool(std::vector<Value, std::allocator<Value>>::iterator iter, Pointer currPtr, Page* currPage, std::forward_list<Pointer>&)> around = nullptr;
        around = [&around, &values, &determineSize, &step, this]
                (std::vector<Value, std::allocator<Value>>::iterator iter, Pointer currPtr, Page* currPage, std::forward_list<Pointer>& rs) {
            auto next = iter + 1;
            // printf("%04lx, %04lx, %04lx, %04lx\n", (Pointer) next.base(), (Pointer) iter.base(), (Pointer) values.cend().base(), (Pointer) values.end().base());
            if (next == values.cend()) {
                //printf("out\n");
                return true;
            }
            auto size = determineSize(next->type);
            char temp[size];
            int curStep = 0;
            //printf("curPage: %04lx\n", (Pointer) currPage);
            while (currPage != nullptr) {
#if !IGNORE_MISSING_PAGE
                if (process->isMissingPage(currPage->start())) {
                    goto nextPage2;
                }
#endif
#if SUPPORT_UNALIGNED_MEMORY
                for (int i = 0; i < (currPage->end() - startPtr); ++i) {
                    if (curStep > step)
                        return false;
                    Pointer addr = startPtr + i;
                    process->read(addr, tmp, valueSize);
                    if(memcmp(temp, &next->value.u8, size) == 0) {
                        rs.push_front(addr);
                        return around(next, addr, currPage, rs);
                    }
                    curStep++;
                }
#else
                for (int i = 0; i < (currPage->end() - currPtr) / size; ++i) {
                    if (curStep > step) {
                        //printf("// 超出步长\n");
                        return false;
                    }
                    Pointer addr = currPtr + i * static_cast<int>(size);
                    process->read(addr, temp, size);
                    if (memcmp(temp, &next->value.u8, size) == 0) {
                        //printf("找到次值\n");
                        rs.push_front(addr);
                        return around(next, addr, currPage, rs);
                    }
                    curStep++;
                }
#endif
                nextPage2:
                currPage = currPage->next();
            }
            //printf("出去了\n");
            return false;
        };

        while (currPage != nullptr) {
            auto startPtr = currPage->start();
#if !IGNORE_MISSING_PAGE
            if (process->isMissingPage(startPtr)) {
                //printf("missing page\n");
                goto nextPage;
            }
#endif
#if SUPPORT_UNALIGNED_MEMORY
            for (int i = 0; i < (currPage->end() - startPtr); ++i) {
                Pointer addr = startPtr + i;
                process->read(addr, tmp, valueSize);
                if(matcher(tmp))
                    results.push_front(addr);
            }
#else
            for (int i = 0; i < (currPage->end() - startPtr) / valueSize; ++i) {
                Pointer addr = startPtr + i * static_cast<int>(valueSize);
                process->read(addr, temp, valueSize);
                int cmp = memcmp(temp, &tv->value.u8, valueSize);
                if ((sign == SIGN_EQ && cmp == 0) ||
                (sign == SIGN_NE && cmp != 0) ||
                (sign == SIGN_GT && cmp > 0) ||
                (sign == SIGN_LT && cmp < 0) ||
                (sign == SIGN_GT && cmp >= 0) ||
                (sign == SIGN_LT && cmp <= 0)) {
                    //printf("找到首值！\n");
                    std::forward_list<Pointer> rs;
                    rs.push_front(addr);
                    if (around(tv, addr, currPage, rs)) {
                        //printf("yes!\n");
                        result.merge(rs);
                        resultSize += values.size();
                    }
                } else {
                    continue;
                }
            }
#endif
            nextPage:
            currPage = currPage->next();
        }

        return result_code;
    }

    template<typename T>
    int MemorySearcher::filter(T data, int sign) {
        if (!std::is_fundamental<T>::value) {
            return RESULT_NOT_FUNDAMENTAL;
        }
        return filter(&data, sizeof(T), sign);
    }

    pid_t getPidByPidOf(std::string& packageName) {
        std::string cmd = "pidof " + packageName;
        auto result = Platform::execCmd(cmd.c_str());
        return (int) std::strtol(result.c_str(), nullptr, 0);
    }

    pid_t getPid(std::string& packageName) {
        auto proc_dir = opendir(PLAT_PROC_DIR);
        if (proc_dir == nullptr) {
            return 0;
        }
        FILE* fp;
        dirent* pid_file;
        char cmd_line[128];

        while ((pid_file = readdir(proc_dir))) {
            if (pid_file->d_type != DT_DIR ||
                strcmp(pid_file->d_name, ".") == 0 ||
                strcmp(pid_file->d_name, "..") == 0)
                continue;

            std::string file_path = "/proc/";
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