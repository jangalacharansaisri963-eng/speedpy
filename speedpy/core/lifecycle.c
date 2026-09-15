#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define SPEEDPY_LIFECYCLE_MAX_HOOKS 128
#define SPEEDPY_LIFECYCLE_NAME_SIZE 96

typedef enum {
    SPEEDPY_LIFECYCLE_CREATED = 0,
    SPEEDPY_LIFECYCLE_INITIALIZING = 1,
    SPEEDPY_LIFECYCLE_INITIALIZED = 2,
    SPEEDPY_LIFECYCLE_STARTING = 3,
    SPEEDPY_LIFECYCLE_RUNNING = 4,
    SPEEDPY_LIFECYCLE_PAUSING = 5,
    SPEEDPY_LIFECYCLE_PAUSED = 6,
    SPEEDPY_LIFECYCLE_STOPPING = 7,
    SPEEDPY_LIFECYCLE_STOPPED = 8,
    SPEEDPY_LIFECYCLE_SHUTTING_DOWN = 9,
    SPEEDPY_LIFECYCLE_DESTROYED = 10,
    SPEEDPY_LIFECYCLE_FAILED = 11
} SpeedPyLifecycleState;

typedef enum {
    SPEEDPY_HOOK_INITIALIZE = 0,
    SPEEDPY_HOOK_START = 1,
    SPEEDPY_HOOK_PAUSE = 2,
    SPEEDPY_HOOK_RESUME = 3,
    SPEEDPY_HOOK_STOP = 4,
    SPEEDPY_HOOK_SHUTDOWN = 5
} SpeedPyLifecycleHookType;

typedef int (*SpeedPyLifecycleCallback)(
    SpeedPyLifecycleHookType type,
    SpeedPyLifecycleState state,
    void *user_data
);

typedef struct {
    uint32_t id;
    char name[SPEEDPY_LIFECYCLE_NAME_SIZE];

    SpeedPyLifecycleHookType type;
    SpeedPyLifecycleCallback callback;

    int32_t priority;

    bool enabled;
    bool once;

    uint64_t invocation_count;
    uint64_t failure_count;

    void *user_data;
} SpeedPyLifecycleHook;

typedef struct {
    char name[SPEEDPY_LIFECYCLE_NAME_SIZE];

    SpeedPyLifecycleState state;

    SpeedPyLifecycleHook hooks[SPEEDPY_LIFECYCLE_MAX_HOOKS];

    size_t hook_count;
    uint32_t next_hook_id;

    uint64_t transition_count;
    uint64_t failure_count;
    uint64_t generation;

    bool locked;

    void *user_data;
} SpeedPyLifecycle;

static bool speedpy_lifecycle_valid_name(
    const char *name
)
{
    return name != NULL && name[0] != '\0';
}

static SpeedPyLifecycleHook *speedpy_lifecycle_find_hook(
    SpeedPyLifecycle *lifecycle,
    const char *name
)
{
    size_t i;

    if (lifecycle == NULL || name == NULL) {
        return NULL;
    }

    for (i = 0; i < lifecycle->hook_count; ++i) {
        if (strcmp(lifecycle->hooks[i].name, name) == 0) {
            return &lifecycle->hooks[i];
        }
    }

    return NULL;
}

static SpeedPyLifecycleHook *speedpy_lifecycle_find_hook_id(
    SpeedPyLifecycle *lifecycle,
    uint32_t id
)
{
    size_t i;

    if (lifecycle == NULL || id == 0) {
        return NULL;
    }

    for (i = 0; i < lifecycle->hook_count; ++i) {
        if (lifecycle->hooks[i].id == id) {
            return &lifecycle->hooks[i];
        }
    }

    return NULL;
}

static bool speedpy_lifecycle_valid_hook_type(
    SpeedPyLifecycleHookType type
)
{
    return type >= SPEEDPY_HOOK_INITIALIZE &&
           type <= SPEEDPY_HOOK_SHUTDOWN;
}

