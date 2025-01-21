#ifndef HAK_WIDE_STR_H
#define HAK_WIDE_STR_H

#include "ida.h"
#include "string"
#include "process.h"
#include "proc.h"

using namespace std;

uint32 MAX_SIZE = 100;

using kaddr = uint64;
typedef unsigned short UTF16;

struct WideStr {
    static int is_surrogate(UTF16 uc) {
        return (uc - 0xd800u) < 2048u;
    }

    static int is_high_surrogate(UTF16 uc) {
        return (uc & 0xfffffc00) == 0xd800;
    }

    static int is_low_surrogate(UTF16 uc) {
        return (uc & 0xfffffc00) == 0xdc00;
    }

    static wchar_t surrogate_to_utf32(UTF16 high, UTF16 low) {
        return (high << 10) + low - 0x35fdc00;
    }

    static wchar_t *w_str(hak::process& p, kaddr str, size_t len) {
        wchar_t *output = new wchar_t[len + 1];

        UTF16 source[len];
        p.readRaw(str, source, len * sizeof(UTF16));

        for (int i = 0; i < len; i++) {
            const UTF16 uc = source[i];
            if (!is_surrogate(uc)) {
                output[i] = uc;
            } else {
                if (is_high_surrogate(uc) && is_low_surrogate(source[i]))
                    output[i] = surrogate_to_utf32(uc, source[i]);
                else
                    output[i] = L'?';
            }
        }

        output[len] = L'\0';
        return output;
    }

    static string getString(hak::process& p, kaddr StrPtr, int StrLength) {
        wstring str = w_str(p, StrPtr, StrLength);

        string result(MAX_SIZE, '\0');

        wcstombs((char *) result.data(), str.c_str(), MAX_SIZE);

        return result;
    }
};

#endif //HAK_WIDE_STR_H
