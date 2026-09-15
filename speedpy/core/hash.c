#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define SPEEDPY_HASH_DEFAULT_CAPACITY 16
#define SPEEDPY_HASH_MAX_CAPACITY 1048576
#define SPEEDPY_HASH_MAX_KEY_LENGTH 4096

typedef struct {
    uint64_t hash;
    void *key;
    size_t key_size;
    void *value;
    bool occupied;
    bool tombstone;
} SpeedPyHashEntry;

typedef struct {
    SpeedPyHashEntry *entries;
    size_t capacity;
    size_t count;
    size_t tombstones;
    uint64_t generation;
    bool locked;
    void *user_data;
} SpeedPyHashMap;

static uint64_t speedpy_hash_bytes(
    const void *data,
    size_t size
) {
    const unsigned char *bytes =
        (const unsigned char *)data;

    uint64_t hash = UINT64_C(14695981039346656037);

    for (size_t i = 0; i < size; i++) {
        hash ^= (uint64_t)bytes[i];
        hash *= UINT64_C(1099511628211);
    }

    return hash;
}

static size_t speedpy_hash_index(
    uint64_t hash,
    size_t capacity
) {
    return (size_t)(hash % capacity);
}

static bool speedpy_hash_valid(
    const SpeedPyHashMap *map
) {
    return map != NULL &&
           map->entries != NULL &&
           map->capacity > 0;
}

static bool speedpy_hash_key_equal(
    const SpeedPyHashEntry *entry,
    const void *key,
    size_t key_size,
    uint64_t hash
) {
    if (!entry->occupied ||
        entry->hash != hash ||
        entry->key_size != key_size) {
        return false;
    }

    return memcmp(
        entry->key,
        key,
        key_size
    ) == 0;
}

static bool speedpy_hash_allocate_entries(
    SpeedPyHashMap *map,
    size_t capacity
) {
    SpeedPyHashEntry *entries;

    if (capacity == 0 ||
        capacity > SPEEDPY_HASH_MAX_CAPACITY) {
        return false;
    }

    entries =
        (SpeedPyHashEntry *)calloc(
            capacity,
            sizeof(SpeedPyHashEntry)
        );

    if (entries == NULL) {
        return false;
    }

    if (map->entries != NULL) {
        for (size_t i = 0; i < map->capacity; i++) {
            SpeedPyHashEntry *old =
                &map->entries[i];

            if (!old->occupied) {
                continue;
            }

            size_t index =
                speedpy_hash_index(
                    old->hash,
                    capacity
                );

            while (entries[index].occupied) {
                index = (index + 1) % capacity;
            }

            entries[index] = *old;
        }

        free(map->entries);
    }

    map->entries = entries;
    map->capacity = capacity;
    map->tombstones = 0;

    return true;
}

SpeedPyHashMap *speedpy_hash_create(
    size_t initial_capacity
) {
    SpeedPyHashMap *map;

    if (initial_capacity == 0) {
        initial_capacity =
            SPEEDPY_HASH_DEFAULT_CAPACITY;
    }

    if (initial_capacity >
        SPEEDPY_HASH_MAX_CAPACITY) {
        return NULL;
    }

    map =
        (SpeedPyHashMap *)calloc(
            1,
            sizeof(SpeedPyHashMap)
        );

    if (map == NULL) {
        return NULL;
    }

    if (!speedpy_hash_allocate_entries(
            map,
            initial_capacity)) {
        free(map);
        return NULL;
    }

    map->generation = 1;

    return map;
}

void speedpy_hash_destroy(
    SpeedPyHashMap *map
) {
    if (map == NULL) {
        return;
    }

    if (map->entries != NULL) {
        for (size_t i = 0; i < map->capacity; i++) {
            if (map->entries[i].occupied) {
                free(map->entries[i].key);
            }
        }
    }

    free(map->entries);
    free(map);
}

