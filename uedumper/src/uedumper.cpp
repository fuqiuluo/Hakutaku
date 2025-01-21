#include <cstring>
#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <process.h>
#include <byteswap.h>
#include <types.h>
#include <buffer_utils.h>
#include "uedumper.h"
#include "StructsSDK.h"
#include <stdint.h>
#ifdef __GNUC__
#include <endian.h>
#endif // __GNUC__

using kaddr = uint64;

#define END 0x0
uint32 MAX_SIZE = 100;
uint32 MAGIC = 0xe8;
uint32_t FNameEntryToNameString = 0x2;

using namespace std;

void FNameDecrypt(char *inputBuf, int namelength, bool isWide) {
    int length = (namelength > 0x400) ? 0x400 : namelength;
    for (int i = 0; i < length; ++i) {
        if (i % 2 == 0 && isWide) continue;
        inputBuf[i] ^= MAGIC;
    }
}

void FNameEncrypt(char *inputBuf, int len, char *dest) {
    memcpy(dest, inputBuf, len);
    int length = (len > 0x400) ? 0x400 : len;
    for (int i = 0; i < length; ++i) {
        //if (i % 2 != 0)
        dest[i] ^= MAGIC;
    }
}

std::string ReadStr(hak::process &p, kaddr address, unsigned int size) {
    char *data = new char[size];
    memset(data, '\0', size);

    auto flag = p.read<int16_t>(address);

    int len = 4;
    if (flag >> 6 > len) {
        len = flag >> 6;
    }

    address += 2;

    //std::cout << "Flag: " << std::hex << flag << std::endl;
    //std::cout << "Len: " << std::dec << len << std::endl;

    for (int i = 0; i < len; i++) {
        //vm_readv((void *) (address + (sizeof(char) * i)), (void *) (&data[0] + i), sizeof(char));
        p.readRaw(address + (sizeof(char) * i), &data[i], sizeof(char));
    }

    FNameDecrypt(data, len, flag & 0x1);

//    std::u16string name;
//    for (int i = 0; i < strlen(data); ++i) {
//        name.push_back(data[i]);
//    }

    std::string name(data, len);

    delete[] data;
    return name;
}
static inline uint16_t byteswap_ushort(uint16_t number)
{
#if defined(_MSC_VER) && _MSC_VER > 1310
    return _byteswap_ushort(number);
#elif defined(__GNUC__)
    return __builtin_bswap16(number);
#else
    return (number >> 8) | (number << 8);
#endif
}

// 从UTF16 LE编码的字符串创建
std::string utf16le_to_utf8(const std::u16string& u16str)
{
    if (u16str.empty()){ return std::string(); }
    const char16_t* p = u16str.data();
    std::u16string::size_type len = u16str.length();
    if (p[0] == 0xFEFF){
        p += 1;	//带有bom标记，后移
        len -= 1;
    }

    // 开始转换
    std::string u8str;
    u8str.reserve(len * 3);

    char16_t u16char;
    for (std::u16string::size_type i = 0; i < len; ++i){
        // 这里假设是在小端序下(大端序不适用)
        u16char = p[i];

        // 1字节表示部分
        if (u16char < 0x0080){
            // u16char <= 0x007f
            // U- 0000 0000 ~ 0000 07ff : 0xxx xxxx
            u8str.push_back((char)(u16char & 0x00FF));	// 取低8bit
            continue;
        }
        // 2 字节能表示部分
        if (u16char >= 0x0080 && u16char <= 0x07FF){
            // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
            u8str.push_back((char)(((u16char >> 6) & 0x1F) | 0xC0));
            u8str.push_back((char)((u16char & 0x3F) | 0x80));
            continue;
        }
        // 代理项对部分(4字节表示)
        if (u16char >= 0xD800 && u16char <= 0xDBFF) {
            // * U-00010000 - U-001FFFFF: 1111 0xxx 10xxxxxx 10xxxxxx 10xxxxxx
            uint32_t highSur = u16char;
            uint32_t lowSur = p[++i];
            // 从代理项对到UNICODE代码点转换
            // 1、从高代理项减去0xD800，获取有效10bit
            // 2、从低代理项减去0xDC00，获取有效10bit
            // 3、加上0x10000，获取UNICODE代码点值
            uint32_t codePoint = highSur - 0xD800;
            codePoint <<= 10;
            codePoint |= lowSur - 0xDC00;
            codePoint += 0x10000;
            // 转为4字节UTF8编码表示
            u8str.push_back((char)((codePoint >> 18) | 0xF0));
            u8str.push_back((char)(((codePoint >> 12) & 0x3F) | 0x80));
            u8str.push_back((char)(((codePoint >> 06) & 0x3F) | 0x80));
            u8str.push_back((char)((codePoint & 0x3F) | 0x80));
            continue;
        }
        // 3 字节表示部分
        {
            // * U-0000E000 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
            u8str.push_back((char)(((u16char >> 12) & 0x0F) | 0xE0));
            u8str.push_back((char)(((u16char >> 6) & 0x3F) | 0x80));
            u8str.push_back((char)((u16char & 0x3F) | 0x80));
            continue;
        }
    }

    return u8str;
}


