#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_sequential_djb2.h"
#include "hash_utils.h"

static void int_to_str(int num, char* buf, size_t buf_size) {
    _snprintf_s(buf, buf_size, _TRUNCATE, "%d", num);
}

void seq_djb2_init(HashTableSeqDjb2* t, int capacity, float load_factor) {
    t->capacity = capacity;
    t->size = 0;
    t->load_factor = load_factor;
    t->collision_count = 0;
    t->resize_count = 0;
    t->entries = (EntrySeqDjb2*)calloc(capacity, sizeof(EntrySeqDjb2));
}

static void seq_djb2_resize(HashTableSeqDjb2* t) {
    int old_cap = t->capacity;
    EntrySeqDjb2* old = t->entries;
    int new_cap = old_cap * 2;
    t->entries = (EntrySeqDjb2*)calloc(new_cap, sizeof(EntrySeqDjb2));
    t->capacity = new_cap;
    t->size = 0;

    int saved_col = t->collision_count;
    int saved_res = t->resize_count;

    for (int i = 0; i < old_cap; i++) {
        if (old[i].state == OCCUPIED) {
            char buf[32];
            int_to_str(old[i].value, buf, sizeof(buf));
            seq_djb2_put(t, old[i].key, buf);
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

int seq_djb2_put(HashTableSeqDjb2* t, const char* key, const char* val_str) {
    unsigned int hash = djb2(key, (unsigned int)strlen(key));
    int idx = hash % t->capacity;
    int first_probe = 1;
    int found_empty = -1;
    int found_deleted = -1;

    for (int i = 0; i < t->capacity; i++) {
        int cur = (idx + i) % t->capacity;
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
        seq_djb2_resize(t);
    return 1;
}

const char* seq_djb2_get(HashTableSeqDjb2* t, const char* key) {
    unsigned int hash = djb2(key, (unsigned int)strlen(key));
    int idx = hash % t->capacity;

    for (int i = 0; i < t->capacity; i++) {
        int cur = (idx + i) % t->capacity;
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

const char* seq_djb2_del(HashTableSeqDjb2* t, const char* key) {
    unsigned int hash = djb2(key, (unsigned int)strlen(key));
    int idx = hash % t->capacity;

    for (int i = 0; i < t->capacity; i++) {
        int cur = (idx + i) % t->capacity;
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

void seq_djb2_list(const HashTableSeqDjb2* t, int num) {
    printf("=== Hash Table (sequential search, djb2) ===\n");
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

void seq_djb2_destroy(HashTableSeqDjb2* t) {
    for (int i = 0; i < t->capacity; i++) {
        if (t->entries[i].state == OCCUPIED || t->entries[i].state == DELETED)
            free(t->entries[i].key);
    }
    free(t->entries);
    t->entries = NULL;
    t->capacity = t->size = 0;
}