static bool speedpy_lifecycle_transition_allowed(
    SpeedPyLifecycleState current,
    SpeedPyLifecycleState next
)
{
    switch (current) {
        case SPEEDPY_LIFECYCLE_CREATED:
            return next == SPEEDPY_LIFECYCLE_INITIALIZING ||
                   next == SPEEDPY_LIFECYCLE_FAILED;

        case SPEEDPY_LIFECYCLE_INITIALIZING:
            return next == SPEEDPY_LIFECYCLE_INITIALIZED ||
                   next == SPEEDPY_LIFECYCLE_FAILED;

        case SPEEDPY_LIFECYCLE_INITIALIZED:
            return next == SPEEDPY_LIFECYCLE_STARTING ||
                   next == SPEEDPY_LIFECYCLE_SHUTTING_DOWN ||
                   next == SPEEDPY_LIFECYCLE_FAILED;

        case SPEEDPY_LIFECYCLE_STARTING:
            return next == SPEEDPY_LIFECYCLE_RUNNING ||
                   next == SPEEDPY_LIFECYCLE_FAILED;

        case SPEEDPY_LIFECYCLE_RUNNING:
            return next == SPEEDPY_LIFECYCLE_PAUSING ||
                   next == SPEEDPY_LIFECYCLE_STOPPING ||
                   next == SPEEDPY_LIFECYCLE_FAILED;

        case SPEEDPY_LIFECYCLE_PAUSING:
            return next == SPEEDPY_LIFECYCLE_PAUSED ||
                   next == SPEEDPY_LIFECYCLE_FAILED;

        case SPEEDPY_LIFECYCLE_PAUSED:
            return next == SPEEDPY_LIFECYCLE_STARTING ||
                   next == SPEEDPY_LIFECYCLE_STOPPING ||
                   next == SPEEDPY_LIFECYCLE_FAILED;

        case SPEEDPY_LIFECYCLE_STOPPING:
            return next == SPEEDPY_LIFECYCLE_STOPPED ||
                   next == SPEEDPY_LIFECYCLE_FAILED;

        case SPEEDPY_LIFECYCLE_STOPPED:
            return next == SPEEDPY_LIFECYCLE_SHUTTING_DOWN ||
                   next == SPEEDPY_LIFECYCLE_STARTING ||
                   next == SPEEDPY_LIFECYCLE_FAILED;

        case SPEEDPY_LIFECYCLE_SHUTTING_DOWN:
            return next == SPEEDPY_LIFECYCLE_DESTROYED ||
                   next == SPEEDPY_LIFECYCLE_FAILED;

        case SPEEDPY_LIFECYCLE_DESTROYED:
            return false;

        case SPEEDPY_LIFECYCLE_FAILED:
            return next == SPEEDPY_LIFECYCLE_SHUTTING_DOWN ||
                   next == SPEEDPY_LIFECYCLE_DESTROYED;

        default:
            return false;
    }
}

static void speedpy_lifecycle_sort_hooks(
    SpeedPyLifecycle *lifecycle
)
{
    size_t i;
    size_t j;

    if (lifecycle == NULL || lifecycle->hook_count < 2) {
        return;
    }

    for (i = 0; i < lifecycle->hook_count - 1; ++i) {
        for (j = i + 1; j < lifecycle->hook_count; ++j) {
            if (lifecycle->hooks[j].priority >
                lifecycle->hooks[i].priority) {
                SpeedPyLifecycleHook temporary =
                    lifecycle->hooks[i];

                lifecycle->hooks[i] =
                    lifecycle->hooks[j];

                lifecycle->hooks[j] =
                    temporary;
            }
        }
    }
}

static int speedpy_lifecycle_invoke(
    SpeedPyLifecycle *lifecycle,
    SpeedPyLifecycleHookType type
)
{
    size_t i;
    int result;

    if (lifecycle == NULL) {
        return -1;
    }

    for (i = 0; i < lifecycle->hook_count; ++i) {
        SpeedPyLifecycleHook *hook = &lifecycle->hooks[i];

        if (!hook->enabled || hook->type != type) {
            continue;
        }

        hook->invocation_count++;

        if (hook->callback == NULL) {
            continue;
        }

        result = hook->callback(
            type,
            lifecycle->state,
            hook->user_data
        );

        if (result != 0) {
            hook->failure_count++;
            lifecycle->failure_count++;

            if (hook->once) {
                hook->enabled = false;
            }

            return result;
        }

        if (hook->once) {
            hook->enabled = false;
        }
    }

    return 0;
}

