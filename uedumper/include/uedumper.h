#ifndef HAK_UEDUMPER_H
#define HAK_UEDUMPER_H

#include "string"
#include "process.h"
#include "proc.h"
#include "ida.h"

const int UObjectToInternalIndex = 0xC;
const int UObjectToOuterPrivate = 0x18;
const int UObjectToClassPrivate = 0x10;
const int UObjectToFNameIndex = 0x24;
const int UFunctionToFunctionFlags = 0xc8;

const int UStructToSuperStruct = 0x48;
const int UStructToChildren = 0X58; // 0x58
const int UFieldToNext = 0x30; // 0x30
const int UStructToPropertyLink = 0x78;//0x70
const int FFieldToNext = 0x58;
const int FFieldToName = 0x28;
const int FFieldToClass = 0x20;
const int UObjectPropertyToPropertyClass = 0x80;
const int UPropertyToOffsetInternal = 0x4c;
const int UClassPropertyToMetaClass = 0x80;
const int UInterfacePropertyToInterfaceClass = 0x80;
//Class: UStructProperty
const int UStructPropertyToStruct = 0x80;
//Class: UArrayProperty
const int UArrayPropertyToInnerProperty = 0x80;
//Class: USetProperty
const int USetPropertyToElementProp = 0x80;
const int UMapPropertyToKeyProp = 0x80;
const int UMapPropertyToValueProp = 0x88;

const int UStructToChildProperties = 0x58;
const int UFunctionToFunc = 0xB0;
const int UPropertyToElementSize = 0x34;
const int UPropertyToPropertyFlags = 0x38;
//Class: UWorld
const int UWorldToPersistentLevel = 0x108;
//Class: ULevel
const int ULevelToAActors = 512;
const int ULevelToAActorsCount = 520;
const int UBoolPropertyToFieldSize = 0x80;
const int UBoolPropertyToByteOffset = 0x81;
const int UBoolPropertyToByteMask = 0x82;
const int UBoolPropertyToFieldMask = 0x83;


void FNameDecrypt(char* inputBuf, int namelength, bool isWide);

void FNameEncrypt(char* inputBuf, int len, char* dest);

auto GetFromFName(hak::process &proc, uint64 NamePoolData, uint32_t actor_id) -> std::string;

void DumpStrings(hak::process &proc, uint64 NamePoolData, std::string out);

void LoopObjects(hak::process &proc, uint64 uworld, uint64 libbase, uint64 NamePoolData, uintptr_t addr);

void DumpSDKW(hak::process &proc, uint64 uworld, uint64 libbase, uint64 NamePoolData, std::string out);

#endif //HAK_UEDUMPER_H