static bool speedpy_hash_should_grow(
    const SpeedPyHashMap *map
) {
    if (map->capacity == 0) {
        return true;
    }

    return
        (map->count + map->tombstones) * 100 >=
        map->capacity * 70;
}

static bool speedpy_hash_find_slot(
    const SpeedPyHashMap *map,
    const void *key,
    size_t key_size,
    uint64_t hash,
    size_t *index,
    bool *found
) {
    size_t start;
    size_t first_tombstone = SIZE_MAX;

    if (!speedpy_hash_valid(map) ||
        key == NULL ||
        key_size == 0 ||
        index == NULL ||
        found == NULL) {
        return false;
    }

    start =
        speedpy_hash_index(
            hash,
            map->capacity
        );

    for (size_t i = 0; i < map->capacity; i++) {
        size_t current =
            (start + i) % map->capacity;

        const SpeedPyHashEntry *entry =
            &map->entries[current];

        if (entry->occupied) {
            if (speedpy_hash_key_equal(
                    entry,
                    key,
                    key_size,
                    hash)) {
                *index = current;
                *found = true;
                return true;
            }

            continue;
        }

        if (entry->tombstone) {
            if (first_tombstone == SIZE_MAX) {
                first_tombstone = current;
            }

            continue;
        }

        if (first_tombstone != SIZE_MAX) {
            *index = first_tombstone;
        } else {
            *index = current;
        }

        *found = false;
        return true;
    }

    if (first_tombstone != SIZE_MAX) {
        *index = first_tombstone;
        *found = false;
        return true;
    }

    return false;
}

bool speedpy_hash_set(
    SpeedPyHashMap *map,
    const void *key,
    size_t key_size,
    void *value
) {
    size_t index;
    bool found;
    void *key_copy;
    size_t new_capacity;

    if (!speedpy_hash_valid(map) ||
        map->locked ||
        key == NULL ||
        key_size == 0) {
        return false;
    }

    if (key_size > SPEEDPY_HASH_MAX_KEY_LENGTH) {
        return false;
    }

    if (speedpy_hash_should_grow(map)) {
        if (map->capacity >=
            SPEEDPY_HASH_MAX_CAPACITY) {
            return false;
        }

        new_capacity = map->capacity * 2;

        if (new_capacity >
            SPEEDPY_HASH_MAX_CAPACITY) {
            new_capacity =
                SPEEDPY_HASH_MAX_CAPACITY;
        }

        if (!speedpy_hash_allocate_entries(
                map,
                new_capacity)) {
            return false;
        }
    }

    if (!speedpy_hash_find_slot(
            map,
            key,
            key_size,
            speedpy_hash_bytes(key, key_size),
            &index,
            &found)) {
        return false;
    }

    if (found) {
        map->entries[index].value = value;
        map->generation++;
        return true;
    }

    key_copy = malloc(key_size);

    if (key_copy == NULL) {
        return false;
    }

    memcpy(key_copy, key, key_size);

    if (map->entries[index].tombstone) {
        map->tombstones--;
    }

    map->entries[index].hash =
        speedpy_hash_bytes(key, key_size);

    map->entries[index].key = key_copy;
    map->entries[index].key_size = key_size;
    map->entries[index].value = value;
    map->entries[index].occupied = true;
    map->entries[index].tombstone = false;

    map->count++;
    map->generation++;

    return true;
}

bool speedpy_hash_get(
    const SpeedPyHashMap *map,
    const void *key,
    size_t key_size,
    void **value
) {
    size_t index;
    bool found;
    uint64_t hash;

    if (!speedpy_hash_valid(map) ||
        key == NULL ||
        key_size == 0) {
        return false;
    }

    hash = speedpy_hash_bytes(key, key_size);

    if (!speedpy_hash_find_slot(
            map,
            key,
            key_size,
            hash,
            &index,
            &found)) {
        return false;
    }

    if (!found) {
        return false;
    }

    if (value != NULL) {
        *value = map->entries[index].value;
    }

    return true;
}

