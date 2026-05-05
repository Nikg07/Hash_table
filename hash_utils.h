#ifndef HASH_UTILS_H
#define HASH_UTILS_H

unsigned int SDBMHash(const char* str, unsigned int length);
unsigned int djb2(const char* str, unsigned int length);
unsigned int adler32(const char* str, unsigned int length);
unsigned int FNV1a(const char* str, unsigned int length);
unsigned int xor_hash(const char* str, unsigned int length);
char* my_strdup(const char* s);

#endif