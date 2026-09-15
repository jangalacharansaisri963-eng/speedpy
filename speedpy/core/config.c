#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define SPEEDPY_CONFIG_NAME_SIZE 128
#define SPEEDPY_CONFIG_PATH_SIZE 256
#define SPEEDPY_CONFIG_MAX_ENTRIES 256

typedef enum {
    SPEEDPY_CONFIG_BOOL = 0,
    SPEEDPY_CONFIG_INT = 1,
    SPEEDPY_CONFIG_UINT = 2,
    SPEEDPY_CONFIG_SIZE = 3,
    SPEEDPY_CONFIG_DOUBLE = 4,
    SPEEDPY_CONFIG_STRING = 5
} SpeedPyConfigType;

typedef struct {
    char key[64];
    SpeedPyConfigType type;

    union {
        bool boolean;
        int64_t integer;
        uint64_t unsigned_integer;
        size_t size;
        double decimal;
        char string[256];
    } value;

    bool modified;
} SpeedPyConfigEntry;

typedef struct {
    char name[SPEEDPY_CONFIG_NAME_SIZE];
    char path[SPEEDPY_CONFIG_PATH_SIZE];

    SpeedPyConfigEntry entries[SPEEDPY_CONFIG_MAX_ENTRIES];

    size_t entry_count;
    uint64_t version;

    bool initialized;
    bool modified;
    bool locked;
} SpeedPyConfig;

static SpeedPyConfigEntry *speedpy_config_find(
    SpeedPyConfig *config,
    const char *key
)
{
    size_t i;

    if (config == NULL || key == NULL) {
        return NULL;
    }

    for (i = 0; i < config->entry_count; i++) {
        if (strcmp(config->entries[i].key, key) == 0) {
            return &config->entries[i];
        }
    }

    return NULL;
}

static const SpeedPyConfigEntry *speedpy_config_find_const(
    const SpeedPyConfig *config,
    const char *key
)
{
    size_t i;

    if (config == NULL || key == NULL) {
        return NULL;
    }

    for (i = 0; i < config->entry_count; i++) {
        if (strcmp(config->entries[i].key, key) == 0) {
            return &config->entries[i];
        }
    }

    return NULL;
}

static bool speedpy_config_valid_key(const char *key)
{
    size_t length;

    if (key == NULL) {
        return false;
    }

    length = strlen(key);

    if (length == 0 || length >= sizeof(((SpeedPyConfigEntry *)0)->key)) {
        return false;
    }

    return true;
}

SpeedPyConfig *speedpy_config_create(void)
{
    SpeedPyConfig *config;

    config = (SpeedPyConfig *)calloc(1, sizeof(SpeedPyConfig));

    if (config == NULL) {
        return NULL;
    }

    config->entry_count = 0;
    config->version = 1;

    config->initialized = true;
    config->modified = false;
    config->locked = false;

    config->name[0] = '\0';
    config->path[0] = '\0';

    return config;
}

void speedpy_config_destroy(SpeedPyConfig *config)
{
    if (config == NULL) {
        return;
    }

    free(config);
}

bool speedpy_config_set_name(
    SpeedPyConfig *config,
    const char *name
)
{
    size_t length;

    if (config == NULL || name == NULL || config->locked) {
        return false;
    }

    length = strlen(name);

    if (length >= sizeof(config->name)) {
        return false;
    }

    memcpy(config->name, name, length);
    config->name[length] = '\0';

    config->modified = true;
    config->version++;

    return true;
}

const char *speedpy_config_get_name(
    const SpeedPyConfig *config
)
{
    if (config == NULL) {
        return NULL;
    }

    return config->name;
}

bool speedpy_config_set_path(
    SpeedPyConfig *config,
    const char *path
)
{
    size_t length;

    if (config == NULL || path == NULL || config->locked) {
        return false;
    }

    length = strlen(path);

    if (length >= sizeof(config->path)) {
        return false;
    }

    memcpy(config->path, path, length);
    config->path[length] = '\0';

    config->modified = true;
    config->version++;

    return true;
}

const char *speedpy_config_get_path(
    const SpeedPyConfig *config
)
{
    if (config == NULL) {
        return NULL;
    }

    return config->path;
}