SpeedPyLifecycle *speedpy_lifecycle_create(
    const char *name
)
{
    SpeedPyLifecycle *lifecycle;

    if (!speedpy_lifecycle_valid_name(name)) {
        return NULL;
    }

    lifecycle = (SpeedPyLifecycle *)calloc(
        1,
        sizeof(SpeedPyLifecycle)
    );

    if (lifecycle == NULL) {
        return NULL;
    }

    strncpy(
        lifecycle->name,
        name,
        SPEEDPY_LIFECYCLE_NAME_SIZE - 1
    );

    lifecycle->name[SPEEDPY_LIFECYCLE_NAME_SIZE - 1] = '\0';

    lifecycle->state = SPEEDPY_LIFECYCLE_CREATED;
    lifecycle->next_hook_id = 1;
    lifecycle->generation = 1;

    return lifecycle;
}

void speedpy_lifecycle_destroy(
    SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL) {
        return;
    }

    free(lifecycle);
}

int speedpy_lifecycle_add_hook(
    SpeedPyLifecycle *lifecycle,
    const char *name,
    SpeedPyLifecycleHookType type,
    SpeedPyLifecycleCallback callback,
    int32_t priority,
    bool once,
    void *user_data
)
{
    SpeedPyLifecycleHook *hook;

    if (lifecycle == NULL ||
        lifecycle->locked ||
        !speedpy_lifecycle_valid_name(name) ||
        !speedpy_lifecycle_valid_hook_type(type) ||
        lifecycle->hook_count >= SPEEDPY_LIFECYCLE_MAX_HOOKS) {
        return -1;
    }

    if (speedpy_lifecycle_find_hook(lifecycle, name) != NULL) {
        return -2;
    }

    hook = &lifecycle->hooks[lifecycle->hook_count];

    memset(hook, 0, sizeof(SpeedPyLifecycleHook));

    strncpy(
        hook->name,
        name,
        SPEEDPY_LIFECYCLE_NAME_SIZE - 1
    );

    hook->name[SPEEDPY_LIFECYCLE_NAME_SIZE - 1] = '\0';

    hook->id = lifecycle->next_hook_id++;
    hook->type = type;
    hook->callback = callback;
    hook->priority = priority;
    hook->enabled = true;
    hook->once = once;
    hook->user_data = user_data;

    lifecycle->hook_count++;
    lifecycle->generation++;

    speedpy_lifecycle_sort_hooks(lifecycle);

    return 0;
}

int speedpy_lifecycle_remove_hook(
    SpeedPyLifecycle *lifecycle,
    const char *name
)
{
    size_t i;
    SpeedPyLifecycleHook *hook;

    if (lifecycle == NULL ||
        lifecycle->locked ||
        name == NULL) {
        return -1;
    }

    hook = speedpy_lifecycle_find_hook(lifecycle, name);

    if (hook == NULL) {
        return -2;
    }

    i = (size_t)(hook - lifecycle->hooks);

    if (i + 1 < lifecycle->hook_count) {
        memmove(
            &lifecycle->hooks[i],
            &lifecycle->hooks[i + 1],
            (lifecycle->hook_count - i - 1) *
                sizeof(SpeedPyLifecycleHook)
        );
    }

    lifecycle->hook_count--;
    lifecycle->generation++;

    return 0;
}

int speedpy_lifecycle_enable_hook(
    SpeedPyLifecycle *lifecycle,
    const char *name
)
{
    SpeedPyLifecycleHook *hook;

    if (lifecycle == NULL || lifecycle->locked) {
        return -1;
    }

    hook = speedpy_lifecycle_find_hook(lifecycle, name);

    if (hook == NULL) {
        return -2;
    }

    hook->enabled = true;
    lifecycle->generation++;

    return 0;
}

