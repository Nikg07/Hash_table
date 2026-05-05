#define _CRT_SECURE_NO_WARNINGS
#include "hash_utils.h"
#include <stdlib.h>
#include <string.h>

unsigned int SDBMHash(const char* str, unsigned int length) {
    unsigned int hash = 0;
    for (unsigned int i = 0; i < length; str++, i++)
        hash = (*str) + (hash << 6) + (hash << 16) - hash;
    return hash;
}

unsigned int djb2(const char* str, unsigned int length) {
    unsigned int hash = 5381;
    for (unsigned int i = 0; i < length; str++, i++)
        hash = ((hash << 5) + hash) + (*str);
    return hash;
}

unsigned int adler32(const char* str, unsigned int length) {
    unsigned int s1 = 1;
    unsigned int s2 = 0;
    for (unsigned int i = 0; i < length; str++, i++) {
        s1 = (s1 + (*str)) % 65521;
        s2 = (s2 + s1) % 65521;
    }
    return (s2 << 16) | s1;
}

unsigned int FNV1a(const char* str, unsigned int length) {
    unsigned int hash = 2166136261u;
    for (unsigned int i = 0; i < length; str++, i++) {
        hash ^= (unsigned char)(*str);
        hash *= 16777619u;
    }
    return hash;
}

unsigned int xor_hash(const char* str, unsigned int length) {
    unsigned int hash = 0;
    for (unsigned int i = 0; i < length; str++, i++) {
        hash ^= (*str);
        hash = (hash << 7) | (hash >> 25);
    }
    return hash;
}

char* my_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char* copy = (char*)malloc(len);
    if (copy) memcpy(copy, s, len);
    return copy;
}