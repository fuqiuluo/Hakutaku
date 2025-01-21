#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/xattr.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <iostream>
#include "process.h"
#include "proc.h"
#include "exception.h"
#include "memory.h"
#include "uedumper.h"
#include "StructsSDK.h"

std::string packageName = "com.tencent.tmgp.dfm";
std::string libUE4 = "libUE4.so";

using namespace std;

int main() {
    pid_t pid;
    try {
        pid = hak::find_process(packageName);
    } catch (hak::no_process_error& e) {
        std::cout << "error: no_process_error" << "\n";
        return 1;
    }
    auto process = std::make_shared<hak::process>(pid);
    process->set_memory_mode(hak::SYSCALL);

    auto base_libUE4 = hak::get_process_map(pid, libUE4)->start();

    auto offset_GName = 0x179AEBC0;
    auto offset_GWorld = 0x17cb3018;

    auto GWorld = process->read<pointer>(base_libUE4 + offset_GWorld);
    int id = process->read<int>(GWorld + 36);
    std::cout << "GWorld: 0x" << std::hex << GWorld <<", Name: " << GetFromFName(*process, base_libUE4 + offset_GName, id) << std::endl;

    LoopObjects(*process, GWorld, base_libUE4, base_libUE4 + offset_GName, 0x769d7aaf60);

    DumpSDKW(*process, GWorld, base_libUE4, base_libUE4 + offset_GName, "/sdcard");

//    auto GameInstance = process->read<pointer>(GWorld + 0x1a0);
//    std::cout << "GameInstance: 0x" << std::hex << GameInstance << ", Name: " << UObject::getName(*process, base_libUE4 + offset_GName, GameInstance) << std::endl;
//
//    auto LocalPlayer = process->read<pointer>(process->read<pointer>(GameInstance + 0x48));
//    std::cout << "LocalPlayer: 0x" << std::hex << LocalPlayer << ", Name: " << UObject::getName(*process, base_libUE4 + offset_GName, LocalPlayer) << std::endl;

    std::cout << "Name: " << GetFromFName(*process, base_libUE4 + offset_GName, 0) << std::endl;

    return 0;
}
