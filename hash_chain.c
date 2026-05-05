#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_chain.h"
#include "hash_utils.h"

void chain_init(HashTableChain* t, int capacity, float load_factor) {
    t->capacity = capacity;
    t->size = 0;
    t->load_factor = load_factor;
    t->collision_count = 0;
    t->resize_count = 0;
    t->buckets = (ChainNode**)calloc(capacity, sizeof(ChainNode*));
}

static void chain_resize(HashTableChain* t) {
    int old_cap = t->capacity;
    int new_cap = old_cap * 2;
    ChainNode** new_buckets = (ChainNode**)calloc(new_cap, sizeof(ChainNode*));

    for (int i = 0; i < old_cap; i++) {
        ChainNode* node = t->buckets[i];
        while (node) {
            ChainNode* next = node->next;
            unsigned int hash = SDBMHash(node->key, strlen(node->key));
            int idx = hash % new_cap;
            node->next = new_buckets[idx];
            new_buckets[idx] = node;
            node = next;
        }
    }

    free(t->buckets);
    t->buckets = new_buckets;
    t->capacity = new_cap;
    t->resize_count++;
}

int chain_put(HashTableChain* t, const char* key, const char* val) {
    unsigned int hash = SDBMHash(key, strlen(key));
    int idx = hash % t->capacity;

    ChainNode* cur = t->buckets[idx];
    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            free(cur->value);
            cur->value = my_strdup(val);
            return 1;
        }
        cur = cur->next;
    }

    ChainNode* node = (ChainNode*)malloc(sizeof(ChainNode));
    node->key = my_strdup(key);
    node->value = my_strdup(val);
    node->next = t->buckets[idx];
    t->buckets[idx] = node;
    t->size++;

    if (node->next != NULL)
        t->collision_count++;

    if ((float)t->size / t->capacity >= t->load_factor)
        chain_resize(t);

    return 1;
}

const char* chain_get(HashTableChain* t, const char* key) {
    unsigned int hash = SDBMHash(key, strlen(key));
    int idx = hash % t->capacity;
    ChainNode* cur = t->buckets[idx];
    while (cur) {
        if (strcmp(cur->key, key) == 0)
            return cur->value;
        cur = cur->next;
    }
    return NULL;
}

const char* chain_del(HashTableChain* t, const char* key) {
    unsigned int hash = SDBMHash(key, strlen(key));
    int idx = hash % t->capacity;
    ChainNode** pp = &(t->buckets[idx]);
    while (*pp) {
        ChainNode* node = *pp;
        if (strcmp(node->key, key) == 0) {
            static char buf[256];
            strncpy(buf, node->value, 255);
            buf[255] = '\0';
            *pp = node->next;
            free(node->key);
            free(node->value);
            free(node);
            t->size--;
            return buf;
        }
        pp = &((*pp)->next);
    }
    return NULL;
}

void chain_list(const HashTableChain* t, int num) {
    printf("=== Hash Table (chain, SDBMHash) ===\n");
    printf("capacity = %d, size = %d, load = %.3f\n",
        t->capacity, t->size, (float)t->size / t->capacity);
    if (num > t->capacity) num = t->capacity;
    for (int i = 0; i < num; i++) {
        printf("[%d]: ", i);
        if (t->buckets[i] == NULL) {
            printf("EMPTY\n");
        }
        else {
            ChainNode* cur = t->buckets[i];
            while (cur) {
                printf("[%s: %s]", cur->key, cur->value);
                cur = cur->next;
                if (cur) printf(" -> ");
            }
            printf("\n");
        }
    }
    printf("Collisions: %d, Resizes: %d\n", t->collision_count, t->resize_count);
}

void chain_destroy(HashTableChain* t) {
    for (int i = 0; i < t->capacity; i++) {
        ChainNode* cur = t->buckets[i];
        while (cur) {
            ChainNode* next = cur->next;
            free(cur->key);
            free(cur->value);
            free(cur);
            cur = next;
        }
    }
    free(t->buckets);
    t->buckets = NULL;
    t->capacity = t->size = 0;
}