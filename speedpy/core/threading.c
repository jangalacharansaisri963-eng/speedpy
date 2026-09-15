#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define SPEEDPY_THREAD_MAX 256
#define SPEEDPY_THREAD_NAME_SIZE 96
#define SPEEDPY_THREAD_DEFAULT_PRIORITY 0

typedef uint64_t SpeedPyThreadId;

typedef enum {
    SPEEDPY_THREAD_CREATED = 0,
    SPEEDPY_THREAD_READY,
    SPEEDPY_THREAD_RUNNING,
    SPEEDPY_THREAD_PAUSED,
    SPEEDPY_THREAD_STOPPING,
    SPEEDPY_THREAD_STOPPED,
    SPEEDPY_THREAD_FAILED,
    SPEEDPY_THREAD_DESTROYED
} SpeedPyThreadState;

typedef enum {
    SPEEDPY_THREAD_PRIORITY_LOW = -2,
    SPEEDPY_THREAD_PRIORITY_BELOW_NORMAL = -1,
    SPEEDPY_THREAD_PRIORITY_NORMAL = 0,
    SPEEDPY_THREAD_PRIORITY_ABOVE_NORMAL = 1,
    SPEEDPY_THREAD_PRIORITY_HIGH = 2
} SpeedPyThreadPriority;

typedef void (*SpeedPyThreadEntry)(
    void *user_data
);

typedef struct {
    SpeedPyThreadId id;
    char name[SPEEDPY_THREAD_NAME_SIZE];
    SpeedPyThreadState state;
    SpeedPyThreadPriority priority;

    SpeedPyThreadEntry entry;
    void *user_data;

    uint64_t created_tick;
    uint64_t started_tick;
    uint64_t stopped_tick;

    uint64_t run_count;
    uint64_t update_count;
    uint64_t failure_count;

    uint32_t processor_hint;

    bool enabled;
    bool detached;
    bool stop_requested;
} SpeedPyThread;

typedef struct {
    SpeedPyThread threads[SPEEDPY_THREAD_MAX];

    size_t count;
    SpeedPyThreadId next_id;

    uint64_t current_tick;
    uint64_t generation;

    uint64_t total_started;
    uint64_t total_stopped;
    uint64_t total_failed;

    bool accepting;
    bool locked;

    void *user_data;
} SpeedPyThreadManager;

static bool speedpy_thread_manager_valid(
    const SpeedPyThreadManager *manager
) {
    return manager != NULL &&
           manager->count <= SPEEDPY_THREAD_MAX;
}

static SpeedPyThread *speedpy_thread_find(
    SpeedPyThreadManager *manager,
    SpeedPyThreadId id
) {
    if (!speedpy_thread_manager_valid(manager) ||
        id == 0) {
        return NULL;
    }

    for (size_t i = 0; i < SPEEDPY_THREAD_MAX; i++) {
        if (manager->threads[i].id == id &&
            manager->threads[i].state !=
                SPEEDPY_THREAD_DESTROYED) {
            return &manager->threads[i];
        }
    }

    return NULL;
}

static const SpeedPyThread *speedpy_thread_find_const(
    const SpeedPyThreadManager *manager,
    SpeedPyThreadId id
) {
    if (!speedpy_thread_manager_valid(manager) ||
        id == 0) {
        return NULL;
    }

    for (size_t i = 0; i < SPEEDPY_THREAD_MAX; i++) {
        if (manager->threads[i].id == id &&
            manager->threads[i].state !=
                SPEEDPY_THREAD_DESTROYED) {
            return &manager->threads[i];
        }
    }

    return NULL;
}

static SpeedPyThread *speedpy_thread_find_free_slot(
    SpeedPyThreadManager *manager
) {
    if (!speedpy_thread_manager_valid(manager)) {
        return NULL;
    }

    for (size_t i = 0; i < SPEEDPY_THREAD_MAX; i++) {
        if (manager->threads[i].id == 0 ||
            manager->threads[i].state ==
                SPEEDPY_THREAD_DESTROYED) {
            return &manager->threads[i];
        }
    }

    return NULL;
}

