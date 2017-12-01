#include "utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"

#define MAP_KEY(base, len) (map_key_t) {.key_base = base, .key_len = len}
#define MAP_VAL(base, len) (map_val_t) {.val_base = base, .val_len = len}
#define MAP_NODE(key_arg, val_arg, tombstone_arg) (map_node_t) {.key = key_arg, .val = val_arg, .tombstone = tombstone_arg}

hashmap_t *create_map(uint32_t capacity, hash_func_f hash_function, destructor_f destroy_function) {
    if (hash_function == NULL || destroy_function == NULL || capacity < 0) {
        errno = EINVAL;
        return NULL;
    }
    hashmap_t *newHM = (hashmap_t*)calloc(1, sizeof(hashmap_t));
    if (newHM == NULL) return NULL;
    if (pthread_mutex_init(&(newHM->write_lock), NULL) != 0) return NULL;
    if (pthread_mutex_init(&(newHM->fields_lock), NULL) != 0) return NULL;
    newHM->capacity = capacity;
    newHM->size = 0;
    newHM->hash_function = hash_function;
    newHM->destroy_function = destroy_function;
    newHM->num_readers = 0;
    newHM->invalid = false;
    map_node_t *map_node = (map_node_t*)calloc(capacity, sizeof(map_node_t));
    newHM->nodes = map_node;
    return newHM;
}

bool put(hashmap_t *self, map_key_t key, map_val_t val, bool force) {
    pthread_mutex_lock(&self->write_lock);
    if (self == NULL || self->invalid == true || key.key_base == NULL || key.key_len == 0 || val.val_base == NULL || val.val_len == 0) {
        errno = EINVAL;
        return false;
    }
    uint32_t hashTo = 0;
    hashTo = get_index(self, key); //get hash index
    debug("hashTo = %d", hashTo);
    //if key is already in map, update value
    if (self->nodes[hashTo].key.key_len == key.key_len && memcmp(self->nodes[hashTo].key.key_base, key.key_base, key.key_len) == 0) {

        self->nodes[hashTo].val.val_base = val.val_base;
        self->nodes[hashTo].val.val_len = val.val_len;
    }
    // if index is empty insert key/val
    else if (self->nodes[hashTo].key.key_base == NULL || self->nodes[hashTo].key.key_len == 0) {
        self->nodes[hashTo].key.key_base = key.key_base;
        self->nodes[hashTo].key.key_len = key.key_len;

        self->nodes[hashTo].val.val_base = val.val_base;
        self->nodes[hashTo].val.val_len = val.val_len;

        self->size++;
        //debug("emptyCount = %d", emptyCount++);
    } else {
        int index = hashTo;
        bool found = false;
        //loop through map from hashTo
        while (index != self->capacity) {
            //if index is empty insert key/val
            if (self->nodes[index].key.key_base == NULL) {
                self->nodes[index].key.key_base = key.key_base;
                self->nodes[index].key.key_len = key.key_len;

                self->nodes[index].val.val_base = val.val_base;
                self->nodes[index].val.val_len = val.val_len;
                found = true;
                self->size++;
                break;
            }
            //if not increment index
            else index++;
        }
        //if have not found empty slot from starting index, start from beginning of map
        if (found == false) {
            index = 0;
            while (index != hashTo) {
                if (self->nodes[index].key.key_base == NULL) {
                    self->nodes[index].key.key_base = key.key_base;
                    self->nodes[index].key.key_len = key.key_len;

                    self->nodes[index].val.val_base = val.val_base;
                    self->nodes[index].val.val_len = val.val_len;
                    found = true;
                    self->size++;
                    break;
                } else index++;
            }
        } //if map is full and force is true overwrite index
        if (found == false && force == true) {
            self->destroy_function(self->nodes[hashTo].key, self->nodes[hashTo].val);

            self->nodes[hashTo].key.key_base = key.key_base;
            self->nodes[hashTo].key.key_len = key.key_len;

            self->nodes[hashTo].val.val_base = val.val_base;
            self->nodes[hashTo].val.val_len = val.val_len;
        }
        if (found == false && force == false) {
            errno = ENOMEM;
            pthread_mutex_unlock(&(self->write_lock));
            return false;
        }
    }
    pthread_mutex_unlock(&(self->write_lock));
    return true;
}

