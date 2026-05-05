#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_double_xor.h"
#include "hash_utils.h"

static void int_to_str(int num, char* buf, size_t buf_size) {
    _snprintf_s(buf, buf_size, _TRUNCATE, "%d", num);
}

static unsigned int hash2(const char* key, int capacity) {
    unsigned int h = djb2(key, (unsigned int)strlen(key));
    return 1 + (h % (capacity - 1));
}

void double_xor_init(HashTableDoubleXOR* t, int capacity, float load_factor) {
    t->capacity = capacity;
    t->size = 0;
    t->load_factor = load_factor;
    t->collision_count = 0;
    t->resize_count = 0;
    t->entries = (EntryDoubleXOR*)calloc(capacity, sizeof(EntryDoubleXOR));
}

static void double_xor_resize(HashTableDoubleXOR* t) {
    int old_cap = t->capacity;
    EntryDoubleXOR* old = t->entries;
    int new_cap = old_cap * 2;
    t->entries = (EntryDoubleXOR*)calloc(new_cap, sizeof(EntryDoubleXOR));
    t->capacity = new_cap;
    t->size = 0;

    int saved_col = t->collision_count;
    int saved_res = t->resize_count;

    for (int i = 0; i < old_cap; i++) {
        if (old[i].state == OCCUPIED) {
            char buf[32];
            int_to_str(old[i].value, buf, sizeof(buf));
            double_xor_put(t, old[i].key, buf);
            free(old[i].key);
        }
        else if (old[i].state == DELETED) {
            free(old[i].key);
        }
    }
    free(old);
    t->collision_count = saved_col;
    t->resize_count = saved_res + 1;
}

int double_xor_put(HashTableDoubleXOR* t, const char* key, const char* val_str) {
    unsigned int hash1 = xor_hash(key, (unsigned int)strlen(key));
    unsigned int step = hash2(key, t->capacity);
    int idx = hash1 % t->capacity;
    int first_probe = 1;
    int found_empty = -1;
    int found_deleted = -1;

    for (int i = 0; i < t->capacity; i++) {
        int cur = (idx + i * step) % t->capacity;
        int st = t->entries[cur].state;

        if (st == EMPTY) {
            if (found_empty < 0) found_empty = cur;
            break;
        }
        else if (st == DELETED) {
            if (found_deleted < 0) found_deleted = cur;
        }
        else {
            if (strcmp(t->entries[cur].key, key) == 0) {
                t->entries[cur].value = atoi(val_str);
                return 1;
            }
        }

        if (first_probe) {
            t->collision_count++;
            first_probe = 0;
        }
    }

    int target = (found_deleted >= 0) ? found_deleted : found_empty;
    if (target < 0) return 0;

    if (t->entries[target].state == DELETED)
        free(t->entries[target].key);
    t->entries[target].key = my_strdup(key);
    t->entries[target].value = atoi(val_str);
    t->entries[target].state = OCCUPIED;
    t->size++;

    if ((float)t->size / t->capacity >= t->load_factor)
        double_xor_resize(t);
    return 1;
}

const char* double_xor_get(HashTableDoubleXOR* t, const char* key) {
    unsigned int hash1 = xor_hash(key, (unsigned int)strlen(key));
    unsigned int step = hash2(key, t->capacity);
    int idx = hash1 % t->capacity;

    for (int i = 0; i < t->capacity; i++) {
        int cur = (idx + i * step) % t->capacity;
        if (t->entries[cur].state == EMPTY)
            return NULL;
        if (t->entries[cur].state == OCCUPIED && strcmp(t->entries[cur].key, key) == 0) {
            static char buf[32];
            int_to_str(t->entries[cur].value, buf, sizeof(buf));
            return buf;
        }
    }
    return NULL;
}

const char* double_xor_del(HashTableDoubleXOR* t, const char* key) {
    unsigned int hash1 = xor_hash(key, (unsigned int)strlen(key));
    unsigned int step = hash2(key, t->capacity);
    int idx = hash1 % t->capacity;

    for (int i = 0; i < t->capacity; i++) {
        int cur = (idx + i * step) % t->capacity;
        if (t->entries[cur].state == EMPTY)
            return NULL;
        if (t->entries[cur].state == OCCUPIED && strcmp(t->entries[cur].key, key) == 0) {
            static char buf[32];
            int_to_str(t->entries[cur].value, buf, sizeof(buf));
            free(t->entries[cur].key);
            t->entries[cur].key = NULL;
            t->entries[cur].value = 0;
            t->entries[cur].state = DELETED;
            t->size--;
            return buf;
        }
    }
    return NULL;
}

void double_xor_list(const HashTableDoubleXOR* t, int num) {
    printf("=== Hash Table (double hash, XOR + djb2) ===\n");
    printf("capacity = %d, size = %d, load = %.3f\n",
        t->capacity, t->size, (float)t->size / t->capacity);
    if (num > t->capacity) num = t->capacity;
    for (int i = 0; i < num; i++) {
        printf("[%d]: ", i);
        switch (t->entries[i].state) {
        case EMPTY:   printf("EMPTY\n"); break;
        case DELETED: printf("DELETED\n"); break;
        case OCCUPIED:
            printf("\"%s\" -> %d\n", t->entries[i].key, t->entries[i].value);
            break;
        }
    }
    printf("Collisions: %d, Resizes: %d\n", t->collision_count, t->resize_count);
}

void double_xor_destroy(HashTableDoubleXOR* t) {
    for (int i = 0; i < t->capacity; i++) {
        if (t->entries[i].state == OCCUPIED || t->entries[i].state == DELETED)
            free(t->entries[i].key);
    }
    free(t->entries);
    t->entries = NULL;
    t->capacity = t->size = 0;
}