static bool speedpy_thread_valid_transition(
    SpeedPyThreadState from,
    SpeedPyThreadState to
) {
    if (from == to) {
        return true;
    }

    switch (from) {
        case SPEEDPY_THREAD_CREATED:
            return to == SPEEDPY_THREAD_READY ||
                   to == SPEEDPY_THREAD_DESTROYED;

        case SPEEDPY_THREAD_READY:
            return to == SPEEDPY_THREAD_RUNNING ||
                   to == SPEEDPY_THREAD_STOPPING ||
                   to == SPEEDPY_THREAD_DESTROYED;

        case SPEEDPY_THREAD_RUNNING:
            return to == SPEEDPY_THREAD_PAUSED ||
                   to == SPEEDPY_THREAD_STOPPING ||
                   to == SPEEDPY_THREAD_FAILED;

        case SPEEDPY_THREAD_PAUSED:
            return to == SPEEDPY_THREAD_RUNNING ||
                   to == SPEEDPY_THREAD_STOPPING ||
                   to == SPEEDPY_THREAD_FAILED;

        case SPEEDPY_THREAD_STOPPING:
            return to == SPEEDPY_THREAD_STOPPED ||
                   to == SPEEDPY_THREAD_FAILED;

        case SPEEDPY_THREAD_STOPPED:
            return to == SPEEDPY_THREAD_READY ||
                   to == SPEEDPY_THREAD_DESTROYED;

        case SPEEDPY_THREAD_FAILED:
            return to == SPEEDPY_THREAD_STOPPED ||
                   to == SPEEDPY_THREAD_DESTROYED;

        case SPEEDPY_THREAD_DESTROYED:
            return false;

        default:
            return false;
    }
}

SpeedPyThreadManager *speedpy_thread_manager_create(void) {
    SpeedPyThreadManager *manager =
        (SpeedPyThreadManager *)calloc(
            1,
            sizeof(SpeedPyThreadManager)
        );

    if (manager == NULL) {
        return NULL;
    }

    manager->next_id = 1;
    manager->generation = 1;
    manager->accepting = true;

    return manager;
}

void speedpy_thread_manager_destroy(
    SpeedPyThreadManager *manager
) {
    if (manager == NULL) {
        return;
    }

    free(manager);
}

SpeedPyThreadId speedpy_thread_create(
    SpeedPyThreadManager *manager,
    const char *name,
    SpeedPyThreadEntry entry,
    void *user_data
) {
    SpeedPyThread *thread;
    SpeedPyThreadId id;

    if (!speedpy_thread_manager_valid(manager) ||
        manager->locked ||
        !manager->accepting ||
        entry == NULL ||
        manager->count >= SPEEDPY_THREAD_MAX) {
        return 0;
    }

    thread = speedpy_thread_find_free_slot(manager);

    if (thread == NULL) {
        return 0;
    }

    memset(thread, 0, sizeof(SpeedPyThread));

    id = manager->next_id++;

    if (id == 0) {
        id = manager->next_id++;
    }

    thread->id = id;
    thread->state = SPEEDPY_THREAD_CREATED;
    thread->priority =
        SPEEDPY_THREAD_PRIORITY_NORMAL;
    thread->entry = entry;
    thread->user_data = user_data;
    thread->enabled = true;

    if (name != NULL) {
        strncpy(
            thread->name,
            name,
            SPEEDPY_THREAD_NAME_SIZE - 1
        );

        thread->name[
            SPEEDPY_THREAD_NAME_SIZE - 1
        ] = '\0';
    } else {
        strcpy(thread->name, "thread");
    }

    manager->count++;
    manager->generation++;

    return id;
}

bool speedpy_thread_destroy(
    SpeedPyThreadManager *manager,
    SpeedPyThreadId id
) {
    SpeedPyThread *thread;

    if (!speedpy_thread_manager_valid(manager) ||
        manager->locked) {
        return false;
    }

    thread = speedpy_thread_find(manager, id);

    if (thread == NULL) {
        return false;
    }

    if (thread->state == SPEEDPY_THREAD_RUNNING ||
        thread->state == SPEEDPY_THREAD_PAUSED ||
        thread->state == SPEEDPY_THREAD_STOPPING) {
        return false;
    }

    memset(thread, 0, sizeof(SpeedPyThread));
    thread->state = SPEEDPY_THREAD_DESTROYED;

    if (manager->count > 0) {
        manager->count--;
    }

    manager->generation++;

    return true;
}

