#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define SPEEDPY_ERROR_MAX 512
#define SPEEDPY_ERROR_MESSAGE_SIZE 512
#define SPEEDPY_ERROR_SOURCE_SIZE 96
#define SPEEDPY_ERROR_CODE_SIZE 64
#define SPEEDPY_ERROR_STACK_DEPTH 32

typedef uint64_t SpeedPyErrorId;

typedef enum {
    SPEEDPY_ERROR_NONE = 0,
    SPEEDPY_ERROR_INFO,
    SPEEDPY_ERROR_WARNING,
    SPEEDPY_ERROR_RUNTIME,
    SPEEDPY_ERROR_INVALID_ARGUMENT,
    SPEEDPY_ERROR_INVALID_STATE,
    SPEEDPY_ERROR_NOT_FOUND,
    SPEEDPY_ERROR_MEMORY,
    SPEEDPY_ERROR_IO,
    SPEEDPY_ERROR_TIMEOUT,
    SPEEDPY_ERROR_OVERFLOW,
    SPEEDPY_ERROR_UNSUPPORTED,
    SPEEDPY_ERROR_INTERNAL,
    SPEEDPY_ERROR_FATAL
} SpeedPyErrorType;

typedef struct {
    SpeedPyErrorId id;

    SpeedPyErrorType type;

    int32_t code;

    char code_name[SPEEDPY_ERROR_CODE_SIZE];
    char source[SPEEDPY_ERROR_SOURCE_SIZE];
    char message[SPEEDPY_ERROR_MESSAGE_SIZE];

    uint64_t tick;

    uint64_t generation;

    bool active;
    bool handled;
    bool fatal;
} SpeedPyError;

typedef struct {
    SpeedPyError entries[SPEEDPY_ERROR_MAX];

    SpeedPyErrorId next_id;

    size_t count;
    size_t active_count;

    SpeedPyErrorId last_error_id;
    SpeedPyErrorId last_fatal_id;

    uint64_t total_errors;
    uint64_t total_warnings;
    uint64_t total_fatals;
    uint64_t handled_count;

    uint64_t generation;

    bool enabled;
    bool locked;

    void *user_data;
} SpeedPyErrorManager;

typedef struct {
    SpeedPyErrorType type;
    int32_t code;
    char message[SPEEDPY_ERROR_MESSAGE_SIZE];
    SpeedPyErrorId error_id;
} SpeedPyErrorState;

static bool speedpy_error_manager_valid(
    const SpeedPyErrorManager *manager
) {
    return manager != NULL &&
           manager->count <= SPEEDPY_ERROR_MAX &&
           manager->active_count <= SPEEDPY_ERROR_MAX;
}

static SpeedPyError *speedpy_error_find(
    SpeedPyErrorManager *manager,
    SpeedPyErrorId id
) {
    if (!speedpy_error_manager_valid(manager) ||
        id == 0) {
        return NULL;
    }

    for (size_t i = 0; i < SPEEDPY_ERROR_MAX; i++) {
        if (manager->entries[i].active &&
            manager->entries[i].id == id) {
            return &manager->entries[i];
        }
    }

    return NULL;
}

static const SpeedPyError *speedpy_error_find_const(
    const SpeedPyErrorManager *manager,
    SpeedPyErrorId id
) {
    if (!speedpy_error_manager_valid(manager) ||
        id == 0) {
        return NULL;
    }

    for (size_t i = 0; i < SPEEDPY_ERROR_MAX; i++) {
        if (manager->entries[i].active &&
            manager->entries[i].id == id) {
            return &manager->entries[i];
        }
    }

    return NULL;
}

static SpeedPyError *speedpy_error_find_slot(
    SpeedPyErrorManager *manager
) {
    if (!speedpy_error_manager_valid(manager)) {
        return NULL;
    }

    for (size_t i = 0; i < SPEEDPY_ERROR_MAX; i++) {
        if (!manager->entries[i].active) {
            return &manager->entries[i];
        }
    }

    return NULL;
}

static void speedpy_error_copy_text(
    char *destination,
    size_t destination_size,
    const char *source
) {
    if (destination == NULL ||
        destination_size == 0) {
        return;
    }

    if (source == NULL) {
        destination[0] = '\0';
        return;
    }

    strncpy(
        destination,
        source,
        destination_size - 1
    );

    destination[destination_size - 1] = '\0';
}

static bool speedpy_error_type_is_fatal(
    SpeedPyErrorType type
) {
    return type == SPEEDPY_ERROR_FATAL;
}

