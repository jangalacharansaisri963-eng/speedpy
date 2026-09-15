#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define SPEEDPY_REGISTRY_MAX_ENTRIES 1024
#define SPEEDPY_REGISTRY_NAME_SIZE 128
#define SPEEDPY_REGISTRY_TAG_SIZE 64

typedef enum {
    SPEEDPY_REGISTRY_OBJECT = 0,
    SPEEDPY_REGISTRY_MODULE = 1,
    SPEEDPY_REGISTRY_SERVICE = 2,
    SPEEDPY_REGISTRY_PIPELINE = 3,
    SPEEDPY_REGISTRY_TASK = 4,
    SPEEDPY_REGISTRY_RESOURCE = 5,
    SPEEDPY_REGISTRY_CUSTOM = 6
} SpeedPyRegistryType;

typedef struct {
    char name[SPEEDPY_REGISTRY_NAME_SIZE];
    char tag[SPEEDPY_REGISTRY_TAG_SIZE];

    uint64_t id;
    SpeedPyRegistryType type;

    void *object;

    bool active;
    bool owned;

    uint64_t references;
    uint64_t generation;
} SpeedPyRegistryEntry;

typedef struct {
    SpeedPyRegistryEntry entries[SPEEDPY_REGISTRY_MAX_ENTRIES];

    size_t count;
    uint64_t next_id;
    uint64_t generation;

    bool locked;
} SpeedPyRegistry;

static SpeedPyRegistryEntry *speedpy_registry_find(
    SpeedPyRegistry *registry,
    const char *name
)
{
    size_t i;

    if (registry == NULL || name == NULL) {
        return NULL;
    }

    for (i = 0; i < registry->count; i++) {
        if (registry->entries[i].active &&
            strcmp(registry->entries[i].name, name) == 0) {
            return &registry->entries[i];
        }
    }

    return NULL;
}

static const SpeedPyRegistryEntry *speedpy_registry_find_const(
    const SpeedPyRegistry *registry,
    const char *name
)
{
    size_t i;

    if (registry == NULL || name == NULL) {
        return NULL;
    }

    for (i = 0; i < registry->count; i++) {
        if (registry->entries[i].active &&
            strcmp(registry->entries[i].name, name) == 0) {
            return &registry->entries[i];
        }
    }

    return NULL;
}

static SpeedPyRegistryEntry *speedpy_registry_find_id(
    SpeedPyRegistry *registry,
    uint64_t id
)
{
    size_t i;

    if (registry == NULL || id == 0) {
        return NULL;
    }

    for (i = 0; i < registry->count; i++) {
        if (registry->entries[i].active &&
            registry->entries[i].id == id) {
            return &registry->entries[i];
        }
    }

    return NULL;
}

SpeedPyRegistry *speedpy_registry_create(void)
{
    SpeedPyRegistry *registry;

    registry = (SpeedPyRegistry *)calloc(
        1,
        sizeof(SpeedPyRegistry)
    );

    if (registry == NULL) {
        return NULL;
    }

    registry->count = 0;
    registry->next_id = 1;
    registry->generation = 1;
    registry->locked = false;

    return registry;
}

void speedpy_registry_destroy(
    SpeedPyRegistry *registry
)
{
    if (registry == NULL) {
        return;
    }

    free(registry);
}

bool speedpy_registry_register(
    SpeedPyRegistry *registry,
    const char *name,
    SpeedPyRegistryType type,
    void *object,
    bool owned
)
{
    SpeedPyRegistryEntry *entry;
    size_t length;

    if (registry == NULL ||
        name == NULL ||
        registry->locked) {
        return false;
    }

    length = strlen(name);

    if (length == 0 ||
        length >= SPEEDPY_REGISTRY_NAME_SIZE) {
        return false;
    }

    if (speedpy_registry_find(registry, name) != NULL) {
        return false;
    }

    if (registry->count >= SPEEDPY_REGISTRY_MAX_ENTRIES) {
        return false;
    }

    entry = &registry->entries[registry->count];

    memset(entry, 0, sizeof(SpeedPyRegistryEntry));

    memcpy(entry->name, name, length);
    entry->name[length] = '\0';

    entry->id = registry->next_id++;
    entry->type = type;
    entry->object = object;
    entry->active = true;
    entry->owned = owned;
    entry->references = 0;
    entry->generation = registry->generation;

    registry->count++;
    registry->generation++;

    return true;
}

bool speedpy_registry_unregister(
    SpeedPyRegistry *registry,
    const char *name
)
{
    SpeedPyRegistryEntry *entry;
    size_t index;
    size_t i;

    if (registry == NULL ||
        name == NULL ||
        registry->locked) {
        return false;
    }

    entry = speedpy_registry_find(registry, name);

    if (entry == NULL) {
        return false;
    }

    if (entry->references != 0) {
        return false;
    }

    index = (size_t)(entry - registry->entries);

    for (i = index; i + 1 < registry->count; i++) {
        registry->entries[i] = registry->entries[i + 1];
    }

    memset(
        &registry->entries[registry->count - 1],
        0,
        sizeof(SpeedPyRegistryEntry)
    );

    registry->count--;
    registry->generation++;

    return true;
}

