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
        // no copy me, if I have a memHandle -> it will be unnaturally released!
        Process(const Process &) = default;
        Process &operator=(const Process &) = delete;

        [[nodiscard]] bool is64Bit() const;

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

        union BasicValue {
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
        };

        class Value {
        public:
            BasicValue value{};

            ValueType type = ValueType::Int;
        };

        class RangeValue {
        public:
            Value min;
            Value max;
            bool isOutRange = false;
        };

    private:
        Process *process;
        std::vector<std::set<Pointer>> result;

        explicit MemorySearcher(Process *process);

        class Lexer {
        public:
            static int llex(std::vector<std::any> &values, unsigned int& group_size, const char* expr);

            static ValueType getRealType(std::vector<std::any, std::allocator<std::any>>::iterator value);

            static size_t determineSize(ValueType type);

            static bool isRange(std::vector<std::any, std::allocator<std::any>>::iterator value);
        };
    public:
        int search(size_t size, Range range, const std::function<bool(void*)>& matcher);

        int filter(size_t size, const std::function<bool(void*)>& matcher);

        /*
         * 自动清空上一次搜索的结果，并重新获取Maps重新搜索
         */
        int search(void* data, size_t size, Range range = RANGE_ALL); // Originally search

        template<typename T>
        int search(T data, Range range = RANGE_ALL);

        int searchNumber(const char *expr, Range range = RANGE_ALL);

        int filterNumber(const char *expr);

        /*
         * 使用上一次搜索的结果进行过滤
         */
        int filter(void* data, size_t size);

        template<typename T>
        int filter(T data);

        /*
         * 清空上一次搜索的结果
         */
        void clearResult();

        std::vector<std::set<Pointer>>& getResult();

        bool empty();

        [[nodiscard]] size_t size() const;

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
        int dumpMemory(Process &process, Pointer start, size_t size, const std::function<void(char buf[1024 * 4], size_t size)>& receiver);

        void hexDump(Process &process, Pointer addr, int lines);

        void printMaps(Maps& map);

        void sleep_s(long long sec);

        void sleep_ms(long long ms);

        void sleep_us(long long us);
    }

    namespace Platform {
        bool rootPermit();

        inline int getPageSize();

        Pointer virtualToPhysical(pid_t pid, Pointer virAddr);

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
    int dumpMemory(Process &process, Pointer start, size_t size, const std::function<void(char buf[1024 * 4], size_t size)>& receiver) {
        int block = 1024 * 4;
        size_t progress = 0;
        char buf[block];
        while (true) {
            auto diff = size - progress;
            if (block > diff) {
                block = (int) diff;
            }
            if (int ret = process.read(start, buf, block) != RESULT_SUCCESS) {
                return ret;
            }
            receiver(buf, block);
            if (progress >= size)
                break;
        }
        return RESULT_SUCCESS;
    }

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

    Pointer virtualToPhysical(pid_t pid, Pointer virAddr) {
        char path[30] = {0};
        sprintf(path , "/proc/%d/pagemap", pid);
        int fd = open(path, O_RDONLY);
        if(fd < 0) {
            printf("open '/proc/self/pagemap' failed!\n");
            return 0;
        }
        size_t pagesize = getpagesize();
        size_t offset = (virAddr / pagesize) * sizeof(uint64_t);
        if(lseek(fd, (long) offset, SEEK_SET) < 0) {
            printf("lseek() failed!\n");
            close(fd);
            return 0 ;
        }
        uint64_t info;
        if(read(fd, &info, sizeof(uint64_t)) != sizeof(uint64_t)) {
            printf("read() failed!\n");
            close(fd);
            return 0;
        }
        if((info & (((uint64_t)1 << 63))) == 0) {
            printf("page is not present!\n");
            close(fd);
            return 0;
        }
        size_t pageframenum = info & (((uint64_t)1 << 55) -1);
        size_t phyaddr = pageframenum * pagesize + virAddr % pagesize;
        close(fd);
        return phyaddr; // NOLINT(cppcoreguidelines-narrowing-conversions)
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
        iovec local{ data, len };
        iovec remote{ (void*) addr, len };
        if (process_vm_readv(pid, &local, 1, &remote, 1, 0) != len) {
            return RESULT_ADDR_NRA;
        }
        return RESULT_SUCCESS;
    }

    int writeBySyscall(pid_t pid, Pointer addr, void *data, size_t len) {
        iovec local{ data, len };
        iovec remote{ (void*) addr, len };
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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
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
#pragma clang diagnostic pop

    static int PAGE_ENTRY = Platform::getPageSize() / 1024;

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
            Pointer file_offset = (addr / Platform::getPageSize()) * PAGE_ENTRY;
            Pointer item_bit = 0;
            struct iovec iov{ &item_bit, 8 };
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

    bool Process::is64Bit() const {
        std::string path = "/proc/" + std::to_string(pid) + "/exe";
        char buf[1024];
        readlink(path.c_str(), buf, sizeof(buf)); // NOLINT(bugprone-unused-return-value)
        return strstr(buf, "app_process32") == nullptr;
    }

    MemorySearcher::MemorySearcher(Process *process): process(process) {

    }

    int MemorySearcher::search(void *data, size_t size, Range range) {
        if (!result.empty())
            clearResult();
        return search(size, range, [&](void *tmp) {
            return memcmp(data, tmp, size) == 0;
        });
    }

    template<typename T>
    int MemorySearcher::search(T data, Range range) {
        if (!std::is_fundamental<T>::value) { // 不支持输入非基础类型
            return RESULT_NOT_FUNDAMENTAL;
        }
        return search(&data, sizeof(T), range);
    }

    int MemorySearcher::search(size_t size, Range range, const std::function<bool(void *)>& matcher) {
        Maps map = Maps();
        int ret = process->getMaps(map, range);
        if (ret != RESULT_SUCCESS)
            return ret;
        if (map.empty())
            return RESULT_EMPTY_MAPS;
        Page* start = map.start();
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
                    std::set<Pointer> addrSet { addr };
                    result.push_back(addrSet);
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

    std::vector<std::set<Pointer>> &MemorySearcher::getResult() {
        return result;
    }

    void MemorySearcher::clearResult() {
        result.clear();
    }

    int MemorySearcher::filter(void *data, size_t size) {
        return filter(size, [&](void *tmp) {
            return memcmp(data, tmp, size) == 0;
        });
    }

    int MemorySearcher::filter(size_t size, const std::function<bool(void *)> &matcher) {
        if (result.empty())
            return RESULT_EMPTY_RESULT;
        char tmp[size];
        erase_if(result, [&](const std::set<Pointer> &item) {
            process->read(*item.cbegin(), tmp, size);
            if (matcher(tmp)) {
                return false;
            }
            return true;
        });
        return RESULT_SUCCESS;
    }

    size_t MemorySearcher::size() const {
        return result.size();
    }


    int MemorySearcher::Lexer::llex(std::vector<std::any> &values, unsigned int &group_size, const char* expr) {
        std::string cache; // 缓冲区
        ValueType cache_type = Unknown;
        RangeValue cache_range_value;

        bool is_range = false; // 是否是范围值
        bool is_out_range = false; // 是否是反范围值
        bool is_ready_left = false; // 范围最小值是否准备好
        bool is_ready_right = false; // 范围最大值是否准备好

        bool is_unsigned = false; // 无符号
        bool is_inputting_value = false; // 正在输入值
        bool is_determined_type = false; // 已决断类型
        bool is_inputting_step = false; // 正在输入步长

        // 类型决断
        auto determineType = [](bool is_unsigned, char type) {
            switch (type) {
                case 'd':
                case 'D': {
                    if (is_unsigned) {
                        return ValueType::UInt;
                    } else {
                        return ValueType::Int;
                    }
                }
                case 'b':
                case 'B': {
                    if (is_unsigned) {
                        return ValueType::UByte;
                    } else {
                        return ValueType::Byte;
                    }
                }
                case 'w':
                case 'W': {
                    if (is_unsigned) {
                        return ValueType::UShort;
                    } else {
                        return ValueType::Short;
                    }
                }
                case 'q':
                case 'Q': {
                    if (is_unsigned) {
                        return ValueType::ULong;
                    } else {
                        return ValueType::Long;
                    }
                }
                case 'f':
                case 'F': {
                    //printf("Float\n");
                    return ValueType::Float;
                }
                case 'e':
                case 'E': {
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
            if (!is_determined_type && is_range)
                cache_type = ULong; // 兼容无最小值类型的范围搜索
            switch (cache_type) {
                case Byte: {
                    auto temp = std::stol(cache);
                    if (temp > INT8_MAX) {
                        value.value.u8 = temp;
                        cache_type = UByte;
                    } else {
                        value.value.i8 = (std::int8_t) temp;
                    }
                    break;
                }
                case Short: {
                    auto temp = std::stol(cache);
                    if (temp > INT16_MAX) {
                        value.value.u16 = temp;
                        cache_type = UShort;
                    } else {
                        value.value.i16 = (std::int16_t) temp;
                    }
                    break;
                }
                case Int: {
                    auto temp = std::stol(cache);
                    if (temp > INT32_MAX) {
                        value.value.u32 = temp;
                        cache_type = UInt;
                    } else {
                        value.value.i32 = (std::int32_t) temp;
                    }
                    break;
                }
                case Long: {
                    if (cache.starts_with('-')) {
                        value.value.i64 = (std::int64_t) std::stol(cache);
                    } else {
                        std::uint64_t temp = std::stoul(cache);
#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantConditionsOC"
#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"
                        if (temp > INT64_MAX) {
                            value.value.u64 = temp;
                            cache_type = ULong;
                        } else {
#pragma clang diagnostic pop
                            value.value.i64 = (std::int64_t) temp;
                        }
#pragma clang diagnostic pop
                    }
                    break;
                }
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
            if (is_range) {
                if (!is_ready_left) {
                    cache_range_value.min = value;
                    is_ready_left = true;
                } else {
                    cache_range_value.max = value;
                    // 类型统一
                    //if (cache_type != cache_range_value.min.type) {
                    //    cache_range_value.min.type = cache_type;
                    //}
                    // 默认取最大值类型，不需要做类型统一
                    //printf("in a max range: %d\n", value.value.i32);
                    is_ready_right = true;
                    cache_range_value.isOutRange = is_out_range;
                    values.emplace_back(cache_range_value);
                }
            } else {
                values.emplace_back(value);
            }
            cache.clear();
            if (reload) {
                // 清空标记
                is_unsigned = false;
                is_determined_type = false;
                is_inputting_value = false;
                cache_type = Unknown;
                // 清空范围值标记
                if (is_range && is_ready_right) {
                    is_range = false;
                    is_out_range = false;
                    is_ready_left = false;
                    is_ready_right = false;
                    cache_range_value = RangeValue();
                }
            }
        };
        // 期盼尝试
        auto tryNext = [&expr](int& index, char expect) {
            bool tmp = expect == expr[index + 1];
            if (tmp) index++;
            return tmp;
        };
        // 期盼截断
        auto expectNext = [&expr](int& index, char expect) {
            index++;
            char curr = expr[index];
            return expect == curr;
        };

        size_t s = strlen(expr);
        if (s == 0) return RESULT_INVALID_ARGUMENT;

        for (int i = 0; i < s; ++i) {
            char tmp = expr[i];
            ValueType tryDetermineType = determineType(is_unsigned, tmp);
            if (tryDetermineType == Unknown) {
                if (is_determined_type && tmp != ';') {
                    if (tmp == ':' && expectNext(i, ':')) {
                        parseValue(true);
                        is_inputting_step = true;
                    } else if (tmp == '~') {
                        is_range = true;
                        if (tryNext(i, '~')) {
                            is_out_range = true;
                        }
                        parseValue(true);
                    } else {
                        // 搜索规范，一旦决断了类型必须以';'结尾，除非声明步长
                        throw std::runtime_error("Expected ';' ended up as a value but not found.");
                    }
                } else if (tmp == 'u' || tmp == 'U') {
                    is_unsigned = true; // 决断为无符号数字
                } else if (tmp == '~') {
                    is_range = true;
                    if (tryNext(i, '~')) {
                        is_out_range = true;
                    }
                    parseValue(true);
                } else if(tmp == ':' && expectNext(i, ':')) {
                    is_inputting_step = true;
                } else if (tmp == ';' && !cache.empty()) {
                    if (is_inputting_step)
                        throw std::runtime_error("Input ';' not expected when entering group_size.");
                    if (!is_determined_type)
                        throw std::runtime_error("Expected input value type, but not found.");
                    parseValue(true);
                } else {
                    if (!is_inputting_step)
                        is_inputting_value = true;
                    cache.append(expr, i, 1);
                }
                continue;
            } else {
                if (is_inputting_step)
                    throw std::runtime_error("Declared type not expected on input group_size.");
                cache_type = tryDetermineType;
                is_determined_type = true;
            }
        } // for (int i = 0; i < s; ++i)
        if (!cache.empty()) { // 处理尾值
            if (is_inputting_value) {
                parseValue(true);
            } else if (is_inputting_step) {
                group_size = (std::uint32_t) std::stoul(cache);
            }
        }

        return RESULT_SUCCESS;
    }

    MemorySearcher::ValueType
    MemorySearcher::Lexer::getRealType(std::vector<std::any, std::allocator<std::any>>::iterator value)  {
        ValueType type;
        if (value->type() == typeid(Value)) {
            auto v = std::any_cast<Value>(*value);
            type = v.type;
        } else if (value->type() == typeid(RangeValue)) {
            type = std::any_cast<RangeValue>(*value).max.type;
        } else {
            throw std::runtime_error("Unknown value any::type.");
        }
        return type;
    }

    size_t MemorySearcher::Lexer::determineSize(MemorySearcher::ValueType type) {
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
    }

    bool MemorySearcher::Lexer::isRange(std::vector<std::any, std::allocator<std::any>>::iterator value)  {
        if (value->type() == typeid(Value)) {
            return false;
        } else if (value->type() == typeid(RangeValue)) {
            return true;
        } else {
            throw std::runtime_error("Unknown value any::type.");
        }
    }

    int MemorySearcher::searchNumber(const char *expr, Range range) {
        if (!result.empty())
            clearResult();

        std::vector<std::any> values;
        unsigned int group_size = 512;

        if(Lexer::llex(values, group_size, expr))
            return RESULT_INVALID_ARGUMENT;

        Maps map = Maps();
        process->getMapsLite(map, range);
        if (map.empty())
            return RESULT_EMPTY_MAPS;

        if (values.empty()) {
            return RESULT_INVALID_ARGUMENT;
        }

        Page *currPage = map.start();
        auto tv = values.begin();
        size_t valueSize = Lexer::determineSize(Lexer::getRealType(tv));
        BasicValue temp{};

        size_t distance = 0;

        // 决断周围特征值
        std::function<bool(std::__wrap_iter<std::any*>, Pointer, Page*, std::set<Pointer>&)> around = [&around, &values, &group_size, &distance, this]
                (auto iter, auto currPtr, auto* currPage, auto& addrSet) {
            auto next = iter + 1;
            if (next == values.cend()) {
                return true;
            }

            bool is_range = Lexer::isRange(next); // 是否是范围值
            ValueType type = Lexer::getRealType(next);
            auto size = Lexer::determineSize(type);
            BasicValue temp{};
            while (currPage != nullptr) {
#if !IGNORE_MISSING_PAGE
                if (process->isMissingPage(currPage->start())) {
                    goto nextPageV2;
                }
#endif
#if SUPPORT_UNALIGNED_MEMORY
                for (int i = 0; i < (currPage->end() - startPtr); ++i) {
                    if (distance > group_size)
                        return false;
                    Pointer addr = currPtr + i * static_cast<int>(size);
                    process->read(addr, temp, size);
                    if (is_range) {
                        auto value = std::any_cast<RangeValue>(*next);
                        int cmpMin = memcmp(temp, &value.min.value, size);
                        int cmpMax = memcmp(temp, &value.max.value, size);
                        if (cmpMin >= 0 && cmpMax <= 0) {
                            addrSet.insert(addr);
                            if (around(next, addr, currPage, addrSet)) {
                                result.push_back(addrSet);
                            }
                        }
                    } else {
                        auto value = std::any_cast<Value>(*next);
                        if (memcmp(temp, &value.value, size) == 0) {
                            addrSet.insert(addr);
                            return around(next, addr, currPage, addrSet);
                        }
                    }
                    distance++;
                }
#else
                for (int i = 0; i < (currPage->end() - currPtr) / size; ++i) {
                    if (distance > group_size) {
                        return false;
                    }
                    Pointer addr = currPtr + i * static_cast<long >(size);
                    process->read(addr, &temp, size);

                    if (is_range) {
                        auto value = std::any_cast<RangeValue>(*next);
                        auto min = value.min.value.i64;
                        auto max = value.max.value.i64;

                        if ((temp.i64 >= min && temp.i64 <= max && !value.isOutRange) ||
                            ((temp.i64 < min || temp.i64 > max) && value.isOutRange)) {
                            addrSet.insert(addr);
                            return around(next, addr, currPage, addrSet);
                        }
                    } else {
                        auto value = std::any_cast<Value>(*next);
                        if (memcmp(&temp, &value.value, size) == 0) {
                            addrSet.insert(addr);
                            return around(next, addr, currPage, addrSet);
                        }
                    }
                    distance += size;
                }
#endif
                nextPageV2:
                currPage = currPage->next();
            }
            return false;
        };

        while (currPage != nullptr) {
            auto startPtr = currPage->start();
#if !IGNORE_MISSING_PAGE
            if (process->isMissingPage(startPtr)) {
                goto nextPage;
            }
#endif
#if SUPPORT_UNALIGNED_MEMORY
            for (int i = 0; i < (currPage->end() - startPtr); ++i) {
                Pointer addr = startPtr + i;
                process->read(addr, &temp, valueSize);

                if (tv->type() == typeid(Value)) {
                    auto value = std::any_cast<Value>(*tv);
                    if (memcmp(&temp, &value.value, valueSize) == 0) {
                        std::set<Pointer> addrSet;
                        addrSet.insert(addr);
                        if (around(tv, addr, currPage, addrSet)) {
                            result.emplace_back(addrSet);
                        }
                    }
                } else if (tv->type() == typeid(RangeValue)) {
                    auto value = std::any_cast<RangeValue>(*tv);
                    auto min = value.min.value.i64;
                    auto max = value.max.value.i64;

                    if ((temp.i64 >= min && temp.i64 <= max && !value.isOutRange) ||
                            ((temp.i64 < min || temp.i64 > max) && value.isOutRange)
                    ) {
                        std::set<Pointer> addrSet;
                        addrSet.insert(addr);
                        if (around(tv, addr, currPage, addrSet)) {
                            result.emplace_back(addrSet);
                        }
                    }
                } else {
                    throw std::runtime_error("Unknown value any::type");
                }
            }
#else
            for (int i = 0; i < (currPage->end() - startPtr) / valueSize; ++i) {
                Pointer addr = startPtr + i * static_cast<long>(valueSize);
                process->read(addr, &temp, valueSize);

                if (tv->type() == typeid(Value)) {
                    auto value = std::any_cast<Value>(*tv);
                    if (memcmp(&temp, &value.value, valueSize) == 0) {
                        distance += valueSize;
                        std::set<Pointer> addrSet;
                        addrSet.insert(addr);
                        if (around(tv, addr, currPage, addrSet)) {
                            result.emplace_back(addrSet);
                        }
                    }
                } else if (tv->type() == typeid(RangeValue)) {
                    auto value = std::any_cast<RangeValue>(*tv);
                    auto min = value.min.value.i64;
                    auto max = value.max.value.i64;

                    if ((temp.i64 >= min && temp.i64 <= max && !value.isOutRange) ||
                            ((temp.i64 < min || temp.i64 > max) && value.isOutRange)
                    ) {
                        distance += valueSize;
                        std::set<Pointer> addrSet;
                        addrSet.insert(addr);
                        if (around(tv, addr, currPage, addrSet)) {
                            result.emplace_back(addrSet);
                        }
                    }
                } else {
                    throw std::runtime_error("Unknown value any::type");
                }
            }
#endif
            nextPage:
            currPage = currPage->next();
        }

        return RESULT_SUCCESS;
    }

    int MemorySearcher::filterNumber(const char *expr) {
        if (result.empty())
            return RESULT_EMPTY_RESULT;

        std::vector<std::any> values;
        unsigned int group_size = 512;

        if(Lexer::llex(values, group_size, expr))
            return RESULT_INVALID_ARGUMENT;

        auto tv = values.begin();
        size_t readyMatch = 0;
        size_t valueSize = Lexer::determineSize(Lexer::getRealType(tv));
        BasicValue temp{};
        size_t distance = 0;

        std::function<bool(
                std::__wrap_iter<std::any*>,
                std::set<Pointer>::iterator,
                std::set<Pointer>&
        )> around = [&around, &distance, &group_size, &values, &readyMatch, this](auto value_iter, std::set<Pointer>::iterator addr_iter, auto &newSet) {
            auto curr_addr = addr_iter++;
            auto curr_value = value_iter + 1;
            if ((curr_addr == addr_iter || *addr_iter == 0)|| distance >= group_size) {
                return readyMatch >= values.size();
            }

            bool is_range = Lexer::isRange(curr_value);
            auto size = Lexer::determineSize(Lexer::getRealType(curr_value));
            BasicValue temp{};

            process->read(*addr_iter, &temp, size);
            if (is_range) {
                auto value = std::any_cast<RangeValue>(*curr_value);
                auto min = value.min.value.i64;
                auto max = value.max.value.i64;
                if (
                        ((temp.i64 >= min && temp.i64 <= max && !value.isOutRange) || ((temp.i64 < min || temp.i64 > max) && value.isOutRange))
                        && (distance += size) <= group_size
                ) {
                    readyMatch++;
                    newSet.insert(*addr_iter);
                } else {
                    curr_value = value_iter;
                }
            } else {
                auto value = std::any_cast<Value>(*curr_value);
                if (memcmp(&temp, &value.value, size) == 0 && (distance += size) <= group_size)  {
                    readyMatch++;
                    newSet.insert(*addr_iter);
                } else {
                    curr_value = value_iter;
                }
            }
            return around(curr_value, addr_iter, newSet);
        };

        std::vector<std::set<Pointer>> newResult;
        std::for_each(result.begin(), result.end(), [&](const std::set<Pointer> &group) {
            auto head_addr = group.begin();
            process->read(*head_addr, &temp, valueSize);
            if (tv->type() == typeid(Value)) {
                auto value = std::any_cast<Value>(*tv);
                if (memcmp(&temp, &value.value, valueSize) == 0) {
                    distance += valueSize;
                    readyMatch++;
                    std::set<Pointer> newSet;
                    newSet.insert(*head_addr);
                    if (around(tv, head_addr, newSet)) {
                        newResult.push_back(newSet);
                    }
                }
            } else if (tv->type() == typeid(RangeValue)) {
                auto value = std::any_cast<RangeValue>(*tv);
                auto min = value.min.value.i64;
                auto max = value.max.value.i64;

                if ((temp.i64 >= min && temp.i64 <= max && !value.isOutRange) ||
                    ((temp.i64 < min || temp.i64 > max) && value.isOutRange)) {
                    distance += valueSize;
                    readyMatch++;
                    std::set<Pointer> newSet;
                    newSet.insert(*head_addr);
                    if (around(tv, head_addr, newSet)) {
                        newResult.push_back(newSet);
                    }
                }
            } else {
                throw std::runtime_error("Unknown value any::type");
            }
        });
        result.clear();
        result = std::move(newResult);
        return RESULT_SUCCESS;
    }

    template<typename T>
    int MemorySearcher::filter(T data) {
        if (!std::is_fundamental<T>::value) {
            return RESULT_NOT_FUNDAMENTAL;
        }
        return filter(&data, sizeof(T));
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