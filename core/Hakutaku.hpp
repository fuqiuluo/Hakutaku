#include <string>
#include <cmath>
#include <string>

#include <unistd.h>
#include <dirent.h>

#define PLAT_PROC_DIR "/proc"

#ifdef HAKUTAKU_64BIT
typedef long long Pointer;
#else
typedef long Pointer;
#endif
typedef int Pid;

namespace Hakutaku {
    namespace Platform {
        bool rootPermit();

        inline int getPageSize();

        Pointer getPageBegin(Pointer ptr);

        void stopProcess(Pid pid);

        void recoverProcess(Pid pid);

        void killProcess(Pid pid);

        std::string execCmd(const char *cmd);
    }

    class Process {
    protected:
        Pid pid;

    public:
        explicit Process(Pid pid);

        ~Process();

        void stop() const;

        void recover() const;

        void kill() const;
    };

    /**
     * Get Pid by packageName(cmd_line)
     */
    Pid getPid(std::string& packageName);

    inline Process openProcess(Pid pid);
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
}

// ############## CORE ######################
namespace Hakutaku {
    Process::Process(Pid pid) : pid(pid) {

    }

    void Process::stop() const {
        Platform::stopProcess(pid);
    }

    void Process::recover() const {
        Platform::recoverProcess(pid);
    }

    void Process::kill() const {
        Platform::killProcess(pid);
    }

    Process::~Process() = default;

    Pid getPid(std::string& packageName) {
        std::string cmd = "pidof ";
        cmd += packageName;
        auto result = Platform::execCmd(cmd.c_str());
        if (!result.empty()) {
            return (int) std::strtol(result.c_str(), nullptr, 0);
        }
        auto proc_dir = opendir(PLAT_PROC_DIR);
        if (proc_dir == nullptr) {
            return 0;
        }
        FILE* fp;
        std::string file_path;
        dirent* pid_file;
        char cmd_line[128];

        while ((pid_file = readdir(proc_dir))) {
            if (pid_file->d_type != DT_DIR ||
                strcmp(pid_file->d_name, ".") == 0 ||
                strcmp(pid_file->d_name, "..") == 0)
                continue;

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

    inline Process openProcess(Pid pid) {
        auto process = Process(pid);
        return process;
    }
}