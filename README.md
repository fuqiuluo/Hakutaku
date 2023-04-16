# Hakutaku

Language: [简体中文](README_zh.md) | [English](README.md)

![GitHub release](https://img.shields.io/github/release/fuqiuluo/Hakutaku.svg)
![GitHub license](https://img.shields.io/github/license/fuqiuluo/HakutakU.svg)
![GitHub repo size](https://img.shields.io/github/repo-size/fuqiuluo/Hakutaku.svg)
![GitHub top language](https://img.shields.io/github/languages/top/fuqiuluo/Hakutaku.svg)

Hakutaku's name is Bai Ze, which means omniscient. A memory modifier key wizard core program, supporting Android SDK 24 and above.

Hakutaku is a memory modification tool based on the Android SDK 24+.

# Instructions

#### Supported Memory Ranges

```c++
#define RANGE_ALL 8190 // all
#define RANGE_BAD 2 // b
#define RANGE_V 4 // v
#define RANGE_CA 8 // ca
#define RANGE_CB 16 // cb
#define RANGE_CD 32 // cd
#define RANGE_CH 64 // ch
#define RANGE_JH 128 // jh
#define RANGE_A 256 // a
#define RANGE_XS 512 // xs
#define RANGE_S 1024 // s
#define RANGE_AS 2048 // as
#define RANGE_OTHER 4096 // other
```

#### StatusCode/ErrorCode
```c++
// result-code of searching
#define RESULT_SUCCESS 0 // success
#define RESULT_OFE (-1) // error opening file
#define RESULT_ADDR_NRA (-2) // address not readable
#define RESULT_ADDR_NWA (-3) // address not writable
#define RESULT_UNKNOWN_WORK_MODE (-4) // unknown memory read write mode
#define RESULT_EMPTY_MAPS (-5) // maps is empty
#define RESULT_NOT_FUNDAMENTAL (-6) // Non-basic types (only basic types are supported when using a mode that does not need to provide a size to search for values)
#define RESULT_EMPTY_RESULT (-7) // search result is empty
// The above code does not need to be written into your own file, just for you to see
```
#### Caution

- Copying of the **Process** object is prohibited, otherwise the /proc/[pid]/mem handle will be released early, resulting in an exception.

## Demo 

### [GetPid Demo](/test/pidof.cpp)

### [SUSPEND/RESUME/KILL PROCESS Demo](/test/stop_and_recover.cpp)

### [GetModuleBaseAddress Demo](/test/get_module_base.cpp)

### [Read/Write Memory Demo](/test/read_and_write.cpp)

### [GetTheSpecifiedMemoryRangeMaps](/test/get_maps.cpp)

### [Determine whether the current memory page is faulty](/test/is_memory_trap.cpp)

### [SetTheCurrentReadAndWriteMemoryMode](/test/set_mem_mode.cpp)

### [SingleValueSearch/FilterOperation](/test/search_and_filter.cpp)

### [FederatedSearch](/test/search_and_filter.cpp#L78)

### [DumpMemory](/test/mem_tools.cpp)

### [MemoryTool](/test/mem_tools.cpp)

### TouchTool（TODO）