int speedpy_lifecycle_disable_hook(
    SpeedPyLifecycle *lifecycle,
    const char *name
)
{
    SpeedPyLifecycleHook *hook;

    if (lifecycle == NULL || lifecycle->locked) {
        return -1;
    }

    hook = speedpy_lifecycle_find_hook(lifecycle, name);

    if (hook == NULL) {
        return -2;
    }

    hook->enabled = false;
    lifecycle->generation++;

    return 0;
}

SpeedPyLifecycleHook *speedpy_lifecycle_get_hook(
    SpeedPyLifecycle *lifecycle,
    const char *name
)
{
    return speedpy_lifecycle_find_hook(lifecycle, name);
}

SpeedPyLifecycleHook *speedpy_lifecycle_get_hook_id(
    SpeedPyLifecycle *lifecycle,
    uint32_t id
)
{
    return speedpy_lifecycle_find_hook_id(lifecycle, id);
}

int speedpy_lifecycle_transition(
    SpeedPyLifecycle *lifecycle,
    SpeedPyLifecycleState next
)
{
    int result;

    if (lifecycle == NULL || lifecycle->locked) {
        return -1;
    }

    if (!speedpy_lifecycle_transition_allowed(
            lifecycle->state,
            next)) {
        return -2;
    }

    lifecycle->state = next;
    lifecycle->transition_count++;
    lifecycle->generation++;

    switch (next) {
        case SPEEDPY_LIFECYCLE_INITIALIZING:
            result = speedpy_lifecycle_invoke(
                lifecycle,
                SPEEDPY_HOOK_INITIALIZE
            );
            break;

        case SPEEDPY_LIFECYCLE_STARTING:
            result = speedpy_lifecycle_invoke(
                lifecycle,
                SPEEDPY_HOOK_START
            );
            break;

        case SPEEDPY_LIFECYCLE_PAUSING:
            result = speedpy_lifecycle_invoke(
                lifecycle,
                SPEEDPY_HOOK_PAUSE
            );
            break;

        case SPEEDPY_LIFECYCLE_STOPPING:
            result = speedpy_lifecycle_invoke(
                lifecycle,
                SPEEDPY_HOOK_STOP
            );
            break;

        case SPEEDPY_LIFECYCLE_SHUTTING_DOWN:
            result = speedpy_lifecycle_invoke(
                lifecycle,
                SPEEDPY_HOOK_SHUTDOWN
            );
            break;

        default:
            result = 0;
            break;
    }

    if (result != 0) {
        lifecycle->state = SPEEDPY_LIFECYCLE_FAILED;
        lifecycle->failure_count++;
        lifecycle->generation++;
        return result;
    }

    if (next == SPEEDPY_LIFECYCLE_PAUSED) {
        result = speedpy_lifecycle_invoke(
            lifecycle,
            SPEEDPY_HOOK_PAUSE
        );

        if (result != 0) {
            lifecycle->state = SPEEDPY_LIFECYCLE_FAILED;
            lifecycle->failure_count++;
            lifecycle->generation++;
            return result;
        }
    }

    if (next == SPEEDPY_LIFECYCLE_RUNNING) {
        result = speedpy_lifecycle_invoke(
            lifecycle,
            SPEEDPY_HOOK_RESUME
        );

        if (result != 0) {
            lifecycle->state = SPEEDPY_LIFECYCLE_FAILED;
            lifecycle->failure_count++;
            lifecycle->generation++;
            return result;
        }
    }

    return 0;
}

int speedpy_lifecycle_initialize(
    SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL) {
        return -1;
    }

    return speedpy_lifecycle_transition(
        lifecycle,
        SPEEDPY_LIFECYCLE_INITIALIZING
    );
}

int speedpy_lifecycle_mark_initialized(
    SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL) {
        return -1;
    }

    return speedpy_lifecycle_transition(
        lifecycle,
        SPEEDPY_LIFECYCLE_INITIALIZED
    );
}

int speedpy_lifecycle_start(
    SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL) {
        return -1;
    }

    return speedpy_lifecycle_transition(
        lifecycle,
        SPEEDPY_LIFECYCLE_STARTING
    );
}

int speedpy_lifecycle_mark_running(
    SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL) {
        return -1;
    }

    return speedpy_lifecycle_transition(
        lifecycle,
        SPEEDPY_LIFECYCLE_RUNNING
    );
}

