#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <algorithm>
#include "../core/Hakutaku.hpp"

static std::shared_ptr<Hakutaku::Process> process;

void err(const char *s);
int select(std::string &package);
int read(Pointer ptr, size_t size);
int write(Pointer ptr, size_t size, void *data);

int main() {
    std::string command;
    int result;
    while (true) {
        std::cout << ">>> ";
        getline(std::cin, command);
        std::stringstream parser(command);
        std::string cmd;
        parser >> cmd;
        if (cmd == "exit") break;
        else {
            if(cmd == "select") {
                std::string package;
                parser >> package;
                result = select(package);
            } else if (cmd == "is64") {
                std::cout << process->is64Bit() << std::endl;
            } else if (cmd == "read") {
                Pointer ptr;
                size_t size;
                parser >> std::hex >> ptr;
                parser >> std::dec >> size;
                result = read(ptr, size);
            } else if (cmd == "write") {
                Pointer ptr;
                size_t size;
                long data;
                parser >> std::hex >> ptr;
                parser >> std::dec >> data;
                parser >> std::dec >> size;
                result = write(ptr, size, &data);
                std::cout << "write data to " << ptr << ", data = " << data << std::endl;
            } else if (cmd == "searchSingle") {

            } else {
                err("Unknown input.");
            }
        }
    }
    return result;
}

std::string trim(std::string& s) {
    auto start = std::find_if(s.begin(), s.end(), [](int c) { return !std::isspace(c); });
    auto end = std::find_if(s.rbegin(), s.rend(), [](int c) { return !std::isspace(c); });
    return {start, end.base()};
}

void err(const char *s) {
    std::cout << "\033[31m" << s << "\033[0m" << std::endl;
}

int select(std::string& package) {
    package = trim(package);
    auto pid = Hakutaku::getPid(package);
    if (pid == 0) {
        err("No corresponding process found.");
        return -1;
    }
    if (process != nullptr)
        process.reset();
    process = Hakutaku::openProcess(pid);
    std::cout << "Successfully selected process: " << package << ",pid: "<< pid << std::endl;
    return 0;
}

int read(Pointer ptr, size_t size) {
    if (process == nullptr) {
        err("No process selected.");
        return -1;
    }
    if (size > INT64_MAX) {
        err("Size too much big.");
        return -2;
    }
    if (size < 8) {
        size = 8;
    }
    Hakutaku::Utils::hexDump(process, ptr, (int) (size / 8));
    return 0;
}

int write(Pointer ptr, size_t size, void *data) {
    if (process == nullptr) {
        err("No process selected.");
        return -1;
    }
    if (size > INT64_MAX) {
        err("Size too much big.");
        return -2;
    }
    if (size < 8) {
        size = 8;
    }
    process->write(ptr, data, size);
    return 0;
}