bool speedpy_thread_prepare(
    SpeedPyThreadManager *manager,
    SpeedPyThreadId id,
    uint64_t current_tick
) {
    SpeedPyThread *thread;

    if (!speedpy_thread_manager_valid(manager) ||
        manager->locked) {
        return false;
    }

    thread = speedpy_thread_find(manager, id);

    if (thread == NULL ||
        !thread->enabled) {
        return false;
    }

    if (!speedpy_thread_valid_transition(
            thread->state,
            SPEEDPY_THREAD_READY)) {
        return false;
    }

    thread->state = SPEEDPY_THREAD_READY;
    thread->created_tick = current_tick;

    manager->current_tick = current_tick;
    manager->generation++;

    return true;
}

bool speedpy_thread_start(
    SpeedPyThreadManager *manager,
    SpeedPyThreadId id,
    uint64_t current_tick
) {
    SpeedPyThread *thread;

    if (!speedpy_thread_manager_valid(manager) ||
        manager->locked) {
        return false;
    }

    thread = speedpy_thread_find(manager, id);

    if (thread == NULL ||
        !thread->enabled) {
        return false;
    }

    if (!speedpy_thread_valid_transition(
            thread->state,
            SPEEDPY_THREAD_RUNNING)) {
        return false;
    }

    thread->state = SPEEDPY_THREAD_RUNNING;
    thread->started_tick = current_tick;
    thread->stop_requested = false;
    thread->run_count++;

    manager->current_tick = current_tick;
    manager->total_started++;
    manager->generation++;

    return true;
}

bool speedpy_thread_run(
    SpeedPyThreadManager *manager,
    SpeedPyThreadId id,
    uint64_t current_tick
) {
    SpeedPyThread *thread;

    if (!speedpy_thread_manager_valid(manager) ||
        manager->locked) {
        return false;
    }

    thread = speedpy_thread_find(manager, id);

    if (thread == NULL ||
        thread->state != SPEEDPY_THREAD_RUNNING ||
        !thread->enabled ||
        thread->entry == NULL) {
        return false;
    }

    manager->current_tick = current_tick;

    thread->update_count++;

    thread->entry(thread->user_data);

    manager->generation++;

    return true;
}

bool speedpy_thread_pause(
    SpeedPyThreadManager *manager,
    SpeedPyThreadId id
) {
    SpeedPyThread *thread;

    if (!speedpy_thread_manager_valid(manager) ||
        manager->locked) {
        return false;
    }

    thread = speedpy_thread_find(manager, id);

    if (thread == NULL ||
        !speedpy_thread_valid_transition(
            thread->state,
            SPEEDPY_THREAD_PAUSED)) {
        return false;
    }

    thread->state = SPEEDPY_THREAD_PAUSED;
    manager->generation++;

    return true;
}

bool speedpy_thread_resume(
    SpeedPyThreadManager *manager,
    SpeedPyThreadId id
) {
    SpeedPyThread *thread;

    if (!speedpy_thread_manager_valid(manager) ||
        manager->locked) {
        return false;
    }

    thread = speedpy_thread_find(manager, id);

    if (thread == NULL ||
        !speedpy_thread_valid_transition(
            thread->state,
            SPEEDPY_THREAD_RUNNING)) {
        return false;
    }

    thread->state = SPEEDPY_THREAD_RUNNING;
    manager->generation++;

    return true;
}

bool speedpy_thread_request_stop(
    SpeedPyThreadManager *manager,
    SpeedPyThreadId id
) {
    SpeedPyThread *thread;

    if (!speedpy_thread_manager_valid(manager) ||
        manager->locked) {
        return false;
    }

    thread = speedpy_thread_find(manager, id);

    if (thread == NULL) {
        return false;
    }

    if (!speedpy_thread_valid_transition(
            thread->state,
            SPEEDPY_THREAD_STOPPING)) {
        return false;
    }

    thread->stop_requested = true;
    thread->state = SPEEDPY_THREAD_STOPPING;

    manager->generation++;

    return true;
}

bool speedpy_thread_finish(
    SpeedPyThreadManager *manager,
    SpeedPyThreadId id,
    uint64_t current_tick
) {
    SpeedPyThread *thread;

    if (!speedpy_thread_manager_valid(manager) ||
        manager->locked) {
        return false;
    }

    thread = speedpy_thread_find(manager, id);

    if (thread == NULL ||
        !speedpy_thread_valid_transition(
            thread->state,
            SPEEDPY_THREAD_STOPPED)) {
        return false;
    }

    thread->state = SPEEDPY_THREAD_STOPPED;
    thread->stopped_tick = current_tick;
    thread->stop_requested = false;

    manager->current_tick = current_tick;
    manager->total_stopped++;
    manager->generation++;

    return true;
}

