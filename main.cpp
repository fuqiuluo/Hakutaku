#include <thread>
#include <iostream>
#include "process.h"
#include "proc.h"
#include "exception.h"
#include "memory.h"

std::string packageName = "com.tencent.tmgp.sgame";
std::string nameGameCore = "libGameCore.so";
std::string nameIL2cpp = "libil2cpp.so";

class player {
public:
    int heroId;
    int teamId;
    pointer health_ptr, healthMax_ptr;
    pointer skill3_CD_ptr, skill3_CD_Max_ptr;
    pointer summonerSpells_ptr, summonerSpellsMax_ptr;
    pointer activeEquipment_ptr, activeEquipmentMax_ptr;
    pointer x_ptr, y_ptr;

    void update(hak::process& process) const;
};

int localTeamId = 1;
std::vector<player> players;

void load_unity(std::shared_ptr<hak::process>& process, std::shared_ptr<hak::proc_maps>& baseIL2CPP);
int load_players(std::shared_ptr<hak::process>& process, std::shared_ptr<hak::proc_maps>& baseGameCore);

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

    auto baseGameCore = hak::get_process_map(pid, nameGameCore);
    auto baseIL2CPP = hak::get_process_map(pid, nameIL2cpp);
    if (!baseGameCore || !baseIL2CPP) {
        std::cout << "error: get_process_map" << "\n";
        return 2;
    }

    std::cout << "libGameCore.so: 0x" << std::hex << baseGameCore->start() << "\n";
    std::cout << "libil2cpp.so: 0x" << std::hex << baseIL2CPP->start() << "\n";

    // 计算耗时
    auto start = std::chrono::steady_clock::now();
    load_unity(process, baseIL2CPP);
    load_players(process, baseGameCore);
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";


    return 0;
}

void load_unity(std::shared_ptr<hak::process> &process, std::shared_ptr<hak::proc_maps> &baseIL2CPP) {
    pointer tmp;
    //process->read(tmp + 232, &localTeamId, sizeof(int));
    if (localTeamId > 0) {
        localTeamId = 1;
    } else {
        localTeamId = -1;
    }
    std::cout << "localTeamId = " << std::dec << localTeamId << "\n";
}

