#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_quad_fnv.h"
#include "hash_utils.h"

void quad_fnv_init(HashTableQuadFNV* t, int capacity, float load_factor) {
    t->capacity = capacity;
    t->size = 0;
    t->load_factor = load_factor;
    t->collision_count = 0;
    t->resize_count = 0;
    t->entries = (EntryQuadFNV*)calloc(capacity, sizeof(EntryQuadFNV));
}

static void quad_fnv_resize(HashTableQuadFNV* t) {
    int old_cap = t->capacity;
    EntryQuadFNV* old = t->entries;
    int new_cap = old_cap * 2;
    t->entries = (EntryQuadFNV*)calloc(new_cap, sizeof(EntryQuadFNV));
    t->capacity = new_cap;
    t->size = 0;

    int saved_col = t->collision_count;
    int saved_res = t->resize_count;

    for (int i = 0; i < old_cap; i++) {
        if (old[i].state == OCCUPIED) {
            quad_fnv_put(t, old[i].key, old[i].value);
            free(old[i].key);
            free(old[i].value);
        }
        else if (old[i].state == DELETED) {
            free(old[i].key);
            free(old[i].value);
        }
    }
    free(old);
    t->collision_count = saved_col;
    t->resize_count = saved_res + 1;
}

int quad_fnv_put(HashTableQuadFNV* t, const char* key, const char* val) {
    unsigned int hash = FNV1a(key, (unsigned int)strlen(key));
    int idx = hash % t->capacity;
    int first_probe = 1;
    int found_empty = -1;
    int found_deleted = -1;

    for (int i = 0; i < t->capacity; i++) {
        int cur = (idx + i * i) % t->capacity;
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
                free(t->entries[cur].value);
                t->entries[cur].value = my_strdup(val);
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

    if (t->entries[target].state == DELETED) {
        free(t->entries[target].key);
        free(t->entries[target].value);
    }
    t->entries[target].key = my_strdup(key);
    t->entries[target].value = my_strdup(val);
    t->entries[target].state = OCCUPIED;
    t->size++;

    if ((float)t->size / t->capacity >= t->load_factor)
        quad_fnv_resize(t);
    return 1;
}

const char* quad_fnv_get(HashTableQuadFNV* t, const char* key) {
    unsigned int hash = FNV1a(key, (unsigned int)strlen(key));
    int idx = hash % t->capacity;

    for (int i = 0; i < t->capacity; i++) {
        int cur = (idx + i * i) % t->capacity;
        if (t->entries[cur].state == EMPTY)
            return NULL;
        if (t->entries[cur].state == OCCUPIED && strcmp(t->entries[cur].key, key) == 0)
            return t->entries[cur].value;
    }
    return NULL;
}

const char* quad_fnv_del(HashTableQuadFNV* t, const char* key) {
    unsigned int hash = FNV1a(key, (unsigned int)strlen(key));
    int idx = hash % t->capacity;

    for (int i = 0; i < t->capacity; i++) {
        int cur = (idx + i * i) % t->capacity;
        if (t->entries[cur].state == EMPTY)
            return NULL;
        if (t->entries[cur].state == OCCUPIED && strcmp(t->entries[cur].key, key) == 0) {
            static char buf[256];
            strncpy(buf, t->entries[cur].value, 255);
            buf[255] = '\0';
            free(t->entries[cur].key);
            free(t->entries[cur].value);
            t->entries[cur].key = NULL;
            t->entries[cur].value = NULL;
            t->entries[cur].state = DELETED;
            t->size--;
            return buf;
        }
    }
    return NULL;
}

void quad_fnv_list(const HashTableQuadFNV* t, int num) {
    printf("=== Hash Table (quadratic probe, FNV1a) ===\n");
    printf("capacity = %d, size = %d, load = %.3f\n",
        t->capacity, t->size, (float)t->size / t->capacity);
    if (num > t->capacity) num = t->capacity;
    for (int i = 0; i < num; i++) {
        printf("[%d]: ", i);
        switch (t->entries[i].state) {
        case EMPTY:   printf("EMPTY\n"); break;
        case DELETED: printf("DELETED\n"); break;
        case OCCUPIED:
            printf("\"%s\" -> \"%s\"\n", t->entries[i].key, t->entries[i].value);
            break;
        }
    }
    printf("Collisions: %d, Resizes: %d\n", t->collision_count, t->resize_count);
}

void quad_fnv_destroy(HashTableQuadFNV* t) {
    for (int i = 0; i < t->capacity; i++) {
        if (t->entries[i].state == OCCUPIED || t->entries[i].state == DELETED) {
            free(t->entries[i].key);
            free(t->entries[i].value);
        }
    }
    free(t->entries);
    t->entries = NULL;
    t->capacity = t->size = 0;
}