bool speedpy_thread_fail(
    SpeedPyThreadManager *manager,
    SpeedPyThreadId id
) {
    SpeedPyThread *thread;

    if (!speedpy_thread_manager_valid(manager) ||
        manager->locked) {
        return false;
    }

    thread = speedpy_thread_find(manager, id);

    if (thread == NULL ||
        !speedpy_thread_valid_transition(
            thread->state,
            SPEEDPY_THREAD_FAILED)) {
        return false;
    }

    thread->state = SPEEDPY_THREAD_FAILED;
    thread->failure_count++;

    manager->total_failed++;
    manager->generation++;

    return true;
}

bool speedpy_thread_enable(
    SpeedPyThreadManager *manager,
    SpeedPyThreadId id
) {
    SpeedPyThread *thread;

    if (!speedpy_thread_manager_valid(manager) ||
        manager->locked) {
        return false;
    }

    thread = speedpy_thread_find(manager, id);

    if (thread == NULL) {
        return false;
    }

    thread->enabled = true;
    manager->generation++;

    return true;
}

bool speedpy_thread_disable(
    SpeedPyThreadManager *manager,
    SpeedPyThreadId id
) {
    SpeedPyThread *thread;

    if (!speedpy_thread_manager_valid(manager) ||
        manager->locked) {
        return false;
    }

    thread = speedpy_thread_find(manager, id);

    if (thread == NULL) {
        return false;
    }

    if (thread->state == SPEEDPY_THREAD_RUNNING ||
        thread->state == SPEEDPY_THREAD_PAUSED) {
        return false;
    }

    thread->enabled = false;
    manager->generation++;

    return true;
}

bool speedpy_thread_set_priority(
    SpeedPyThreadManager *manager,
    SpeedPyThreadId id,
    SpeedPyThreadPriority priority
) {
    SpeedPyThread *thread;

    if (!speedpy_thread_manager_valid(manager) ||
        manager->locked) {
        return false;
    }

    thread = speedpy_thread_find(manager, id);

    if (thread == NULL) {
        return false;
    }

    thread->priority = priority;
    manager->generation++;

    return true;
}

SpeedPyThreadPriority speedpy_thread_get_priority(
    const SpeedPyThreadManager *manager,
    SpeedPyThreadId id
) {
    const SpeedPyThread *thread =
        speedpy_thread_find_const(manager, id);

    if (thread == NULL) {
        return SPEEDPY_THREAD_PRIORITY_NORMAL;
    }

    return thread->priority;
}

bool speedpy_thread_set_processor_hint(
    SpeedPyThreadManager *manager,
    SpeedPyThreadId id,
    uint32_t processor_hint
) {
    SpeedPyThread *thread;

    if (!speedpy_thread_manager_valid(manager) ||
        manager->locked) {
        return false;
    }

    thread = speedpy_thread_find(manager, id);

    if (thread == NULL) {
        return false;
    }

    thread->processor_hint = processor_hint;
    manager->generation++;

    return true;
}

uint32_t speedpy_thread_get_processor_hint(
    const SpeedPyThreadManager *manager,
    SpeedPyThreadId id
) {
    const SpeedPyThread *thread =
        speedpy_thread_find_const(manager, id);

    if (thread == NULL) {
        return 0;
    }

    return thread->processor_hint;
}

bool speedpy_thread_set_detached(
    SpeedPyThreadManager *manager,
    SpeedPyThreadId id,
    bool detached
) {
    SpeedPyThread *thread;

    if (!speedpy_thread_manager_valid(manager) ||
        manager->locked) {
        return false;
    }

    thread = speedpy_thread_find(manager, id);

    if (thread == NULL) {
        return false;
    }

    thread->detached = detached;
    manager->generation++;

    return true;
}

bool speedpy_thread_is_stop_requested(
    const SpeedPyThreadManager *manager,
    SpeedPyThreadId id
) {
    const SpeedPyThread *thread =
        speedpy_thread_find_const(manager, id);

    if (thread == NULL) {
        return false;
    }

    return thread->stop_requested;
}

SpeedPyThreadState speedpy_thread_get_state(
    const SpeedPyThreadManager *manager,
    SpeedPyThreadId id
) {
    const SpeedPyThread *thread =
        speedpy_thread_find_const(manager, id);

    if (thread == NULL) {
        return SPEEDPY_THREAD_DESTROYED;
    }

    return thread->state;
}

