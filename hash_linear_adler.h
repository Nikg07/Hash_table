#ifndef HASH_LINEAR_ADLER_H
#define HASH_LINEAR_ADLER_H

#define EMPTY 0
#define OCCUPIED 1
#define DELETED 2

typedef struct {
    char* key;
    int value;
    int state;
} EntryLinearAdler;

typedef struct {
    EntryLinearAdler* entries;
    int capacity;
    int size;
    float load_factor;
    int collision_count;
    int resize_count;
} HashTableLinearAdler;

void linear_adler_init(HashTableLinearAdler* t, int capacity, float load_factor);
int  linear_adler_put(HashTableLinearAdler* t, const char* key, const char* val_str);
const char* linear_adler_get(HashTableLinearAdler* t, const char* key);
const char* linear_adler_del(HashTableLinearAdler* t, const char* key);
void linear_adler_list(const HashTableLinearAdler* t, int num);
void linear_adler_destroy(HashTableLinearAdler* t);

#endif