static bool speedpy_error_type_is_warning(
    SpeedPyErrorType type
) {
    return type == SPEEDPY_ERROR_WARNING;
}

SpeedPyErrorManager *speedpy_errors_create(void) {
    SpeedPyErrorManager *manager =
        (SpeedPyErrorManager *)calloc(
            1,
            sizeof(SpeedPyErrorManager)
        );

    if (manager == NULL) {
        return NULL;
    }

    manager->next_id = 1;
    manager->enabled = true;
    manager->generation = 1;

    return manager;
}

void speedpy_errors_destroy(
    SpeedPyErrorManager *manager
) {
    if (manager == NULL) {
        return;
    }

    free(manager);
}

SpeedPyErrorId speedpy_error_push(
    SpeedPyErrorManager *manager,
    SpeedPyErrorType type,
    int32_t code,
    const char *code_name,
    const char *source,
    const char *message,
    uint64_t tick
) {
    SpeedPyError *error;
    SpeedPyErrorId id;

    if (!speedpy_error_manager_valid(manager) ||
        manager->locked ||
        !manager->enabled ||
        type == SPEEDPY_ERROR_NONE) {
        return 0;
    }

    error = speedpy_error_find_slot(manager);

    if (error == NULL) {
        return 0;
    }

    memset(
        error,
        0,
        sizeof(SpeedPyError)
    );

    id = manager->next_id++;

    if (id == 0) {
        id = manager->next_id++;
    }

    error->id = id;
    error->type = type;
    error->code = code;
    error->tick = tick;
    error->generation =
        manager->generation + 1;
    error->active = true;
    error->fatal =
        speedpy_error_type_is_fatal(type);

    speedpy_error_copy_text(
        error->code_name,
        SPEEDPY_ERROR_CODE_SIZE,
        code_name
    );

    speedpy_error_copy_text(
        error->source,
        SPEEDPY_ERROR_SOURCE_SIZE,
        source
    );

    speedpy_error_copy_text(
        error->message,
        SPEEDPY_ERROR_MESSAGE_SIZE,
        message
    );

    manager->count++;
    manager->active_count++;
    manager->total_errors++;

    if (speedpy_error_type_is_warning(type)) {
        manager->total_warnings++;
    }

    if (error->fatal) {
        manager->total_fatals++;
        manager->last_fatal_id = id;
    }

    manager->last_error_id = id;
    manager->generation++;

    return id;
}

SpeedPyErrorId speedpy_error_info(
    SpeedPyErrorManager *manager,
    int32_t code,
    const char *source,
    const char *message,
    uint64_t tick
) {
    return speedpy_error_push(
        manager,
        SPEEDPY_ERROR_INFO,
        code,
        NULL,
        source,
        message,
        tick
    );
}

SpeedPyErrorId speedpy_error_warning(
    SpeedPyErrorManager *manager,
    int32_t code,
    const char *source,
    const char *message,
    uint64_t tick
) {
    return speedpy_error_push(
        manager,
        SPEEDPY_ERROR_WARNING,
        code,
        NULL,
        source,
        message,
        tick
    );
}

SpeedPyErrorId speedpy_error_runtime(
    SpeedPyErrorManager *manager,
    int32_t code,
    const char *source,
    const char *message,
    uint64_t tick
) {
    return speedpy_error_push(
        manager,
        SPEEDPY_ERROR_RUNTIME,
        code,
        NULL,
        source,
        message,
        tick
    );
}

SpeedPyErrorId speedpy_error_invalid_argument(
    SpeedPyErrorManager *manager,
    int32_t code,
    const char *source,
    const char *message,
    uint64_t tick
) {
    return speedpy_error_push(
        manager,
        SPEEDPY_ERROR_INVALID_ARGUMENT,
        code,
        NULL,
        source,
        message,
        tick
    );
}

SpeedPyErrorId speedpy_error_invalid_state(
    SpeedPyErrorManager *manager,
    int32_t code,
    const char *source,
    const char *message,
    uint64_t tick
) {
    return speedpy_error_push(
        manager,
        SPEEDPY_ERROR_INVALID_STATE,
        code,
        NULL,
        source,
        message,
        tick
    );
}

SpeedPyErrorId speedpy_error_not_found(
    SpeedPyErrorManager *manager,
    int32_t code,
    const char *source,
    const char *message,
    uint64_t tick
) {
    return speedpy_error_push(
        manager,
        SPEEDPY_ERROR_NOT_FOUND,
        code,
        NULL,
        source,
        message,
        tick
    );
}