const char *speedpy_thread_get_name(
    const SpeedPyThreadManager *manager,
    SpeedPyThreadId id
) {
    const SpeedPyThread *thread =
        speedpy_thread_find_const(manager, id);

    if (thread == NULL) {
        return NULL;
    }

    return thread->name;
}

uint64_t speedpy_thread_get_run_count(
    const SpeedPyThreadManager *manager,
    SpeedPyThreadId id
) {
    const SpeedPyThread *thread =
        speedpy_thread_find_const(manager, id);

    if (thread == NULL) {
        return 0;
    }

    return thread->run_count;
}

uint64_t speedpy_thread_get_update_count(
    const SpeedPyThreadManager *manager,
    SpeedPyThreadId id
) {
    const SpeedPyThread *thread =
        speedpy_thread_find_const(manager, id);

    if (thread == NULL) {
        return 0;
    }

    return thread->update_count;
}

uint64_t speedpy_thread_get_failure_count(
    const SpeedPyThreadManager *manager,
    SpeedPyThreadId id
) {
    const SpeedPyThread *thread =
        speedpy_thread_find_const(manager, id);

    if (thread == NULL) {
        return 0;
    }

    return thread->failure_count;
}

size_t speedpy_thread_count(
    const SpeedPyThreadManager *manager
) {
    if (!speedpy_thread_manager_valid(manager)) {
        return 0;
    }

    return manager->count;
}

size_t speedpy_thread_count_state(
    const SpeedPyThreadManager *manager,
    SpeedPyThreadState state
) {
    size_t count = 0;

    if (!speedpy_thread_manager_valid(manager)) {
        return 0;
    }

    for (size_t i = 0; i < SPEEDPY_THREAD_MAX; i++) {
        if (manager->threads[i].id != 0 &&
            manager->threads[i].state == state) {
            count++;
        }
    }

    return count;
}

uint64_t speedpy_thread_current_tick(
    const SpeedPyThreadManager *manager
) {
    if (!speedpy_thread_manager_valid(manager)) {
        return 0;
    }

    return manager->current_tick;
}

uint64_t speedpy_thread_total_started(
    const SpeedPyThreadManager *manager
) {
    if (!speedpy_thread_manager_valid(manager)) {
        return 0;
    }

    return manager->total_started;
}

uint64_t speedpy_thread_total_stopped(
    const SpeedPyThreadManager *manager
) {
    if (!speedpy_thread_manager_valid(manager)) {
        return 0;
    }

    return manager->total_stopped;
}

uint64_t speedpy_thread_total_failed(
    const SpeedPyThreadManager *manager
) {
    if (!speedpy_thread_manager_valid(manager)) {
        return 0;
    }

    return manager->total_failed;
}

uint64_t speedpy_thread_generation(
    const SpeedPyThreadManager *manager
) {
    if (!speedpy_thread_manager_valid(manager)) {
        return 0;
    }

    return manager->generation;
}

void speedpy_thread_set_accepting(
    SpeedPyThreadManager *manager,
    bool accepting
) {
    if (!speedpy_thread_manager_valid(manager)) {
        return;
    }

    manager->accepting = accepting;
    manager->generation++;
}

bool speedpy_thread_is_accepting(
    const SpeedPyThreadManager *manager
) {
    if (!speedpy_thread_manager_valid(manager)) {
        return false;
    }

    return manager->accepting;
}

bool speedpy_thread_lock(
    SpeedPyThreadManager *manager
) {
    if (!speedpy_thread_manager_valid(manager)) {
        return false;
    }

    manager->locked = true;
    return true;
}

bool speedpy_thread_unlock(
    SpeedPyThreadManager *manager
) {
    if (!speedpy_thread_manager_valid(manager)) {
        return false;
    }

    manager->locked = false;
    return true;
}

bool speedpy_thread_is_locked(
    const SpeedPyThreadManager *manager
) {
    if (!speedpy_thread_manager_valid(manager)) {
        return false;
    }

    return manager->locked;
}

void speedpy_thread_set_user_data(
    SpeedPyThreadManager *manager,
    void *user_data
) {
    if (!speedpy_thread_manager_valid(manager)) {
        return;
    }

    manager->user_data = user_data;
}

void *speedpy_thread_get_user_data(
    const SpeedPyThreadManager *manager
) {
    if (!speedpy_thread_manager_valid(manager)) {
        return NULL;
    }

    return manager->user_data;
}