// 从UTF16BE编码字符串创建
std::string utf16be_to_utf8(const std::u16string& u16str)
{
    if (u16str.empty()){ return std::string(); }
    const char16_t* p = u16str.data();
    std::u16string::size_type len = u16str.length();
    if (p[0] == 0xFEFF){
        p += 1;	//带有bom标记，后移
        len -= 1;
    }


    // 开始转换
    std::string u8str;
    u8str.reserve(len * 2);
    char16_t u16char;	//u16le 低字节存低位，高字节存高位
    for (std::u16string::size_type i = 0; i < len; ++i) {
        // 这里假设是在小端序下(大端序不适用)
        u16char = p[i];
        // 将大端序转为小端序
        u16char = byteswap_ushort(u16char);

        // 1字节表示部分
        if (u16char < 0x0080) {
            // u16char <= 0x007f
            // U- 0000 0000 ~ 0000 07ff : 0xxx xxxx
            u8str.push_back((char)(u16char & 0x00FF));
            continue;
        }
        // 2 字节能表示部分
        if (u16char >= 0x0080 && u16char <= 0x07FF) {
            // * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
            u8str.push_back((char)(((u16char >> 6) & 0x1F) | 0xC0));
            u8str.push_back((char)((u16char & 0x3F) | 0x80));
            continue;
        }
        // 代理项对部分(4字节表示)
        if (u16char >= 0xD800 && u16char <= 0xDBFF) {
            // * U-00010000 - U-001FFFFF: 1111 0xxx 10xxxxxx 10xxxxxx 10xxxxxx
            uint32_t highSur = u16char;
            uint32_t lowSur = byteswap_ushort(p[++i]);
            // 从代理项对到UNICODE代码点转换
            // 1、从高代理项减去0xD800，获取有效10bit
            // 2、从低代理项减去0xDC00，获取有效10bit
            // 3、加上0x10000，获取UNICODE代码点值
            uint32_t codePoint = highSur - 0xD800;
            codePoint <<= 10;
            codePoint |= lowSur - 0xDC00;
            codePoint += 0x10000;
            // 转为4字节UTF8编码表示
            u8str.push_back((char)((codePoint >> 18) | 0xF0));
            u8str.push_back((char)(((codePoint >> 12) & 0x3F) | 0x80));
            u8str.push_back((char)(((codePoint >> 06) & 0x3F) | 0x80));
            u8str.push_back((char)((codePoint & 0x3F) | 0x80));
            continue;
        }
        // 3 字节表示部分
        {
            // * U-0000E000 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx
            u8str.push_back((char)(((u16char >> 12) & 0x0F) | 0xE0));
            u8str.push_back((char)(((u16char >> 6) & 0x3F) | 0x80));
            u8str.push_back((char)((u16char & 0x3F) | 0x80));
            continue;
        }
    }
    return u8str;
}

////////////////////////////////////////
//     以下转换都是在小端序下进行     //
////////////////////////////////////////

// 从UTF16编码字符串构建，需要带BOM标记
std::string utf16_to_utf8(const std::u16string& u16str)
{
    if (u16str.empty()){ return std::string(); }
    //Byte Order Mark
    char16_t bom = u16str[0];
    switch (bom){
        case 0xFEFF:	//Little Endian
            return utf16le_to_utf8(u16str);
            break;
        case 0xFFFE:	//Big Endian
            return utf16be_to_utf8(u16str);
            break;
        default:
            return std::string();
    }
}