SpeedPyErrorId speedpy_error_memory(
    SpeedPyErrorManager *manager,
    int32_t code,
    const char *source,
    const char *message,
    uint64_t tick
) {
    return speedpy_error_push(
        manager,
        SPEEDPY_ERROR_MEMORY,
        code,
        NULL,
        source,
        message,
        tick
    );
}

SpeedPyErrorId speedpy_error_overflow(
    SpeedPyErrorManager *manager,
    int32_t code,
    const char *source,
    const char *message,
    uint64_t tick
) {
    return speedpy_error_push(
        manager,
        SPEEDPY_ERROR_OVERFLOW,
        code,
        NULL,
        source,
        message,
        tick
    );
}

SpeedPyErrorId speedpy_error_unsupported(
    SpeedPyErrorManager *manager,
    int32_t code,
    const char *source,
    const char *message,
    uint64_t tick
) {
    return speedpy_error_push(
        manager,
        SPEEDPY_ERROR_UNSUPPORTED,
        code,
        NULL,
        source,
        message,
        tick
    );
}

SpeedPyErrorId speedpy_error_fatal(
    SpeedPyErrorManager *manager,
    int32_t code,
    const char *source,
    const char *message,
    uint64_t tick
) {
    return speedpy_error_push(
        manager,
        SPEEDPY_ERROR_FATAL,
        code,
        "FATAL",
        source,
        message,
        tick
    );
}

const SpeedPyError *speedpy_error_get(
    const SpeedPyErrorManager *manager,
    SpeedPyErrorId id
) {
    return speedpy_error_find_const(
        manager,
        id
    );
}

SpeedPyErrorId speedpy_error_last(
    const SpeedPyErrorManager *manager
) {
    if (!speedpy_error_manager_valid(manager)) {
        return 0;
    }

    return manager->last_error_id;
}

SpeedPyErrorId speedpy_error_last_fatal(
    const SpeedPyErrorManager *manager
) {
    if (!speedpy_error_manager_valid(manager)) {
        return 0;
    }

    return manager->last_fatal_id;
}

bool speedpy_error_exists(
    const SpeedPyErrorManager *manager,
    SpeedPyErrorId id
) {
    return speedpy_error_find_const(
        manager,
        id
    ) != NULL;
}

bool speedpy_error_mark_handled(
    SpeedPyErrorManager *manager,
    SpeedPyErrorId id
) {
    SpeedPyError *error;

    if (!speedpy_error_manager_valid(manager) ||
        manager->locked) {
        return false;
    }

    error = speedpy_error_find(manager, id);

    if (error == NULL) {
        return false;
    }

    if (!error->handled) {
        error->handled = true;
        manager->handled_count++;
        manager->generation++;
    }

    return true;
}

bool speedpy_error_is_handled(
    const SpeedPyErrorManager *manager,
    SpeedPyErrorId id
) {
    const SpeedPyError *error =
        speedpy_error_find_const(manager, id);

    if (error == NULL) {
        return false;
    }

    return error->handled;
}

bool speedpy_error_remove(
    SpeedPyErrorManager *manager,
    SpeedPyErrorId id
) {
    SpeedPyError *error;

    if (!speedpy_error_manager_valid(manager) ||
        manager->locked) {
        return false;
    }

    error = speedpy_error_find(manager, id);

    if (error == NULL) {
        return false;
    }

    if (error->handled &&
        manager->handled_count > 0) {
        manager->handled_count--;
    }

    memset(
        error,
        0,
        sizeof(SpeedPyError)
    );

    if (manager->count > 0) {
        manager->count--;
    }

    if (manager->active_count > 0) {
        manager->active_count--;
    }

    manager->generation++;

    return true;
}

bool speedpy_error_clear(
    SpeedPyErrorManager *manager
) {
    if (!speedpy_error_manager_valid(manager) ||
        manager->locked) {
        return false;
    }

    memset(
        manager->entries,
        0,
        sizeof(manager->entries)
    );

    manager->count = 0;
    manager->active_count = 0;
    manager->last_error_id = 0;
    manager->last_fatal_id = 0;
    manager->handled_count = 0;

    manager->generation++;

    return true;
}

