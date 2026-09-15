#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define SPEEDPY_CONTEXT_NAME_SIZE 128
#define SPEEDPY_CONTEXT_MAX_VALUES 256
#define SPEEDPY_CONTEXT_KEY_SIZE 64
#define SPEEDPY_CONTEXT_STRING_SIZE 256

typedef enum {
    SPEEDPY_CONTEXT_BOOL = 0,
    SPEEDPY_CONTEXT_INT = 1,
    SPEEDPY_CONTEXT_UINT = 2,
    SPEEDPY_CONTEXT_SIZE = 3,
    SPEEDPY_CONTEXT_DOUBLE = 4,
    SPEEDPY_CONTEXT_STRING = 5,
    SPEEDPY_CONTEXT_POINTER = 6
} SpeedPyContextType;

typedef struct {
    char key[SPEEDPY_CONTEXT_KEY_SIZE];
    SpeedPyContextType type;

    union {
        bool boolean;
        int64_t integer;
        uint64_t unsigned_integer;
        size_t size;
        double decimal;
        void *pointer;
        char string[SPEEDPY_CONTEXT_STRING_SIZE];
    } value;

    bool occupied;
} SpeedPyContextValue;

typedef struct {
    char name[SPEEDPY_CONTEXT_NAME_SIZE];

    SpeedPyContextValue values[SPEEDPY_CONTEXT_MAX_VALUES];

    size_t value_count;
    uint64_t generation;

    void *user_data;

    bool active;
    bool locked;
} SpeedPyContext;

static SpeedPyContextValue *speedpy_context_find(
    SpeedPyContext *context,
    const char *key
)
{
    size_t i;

    if (context == NULL || key == NULL) {
        return NULL;
    }

    for (i = 0; i < context->value_count; i++) {
        if (context->values[i].occupied &&
            strcmp(context->values[i].key, key) == 0) {
            return &context->values[i];
        }
    }

    return NULL;
}

static const SpeedPyContextValue *speedpy_context_find_const(
    const SpeedPyContext *context,
    const char *key
)
{
    size_t i;

    if (context == NULL || key == NULL) {
        return NULL;
    }

    for (i = 0; i < context->value_count; i++) {
        if (context->values[i].occupied &&
            strcmp(context->values[i].key, key) == 0) {
            return &context->values[i];
        }
    }

    return NULL;
}

static bool speedpy_context_valid_key(const char *key)
{
    size_t length;

    if (key == NULL) {
        return false;
    }

    length = strlen(key);

    return length > 0 && length < SPEEDPY_CONTEXT_KEY_SIZE;
}

SpeedPyContext *speedpy_context_create(const char *name)
{
    SpeedPyContext *context;
    size_t length;

    context = (SpeedPyContext *)calloc(
        1,
        sizeof(SpeedPyContext)
    );

    if (context == NULL) {
        return NULL;
    }

    if (name != NULL) {
        length = strlen(name);

        if (length >= SPEEDPY_CONTEXT_NAME_SIZE) {
            length = SPEEDPY_CONTEXT_NAME_SIZE - 1;
        }

        memcpy(context->name, name, length);
        context->name[length] = '\0';
    }

    context->value_count = 0;
    context->generation = 1;
    context->user_data = NULL;
    context->active = true;
    context->locked = false;

    return context;
}

void speedpy_context_destroy(SpeedPyContext *context)
{
    if (context == NULL) {
        return;
    }

    context->active = false;
    context->locked = true;

    free(context);
}

bool speedpy_context_activate(SpeedPyContext *context)
{
    if (context == NULL || context->locked) {
        return false;
    }

    context->active = true;
    context->generation++;

    return true;
}

bool speedpy_context_deactivate(SpeedPyContext *context)
{
    if (context == NULL || context->locked) {
        return false;
    }

    context->active = false;
    context->generation++;

    return true;
}

bool speedpy_context_is_active(
    const SpeedPyContext *context
)
{
    if (context == NULL) {
        return false;
    }

    return context->active;
}

bool speedpy_context_register(
    SpeedPyContext *context,
    const char *key,
    SpeedPyContextType type
)
{
    SpeedPyContextValue *value;

    if (context == NULL ||
        context->locked ||
        !speedpy_context_valid_key(key)) {
        return false;
    }

    if (speedpy_context_find(context, key) != NULL) {
        return false;
    }

    if (context->value_count >= SPEEDPY_CONTEXT_MAX_VALUES) {
        return false;
    }

    value = &context->values[context->value_count];

    memset(value, 0, sizeof(SpeedPyContextValue));

    memcpy(value->key, key, strlen(key));
    value->key[strlen(key)] = '\0';

    value->type = type;
    value->occupied = true;

    context->value_count++;
    context->generation++;

    return true;
}