auto GetFromFName(hak::process &p, uint64 NamePoolData, uint32_t index) -> std::string {
    static kaddr TNameEntryArray;

    if (TNameEntryArray) {//As usual caching ;)
        goto gotGName;
    }

    TNameEntryArray = NamePoolData;

gotGName:
    kaddr FNameEntryArr = p.read_pointer2(
            TNameEntryArray + (((index>>18) + 7) * 8));
    //std::cout << "FNameEntryArr: " << std::hex << FNameEntryArr << std::endl;
    kaddr FNameEntry = FNameEntryArr + ((2 * index) & 0x7FFFE);
    //std::cout << "FNameEntry: " << std::hex << FNameEntry << std::endl;

    if (MAGIC == 0xdf) {

    }

    auto s = ReadStr(p, FNameEntry, MAX_SIZE);
    return s;
}

void DumpStrings(hak::process &proc, uint64 NamePoolData, std::string out) {
    uint32 GNameLimit = 0x1000000;
    uint32 count = 0;
    std::ofstream gname(out + "/Strings.txt", std::ofstream::out);
    if (gname.is_open()) {
        std::cout << "Dumping Strings" << std::endl;
        clock_t begin = clock();

        for (uint32 i = 0; i < GNameLimit; i++) {
            try {
                std::string s = GetFromFName(proc, NamePoolData, i);
                if (!s.empty()) {
                    gname << "[" << i << "]: " << s << std::endl;
                    count++;
                }
            }catch (std::exception& e) {

            }

        }

        gname.close();
        clock_t end = clock();
        double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
        std::cout << count << " Strings Dumped in " << elapsed_secs << "S" << std::endl;
    }
}

bool isEqual(char *s1, const char *s2) {
    return (strcmp(s1, s2) == 0);
}

bool isEqual(std::string s1, const char *check) {
    std::string s2(check);
    return (s1 == s2);
}

bool isEqual(std::string s1, std::string s2) {
    return (s1 == s2);
}

bool isContain(std::string str, std::string check) {
    size_t found = str.find(check);
    return (found != std::string::npos);
}

std::vector<uint32> structIDMap;
int classCount = 0;

bool isScanned(uint32 id) {
    for (int i = 0; i < structIDMap.size(); i++) {
        if (structIDMap[i] == id) { return true; }
    }
    return false;
}

bool isStartWith(std::string str, const char *check) {
    return (str.rfind(check, 0) == 0);
}

string resolveProp(hak::process& p, uint64 NamePoolData, uintptr_t libbase, list<kaddr> &recurrce, kaddr prop) {
    if (prop) {
        string cname;
        try {
            cname = FField::getClassName(p, NamePoolData, prop);
            std::cout << "resolveProp: " << std::hex << prop << " - " << cname << std::endl;
        } catch (std::exception& e) {
            std::cout << "resolvePropFailed: prop=0x" << std::hex << prop << std::endl;
            return "{Unknown}";
        }

        if (isEqual(cname, "ObjectProperty") || isEqual(cname, "WeakObjectProperty")
            || isEqual(cname, "LazyObjectProperty") || isEqual(cname, "AssetObjectProperty")
            || isEqual(cname, "SoftObjectProperty")) {
            kaddr propertyClass = UObjectProperty::getPropertyClass(p, prop);
            recurrce.push_back(propertyClass);
            return UObject::getName(p, NamePoolData, propertyClass) + "*";
        } else if (isEqual(cname, "ClassProperty") || isEqual(cname, "AssetClassProperty") ||
                   isEqual(cname, "SoftClassProperty")) {
            kaddr metaClass = UClassProperty::getMetaClass(p, prop);
            recurrce.push_back(metaClass);
            return "class " + UObject::getName(p, NamePoolData, metaClass);
        } else if (isEqual(cname, "InterfaceProperty")) {
            kaddr interfaceClass = UInterfaceProperty::getInterfaceClass(p, prop);
            recurrce.push_back(interfaceClass);
            return "interface class" + UObject::getName(p, NamePoolData, interfaceClass);
        } else if (isEqual(cname, "StructProperty")) {
            kaddr Struct = UStructProperty::getStruct(p, prop);
            recurrce.push_back(Struct);
            return UObject::getName(p, NamePoolData, Struct);
        } else if (isEqual(cname, "ArrayProperty")) {
            return resolveProp(p, NamePoolData, libbase, recurrce, UArrayProperty::getInner(p, prop)) + "[]";
        } else if (isEqual(cname, "SetProperty")) {
            return "<" + resolveProp(p, NamePoolData, libbase, recurrce, USetProperty::getElementProp(p, prop)) + ">";
        } else if (isEqual(cname, "MapProperty")) {
            return "<" + resolveProp(p, NamePoolData, libbase, recurrce, UMapProperty::getKeyProp(p, prop)) + "," +
                   resolveProp(p, NamePoolData, libbase, recurrce, UMapProperty::getValueProp(p, prop)) + ">";
        } else if (isEqual(cname, "BoolProperty")) {
            return "bool";
        } else if (isEqual(cname, "ByteProperty")) {
            return "byte";
        } else if (isEqual(cname, "IntProperty")) {
            return "int";
        } else if (isEqual(cname, "Int8Property")) {
            return "int8";
        } else if (isEqual(cname, "Int16Property")) {
            return "int16";
        } else if (isEqual(cname, "Int64Property")) {
            return "int64";
        } else if (isEqual(cname, "UInt16Property")) {
            return "uint16";
        } else if (isEqual(cname, "UInt32Property")) {
            return "uint32";
        } else if (isEqual(cname, "UInt64Property")) {
            return "uint64";
        } else if (isEqual(cname, "DoubleProperty")) {
            return "double";
        } else if (isEqual(cname, "FloatProperty")) {
            return "float";
        } else if (isEqual(cname, "EnumProperty")) {
            return "enum";
        } else if (isEqual(cname, "StrProperty")) {
            return "FString";
        } else if (isEqual(cname, "TextProperty")) {
            return "FText";
        } else if (isEqual(cname, "NameProperty")) {
            return "FName";
        } else if (isEqual(cname, "DelegateProperty") ||
                   isEqual(cname, "MulticastDelegateProperty")) {
            return "delegate";
        } else {
            return UObject::getName(p, NamePoolData, prop) + "(" + cname + ")";
        }
    }
    return "NULL";
}