int load_players(std::shared_ptr<hak::process> &process, std::shared_ptr<hak::proc_maps>& baseGameCore) {
    pointer mapLogicObject;
    try {
        process->read_pointer(baseGameCore->start() + 0x3876F10, &mapLogicObject);
    } catch (std::exception& e) {
        std::cout << "error: " << e.what() << "\n";
        return -1;
    }
    std::cout << "libGameCore.so:MapLogic: 0x" << std::hex << mapLogicObject << "\n";

    pointer next = mapLogicObject + 568;

    pointer mapLogicObjectPlayer; // 偏移24
    process->read_pointer(next, &mapLogicObjectPlayer);

    try {
        for (int i = 0; i < 10;) {
            pointer playerItem;
            process->read_pointer(mapLogicObjectPlayer + 24 * i, &playerItem);

            int teamId; // 队伍ID
            process->read(playerItem + 60, &teamId, sizeof(int));
            if (teamId == 1) {
            //    std::cout << "==========> 蓝方PlayerItem\n";
            } else {
            //    std::cout << "==========> 红方PlayerItem\n";
            }

            std::cout << "PlayerItem[pointer = 0x" << std::hex << playerItem << "]\n";

            int heroId;
            process->read(playerItem + 48, &heroId, sizeof(int));
            //std::cout << "英雄ID = " << std::dec << heroId << "\n";

            pointer heroStatus;
            process->read_pointer(playerItem + 352, &heroStatus);

            int health, maxHealth;
            process->read(heroStatus + 160, &health, sizeof(int));
            process->read(heroStatus + 168, &maxHealth, sizeof(int));

            //std::cout << "生命值 = " << std::dec << (health) << "/" << maxHealth << "\n";

            pointer skill3_CD_ptr, skill3_CD_Max_ptr;
            {
                //int skill3_CD, skill3_CD_Max; // 大招
                pointer tmp;
                process->read_pointer(playerItem + 328, &tmp);
                process->read_pointer(tmp + 264, &tmp);
                process->read_pointer(tmp + 168, &tmp);
                //process->read(tmp + 60, &skill3_CD, sizeof(int));
                //process->read(tmp + 68, &skill3_CD_Max, sizeof(int));
                //std::cout << "大招 = " << std::dec << (skill3_CD / 8192000) << ":" << (skill3_CD_Max / 8192000) << "s\n";

                skill3_CD_ptr = tmp + 60;
                skill3_CD_Max_ptr = tmp + 68;
            }
            //std::cout << "skill3_CD_Rate = " << std::dec << (skill3_CD / skill3_CD_Max) << " [pointer = 0x" << std::hex << tmp + 60 << "]\n";

            pointer summonerSpells_ptr, summonerSpellsMax_ptr; // 召唤师技能
            {
                //int summonerSpells, summonerSpellsMax; // 召唤师技能
                pointer tmp;
                process->read_pointer(playerItem + 328, &tmp);
                process->read_pointer(tmp + 336, &tmp);
                process->read_pointer(tmp + 168, &tmp);
                //process->read(tmp + 60, &summonerSpells, sizeof(int));
                //process->read(tmp + 68, &summonerSpellsMax, sizeof(int));
                //std::cout << "召唤师技能 = " << std::dec << (summonerSpells / 8192000) << ":" << (summonerSpellsMax / 8192000) << "s\n";

                summonerSpells_ptr = tmp + 60;
                summonerSpellsMax_ptr = tmp + 68;
            }


            pointer activeEquipment_ptr, activeEquipmentMax_ptr; // 主动装备
            {
                //int activeEquipment, activeEquipmentMax;
                pointer tmp;
                process->read_pointer(playerItem + 328, &tmp);
                process->read_pointer(tmp + 360, &tmp);
                process->read_pointer(tmp + 280, &tmp);
                //process->read(tmp + 32, &activeEquipment, sizeof(int));
                //process->read(tmp + 40, &activeEquipmentMax, sizeof(int));
                //std::cout << "主动装备 = " << std::dec << (activeEquipment) << ":" << activeEquipmentMax << "s\n";

                activeEquipment_ptr = tmp + 32;
                activeEquipmentMax_ptr = tmp + 40;
            }

            pointer x_ptr, y_ptr;
            {
                int a = 0, b = 0;
                pointer tmp;
                process->read_pointer(playerItem + 544, &tmp);
                process->read_pointer(tmp + 16, &tmp);
                process->read_pointer(tmp + 0, &tmp);
                process->read_pointer(tmp + 16, &tmp);

                //std::cout << "[pointer = 0x" << std::hex << tmp << "]\n";

                process->read(tmp + 0, &a, sizeof(int));
                process->read(tmp + 8, &b, sizeof(int));

                //std::cout << "坐标 = (" << std::dec << (a * 0.001) << ", " << (b * 0.001) << ")\n";

                x_ptr = tmp + 0;
                y_ptr = tmp + 8;
            }

            players.push_back(player{
                heroId,
                teamId,
                heroStatus + 160,
                heroStatus + 168,
                skill3_CD_ptr,
                skill3_CD_Max_ptr,
                summonerSpells_ptr,
                summonerSpellsMax_ptr,
                activeEquipment_ptr,
                activeEquipmentMax_ptr,
                x_ptr,
                y_ptr
            });

            i++;
        }

        std::cout << "==========> \n";

        while (true) {
            // 清空控制台
            std::cout << "\033[2J\033[1;1H";

            for (auto& player : players) {
                player.update(*process);
            }

            //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        //process->dump_memory(playerListObject, 15);
    } catch (std::exception& e) {
        std::cout << "error: " << e.what() << "\n";
    }

    return 0;
}

void player::update(hak::process &process) const {
    // 打印信息
    int healthValue, maxHealth;
    process.read(health_ptr, &healthValue, sizeof(int));
    process.read(healthMax_ptr, &maxHealth, sizeof(int));

    int skill3_CD, skill3_CD_Max; // 大招
    process.read(skill3_CD_ptr, &skill3_CD, sizeof(int));
    process.read(skill3_CD_Max_ptr, &skill3_CD_Max, sizeof(int));

    int summonerSpells, summonerSpellsMax; // 召唤师技能
    process.read(summonerSpells_ptr, &summonerSpells, sizeof(int));
    process.read(summonerSpellsMax_ptr, &summonerSpellsMax, sizeof(int));

    int activeEquipment, activeEquipmentMax;
    process.read(activeEquipment_ptr, &activeEquipment, sizeof(int));
    process.read(activeEquipmentMax_ptr, &activeEquipmentMax, sizeof(int));

    int x, y;
    process.read(x_ptr, &x, sizeof(int));
    process.read(y_ptr, &y, sizeof(int));

    std::cout << "Player(team=" << (teamId == 1 ? "蓝方" : "红方") << ", ";
    std::cout << "ID=" << std::dec << heroId << ", ";
    std::cout << "health=" << std::dec << (healthValue) << "/" << maxHealth << ", ";
    std::cout << "skill=" << std::dec << (skill3_CD / 8192000) << ":" << (skill3_CD_Max / 8192000) << "s, ";
    std::cout << "spell=" << std::dec << (summonerSpells / 8192000) << ":" << (summonerSpellsMax / 8192000) << "s, ";
    std::cout << "equipment=" << std::dec << (activeEquipment) << ":" << activeEquipmentMax << "s, ";
    std::cout << "location=[" << std::dec << (x * 0.001) << ", " << (y * 0.001) << "])\n";
}