bool speedpy_context_remove(
    SpeedPyContext *context,
    const char *key
)
{
    size_t i;
    size_t index;

    if (context == NULL ||
        context->locked ||
        key == NULL) {
        return false;
    }

    index = context->value_count;

    for (i = 0; i < context->value_count; i++) {
        if (strcmp(context->values[i].key, key) == 0) {
            index = i;
            break;
        }
    }

    if (index == context->value_count) {
        return false;
    }

    for (i = index; i + 1 < context->value_count; i++) {
        context->values[i] = context->values[i + 1];
    }

    memset(
        &context->values[context->value_count - 1],
        0,
        sizeof(SpeedPyContextValue)
    );

    context->value_count--;
    context->generation++;

    return true;
}

bool speedpy_context_has(
    const SpeedPyContext *context,
    const char *key
)
{
    return speedpy_context_find_const(context, key) != NULL;
}

SpeedPyContextType speedpy_context_get_type(
    const SpeedPyContext *context,
    const char *key
)
{
    const SpeedPyContextValue *value;

    value = speedpy_context_find_const(context, key);

    if (value == NULL) {
        return SPEEDPY_CONTEXT_STRING;
    }

    return value->type;
}

bool speedpy_context_set_bool(
    SpeedPyContext *context,
    const char *key,
    bool value
)
{
    SpeedPyContextValue *entry;

    if (context == NULL || context->locked) {
        return false;
    }

    entry = speedpy_context_find(context, key);

    if (entry == NULL ||
        entry->type != SPEEDPY_CONTEXT_BOOL) {
        return false;
    }

    entry->value.boolean = value;
    context->generation++;

    return true;
}

bool speedpy_context_get_bool(
    const SpeedPyContext *context,
    const char *key,
    bool *value
)
{
    const SpeedPyContextValue *entry;

    if (value == NULL) {
        return false;
    }

    entry = speedpy_context_find_const(context, key);

    if (entry == NULL ||
        entry->type != SPEEDPY_CONTEXT_BOOL) {
        return false;
    }

    *value = entry->value.boolean;

    return true;
}

bool speedpy_context_set_int(
    SpeedPyContext *context,
    const char *key,
    int64_t value
)
{
    SpeedPyContextValue *entry;

    if (context == NULL || context->locked) {
        return false;
    }

    entry = speedpy_context_find(context, key);

    if (entry == NULL ||
        entry->type != SPEEDPY_CONTEXT_INT) {
        return false;
    }

    entry->value.integer = value;
    context->generation++;

    return true;
}

bool speedpy_context_get_int(
    const SpeedPyContext *context,
    const char *key,
    int64_t *value
)
{
    const SpeedPyContextValue *entry;

    if (value == NULL) {
        return false;
    }

    entry = speedpy_context_find_const(context, key);

    if (entry == NULL ||
        entry->type != SPEEDPY_CONTEXT_INT) {
        return false;
    }

    *value = entry->value.integer;

    return true;
}

bool speedpy_context_set_uint(
    SpeedPyContext *context,
    const char *key,
    uint64_t value
)
{
    SpeedPyContextValue *entry;

    if (context == NULL || context->locked) {
        return false;
    }

    entry = speedpy_context_find(context, key);

    if (entry == NULL ||
        entry->type != SPEEDPY_CONTEXT_UINT) {
        return false;
    }

    entry->value.unsigned_integer = value;
    context->generation++;

    return true;
}

bool speedpy_context_get_uint(
    const SpeedPyContext *context,
    const char *key,
    uint64_t *value
)
{
    const SpeedPyContextValue *entry;

    if (value == NULL) {
        return false;
    }

    entry = speedpy_context_find_const(context, key);

    if (entry == NULL ||
        entry->type != SPEEDPY_CONTEXT_UINT) {
        return false;
    }

    *value = entry->value.unsigned_integer;

    return true;
}

bool speedpy_context_set_size(
    SpeedPyContext *context,
    const char *key,
    size_t value
)
{
    SpeedPyContextValue *entry;

    if (context == NULL || context->locked) {
        return false;
    }

    entry = speedpy_context_find(context, key);

    if (entry == NULL ||
        entry->type != SPEEDPY_CONTEXT_SIZE) {
        return false;
    }

    entry->value.size = value;
    context->generation++;

    return true;
}