bool speedpy_hash_contains(
    const SpeedPyHashMap *map,
    const void *key,
    size_t key_size
) {
    return speedpy_hash_get(
        map,
        key,
        key_size,
        NULL
    );
}

bool speedpy_hash_remove(
    SpeedPyHashMap *map,
    const void *key,
    size_t key_size,
    void **value
) {
    size_t index;
    bool found;
    uint64_t hash;

    if (!speedpy_hash_valid(map) ||
        map->locked ||
        key == NULL ||
        key_size == 0) {
        return false;
    }

    hash = speedpy_hash_bytes(key, key_size);

    if (!speedpy_hash_find_slot(
            map,
            key,
            key_size,
            hash,
            &index,
            &found)) {
        return false;
    }

    if (!found) {
        return false;
    }

    if (value != NULL) {
        *value = map->entries[index].value;
    }

    free(map->entries[index].key);

    map->entries[index].key = NULL;
    map->entries[index].key_size = 0;
    map->entries[index].value = NULL;
    map->entries[index].hash = 0;
    map->entries[index].occupied = false;
    map->entries[index].tombstone = true;

    map->count--;
    map->tombstones++;
    map->generation++;

    return true;
}

bool speedpy_hash_clear(
    SpeedPyHashMap *map
) {
    if (!speedpy_hash_valid(map) ||
        map->locked) {
        return false;
    }

    for (size_t i = 0; i < map->capacity; i++) {
        if (map->entries[i].occupied) {
            free(map->entries[i].key);
        }
    }

    memset(
        map->entries,
        0,
        map->capacity *
        sizeof(SpeedPyHashEntry)
    );

    map->count = 0;
    map->tombstones = 0;
    map->generation++;

    return true;
}

bool speedpy_hash_reserve(
    SpeedPyHashMap *map,
    size_t capacity
) {
    if (!speedpy_hash_valid(map) ||
        map->locked ||
        capacity <= map->capacity) {
        return false;
    }

    if (capacity > SPEEDPY_HASH_MAX_CAPACITY) {
        return false;
    }

    return speedpy_hash_allocate_entries(
        map,
        capacity
    );
}

size_t speedpy_hash_count(
    const SpeedPyHashMap *map
) {
    if (!speedpy_hash_valid(map)) {
        return 0;
    }

    return map->count;
}

size_t speedpy_hash_capacity(
    const SpeedPyHashMap *map
) {
    if (!speedpy_hash_valid(map)) {
        return 0;
    }

    return map->capacity;
}

size_t speedpy_hash_tombstones(
    const SpeedPyHashMap *map
) {
    if (!speedpy_hash_valid(map)) {
        return 0;
    }

    return map->tombstones;
}

uint64_t speedpy_hash_generation(
    const SpeedPyHashMap *map
) {
    if (!speedpy_hash_valid(map)) {
        return 0;
    }

    return map->generation;
}

bool speedpy_hash_lock(
    SpeedPyHashMap *map
) {
    if (!speedpy_hash_valid(map)) {
        return false;
    }

    map->locked = true;
    return true;
}

bool speedpy_hash_unlock(
    SpeedPyHashMap *map
) {
    if (!speedpy_hash_valid(map)) {
        return false;
    }

    map->locked = false;
    return true;
}

bool speedpy_hash_is_locked(
    const SpeedPyHashMap *map
) {
    if (!speedpy_hash_valid(map)) {
        return false;
    }

    return map->locked;
}

void speedpy_hash_set_user_data(
    SpeedPyHashMap *map,
    void *user_data
) {
    if (!speedpy_hash_valid(map)) {
        return;
    }

    map->user_data = user_data;
}

void *speedpy_hash_get_user_data(
    const SpeedPyHashMap *map
) {
    if (!speedpy_hash_valid(map)) {
        return NULL;
    }

    return map->user_data;
}