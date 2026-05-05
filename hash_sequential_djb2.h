#ifndef HASH_SEQUENTIAL_DJB2_H
#define HASH_SEQUENTIAL_DJB2_H

#define EMPTY 0
#define OCCUPIED 1
#define DELETED 2

typedef struct {
    char* key;
    int value;
    int state;
} EntrySeqDjb2;

typedef struct {
    EntrySeqDjb2* entries;
    int capacity;
    int size;
    float load_factor;
    int collision_count;
    int resize_count;
} HashTableSeqDjb2;

void seq_djb2_init(HashTableSeqDjb2* t, int capacity, float load_factor);
int  seq_djb2_put(HashTableSeqDjb2* t, const char* key, const char* val_str);
const char* seq_djb2_get(HashTableSeqDjb2* t, const char* key);
const char* seq_djb2_del(HashTableSeqDjb2* t, const char* key);
void seq_djb2_list(const HashTableSeqDjb2* t, int num);
void seq_djb2_destroy(HashTableSeqDjb2* t);

#endif