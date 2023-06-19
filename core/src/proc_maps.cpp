#include "proc_maps.h"

#include <string>
#include <vector>
#include <functional>
#include <fstream>
#include <sstream>
#include "exception.h"

namespace fs = std::filesystem;

hak::proc_maps::proc_maps(pointer start, pointer end) {
    this->_start = start;
    this->_end = end;
}

void hak::proc_maps::insert(std::shared_ptr<hak::proc_maps> maps) { // NOLINT(*-unnecessary-value-param)
    if (maps == shared_from_this()) {
        throw hak::recursive_maps_error();
    }
    if (this->_tail == nullptr) {
        this->_tail = maps;
    } else {
        auto temp = this->_tail;
        maps->_head = shared_from_this();
        maps->last()->_tail = temp;
        this->_tail = maps;
    }
}

void hak::proc_maps::remove() {
    _head->_tail = _tail;
    _tail->_head = _head;
}

auto hak::proc_maps::size() -> size_t {
    size_t size = 1;
    auto curr = shared_from_this();
    while ((curr = curr->next()) != nullptr) {
        size++;
    }
    return size;
}

auto hak::proc_maps::start() const -> pointer {
    return _start;
}

auto hak::proc_maps::end() const -> pointer {
    return _end;
}

auto hak::proc_maps::next() -> std::shared_ptr<hak::proc_maps>& {
    return _tail;
}

auto hak::proc_maps::last() -> std::shared_ptr<hak::proc_maps> {
    auto curr = shared_from_this();
    std::shared_ptr<proc_maps> result = curr;
    while ((curr = curr->next()) != nullptr) {
        result = curr;
    }
    return result;
}

void determine_range(hak::proc_maps* maps, bool last_is_cd) { // NOLINT(*-no-recursion)
    using namespace hak;
    auto *module_name = maps->module_name;
    if (maps->executable) {
        if (module_name[0] == '\0' || (strstr(module_name, "/data/app") != nullptr) || (strstr(module_name, "/data/user") != nullptr)) {
            maps->range = XA;
        } else {
            maps->range = XS;
        }
        return;
    }
    if (module_name[0] != '\0') {
        if ((strncmp(module_name, "/dev/", 5) == 0)
            && ((strstr(module_name, "/dev/mali") != nullptr)
                 || (strstr(module_name, "/dev/kgsl") != nullptr)
                 || (strstr(module_name, "/dev/nv") != nullptr)
                 || (strstr(module_name, "/dev/tegra") != nullptr)
                 || (strstr(module_name, "/dev/ion") != nullptr)
                 || (strstr(module_name, "/dev/pvr") != nullptr)
                 || (strstr(module_name, "/dev/render") != nullptr)
                 || (strstr(module_name, "/dev/galcore") != nullptr)
                 || (strstr(module_name, "/dev/fimg2d") != nullptr)
                 || (strstr(module_name, "/dev/quadd") != nullptr)
                 || (strstr(module_name, "/dev/graphics") != nullptr)
                 || (strstr(module_name, "/dev/mm_") != nullptr)
                 || (strstr(module_name, "/dev/dri/") != nullptr))) {
            maps->range = V;
            return;
        }
        if ( ((strncmp(module_name, "/dev/", 5) == 0) && (strstr(module_name, "/dev/xLog") != nullptr))
             || (strncmp(module_name, "/system/fonts/", 0xe) == 0)
             || (strncmp(module_name, "anon_inode:dmabuf", 0x11) == 0)) {
            maps->range = BAD;
            return;
        }
        if (strstr(module_name, "[anon:.bss]") != nullptr) {
            maps->range = last_is_cd ? CB : OTHER;
            return;
        }
        if (strncmp(module_name, "/system/", 8) == 0) {
            maps->range = OTHER;
            return;
        }
        if (strstr(module_name, "/dev/zero") != nullptr) {
            maps->range = CA;
            return;
        }
        if (strstr(module_name, "PPSSPP_RAM") != nullptr) {
            maps->range = PS;
            return;
        }
        if ( (strstr(module_name, "system@") == nullptr)
             && (strstr(module_name, "gralloc") == nullptr)
             && strncmp(module_name, "[vdso]", 6) != 0
             && strncmp(module_name, "[vectors]", 9) != 0
             && (strncmp(module_name, "/dev/", 5) != 0 || (strncmp(module_name, "/dev/ashmem", 0xB) == 0)) ) {
            if ( strstr(module_name, "dalvik") != nullptr ) {
                if ( ((strstr(module_name, "eap") != nullptr)
                      || (strstr(module_name, "dalvik-alloc") != nullptr)
                      || (strstr(module_name, "dalvik-main") != nullptr)
                      || (strstr(module_name, "dalvik-large") != nullptr)
                      || (strstr(module_name, "dalvik-free") != nullptr))
                     && (strstr(module_name, "itmap") == nullptr)
                     && (strstr(module_name, "ygote") == nullptr)
                     && (strstr(module_name, "ard") == nullptr)
                     && (strstr(module_name, "jit") == nullptr)
                     && (strstr(module_name, "inear") == nullptr) ) {
                    maps->range = JH;
                    return;
                }
                maps->range = J;
                return;
            }
            if ((strstr(module_name, "/lib") != nullptr) && (strstr(module_name, ".so") != nullptr)) {
                if (strstr(module_name, "/data/") != nullptr || (strstr(module_name, "/mnt/") != nullptr)) {
                    maps->range = CD;
                    return;
                }
            }
            if (strstr(module_name, "malloc") != nullptr) {
                maps->range = CA;
                return;
            }
            if (strstr(module_name, "[heap]") != nullptr) {
                maps->range = CH;
                return;
            }
            if (strstr(module_name, "[stack") != nullptr) {
                maps->range = S;
                return;
            }
            if ((strncmp(module_name, "/dev/ashmem", 0xB) == 0) && (strstr(module_name, "MemoryHeapBase") == nullptr)) {
                maps->range = AS;
                return;
            }
        }
        maps->range = OTHER;
        return;
    }
    if (maps->readable && maps->writable && !maps->executable && maps->offset == 0) {
        maps->range = A;
        return;
    }
    maps->range = OTHER;
}