list<kaddr> writeStructChild(hak::process& p, uint64 NamePoolData, uintptr_t libbase, ofstream &sdk, kaddr childprop, bool isFun = true) {
    list<kaddr> recurrce;
    kaddr child = childprop;
    //std::cout << "Child: " << std::hex << child << std::endl;
    while (child) {
        kaddr prop = child;
        string cname;
        string oname;
        if (isFun) {
            try {
                oname = UObject::getName(p, NamePoolData, prop);
            } catch (std::exception& e) {
            }
            try {
                cname = UObject::getClassName(p, NamePoolData, prop);
            } catch (std::exception& e) {
                std::cout << "Prop2: " << oname << " - " << cname << std::endl;
                break;
            }
        } else {
            try {
                oname = FField::getName(p, NamePoolData, prop);
            } catch (std::exception& e) {
            }
            try {
                cname = FField::getClassName(p, NamePoolData, prop);
            } catch (std::exception& e) {
                std::cout << "Prop3: " << oname << " - " << cname << ", Prop: " << prop << std::endl;
                break;
            }
        }

        if (isStartWith(cname, "Function") || isEqual(cname, "DelegateFunction")) {
            string returnVal = "void";
            string params = "";

            sdk << "\t";

            int32 FunctionFlags = UFunction::getFunctionFlags(p, prop);
            if ((FunctionFlags & 0x00002000) == 0x00002000) {
                sdk << "static" << " ";
            }
            if((FunctionFlags & 0x00000001) == 0x00000001){
                sdk << "final" << " ";
            }
            if((FunctionFlags & 0x00020000) == 0x00020000){
                sdk << "public" << " ";
            }
            if((FunctionFlags & 0x00040000) == 0x00040000){
                sdk << "private" << " ";
            }
            if((FunctionFlags & 0x00080000) == 0x00080000){
                sdk << "protected" << " ";
            }

            sdk << returnVal << " " << oname << "(" << params << ");"
                << "// 0x" << setbase(16) << (UFunction::getFunc(p, prop) - libbase) << endl;
            goto next;
        }

        std::cout << "Prop: " << oname << " - " << cname << std::endl;

        if (isEqual(cname, "ObjectProperty") || isEqual(cname, "WeakObjectProperty") ||
            isEqual(cname, "LazyObjectProperty") || isEqual(cname, "AssetObjectProperty") ||
            isEqual(cname, "SoftObjectProperty")) {

           //std::cout << "ObjectProperty: " << oname << " - " << cname << ", Prop: " << prop << std::endl;

            kaddr propertyClass = UObjectProperty::getPropertyClass(p, prop);

            sdk << "\t" << UObject::getName(p, NamePoolData, propertyClass) << "* " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;

            recurrce.push_back(propertyClass);
        } else if (isEqual(cname, "ClassProperty") || isEqual(cname, "AssetClassProperty") ||
                  isEqual(cname, "SoftClassProperty")) {

            std::cout << "ClassProperty: " << oname << " - " << cname << ", Prop: " << prop << std::endl;

            kaddr metaClass = UClassProperty::getMetaClass(p, prop);

            sdk << "\tclass " << UObject::getName(p, NamePoolData, metaClass) << "* " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;

            recurrce.push_back(metaClass);
        } else if (isEqual(cname, "InterfaceProperty")) {
            //std::cout << "InterfaceProperty: " << oname << " - " << cname << ", Prop: " << prop << std::endl;

            kaddr interfaceClass = UInterfaceProperty::getInterfaceClass(p, prop);

            sdk << "\tinterface class " << UObject::getName(p, NamePoolData, interfaceClass) << "* " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else if (isEqual(cname, "StructProperty")) {
            //std::cout << "StructProperty: " << oname << " - " << cname << ", Prop: " << prop << std::endl;
            kaddr Struct = UStructProperty::getStruct(p, prop);

            sdk << "\t" << UObject::getName(p, NamePoolData, Struct) << " " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;

            recurrce.push_back(Struct);
        } else if (isEqual(cname, "ArrayProperty")) {
            std::cout << "ArrayProperty: " << oname << " - " << cname << ", Prop: " << prop << std::endl;
            sdk << "\t" << resolveProp(p, NamePoolData, libbase, recurrce,
                                       UArrayProperty::getInner(p, prop)) << "[] " << oname
                << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else if (isEqual(cname, "SetProperty")) {
            sdk << "\t<" << resolveProp(p, NamePoolData, libbase, recurrce,
                                        USetProperty::getElementProp(p, prop)) << "> "
                << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else if (isEqual(cname, "MapProperty")) {
            sdk << "\t<" << resolveProp(p, NamePoolData, libbase, recurrce,
                                        UMapProperty::getKeyProp(p, prop)) << ","
                << resolveProp(p, NamePoolData, libbase, recurrce, UMapProperty::getValueProp(p, prop)) << "> " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else if (isEqual(cname, "BoolProperty")) {
            sdk << "\tbool " << oname << ";"
                << setbase(10) << "//(ByteOffset: " << (int) UBoolProperty::getByteOffset(p, prop)
                << ", ByteMask: " << (int) UBoolProperty::getByteMask(p, prop)
                << ", FieldMask: " << (int) UBoolProperty::getFieldMask(p, prop) << ")"
                << "[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else if (isEqual(cname, "ByteProperty")) {
            sdk << "\tbyte " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else if (isEqual(cname, "IntProperty")) {
            sdk << "\tint " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else if (isEqual(cname, "Int8Property")) {
            sdk << "\tint8 " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else if (isEqual(cname, "Int16Property")) {
            sdk << "\tint16 " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else if (isEqual(cname, "Int64Property")) {
            sdk << "\tint64 " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else if (isEqual(cname, "UInt16Property")) {
            sdk << "\tuint16 " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else if (isEqual(cname, "UInt32Property")) {
            sdk << "\tuint32 " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else if (isEqual(cname, "UInt64Property")) {
            sdk << "\tuint64 " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else if (isEqual(cname, "DoubleProperty")) {
            sdk << "\tdouble " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else if (isEqual(cname, "FloatProperty")) {
            sdk << "\tfloat " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else if (isEqual(cname, "EnumProperty")) {
            sdk << "\tenum " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else if (isEqual(cname, "StrProperty")) {
            sdk << "\tFString " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else if (isEqual(cname, "TextProperty")) {
            sdk << "\tFText " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else if (isEqual(cname, "NameProperty")) {
            sdk << "\tFName " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else if (isEqual(cname, "DelegateProperty") ||
                   isEqual(cname, "MulticastDelegateProperty") ||
                   isEqual(cname, "MulticastInlineDelegateProperty") ||
                   isEqual(cname, "MulticastSparseDelegateProperty")) {
            sdk << "\tdelegate " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else if (isEqual(cname, "XigPtrProperty")) {
            sdk << "\tXigPtrProperty " << oname << ";"
                << "//[Offset: 0x" << setbase(16) << UProperty::getOffset(p, prop) << ", "
                << "Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        } else {
            sdk << "\t" << cname << " " << oname << ";"
                << "//[Size: 0x" << setbase(16) << UProperty::getElementSize(p, prop) << "]" << endl;
        }

        next:
        if (isFun) {
            child = UField::getNext(p, child);
        } else {
            child = FField::getNext(p, child);
        }
    }
    return recurrce;
}

void writeStruct(hak::process &proc, uint64 NamePoolData, uint64 libbase, std::ofstream &sdk, kaddr clazz) {
    std::list<kaddr> recurrce;

    kaddr currStruct = clazz;
    while (UObject::isValid(proc, currStruct)) {
        std::string name = UObject::getName(proc, NamePoolData, currStruct);
        if (isEqual(name, "None") || isContain(name, "/Game/") || isContain(name, "_png") ||
            name.empty()) {
            break;
        }

        uint32 NameID = UObject::getNameID(proc, currStruct);
        if (!isScanned(NameID)) {
            //Dumping
            structIDMap.push_back(NameID);
            sdk << "Class: " << UStruct::getStructClassPath(proc, NamePoolData, currStruct)
            << " [pointer: 0x" << std::setbase(16) << currStruct << std::setbase(10) << "]" << std::endl;

            auto c = UStruct::getChildren(proc, currStruct);
            if (c > 0x1000) {
                recurrce.merge(writeStructChild(proc, NamePoolData, libbase, sdk, c));
            } else {
                //std::cout << "Child: " << std::setbase(16) << c << std::endl;
            }

            auto PropertyLink = proc.read_pointer2(currStruct + UStructToPropertyLink);
            if (PropertyLink) {
                std::cout << "PropertyLink: " << std::setbase(16) << PropertyLink << std::endl;
                recurrce.merge(writeStructChild(proc, NamePoolData, libbase, sdk, PropertyLink, false));
            }

            sdk << "\n--------------------------------" << std::endl;
            classCount++;
        }

        currStruct = UStruct::getSuperClass(proc, currStruct);
    }

    for (unsigned long long & it : recurrce)
        writeStruct(proc, NamePoolData, libbase, sdk, it);
}

void DumpSDKW(hak::process &proc, uint64 uworld, uint64 libbase, uint64 NamePoolData, std::string out) {
    std::ofstream sdk(out + "/SDK.txt", std::ofstream::out);
    if (sdk.is_open()) {
        std::cout << "Dumping SDK List" << std::endl;
        clock_t begin = clock();
        kaddr gworld = uworld;
        std::cout << "UWorld: " << std::setbase(16) << gworld << std::setbase(10) << " | Name: "
             << UObject::getName(proc, NamePoolData, gworld) << std::endl;
        if (UObject::isValid(proc, gworld)) {
            //Iterate World
            writeStruct(proc, NamePoolData, libbase, sdk, UObject::getClass(proc, gworld));
            //Iterate Entities
            kaddr level = proc.read<kaddr>(gworld + UWorldToPersistentLevel);
            kaddr actorList = proc.read<kaddr>(level + ULevelToAActors);

            int actorsCount = proc.read<int>(level + ULevelToAActorsCount);
            for (int i = 0; i < actorsCount; i++) {
                kaddr actor = proc.read<kaddr>(actorList + (i * 8));
                //std::cout << "Actor: " << std::setbase(16) << actor << std::endl;
                if (UObject::isValid(proc, actor)) {
                    writeStruct(proc, NamePoolData, libbase, sdk, UObject::getClass(proc, actor));
                }
            }
        }
        sdk.close();
        clock_t end = clock();
        double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
        std::cout << classCount << " Items Dumped in SDK in " << elapsed_secs << "S" <<std::endl;
    }
}

void LoopObjects(hak::process &proc, uint64 uworld, uint64 libbase, uint64 NamePoolData, uintptr_t addr) {
    for (int i = 0; i < 0x4000; ++i) {
        try {
            kaddr obj = proc.read<kaddr>(addr + (i * 4));
            std::string name = UObject::getName(proc, NamePoolData, obj);
            if (name == "None" || name == "NULL" || name.empty()
            || !(isalpha(name[0]) || isdigit(name[0]) || name[0] == '/')
            || name == "e252") {
                continue;
            }
            //kaddr classPrivate = UObject::getClass(proc, obj);
            //kaddr outerObject = UObject::getOuter(proc, classPrivate);
            std::cout << "Object: " << std::setbase(16) << obj << std::setbase(10) << " | Name: " << name << std::hex << " | Offset: 0x" << (i * 4) << std::endl;
        } catch (std::exception& e) {
            //std::cout << "Error: " << e.what() << std::endl;
        }
    }
}
