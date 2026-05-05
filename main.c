#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hash_chain.h"
#include "hash_sequential_djb2.h"
#include "hash_linear_adler.h"
#include "hash_quad_fnv.h"
#include "hash_double_xor.h"

/* ---------- вспомогательные функции интерфейса ---------- */
void print_commands(int mode) {
    printf("Available commands:\n");
        printf("  put <key> <value>\n");
        printf("  get <key>\n");
        printf("  del <key>\n");
        printf("  list <num>\n");
        printf("  exit\n");
    }

void print_chain_stats(const HashTableChain* t) {
    printf("[Mode 1] chain SDBMHash string/string | capacity=%d size=%d load=%.3f collisions=%d resizes=%d\n",
        t->capacity, t->size, (float)t->size / t->capacity, t->collision_count, t->resize_count);
}
void print_seq_stats(const HashTableSeqDjb2* t) {
    printf("[Mode 2] sequential djb2 string/int | capacity=%d size=%d load=%.3f collisions=%d resizes=%d\n",
        t->capacity, t->size, (float)t->size / t->capacity, t->collision_count, t->resize_count);
}
void print_adler_stats(const HashTableLinearAdler* t) {
    printf("[Mode 3] linear adler32 string/int | capacity=%d size=%d load=%.3f collisions=%d resizes=%d\n",
        t->capacity, t->size, (float)t->size / t->capacity, t->collision_count, t->resize_count);
}
void print_quad_stats(const HashTableQuadFNV* t) {
    printf("[Mode 4] quadratic FNV1a string/string | capacity=%d size=%d load=%.3f collisions=%d resizes=%d\n",
        t->capacity, t->size, (float)t->size / t->capacity, t->collision_count, t->resize_count);
}
void print_double_stats(const HashTableDoubleXOR* t) {
    printf("[Mode 5] double XOR string/int | capacity=%d size=%d load=%.3f collisions=%d resizes=%d\n",
        t->capacity, t->size, (float)t->size / t->capacity, t->collision_count, t->resize_count);
}

float compute_auto_load_factor() {
    MEMORYSTATUSEX ms = { sizeof(ms) };
    GlobalMemoryStatusEx(&ms);
    long long total_mb = ms.ullTotalPhys / (1024 * 1024);

    DWORD cpu_mhz = 0;
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        TEXT("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"),
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD size = sizeof(cpu_mhz);
        RegQueryValueEx(hKey, TEXT("~MHz"), NULL, NULL, (LPBYTE)&cpu_mhz, &size);
        RegCloseKey(hKey);
    }

    float lf = 0.75f;
    if (total_mb > 8192) lf -= 0.15f;
    else if (total_mb < 512) lf += 0.15f;
    if (cpu_mhz > 3000) lf -= 0.05f;
    else if (cpu_mhz < 1000) lf += 0.05f;
    if (lf < 0.30f) lf = 0.30f;
    if (lf > 0.95f) lf = 0.95f;

    printf("=== Auto load_factor mode ===\n");
    printf("RAM total: %lld MB\n", total_mb);
    printf("CPU freq: %lu MHz\n", cpu_mhz);
    printf("Computed load_factor: %.3f\n", lf);
    return lf;
}