bool speedpy_config_register(
    SpeedPyConfig *config,
    const char *key,
    SpeedPyConfigType type
)
{
    SpeedPyConfigEntry *entry;

    if (config == NULL ||
        !speedpy_config_valid_key(key) ||
        config->locked) {
        return false;
    }

    if (speedpy_config_find(config, key) != NULL) {
        return false;
    }

    if (config->entry_count >= SPEEDPY_CONFIG_MAX_ENTRIES) {
        return false;
    }

    entry = &config->entries[config->entry_count];

    memset(entry, 0, sizeof(SpeedPyConfigEntry));

    memcpy(entry->key, key, strlen(key));
    entry->key[strlen(key)] = '\0';

    entry->type = type;
    entry->modified = false;

    config->entry_count++;
    config->modified = true;
    config->version++;

    return true;
}

bool speedpy_config_remove(
    SpeedPyConfig *config,
    const char *key
)
{
    size_t index;
    size_t i;

    if (config == NULL || key == NULL || config->locked) {
        return false;
    }

    for (index = 0; index < config->entry_count; index++) {
        if (strcmp(config->entries[index].key, key) == 0) {
            break;
        }
    }

    if (index == config->entry_count) {
        return false;
    }

    for (i = index; i + 1 < config->entry_count; i++) {
        config->entries[i] = config->entries[i + 1];
    }

    memset(
        &config->entries[config->entry_count - 1],
        0,
        sizeof(SpeedPyConfigEntry)
    );

    config->entry_count--;
    config->modified = true;
    config->version++;

    return true;
}

bool speedpy_config_has(
    const SpeedPyConfig *config,
    const char *key
)
{
    return speedpy_config_find_const(config, key) != NULL;
}

SpeedPyConfigType speedpy_config_get_type(
    const SpeedPyConfig *config,
    const char *key
)
{
    const SpeedPyConfigEntry *entry;

    entry = speedpy_config_find_const(config, key);

    if (entry == NULL) {
        return SPEEDPY_CONFIG_STRING;
    }

    return entry->type;
}

bool speedpy_config_set_bool(
    SpeedPyConfig *config,
    const char *key,
    bool value
)
{
    SpeedPyConfigEntry *entry;

    if (config == NULL || config->locked) {
        return false;
    }

    entry = speedpy_config_find(config, key);

    if (entry == NULL || entry->type != SPEEDPY_CONFIG_BOOL) {
        return false;
    }

    entry->value.boolean = value;
    entry->modified = true;

    config->modified = true;
    config->version++;

    return true;
}

bool speedpy_config_get_bool(
    const SpeedPyConfig *config,
    const char *key,
    bool *value
)
{
    const SpeedPyConfigEntry *entry;

    if (value == NULL) {
        return false;
    }

    entry = speedpy_config_find_const(config, key);

    if (entry == NULL || entry->type != SPEEDPY_CONFIG_BOOL) {
        return false;
    }

    *value = entry->value.boolean;

    return true;
}

bool speedpy_config_set_int(
    SpeedPyConfig *config,
    const char *key,
    int64_t value
)
{
    SpeedPyConfigEntry *entry;

    if (config == NULL || config->locked) {
        return false;
    }

    entry = speedpy_config_find(config, key);

    if (entry == NULL || entry->type != SPEEDPY_CONFIG_INT) {
        return false;
    }

    entry->value.integer = value;
    entry->modified = true;

    config->modified = true;
    config->version++;

    return true;
}

bool speedpy_config_get_int(
    const SpeedPyConfig *config,
    const char *key,
    int64_t *value
)
{
    const SpeedPyConfigEntry *entry;

    if (value == NULL) {
        return false;
    }

    entry = speedpy_config_find_const(config, key);

    if (entry == NULL || entry->type != SPEEDPY_CONFIG_INT) {
        return false;
    }

    *value = entry->value.integer;

    return true;
}

bool speedpy_config_set_uint(
    SpeedPyConfig *config,
    const char *key,
    uint64_t value
)
{
    SpeedPyConfigEntry *entry;

    if (config == NULL || config->locked) {
        return false;
    }

    entry = speedpy_config_find(config, key);

    if (entry == NULL || entry->type != SPEEDPY_CONFIG_UINT) {
        return false;
    }

    entry->value.unsigned_integer = value;
    entry->modified = true;

    config->modified = true;
    config->version++;

    return true;
}

bool speedpy_config_get_uint(
    const SpeedPyConfig *config,
    const char *key,
    uint64_t *value
)
{
    const SpeedPyConfigEntry *entry;

    if (value == NULL) {
        return false;
    }

    entry = speedpy_config_find_const(config, key);

    if (entry == NULL || entry->type != SPEEDPY_CONFIG_UINT) {
        return false;
    }

    *value = entry->value.unsigned_integer;

    return true;
}