void llex_maps(pid_t pid, const std::function<void(std::shared_ptr<hak::proc_maps>)>& callback) {
    std::ifstream maps(std::string("/proc/") + std::to_string(pid) + "/maps");
    if (!maps.is_open()) {
        throw hak::file_not_found();
    }
    std::string line;
    bool last_is_cd = false;
    while (getline(maps, line)) {
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;

        while (getline(iss, token, ' ')) {
            tokens.push_back(token);
        }

        auto address = tokens[0];
        std::string::size_type pos = address.find('-');
        pointer start_addr = std::stol(address.substr(0, pos), nullptr, 16);
        pointer end_addr = std::stol(address.substr(pos + 1), nullptr, 16);
        auto pmaps = std::make_shared<hak::proc_maps>(start_addr, end_addr);
        auto perms = tokens[1];
        pmaps->readable = perms[0] == 'r';
        pmaps->writable = perms[1] == 'w';
        pmaps->executable = perms[2] == 'x';
        pmaps->is_private = perms[3] == 'p';
        pmaps->offset = std::stoll(tokens[2], nullptr, 16);
        pmaps->inode = std::stoi(tokens[4]);
        std::string module_name;
        if (tokens.size() > 5) {
            for (int i = 5; i < tokens.size(); i++) {
                module_name += tokens[i];
            }
        }
        if (module_name.size() < 128) {
            module_name.copy(pmaps->module_name, module_name.size());
            pmaps->module_name[module_name.size()] = '\0';
        }
        determine_range(pmaps.get(), last_is_cd);
        last_is_cd = pmaps->range == hak::memory_range::CD;
        callback(pmaps);
    }
    maps.close();
}

auto hak::get_maps(pid_t pid, i32 range) -> std::shared_ptr<proc_maps> {
    std::shared_ptr<proc_maps> head;
    llex_maps(pid, [&](std::shared_ptr<proc_maps> maps) { // NOLINT(*-unnecessary-value-param)
        if ((range & maps->range) == maps->range) {
            if (head == nullptr) {
                head.swap(maps);
            } else {
                head->insert(maps);
            }
        }
    });
    return head;
}