/* ---------- генерация случайной строки ---------- */
void random_string(char* buf, int len) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < len; i++) {
        buf[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    buf[len] = '\0';
}

double get_time_us() {
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart * 1000000.0 / freq.QuadPart;
}

/* ---------- эксперимент для одного load_factor ---------- */
typedef struct {
    double insert_time_us;      // общее время вставки всех N элементов (мкс)
    double avg_get_time_us;     // среднее время одного get (мкс)
    int    collisions;
    int    resizes;
    int    final_capacity;
} ExpResult;

ExpResult run_experiment(int mode, float lf, int N) {
    ExpResult res = { 0 };
    srand(12345);

    // Генерируем ключи и значения заранее 
    char** keys = (char**)malloc(N * sizeof(char*));
    char** values = (char**)malloc(N * sizeof(char*));
    for (int i = 0; i < N; i++) {
        keys[i] = (char*)malloc(16);
        values[i] = (char*)malloc(16);
        random_string(keys[i], 10);
        random_string(values[i], 10);
    }

    double t_start, t_end;

    // ---------- ЗАМЕР ВСТАВКИ ----------
    if (mode == 1) {
        HashTableChain ht;
        chain_init(&ht, 8, lf);
        t_start = get_time_us();
        for (int i = 0; i < N; i++) chain_put(&ht, keys[i], values[i]);
        t_end = get_time_us();
        res.collisions = ht.collision_count;
        res.resizes = ht.resize_count;
        res.final_capacity = ht.capacity;
        chain_destroy(&ht);
    }
    else if (mode == 2) {
        HashTableSeqDjb2 ht;
        seq_djb2_init(&ht, 16, lf);
        t_start = get_time_us();
        for (int i = 0; i < N; i++) seq_djb2_put(&ht, keys[i], values[i]);
        t_end = get_time_us();
        res.collisions = ht.collision_count;
        res.resizes = ht.resize_count;
        res.final_capacity = ht.capacity;
        seq_djb2_destroy(&ht);
    }
    else if (mode == 3) {
        HashTableLinearAdler ht;
        linear_adler_init(&ht, 32, lf);
        t_start = get_time_us();
        for (int i = 0; i < N; i++) linear_adler_put(&ht, keys[i], values[i]);
        t_end = get_time_us();
        res.collisions = ht.collision_count;
        res.resizes = ht.resize_count;
        res.final_capacity = ht.capacity;
        linear_adler_destroy(&ht);
    }
    else if (mode == 4) {
        HashTableQuadFNV ht;
        quad_fnv_init(&ht, 16, lf);
        t_start = get_time_us();
        for (int i = 0; i < N; i++) quad_fnv_put(&ht, keys[i], values[i]);
        t_end = get_time_us();
        res.collisions = ht.collision_count;
        res.resizes = ht.resize_count;
        res.final_capacity = ht.capacity;
        quad_fnv_destroy(&ht);
    }
    else if (mode == 5) {
        HashTableDoubleXOR ht;
        double_xor_init(&ht, 16, lf);
        t_start = get_time_us();
        for (int i = 0; i < N; i++) double_xor_put(&ht, keys[i], values[i]);
        t_end = get_time_us();
        res.collisions = ht.collision_count;
        res.resizes = ht.resize_count;
        res.final_capacity = ht.capacity;
        double_xor_destroy(&ht);
    }
    res.insert_time_us = t_end - t_start;

    // ---------- ЗАМЕР ПОИСКА ----------
    int M = N < 10000 ? N : 10000;  // количество проб для поиска
    if (mode == 1) {
        HashTableChain ht;
        chain_init(&ht, 8, lf);
        for (int i = 0; i < N; i++) chain_put(&ht, keys[i], values[i]);
        t_start = get_time_us();
        for (int i = 0; i < M; i++) chain_get(&ht, keys[i]);
        t_end = get_time_us();
        chain_destroy(&ht);
    }
    else if (mode == 2) {
        HashTableSeqDjb2 ht;
        seq_djb2_init(&ht, 16, lf);
        for (int i = 0; i < N; i++) seq_djb2_put(&ht, keys[i], values[i]);
        t_start = get_time_us();
        for (int i = 0; i < M; i++) seq_djb2_get(&ht, keys[i]);
        t_end = get_time_us();
        seq_djb2_destroy(&ht);
    }
    else if (mode == 3) {
        HashTableLinearAdler ht;
        linear_adler_init(&ht, 32, lf);
        for (int i = 0; i < N; i++) linear_adler_put(&ht, keys[i], values[i]);
        t_start = get_time_us();
        for (int i = 0; i < M; i++) linear_adler_get(&ht, keys[i]);
        t_end = get_time_us();
        linear_adler_destroy(&ht);
    }
    else if (mode == 4) {
        HashTableQuadFNV ht;
        quad_fnv_init(&ht, 16, lf);
        for (int i = 0; i < N; i++) quad_fnv_put(&ht, keys[i], values[i]);
        t_start = get_time_us();
        for (int i = 0; i < M; i++) quad_fnv_get(&ht, keys[i]);
        t_end = get_time_us();
        quad_fnv_destroy(&ht);
    }
    else if (mode == 5) {
        HashTableDoubleXOR ht;
        double_xor_init(&ht, 16, lf);
        for (int i = 0; i < N; i++) double_xor_put(&ht, keys[i], values[i]);
        t_start = get_time_us();
        for (int i = 0; i < M; i++) double_xor_get(&ht, keys[i]);
        t_end = get_time_us();
        double_xor_destroy(&ht);
    }

    res.avg_get_time_us = (t_end - t_start) / M;

    // Очистка ключей
    for (int i = 0; i < N; i++) {
        free(keys[i]);
        free(values[i]);
    }
    free(keys);
    free(values);

    return res;
}

/* ---------- основное меню ---------- */
int main() {
    SetConsoleCP(65001);
    char input[256];
    int mode;
    float lf;

    while (1) {
        printf("\n====== Main Menu ======\n");
        printf("1 - chain (SDBM)\n");
        printf("2 - sequential djb2\n");
        printf("3 - linear adler32\n");
        printf("4 - quad FNV1a\n");
        printf("5 - double XOR\n");
        printf("E - Experiment \n");
        printf("0 - Exit program\n");
        printf("Select: ");

        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = '\0';

        if (input[0] == '0') break;

        if (input[0] == 'E' || input[0] == 'e') {
            printf("Select mode for experiment (1-5): ");
            if (!fgets(input, sizeof(input), stdin)) break;
            mode = atoi(input);
            if (mode < 1 || mode > 5) {
                printf("Invalid mode.\n");
                continue;
            }
            int N = 100; // количество вставляемых элементов
            printf("Experiment for mode %d, N=%d\n", mode, N);
            printf("load_factor | insert(us) | avg_get(us) | collisions | resizes | final_capacity\n");
            printf("------------+------------+-------------+------------+---------+----------------\n");
            for (float test_lf = 0.2f; test_lf <= 0.9f; test_lf += 0.1f) {
                ExpResult r = run_experiment(mode, test_lf, N);
                printf("%11.2f | %10.1f | %11.3f | %10d | %7d | %14d\n",
                    test_lf, r.insert_time_us, r.avg_get_time_us,
                    r.collisions, r.resizes, r.final_capacity);
            }
            printf("Experiment finished.\n");
            continue;
        }

        mode = atoi(input);
        if (mode < 1 || mode > 5) {
            printf("Invalid mode. Try again.\n");
            continue;
        }

        printf("Enter load_factor (0.1 .. 0.95) or 'auto': ");
        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = '\0';
        if (strcmp(input, "auto") == 0)
            lf = compute_auto_load_factor();
        else {
            lf = (float)atof(input);
            if (lf < 0.1f) lf = 0.1f;
            if (lf > 0.95f) lf = 0.95f;
        }

        HashTableChain ht_chain;
        HashTableSeqDjb2 ht_seq;
        HashTableLinearAdler ht_adler;
        HashTableQuadFNV ht_quad;
        HashTableDoubleXOR ht_double;

        switch (mode) {
        case 1: chain_init(&ht_chain, 8, lf); break;
        case 2: seq_djb2_init(&ht_seq, 16, lf); break;
        case 3: linear_adler_init(&ht_adler, 32, lf); break;
        case 4: quad_fnv_init(&ht_quad, 16, lf); break;
        case 5: double_xor_init(&ht_double, 16, lf); break;
        }

        switch (mode) {
        case 1: print_chain_stats(&ht_chain); break;
        case 2: print_seq_stats(&ht_seq); break;
        case 3: print_adler_stats(&ht_adler); break;
        case 4: print_quad_stats(&ht_quad); break;
        case 5: print_double_stats(&ht_double); break;
        }
        print_commands(mode);

        char cmd[256];
        char key[256], val[256];

        while (1) {
            printf("> ");
            if (!fgets(cmd, sizeof(cmd), stdin)) break;
            cmd[strcspn(cmd, "\n")] = '\0';

            if (strncmp(cmd, "put ", 4) == 0) {
                if (sscanf(cmd + 4, "%s %s", key, val) == 2) {
                    int ok = 0;
                    switch (mode) {
                    case 1: ok = chain_put(&ht_chain, key, val); break;
                    case 2: ok = seq_djb2_put(&ht_seq, key, val); break;
                    case 3: ok = linear_adler_put(&ht_adler, key, val); break;
                    case 4: ok = quad_fnv_put(&ht_quad, key, val); break;
                    case 5: ok = double_xor_put(&ht_double, key, val); break;
                    }
                    printf(ok ? "OK\n" : "ERROR\n");
                }
                else printf("Bad put command\n");
            }
            else if (strncmp(cmd, "get ", 4) == 0) {
                if (sscanf(cmd + 4, "%s", key) == 1) {
                    const char* v = NULL;
                    switch (mode) {
                    case 1: v = chain_get(&ht_chain, key); break;
                    case 2: v = seq_djb2_get(&ht_seq, key); break;
                    case 3: v = linear_adler_get(&ht_adler, key); break;
                    case 4: v = quad_fnv_get(&ht_quad, key); break;
                    case 5: v = double_xor_get(&ht_double, key); break;
                    }
                    printf(v ? "%s\n" : "NOT FOUND\n");
                }
            }
            else if (strncmp(cmd, "del ", 4) == 0) {
                if (sscanf(cmd + 4, "%s", key) == 1) {
                    const char* v = NULL;
                    switch (mode) {
                    case 1: v = chain_del(&ht_chain, key); break;
                    case 2: v = seq_djb2_del(&ht_seq, key); break;
                    case 3: v = linear_adler_del(&ht_adler, key); break;
                    case 4: v = quad_fnv_del(&ht_quad, key); break;
                    case 5: v = double_xor_del(&ht_double, key); break;
                    }
                    printf(v ? "%s\n" : "NOT FOUND\n");
                }
            }
            else if (strncmp(cmd, "list ", 5) == 0) {
                int n = atoi(cmd + 5);
                switch (mode) {
                case 1: chain_list(&ht_chain, n); break;
                case 2: seq_djb2_list(&ht_seq, n); break;
                case 3: linear_adler_list(&ht_adler, n); break;
                case 4: quad_fnv_list(&ht_quad, n); break;
                case 5: double_xor_list(&ht_double, n); break;
                }
            }
            else if (strcmp(cmd, "exit") == 0) {
                break;
            }
            else {
                printf("Unknown command\n");
            }

            switch (mode) {
            case 1: print_chain_stats(&ht_chain); break;
            case 2: print_seq_stats(&ht_seq); break;
            case 3: print_adler_stats(&ht_adler); break;
            case 4: print_quad_stats(&ht_quad); break;
            case 5: print_double_stats(&ht_double); break;
            }
            print_commands(mode);
        }

        switch (mode) {
        case 1: chain_destroy(&ht_chain); break;
        case 2: seq_djb2_destroy(&ht_seq); break;
        case 3: linear_adler_destroy(&ht_adler); break;
        case 4: quad_fnv_destroy(&ht_quad); break;
        case 5: double_xor_destroy(&ht_double); break;
        }

        printf("Returning to main menu...\n");
    }

    printf("Program finished.\n");
    system("pause");
    return 0;
}