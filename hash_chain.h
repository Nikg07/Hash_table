#ifndef HASH_CHAIN_H
#define HASH_CHAIN_H

typedef struct ChainNode {
    char* key;
    char* value;
    struct ChainNode* next;
} ChainNode;

typedef struct {
    ChainNode** buckets;
    int capacity;
    int size;
    float load_factor;
    int collision_count;
    int resize_count;
} HashTableChain;

void chain_init(HashTableChain* t, int capacity, float load_factor);
int  chain_put(HashTableChain* t, const char* key, const char* val);
const char* chain_get(HashTableChain* t, const char* key);
const char* chain_del(HashTableChain* t, const char* key);
void chain_list(const HashTableChain* t, int num);
void chain_destroy(HashTableChain* t);

#endif