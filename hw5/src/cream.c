#include "cream.h"
#include "utils.h"
#include "queue.h"
#include "hashmap.h"
#include <stdlib.h>
#include <stdio.h>
#define NUM_THREADS 100

//#define MAP_KEY(kbase, klen) (map_key_t) {.key_base = kbase, .key_len = klen}
//#define MAP_VAL(vbase, vlen) (map_val_t) {.val_base = vbase, .val_len = vlen}

hashmap_t *global_map;

uint32_t someHash(map_key_t map_key) {
    const uint8_t *key = map_key.key_base;
    size_t length = map_key.key_len;
    size_t i = 0;
    uint32_t hash = 0;

    while (i != length) {
        hash += key[i++];
        hash += hash << 10;
        hash ^= hash >> 6;
    }

    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

void freeMe(map_key_t key, map_val_t val) {
    free(key.key_base);
    free(val.val_base);
}

int main(int argc, char *argv[]) {
    global_map = create_map(10, someHash, freeMe);
    int x = 5;
    put(global_map, MAP_KEY(&x, sizeof(int)), MAP_VAL(&x, sizeof(int)), false);
    int y = 6;
    put(global_map, MAP_KEY(&y, sizeof(int)), MAP_VAL(&y, sizeof(int)), false);
    map_val_t val = get(global_map, MAP_KEY(&x, sizeof(int)));
    printf("%d, %lu\n", *(int*)val.val_base, val.val_len);
    exit(0);
}
