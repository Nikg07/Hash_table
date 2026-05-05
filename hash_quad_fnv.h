#ifndef HASH_QUAD_FNV_H
#define HASH_QUAD_FNV_H

#define EMPTY 0
#define OCCUPIED 1
#define DELETED 2

typedef struct {
    char* key;
    char* value;
    int state;
} EntryQuadFNV;

typedef struct {
    EntryQuadFNV* entries;
    int capacity;
    int size;
    float load_factor;
    int collision_count;
    int resize_count;
} HashTableQuadFNV;

void quad_fnv_init(HashTableQuadFNV* t, int capacity, float load_factor);
int  quad_fnv_put(HashTableQuadFNV* t, const char* key, const char* val);
const char* quad_fnv_get(HashTableQuadFNV* t, const char* key);
const char* quad_fnv_del(HashTableQuadFNV* t, const char* key);
void quad_fnv_list(const HashTableQuadFNV* t, int num);
void quad_fnv_destroy(HashTableQuadFNV* t);

#endif