int speedpy_lifecycle_pause(
    SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL) {
        return -1;
    }

    return speedpy_lifecycle_transition(
        lifecycle,
        SPEEDPY_LIFECYCLE_PAUSING
    );
}

int speedpy_lifecycle_mark_paused(
    SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL) {
        return -1;
    }

    return speedpy_lifecycle_transition(
        lifecycle,
        SPEEDPY_LIFECYCLE_PAUSED
    );
}

int speedpy_lifecycle_resume(
    SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL) {
        return -1;
    }

    return speedpy_lifecycle_transition(
        lifecycle,
        SPEEDPY_LIFECYCLE_STARTING
    );
}

int speedpy_lifecycle_stop(
    SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL) {
        return -1;
    }

    return speedpy_lifecycle_transition(
        lifecycle,
        SPEEDPY_LIFECYCLE_STOPPING
    );
}

int speedpy_lifecycle_mark_stopped(
    SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL) {
        return -1;
    }

    return speedpy_lifecycle_transition(
        lifecycle,
        SPEEDPY_LIFECYCLE_STOPPED
    );
}

int speedpy_lifecycle_shutdown(
    SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL) {
        return -1;
    }

    return speedpy_lifecycle_transition(
        lifecycle,
        SPEEDPY_LIFECYCLE_SHUTTING_DOWN
    );
}

int speedpy_lifecycle_destroy_state(
    SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL) {
        return -1;
    }

    return speedpy_lifecycle_transition(
        lifecycle,
        SPEEDPY_LIFECYCLE_DESTROYED
    );
}

int speedpy_lifecycle_fail(
    SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL || lifecycle->locked) {
        return -1;
    }

    lifecycle->state = SPEEDPY_LIFECYCLE_FAILED;
    lifecycle->failure_count++;
    lifecycle->transition_count++;
    lifecycle->generation++;

    return 0;
}

SpeedPyLifecycleState speedpy_lifecycle_state(
    const SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL) {
        return SPEEDPY_LIFECYCLE_FAILED;
    }

    return lifecycle->state;
}

size_t speedpy_lifecycle_hook_count(
    const SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL) {
        return 0;
    }

    return lifecycle->hook_count;
}

uint64_t speedpy_lifecycle_transition_count(
    const SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL) {
        return 0;
    }

    return lifecycle->transition_count;
}

uint64_t speedpy_lifecycle_failure_count(
    const SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL) {
        return 0;
    }

    return lifecycle->failure_count;
}

uint64_t speedpy_lifecycle_generation(
    const SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL) {
        return 0;
    }

    return lifecycle->generation;
}

const char *speedpy_lifecycle_name(
    const SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL) {
        return NULL;
    }

    return lifecycle->name;
}

void speedpy_lifecycle_lock(
    SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle != NULL) {
        lifecycle->locked = true;
    }
}

void speedpy_lifecycle_unlock(
    SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle != NULL) {
        lifecycle->locked = false;
    }
}

bool speedpy_lifecycle_is_locked(
    const SpeedPyLifecycle *lifecycle
)
{
    return lifecycle != NULL && lifecycle->locked;
}

void speedpy_lifecycle_set_user_data(
    SpeedPyLifecycle *lifecycle,
    void *user_data
)
{
    if (lifecycle != NULL) {
        lifecycle->user_data = user_data;
    }
}

void *speedpy_lifecycle_get_user_data(
    const SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL) {
        return NULL;
    }

    return lifecycle->user_data;
}

void speedpy_lifecycle_reset(
    SpeedPyLifecycle *lifecycle
)
{
    if (lifecycle == NULL || lifecycle->locked) {
        return;
    }

    memset(
        lifecycle->hooks,
        0,
        sizeof(lifecycle->hooks)
    );

    lifecycle->state = SPEEDPY_LIFECYCLE_CREATED;
    lifecycle->hook_count = 0;
    lifecycle->next_hook_id = 1;
    lifecycle->transition_count = 0;
    lifecycle->failure_count = 0;
    lifecycle->generation++;

    lifecycle->locked = false;
}