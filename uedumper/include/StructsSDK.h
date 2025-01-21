#ifndef HAK_STRUCTSSDK_H
#define HAK_STRUCTSSDK_H

#include "ida.h"
#include "uedumper.h"
#include "process.h"
#include "iostream"

using kaddr = uint64;

struct UObject {
    static int32 getIndex(hak::process& proc, kaddr object) {
        return proc.read<int32>(object + UObjectToInternalIndex);
    }

    static kaddr getClass(hak::process& proc, kaddr object) {//UClass*
        return proc.read<kaddr>(object + UObjectToClassPrivate);
    }

    static uint32 getNameID(hak::process& proc, kaddr object) {
        return proc.read<uint32>(object + UObjectToFNameIndex);
    }

    static kaddr getOuter(hak::process& proc, kaddr object) {//UObject*
        return proc.read<kaddr>(object + UObjectToOuterPrivate);
    }

    static std::string getName(hak::process& proc, uint64 NamePoolData, kaddr object) {
        auto id = getNameID(proc, object);
        //std::cout << "ID: " << id << "|";
        return GetFromFName(proc, NamePoolData, id);
    }

    static std::string getClassName(hak::process& proc, uint64 NamePoolData, kaddr object) {
        return getName(proc, NamePoolData, getClass(proc, object));
    }

    static bool isValid(hak::process& proc, kaddr object) {
        return (object > 0 && getNameID(proc, object) > 0 && getClass(proc, object) > 0);
    }
};

struct UStruct {
    static kaddr getSuperClass(hak::process& proc, kaddr structz) {//UStruct*
        return proc.read<kaddr>(structz + UStructToSuperStruct);
    }

    static kaddr getChildren(hak::process& proc, kaddr structz) {//UField*
        //std::cout << "Struct: " << std::setbase(16) << structz + UStructToChildren << std::endl;
        return proc.read<kaddr>(structz + UStructToChildren);
    }

    static kaddr getChildProperties(hak::process& proc, kaddr structz) {//UField*
        return proc.read<kaddr>(structz + UStructToChildProperties);
    }

    static std::string getClassName(hak::process& proc, uint64 NamePoolData, kaddr clazz) {
        return UObject::getName(proc, NamePoolData, clazz);
    }

    static std::string getClassPath(hak::process& proc, uint64 NAmePoolData, kaddr object) {
        kaddr clazz = UObject::getClass(proc, object);
        std::string classname = UObject::getName(proc, NAmePoolData, clazz);

        kaddr superclass = getSuperClass(proc, clazz);
        while (superclass) {
            classname += ".";
            classname += UObject::getName(proc, NAmePoolData, superclass);

            superclass = getSuperClass(proc, superclass);
        }

        return classname;
    }

    static std::string getStructClassPath(hak::process& proc, uint64 NamePoolData, kaddr clazz) {
        std::string classname = UObject::getName(proc, NamePoolData, clazz);

        kaddr superclass = getSuperClass(proc, clazz);
        while (superclass) {
            classname += ".";
            classname += UObject::getName(proc, NamePoolData, superclass);

            superclass = getSuperClass(proc, superclass);
        }

        return classname;
    }
};

struct FField {
    static std::string getName(hak::process& proc, uint64 namePoolData, kaddr fField) {
        return GetFromFName(proc, namePoolData, proc.read<uint32>(fField + FFieldToName));
    }

    static std::string getClassName(hak::process& proc, uint64 namePoolData, kaddr fField) {
        return GetFromFName(proc, namePoolData, proc.read<uint32>(
                proc.read<kaddr>(fField + FFieldToClass)
                        ));
    }

    static kaddr getNext(hak::process& proc, kaddr fField) {//UField*
        return proc.read<kaddr>(fField + FFieldToNext);
    }
};

struct UObjectProperty {
    static kaddr getPropertyClass(hak::process& proc, kaddr prop) {//class UClass*
        return proc.read<kaddr>(prop + UObjectPropertyToPropertyClass);
    }
};

struct UProperty {
    static int32 getElementSize(hak::process& proc, kaddr prop) {
        return proc.read<int32>(prop + UPropertyToElementSize);
    }

    static uint64 getPropertyFlags(hak::process& proc, kaddr prop) {
        return proc.read<uint64>(prop + UPropertyToPropertyFlags);
    }

    static int32 getOffset(hak::process& proc, kaddr prop) {
        return proc.read<int32>(prop + UPropertyToOffsetInternal);
    }
};

struct UClassProperty {
    static kaddr getMetaClass(hak::process& proc, kaddr prop) {//class UClass*
        return proc.read<kaddr>(prop + UClassPropertyToMetaClass);
    }
};

struct UInterfaceProperty {
    static kaddr getInterfaceClass(hak::process& proc, kaddr prop) {//class UClass*
        return proc.read<kaddr>(prop + UInterfacePropertyToInterfaceClass);
    }
};

struct UArrayProperty {
    static kaddr getInner(hak::process& proc, kaddr prop) {//UProperty*
        return proc.read<kaddr>(prop + UArrayPropertyToInnerProperty);
    }
};

struct UMapProperty {
    static kaddr getKeyProp(hak::process& proc, kaddr prop) {//UProperty*
        return proc.read<kaddr>(prop + UMapPropertyToKeyProp);
    }

    static kaddr getValueProp(hak::process& proc, kaddr prop) {//UProperty*
        return proc.read<kaddr>(prop + UMapPropertyToValueProp);
    }
};

struct USetProperty {
    static kaddr getElementProp(hak::process& proc, kaddr prop) {//UProperty*
        return proc.read<kaddr>(prop + USetPropertyToElementProp);
    }
};

struct UStructProperty {
    static kaddr getStruct(hak::process& proc, kaddr prop) {//UStruct*
        return proc.read<kaddr>(prop + UStructPropertyToStruct);
    }
};

struct UBoolProperty {
    static uint8 getFieldSize(hak::process& proc, kaddr prop) {
        return proc.read<uint8>(prop + UBoolPropertyToFieldSize);
    }

    static uint8 getByteOffset(hak::process& proc, kaddr prop) {
        return proc.read<uint8>(prop + UBoolPropertyToByteOffset);
    }

    static uint8 getByteMask(hak::process& proc, kaddr prop) {
        return proc.read<uint8>(prop + UBoolPropertyToByteMask);
    }

    static uint8 getFieldMask(hak::process& proc, kaddr prop) {
        return proc.read<uint8>(prop + UBoolPropertyToFieldMask);
    }
};

struct UFunction {
    static int32 getFunctionFlags(hak::process& proc, kaddr func) {
        return proc.read<int32>(func + UFunctionToFunctionFlags);
    }

    static kaddr getFunc(hak::process& proc, kaddr func) {
        return proc.read<kaddr>(func + UFunctionToFunc);
    }
};

struct UField {
    static kaddr getNext(hak::process& proc, kaddr field) {//UField*
        return proc.read<kaddr>(field + UFieldToNext);
    }
};

#endif //HAK_STRUCTSSDK_H