bool speedpy_registry_exists(
    const SpeedPyRegistry *registry,
    const char *name
)
{
    return speedpy_registry_find_const(
        registry,
        name
    ) != NULL;
}

bool speedpy_registry_exists_id(
    const SpeedPyRegistry *registry,
    uint64_t id
)
{
    size_t i;

    if (registry == NULL || id == 0) {
        return false;
    }

    for (i = 0; i < registry->count; i++) {
        if (registry->entries[i].active &&
            registry->entries[i].id == id) {
            return true;
        }
    }

    return false;
}

void *speedpy_registry_get(
    SpeedPyRegistry *registry,
    const char *name
)
{
    SpeedPyRegistryEntry *entry;

    entry = speedpy_registry_find(registry, name);

    if (entry == NULL) {
        return NULL;
    }

    return entry->object;
}

void *speedpy_registry_get_id(
    SpeedPyRegistry *registry,
    uint64_t id
)
{
    SpeedPyRegistryEntry *entry;

    entry = speedpy_registry_find_id(registry, id);

    if (entry == NULL) {
        return NULL;
    }

    return entry->object;
}

SpeedPyRegistryEntry *speedpy_registry_get_entry(
    SpeedPyRegistry *registry,
    const char *name
)
{
    return speedpy_registry_find(registry, name);
}

const SpeedPyRegistryEntry *speedpy_registry_get_entry_const(
    const SpeedPyRegistry *registry,
    const char *name
)
{
    return speedpy_registry_find_const(registry, name);
}

bool speedpy_registry_set_tag(
    SpeedPyRegistry *registry,
    const char *name,
    const char *tag
)
{
    SpeedPyRegistryEntry *entry;
    size_t length;

    if (registry == NULL ||
        name == NULL ||
        tag == NULL ||
        registry->locked) {
        return false;
    }

    entry = speedpy_registry_find(registry, name);

    if (entry == NULL) {
        return false;
    }

    length = strlen(tag);

    if (length >= SPEEDPY_REGISTRY_TAG_SIZE) {
        return false;
    }

    memcpy(entry->tag, tag, length);
    entry->tag[length] = '\0';

    entry->generation = ++registry->generation;

    return true;
}

const char *speedpy_registry_get_tag(
    const SpeedPyRegistry *registry,
    const char *name
)
{
    const SpeedPyRegistryEntry *entry;

    entry = speedpy_registry_find_const(
        registry,
        name
    );

    if (entry == NULL) {
        return NULL;
    }

    return entry->tag;
}

bool speedpy_registry_acquire(
    SpeedPyRegistry *registry,
    const char *name
)
{
    SpeedPyRegistryEntry *entry;

    if (registry == NULL || name == NULL) {
        return false;
    }

    entry = speedpy_registry_find(registry, name);

    if (entry == NULL) {
        return false;
    }

    entry->references++;

    return true;
}

bool speedpy_registry_release(
    SpeedPyRegistry *registry,
    const char *name
)
{
    SpeedPyRegistryEntry *entry;

    if (registry == NULL || name == NULL) {
        return false;
    }

    entry = speedpy_registry_find(registry, name);

    if (entry == NULL || entry->references == 0) {
        return false;
    }

    entry->references--;

    return true;
}

uint64_t speedpy_registry_get_id(
    const SpeedPyRegistry *registry,
    const char *name
)
{
    const SpeedPyRegistryEntry *entry;

    entry = speedpy_registry_find_const(
        registry,
        name
    );

    if (entry == NULL) {
        return 0;
    }

    return entry->id;
}

SpeedPyRegistryType speedpy_registry_get_type(
    const SpeedPyRegistry *registry,
    const char *name
)
{
    const SpeedPyRegistryEntry *entry;

    entry = speedpy_registry_find_const(
        registry,
        name
    );

    if (entry == NULL) {
        return SPEEDPY_REGISTRY_CUSTOM;
    }

    return entry->type;
}

size_t speedpy_registry_count(
    const SpeedPyRegistry *registry
)
{
    if (registry == NULL) {
        return 0;
    }

    return registry->count;
}

uint64_t speedpy_registry_generation(
    const SpeedPyRegistry *registry
)
{
    if (registry == NULL) {
        return 0;
    }

    return registry->generation;
}

bool speedpy_registry_lock(
    SpeedPyRegistry *registry
)
{
    if (registry == NULL) {
        return false;
    }

    registry->locked = true;

    return true;
}

bool speedpy_registry_unlock(
    SpeedPyRegistry *registry
)
{
    if (registry == NULL) {
        return false;
    }

    registry->locked = false;

    return true;
}

bool speedpy_registry_is_locked(
    const SpeedPyRegistry *registry
)
{
    if (registry == NULL) {
        return false;
    }

    return registry->locked;
}

void speedpy_registry_clear(
    SpeedPyRegistry *registry
)
{
    if (registry == NULL || registry->locked) {
        return;
    }

    memset(
        registry->entries,
        0,
        sizeof(registry->entries)
    );

    registry->count = 0;
    registry->generation++;
}