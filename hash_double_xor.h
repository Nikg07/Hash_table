#ifndef HASH_DOUBLE_XOR_H
#define HASH_DOUBLE_XOR_H

#define EMPTY 0
#define OCCUPIED 1
#define DELETED 2

typedef struct {
    char* key;
    int value;
    int state;
} EntryDoubleXOR;

typedef struct {
    EntryDoubleXOR* entries;
    int capacity;
    int size;
    float load_factor;
    int collision_count;
    int resize_count;
} HashTableDoubleXOR;

void double_xor_init(HashTableDoubleXOR* t, int capacity, float load_factor);
int  double_xor_put(HashTableDoubleXOR* t, const char* key, const char* val_str);
const char* double_xor_get(HashTableDoubleXOR* t, const char* key);
const char* double_xor_del(HashTableDoubleXOR* t, const char* key);
void double_xor_list(const HashTableDoubleXOR* t, int num);
void double_xor_destroy(HashTableDoubleXOR* t);

#endif