bool speedpy_config_set_size(
    SpeedPyConfig *config,
    const char *key,
    size_t value
)
{
    SpeedPyConfigEntry *entry;

    if (config == NULL || config->locked) {
        return false;
    }

    entry = speedpy_config_find(config, key);

    if (entry == NULL || entry->type != SPEEDPY_CONFIG_SIZE) {
        return false;
    }

    entry->value.size = value;
    entry->modified = true;

    config->modified = true;
    config->version++;

    return true;
}

bool speedpy_config_get_size(
    const SpeedPyConfig *config,
    const char *key,
    size_t *value
)
{
    const SpeedPyConfigEntry *entry;

    if (value == NULL) {
        return false;
    }

    entry = speedpy_config_find_const(config, key);

    if (entry == NULL || entry->type != SPEEDPY_CONFIG_SIZE) {
        return false;
    }

    *value = entry->value.size;

    return true;
}

bool speedpy_config_set_double(
    SpeedPyConfig *config,
    const char *key,
    double value
)
{
    SpeedPyConfigEntry *entry;

    if (config == NULL || config->locked) {
        return false;
    }

    entry = speedpy_config_find(config, key);

    if (entry == NULL || entry->type != SPEEDPY_CONFIG_DOUBLE) {
        return false;
    }

    entry->value.decimal = value;
    entry->modified = true;

    config->modified = true;
    config->version++;

    return true;
}

bool speedpy_config_get_double(
    const SpeedPyConfig *config,
    const char *key,
    double *value
)
{
    const SpeedPyConfigEntry *entry;

    if (value == NULL) {
        return false;
    }

    entry = speedpy_config_find_const(config, key);

    if (entry == NULL || entry->type != SPEEDPY_CONFIG_DOUBLE) {
        return false;
    }

    *value = entry->value.decimal;

    return true;
}

bool speedpy_config_set_string(
    SpeedPyConfig *config,
    const char *key,
    const char *value
)
{
    SpeedPyConfigEntry *entry;
    size_t length;

    if (config == NULL ||
        key == NULL ||
        value == NULL ||
        config->locked) {
        return false;
    }

    entry = speedpy_config_find(config, key);

    if (entry == NULL || entry->type != SPEEDPY_CONFIG_STRING) {
        return false;
    }

    length = strlen(value);

    if (length >= sizeof(entry->value.string)) {
        return false;
    }

    memcpy(entry->value.string, value, length);
    entry->value.string[length] = '\0';

    entry->modified = true;

    config->modified = true;
    config->version++;

    return true;
}

const char *speedpy_config_get_string(
    const SpeedPyConfig *config,
    const char *key
)
{
    const SpeedPyConfigEntry *entry;

    entry = speedpy_config_find_const(config, key);

    if (entry == NULL || entry->type != SPEEDPY_CONFIG_STRING) {
        return NULL;
    }

    return entry->value.string;
}

size_t speedpy_config_count(
    const SpeedPyConfig *config
)
{
    if (config == NULL) {
        return 0;
    }

    return config->entry_count;
}

uint64_t speedpy_config_version(
    const SpeedPyConfig *config
)
{
    if (config == NULL) {
        return 0;
    }

    return config->version;
}

bool speedpy_config_is_modified(
    const SpeedPyConfig *config
)
{
    if (config == NULL) {
        return false;
    }

    return config->modified;
}

bool speedpy_config_lock(SpeedPyConfig *config)
{
    if (config == NULL) {
        return false;
    }

    config->locked = true;

    return true;
}

bool speedpy_config_unlock(SpeedPyConfig *config)
{
    if (config == NULL) {
        return false;
    }

    config->locked = false;

    return true;
}

bool speedpy_config_is_locked(
    const SpeedPyConfig *config
)
{
    if (config == NULL) {
        return false;
    }

    return config->locked;
}

void speedpy_config_mark_clean(SpeedPyConfig *config)
{
    size_t i;

    if (config == NULL) {
        return;
    }

    config->modified = false;

    for (i = 0; i < config->entry_count; i++) {
        config->entries[i].modified = false;
    }
}

void speedpy_config_reset(SpeedPyConfig *config)
{
    size_t i;

    if (config == NULL || config->locked) {
        return;
    }

    for (i = 0; i < config->entry_count; i++) {
        memset(
            &config->entries[i].value,
            0,
            sizeof(config->entries[i].value)
        );

        config->entries[i].modified = false;
    }

    config->modified = true;
    config->version++;
}