bool speedpy_context_get_size(
    const SpeedPyContext *context,
    const char *key,
    size_t *value
)
{
    const SpeedPyContextValue *entry;

    if (value == NULL) {
        return false;
    }

    entry = speedpy_context_find_const(context, key);

    if (entry == NULL ||
        entry->type != SPEEDPY_CONTEXT_SIZE) {
        return false;
    }

    *value = entry->value.size;

    return true;
}

bool speedpy_context_set_double(
    SpeedPyContext *context,
    const char *key,
    double value
)
{
    SpeedPyContextValue *entry;

    if (context == NULL || context->locked) {
        return false;
    }

    entry = speedpy_context_find(context, key);

    if (entry == NULL ||
        entry->type != SPEEDPY_CONTEXT_DOUBLE) {
        return false;
    }

    entry->value.decimal = value;
    context->generation++;

    return true;
}

bool speedpy_context_get_double(
    const SpeedPyContext *context,
    const char *key,
    double *value
)
{
    const SpeedPyContextValue *entry;

    if (value == NULL) {
        return false;
    }

    entry = speedpy_context_find_const(context, key);

    if (entry == NULL ||
        entry->type != SPEEDPY_CONTEXT_DOUBLE) {
        return false;
    }

    *value = entry->value.decimal;

    return true;
}

bool speedpy_context_set_string(
    SpeedPyContext *context,
    const char *key,
    const char *value
)
{
    SpeedPyContextValue *entry;
    size_t length;

    if (context == NULL ||
        value == NULL ||
        context->locked) {
        return false;
    }

    entry = speedpy_context_find(context, key);

    if (entry == NULL ||
        entry->type != SPEEDPY_CONTEXT_STRING) {
        return false;
    }

    length = strlen(value);

    if (length >= SPEEDPY_CONTEXT_STRING_SIZE) {
        return false;
    }

    memcpy(entry->value.string, value, length);
    entry->value.string[length] = '\0';

    context->generation++;

    return true;
}

const char *speedpy_context_get_string(
    const SpeedPyContext *context,
    const char *key
)
{
    const SpeedPyContextValue *entry;

    entry = speedpy_context_find_const(context, key);

    if (entry == NULL ||
        entry->type != SPEEDPY_CONTEXT_STRING) {
        return NULL;
    }

    return entry->value.string;
}

bool speedpy_context_set_pointer(
    SpeedPyContext *context,
    const char *key,
    void *value
)
{
    SpeedPyContextValue *entry;

    if (context == NULL || context->locked) {
        return false;
    }

    entry = speedpy_context_find(context, key);

    if (entry == NULL ||
        entry->type != SPEEDPY_CONTEXT_POINTER) {
        return false;
    }

    entry->value.pointer = value;
    context->generation++;

    return true;
}

void *speedpy_context_get_pointer(
    const SpeedPyContext *context,
    const char *key
)
{
    const SpeedPyContextValue *entry;

    entry = speedpy_context_find_const(context, key);

    if (entry == NULL ||
        entry->type != SPEEDPY_CONTEXT_POINTER) {
        return NULL;
    }

    return entry->value.pointer;
}

size_t speedpy_context_count(
    const SpeedPyContext *context
)
{
    if (context == NULL) {
        return 0;
    }

    return context->value_count;
}

uint64_t speedpy_context_generation(
    const SpeedPyContext *context
)
{
    if (context == NULL) {
        return 0;
    }

    return context->generation;
}

bool speedpy_context_lock(SpeedPyContext *context)
{
    if (context == NULL) {
        return false;
    }

    context->locked = true;

    return true;
}

bool speedpy_context_unlock(SpeedPyContext *context)
{
    if (context == NULL) {
        return false;
    }

    context->locked = false;

    return true;
}

bool speedpy_context_is_locked(
    const SpeedPyContext *context
)
{
    if (context == NULL) {
        return false;
    }

    return context->locked;
}

bool speedpy_context_set_user_data(
    SpeedPyContext *context,
    void *user_data
)
{
    if (context == NULL || context->locked) {
        return false;
    }

    context->user_data = user_data;
    context->generation++;

    return true;
}

void *speedpy_context_get_user_data(
    const SpeedPyContext *context
)
{
    if (context == NULL) {
        return NULL;
    }

    return context->user_data;
}

const char *speedpy_context_get_name(
    const SpeedPyContext *context
)
{
    if (context == NULL) {
        return NULL;
    }

    return context->name;
}

void speedpy_context_clear(SpeedPyContext *context)
{
    size_t i;

    if (context == NULL || context->locked) {
        return;
    }

    for (i = 0; i < context->value_count; i++) {
        memset(
            &context->values[i],
            0,
            sizeof(SpeedPyContextValue)
        );
    }

    context->value_count = 0;
    context->generation++;
}