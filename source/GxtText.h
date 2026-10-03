#pragma once
#include "CText.h"
#include <cstring>
#include <cstdio>

// True if the string is pure ASCII (likely untranslated English in a CN pack).
inline bool GxtLooksAscii(const char* s) {
    if (!s) return true;
    for (const char* p = s; *p; ++p) {
        if (static_cast<unsigned char>(*p) >= 0x80)
            return false;
    }
    return true;
}

// Returns GXT text if the key resolves to a non-ASCII string,
// otherwise the fallback (keeps Chinese packs from showing leftover English).
inline const char* GxtOr(const char* key, const char* fallback) {
    if (!fallback)
        return "";
    if (!key || !key[0])
        return fallback;
    const char* result = TheText.Get(key);
    if (result && result[0] && strcmp(result, key) != 0 && !GxtLooksAscii(result))
        return result;
    return fallback;
}

// Like GxtOr, but accepts ASCII GXT results (for keys that are meant to stay ASCII).
inline const char* GxtOrRaw(const char* key, const char* fallback) {
    if (!key || !key[0] || !fallback)
        return fallback;
    const char* result = TheText.Get(key);
    if (result && result[0] && strcmp(result, key) != 0)
        return result;
    return fallback;
}

inline void GxtCopy(char* dest, size_t destSize, const char* key, const char* fallback) {
    if (!dest || destSize == 0)
        return;
    const char* src = GxtOr(key, fallback);
    strncpy(dest, src, destSize - 1);
    dest[destSize - 1] = '\0';
}

// SA stat GXT keys are STAT000..STATxxx
inline void FormatStatKey(char* buf, size_t bufSize, int sid) {
    if (sid < 10)
        sprintf(buf, "STAT00%d", sid);
    else if (sid < 100)
        sprintf(buf, "STAT0%d", sid);
    else
        sprintf(buf, "STAT%d", sid);
}

// Copies STAT### name; falls back if the key is missing.
inline void GetStatName(char* dest, size_t destSize, int sid, const char* fallback) {
    char key[16];
    FormatStatKey(key, sizeof(key), sid);
    GxtCopy(dest, destSize, key, fallback);
}