map_val_t get(hashmap_t *self, map_key_t key) {
    pthread_mutex_lock(&(self->fields_lock));
    if (self == NULL || self->invalid == true || key.key_base == NULL || key.key_len == 0) {
        errno = EINVAL;
        map_val_t errorVal = {NULL, 0};
        pthread_mutex_unlock(&(self->fields_lock));
        return errorVal;
    }
    map_val_t returnMapVal;
    self->num_readers++;
    if (self->num_readers == 1)
        pthread_mutex_lock(&(self->write_lock));
    pthread_mutex_unlock(&(self->fields_lock));

    uint32_t hashTo = get_index(self, key);
    if (self->nodes[hashTo].key.key_len == key.key_len && memcmp(self->nodes[hashTo].key.key_base, key.key_base, key.key_len) == 0) {
        returnMapVal = self->nodes[hashTo].val;
    } else {
        int index = hashTo + 1;
        bool found = false;
        while (index != self->capacity) {
            if (self->nodes[index].key.key_len == key.key_len && memcmp(self->nodes[index].key.key_base, key.key_base, key.key_len) == 0) {
                returnMapVal = self->nodes[index].val;
                found = true;
                break;
            } else index++;
        }
        if (found == false) {
            index = 0;
            while (index != hashTo) {
                if (self->nodes[index].key.key_len == key.key_len && memcmp(self->nodes[index].key.key_base, key.key_base, key.key_len) == 0) {
                    returnMapVal = self->nodes[index].val;
                    found = true;
                    break;
                } else index++;
            }
            if (found == false) {
                returnMapVal.val_base = NULL;
                returnMapVal.val_len = 0;
            }
        }
    }

    pthread_mutex_lock(&(self->fields_lock));
    self->num_readers--;
    if (self->num_readers == 0)
        pthread_mutex_unlock(&(self->write_lock));
    pthread_mutex_unlock(&(self->fields_lock));
    return returnMapVal;
    //return MAP_VAL(NULL, 0);
}

map_node_t delete(hashmap_t *self, map_key_t key) {
    pthread_mutex_lock(&self->write_lock);
    map_node_t returnMapNode;
    if (self == NULL || self->invalid == true || key.key_base == NULL || key.key_len == 0) {
        errno = EINVAL;
        pthread_mutex_unlock(&self->write_lock);
        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }
    uint32_t hashTo = get_index(self, key);
    if (self->nodes[hashTo].key.key_len == key.key_len && memcmp(self->nodes[hashTo].key.key_base, key.key_base, key.key_len) == 0) {
        returnMapNode = self->nodes[hashTo];

        self->nodes[hashTo].key.key_base = NULL;
        self->nodes[hashTo].key.key_len = 0;

        self->nodes[hashTo].val.val_base = NULL;
        self->nodes[hashTo].val.val_len = 0;

        self->nodes[hashTo].tombstone = true;
        self->size--;
    } else {
        int index = hashTo + 1;
        bool found = false;
        while (index != self->capacity) {
            if (self->nodes[index].key.key_len == key.key_len && memcmp(self->nodes[index].key.key_base, key.key_base, key.key_len) == 0) {
                returnMapNode = self->nodes[index];

                self->nodes[index].key.key_base = NULL;
                self->nodes[index].key.key_len = 0;

                self->nodes[index].val.val_base = NULL;
                self->nodes[index].val.val_len = 0;

                self->nodes[index].tombstone = true;

                found = true;
                self->size--;
                break;
            } else index++;
        }
        if (found == false) {
            index = 0;
            while (index != hashTo) {
                if (self->nodes[index].key.key_len == key.key_len && memcmp(self->nodes[index].key.key_base, key.key_base, key.key_len) == 0) {
                    returnMapNode = self->nodes[index];

                    self->nodes[index].key.key_base = NULL;
                    self->nodes[index].key.key_len = 0;

                    self->nodes[index].val.val_base = NULL;
                    self->nodes[index].val.val_len = 0;

                    self->nodes[index].tombstone = true;

                    found = true;
                    self->size--;
                    break;
                } else index++;
            }
        }
    }

    pthread_mutex_unlock(&self->write_lock);
    return returnMapNode;
}

bool clear_map(hashmap_t *self) {
    pthread_mutex_lock(&self->write_lock);
    if (self == NULL || self->invalid == true) {
        errno = EINVAL;
        pthread_mutex_unlock(&self->write_lock);
        return false;
    }
    for (int i = 0; i < self->capacity; i++) {
        if (self->nodes[i].key.key_len != 0 || self->nodes[i].val.val_len != 0) {
            self->destroy_function(self->nodes[i].key, self->nodes[i].val);
            self->nodes[i].key.key_len = 0;
            self->nodes[i].val.val_len = 0;
        }
    }
    pthread_mutex_unlock(&self->write_lock);
	return true;
}

bool invalidate_map(hashmap_t *self) {
    pthread_mutex_lock(&self->write_lock);
    if (self == NULL || self->invalid == true) {
        errno = EINVAL;
        pthread_mutex_unlock(&self->write_lock);
        return false;
    }
    for (int i = 0; i < self->capacity; i++) {
        if (self->nodes[i].key.key_len != 0 || self->nodes[i].val.val_len != 0) {
            self->destroy_function(self->nodes[i].key, self->nodes[i].val);
            self->nodes[i].key.key_len = 0;
            self->nodes[i].val.val_len = 0;
        }
    }
    free(self->nodes);
    self->invalid = true;
    pthread_mutex_unlock(&self->write_lock);
    return true;
}