bool speedpy_error_clear_nonfatal(
    SpeedPyErrorManager *manager
) {
    size_t removed = 0;
    size_t handled_removed = 0;

    if (!speedpy_error_manager_valid(manager) ||
        manager->locked) {
        return false;
    }

    for (size_t i = 0; i < SPEEDPY_ERROR_MAX; i++) {
        SpeedPyError *error =
            &manager->entries[i];

        if (!error->active || error->fatal) {
            continue;
        }

        if (error->handled) {
            handled_removed++;
        }

        memset(
            error,
            0,
            sizeof(SpeedPyError)
        );

        removed++;
    }

    if (removed > manager->count) {
        manager->count = 0;
    } else {
        manager->count -= removed;
    }

    if (removed > manager->active_count) {
        manager->active_count = 0;
    } else {
        manager->active_count -= removed;
    }

    if (handled_removed > manager->handled_count) {
        manager->handled_count = 0;
    } else {
        manager->handled_count -= handled_removed;
    }

    manager->generation++;

    return true;
}

size_t speedpy_error_count(
    const SpeedPyErrorManager *manager
) {
    if (!speedpy_error_manager_valid(manager)) {
        return 0;
    }

    return manager->count;
}

size_t speedpy_error_active_count(
    const SpeedPyErrorManager *manager
) {
    if (!speedpy_error_manager_valid(manager)) {
        return 0;
    }

    return manager->active_count;
}

uint64_t speedpy_error_total(
    const SpeedPyErrorManager *manager
) {
    if (!speedpy_error_manager_valid(manager)) {
        return 0;
    }

    return manager->total_errors;
}

uint64_t speedpy_error_warning_count(
    const SpeedPyErrorManager *manager
) {
    if (!speedpy_error_manager_valid(manager)) {
        return 0;
    }

    return manager->total_warnings;
}

uint64_t speedpy_error_fatal_count(
    const SpeedPyErrorManager *manager
) {
    if (!speedpy_error_manager_valid(manager)) {
        return 0;
    }

    return manager->total_fatals;
}

uint64_t speedpy_error_handled_count(
    const SpeedPyErrorManager *manager
) {
    if (!speedpy_error_manager_valid(manager)) {
        return 0;
    }

    return manager->handled_count;
}

uint64_t speedpy_error_generation(
    const SpeedPyErrorManager *manager
) {
    if (!speedpy_error_manager_valid(manager)) {
        return 0;
    }

    return manager->generation;
}

bool speedpy_error_get_state(
    const SpeedPyErrorManager *manager,
    SpeedPyErrorState *state
) {
    const SpeedPyError *error;

    if (!speedpy_error_manager_valid(manager) ||
        state == NULL) {
        return false;
    }

    error =
        speedpy_error_find_const(
            manager,
            manager->last_error_id
        );

    memset(
        state,
        0,
        sizeof(SpeedPyErrorState)
    );

    if (error == NULL) {
        state->type = SPEEDPY_ERROR_NONE;
        state->error_id = 0;
        return false;
    }

    state->type = error->type;
    state->code = error->code;
    state->error_id = error->id;

    speedpy_error_copy_text(
        state->message,
        SPEEDPY_ERROR_MESSAGE_SIZE,
        error->message
    );

    return true;
}

bool speedpy_errors_enable(
    SpeedPyErrorManager *manager
) {
    if (!speedpy_error_manager_valid(manager) ||
        manager->locked) {
        return false;
    }

    manager->enabled = true;
    manager->generation++;

    return true;
}

bool speedpy_errors_disable(
    SpeedPyErrorManager *manager
) {
    if (!speedpy_error_manager_valid(manager) ||
        manager->locked) {
        return false;
    }

    manager->enabled = false;
    manager->generation++;

    return true;
}

bool speedpy_errors_is_enabled(
    const SpeedPyErrorManager *manager
) {
    if (!speedpy_error_manager_valid(manager)) {
        return false;
    }

    return manager->enabled;
}

bool speedpy_errors_lock(
    SpeedPyErrorManager *manager
) {
    if (!speedpy_error_manager_valid(manager)) {
        return false;
    }

    manager->locked = true;
    return true;
}

bool speedpy_errors_unlock(
    SpeedPyErrorManager *manager
) {
    if (!speedpy_error_manager_valid(manager)) {
        return false;
    }

    manager->locked = false;
    return true;
}

bool speedpy_errors_is_locked(
    const SpeedPyErrorManager *manager
) {
    if (!speedpy_error_manager_valid(manager)) {
        return false;
    }

    return manager->locked;
}

void speedpy_errors_set_user_data(
    SpeedPyErrorManager *manager,
    void *user_data
) {
    if (!speedpy_error_manager_valid(manager)) {
        return;
    }

    manager->user_data = user_data;
}

void *speedpy_errors_get_user_data(
    const SpeedPyErrorManager *manager
) {
    if (!speedpy_error_manager_valid(manager)) {
        return NULL;
    }

    return manager->user_data;
}