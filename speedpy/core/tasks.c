#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define SPEEDPY_TASK_MAX 1024
#define SPEEDPY_TASK_NAME_SIZE 96
#define SPEEDPY_TASK_TAG_SIZE 64

typedef enum {
    SPEEDPY_TASK_CREATED = 0,
    SPEEDPY_TASK_QUEUED = 1,
    SPEEDPY_TASK_RUNNING = 2,
    SPEEDPY_TASK_PAUSED = 3,
    SPEEDPY_TASK_COMPLETED = 4,
    SPEEDPY_TASK_CANCELLED = 5,
    SPEEDPY_TASK_FAILED = 6
} SpeedPyTaskState;

typedef enum {
    SPEEDPY_TASK_PRIORITY_LOW = 0,
    SPEEDPY_TASK_PRIORITY_NORMAL = 1,
    SPEEDPY_TASK_PRIORITY_HIGH = 2,
    SPEEDPY_TASK_PRIORITY_CRITICAL = 3
} SpeedPyTaskPriority;

typedef struct {
    uint32_t id;
    char name[SPEEDPY_TASK_NAME_SIZE];
    char tag[SPEEDPY_TASK_TAG_SIZE];

    SpeedPyTaskState state;
    uint32_t priority;

    uint64_t created_tick;
    uint64_t queued_tick;
    uint64_t started_tick;
    uint64_t finished_tick;

    uint64_t run_count;
    uint64_t retry_count;
    uint64_t failure_count;

    uint32_t max_retries;
    uint32_t retries_used;

    bool enabled;
    bool cancellable;
    bool retry_enabled;

    void *user_data;
} SpeedPyTask;

typedef struct {
    char name[SPEEDPY_TASK_NAME_SIZE];

    SpeedPyTask tasks[SPEEDPY_TASK_MAX];

    size_t task_count;
    uint32_t next_task_id;

    uint64_t generation;

    uint64_t created_count;
    uint64_t queued_count;
    uint64_t completed_count;
    uint64_t cancelled_count;
    uint64_t failed_count;

    bool accepting;
    bool locked;

    void *user_data;
} SpeedPyTaskManager;

static bool speedpy_task_valid_name(const char *name)
{
    return name != NULL && name[0] != '\0';
}

static SpeedPyTask *speedpy_task_find(
    SpeedPyTaskManager *manager,
    const char *name
)
{
    size_t i;

    if (manager == NULL || name == NULL) {
        return NULL;
    }

    for (i = 0; i < manager->task_count; ++i) {
        if (strcmp(manager->tasks[i].name, name) == 0) {
            return &manager->tasks[i];
        }
    }

    return NULL;
}

static SpeedPyTask *speedpy_task_find_id(
    SpeedPyTaskManager *manager,
    uint32_t id
)
{
    size_t i;

    if (manager == NULL || id == 0) {
        return NULL;
    }

    for (i = 0; i < manager->task_count; ++i) {
        if (manager->tasks[i].id == id) {
            return &manager->tasks[i];
        }
    }

    return NULL;
}

static void speedpy_task_remove_at(
    SpeedPyTaskManager *manager,
    size_t index
)
{
    if (manager == NULL || index >= manager->task_count) {
        return;
    }

    if (index + 1 < manager->task_count) {
        memmove(
            &manager->tasks[index],
            &manager->tasks[index + 1],
            (manager->task_count - index - 1) *
                sizeof(SpeedPyTask)
        );
    }

    manager->task_count--;
    manager->generation++;
}

static void speedpy_task_sort(
    SpeedPyTaskManager *manager
)
{
    size_t i;
    size_t j;

    if (manager == NULL || manager->task_count < 2) {
        return;
    }

    for (i = 0; i < manager->task_count - 1; ++i) {
        for (j = i + 1; j < manager->task_count; ++j) {
            SpeedPyTask *a = &manager->tasks[i];
            SpeedPyTask *b = &manager->tasks[j];

            if (b->priority > a->priority ||
                (b->priority == a->priority &&
                 b->created_tick < a->created_tick)) {
                SpeedPyTask temporary = *a;
                *a = *b;
                *b = temporary;
            }
        }
    }
}

SpeedPyTaskManager *speedpy_tasks_create(
    const char *name
)
{
    SpeedPyTaskManager *manager;

    if (!speedpy_task_valid_name(name)) {
        return NULL;
    }

    manager = (SpeedPyTaskManager *)calloc(
        1,
        sizeof(SpeedPyTaskManager)
    );

    if (manager == NULL) {
        return NULL;
    }

    strncpy(
        manager->name,
        name,
        SPEEDPY_TASK_NAME_SIZE - 1
    );

    manager->name[SPEEDPY_TASK_NAME_SIZE - 1] = '\0';

    manager->next_task_id = 1;
    manager->accepting = true;
    manager->generation = 1;

    return manager;
}

void speedpy_tasks_destroy(
    SpeedPyTaskManager *manager
)
{
    if (manager == NULL) {
        return;
    }

    free(manager);
}

int speedpy_tasks_create_task(
    SpeedPyTaskManager *manager,
    const char *name,
    uint32_t priority,
    uint64_t created_tick,
    void *user_data
)
{
    SpeedPyTask *task;

    if (manager == NULL ||
        manager->locked ||
        !manager->accepting ||
        !speedpy_task_valid_name(name)) {
        return -1;
    }

    if (manager->task_count >= SPEEDPY_TASK_MAX) {
        return -2;
    }

    if (priority > SPEEDPY_TASK_PRIORITY_CRITICAL) {
        return -3;
    }

    if (speedpy_task_find(manager, name) != NULL) {
        return -4;
    }

    task = &manager->tasks[manager->task_count];

    memset(task, 0, sizeof(SpeedPyTask));

    strncpy(
        task->name,
        name,
        SPEEDPY_TASK_NAME_SIZE - 1
    );

    task->name[SPEEDPY_TASK_NAME_SIZE - 1] = '\0';

    task->id = manager->next_task_id++;
    task->state = SPEEDPY_TASK_CREATED;
    task->priority = priority;
    task->created_tick = created_tick;
    task->enabled = true;
    task->cancellable = true;
    task->retry_enabled = false;
    task->user_data = user_data;

    manager->task_count++;
    manager->created_count++;
    manager->generation++;

    speedpy_task_sort(manager);

    return 0;
}

int speedpy_tasks_destroy_task(
    SpeedPyTaskManager *manager,
    const char *name
)
{
    size_t i;
    SpeedPyTask *task;

    if (manager == NULL ||
        manager->locked ||
        name == NULL) {
        return -1;
    }

    task = speedpy_task_find(manager, name);

    if (task == NULL) {
        return -2;
    }

    if (task->state == SPEEDPY_TASK_RUNNING) {
        return -3;
    }

    i = (size_t)(task - manager->tasks);

    speedpy_task_remove_at(manager, i);

    return 0;
}

int speedpy_tasks_queue(
    SpeedPyTaskManager *manager,
    const char *name,
    uint64_t tick
)
{
    SpeedPyTask *task;

    if (manager == NULL || name == NULL) {
        return -1;
    }

    task = speedpy_task_find(manager, name);

    if (task == NULL) {
        return -2;
    }

    if (!task->enabled) {
        return -3;
    }

    if (task->state != SPEEDPY_TASK_CREATED &&
        task->state != SPEEDPY_TASK_PAUSED) {
        return -4;
    }

    task->state = SPEEDPY_TASK_QUEUED;
    task->queued_tick = tick;

    manager->queued_count++;
    manager->generation++;

    return 0;
}

int speedpy_tasks_start(
    SpeedPyTaskManager *manager,
    const char *name,
    uint64_t tick
)
{
    SpeedPyTask *task;

    if (manager == NULL || name == NULL) {
        return -1;
    }

    task = speedpy_task_find(manager, name);

    if (task == NULL) {
        return -2;
    }

    if (!task->enabled ||
        task->state != SPEEDPY_TASK_QUEUED) {
        return -3;
    }

    task->state = SPEEDPY_TASK_RUNNING;
    task->started_tick = tick;
    task->run_count++;

    manager->generation++;

    return 0;
}

int speedpy_tasks_complete(
    SpeedPyTaskManager *manager,
    const char *name,
    uint64_t tick
)
{
    SpeedPyTask *task;

    if (manager == NULL || name == NULL) {
        return -1;
    }

    task = speedpy_task_find(manager, name);

    if (task == NULL) {
        return -2;
    }

    if (task->state != SPEEDPY_TASK_RUNNING) {
        return -3;
    }

    task->state = SPEEDPY_TASK_COMPLETED;
    task->finished_tick = tick;

    manager->completed_count++;
    manager->generation++;

    return 0;
}

int speedpy_tasks_fail(
    SpeedPyTaskManager *manager,
    const char *name,
    uint64_t tick
)
{
    SpeedPyTask *task;

    if (manager == NULL || name == NULL) {
        return -1;
    }

    task = speedpy_task_find(manager, name);

    if (task == NULL) {
        return -2;
    }

    if (task->state != SPEEDPY_TASK_RUNNING) {
        return -3;
    }

    task->failure_count++;
    task->finished_tick = tick;

    if (task->retry_enabled &&
        task->retries_used < task->max_retries) {
        task->retries_used++;
        task->retry_count++;
        task->state = SPEEDPY_TASK_QUEUED;
    } else {
        task->state = SPEEDPY_TASK_FAILED;
        manager->failed_count++;
    }

    manager->generation++;

    return 0;
}

int speedpy_tasks_cancel(
    SpeedPyTaskManager *manager,
    const char *name
)
{
    SpeedPyTask *task;

    if (manager == NULL || name == NULL) {
        return -1;
    }

    task = speedpy_task_find(manager, name);

    if (task == NULL) {
        return -2;
    }

    if (!task->cancellable ||
        task->state == SPEEDPY_TASK_COMPLETED ||
        task->state == SPEEDPY_TASK_FAILED) {
        return -3;
    }

    task->state = SPEEDPY_TASK_CANCELLED;
    task->enabled = false;

    manager->cancelled_count++;
    manager->generation++;

    return 0;
}

int speedpy_tasks_pause(
    SpeedPyTaskManager *manager,
    const char *name
)
{
    SpeedPyTask *task;

    if (manager == NULL || name == NULL) {
        return -1;
    }

    task = speedpy_task_find(manager, name);

    if (task == NULL) {
        return -2;
    }

    if (task->state != SPEEDPY_TASK_RUNNING &&
        task->state != SPEEDPY_TASK_QUEUED) {
        return -3;
    }

    task->state = SPEEDPY_TASK_PAUSED;
    manager->generation++;

    return 0;
}

int speedpy_tasks_resume(
    SpeedPyTaskManager *manager,
    const char *name
)
{
    SpeedPyTask *task;

    if (manager == NULL || name == NULL) {
        return -1;
    }

    task = speedpy_task_find(manager, name);

    if (task == NULL) {
        return -2;
    }

    if (task->state != SPEEDPY_TASK_PAUSED) {
        return -3;
    }

    task->state = SPEEDPY_TASK_QUEUED;
    task->enabled = true;

    manager->generation++;

    return 0;
}

int speedpy_tasks_enable(
    SpeedPyTaskManager *manager,
    const char *name
)
{
    SpeedPyTask *task;

    if (manager == NULL || manager->locked) {
        return -1;
    }

    task = speedpy_task_find(manager, name);

    if (task == NULL) {
        return -2;
    }

    if (task->state == SPEEDPY_TASK_CANCELLED) {
        return -3;
    }

    task->enabled = true;
    manager->generation++;

    return 0;
}

int speedpy_tasks_disable(
    SpeedPyTaskManager *manager,
    const char *name
)
{
    SpeedPyTask *task;

    if (manager == NULL || manager->locked) {
        return -1;
    }

    task = speedpy_task_find(manager, name);

    if (task == NULL) {
        return -2;
    }

    task->enabled = false;

    if (task->state == SPEEDPY_TASK_QUEUED) {
        task->state = SPEEDPY_TASK_PAUSED;
    }

    manager->generation++;

    return 0;
}

int speedpy_tasks_set_retry(
    SpeedPyTaskManager *manager,
    const char *name,
    bool enabled,
    uint32_t max_retries
)
{
    SpeedPyTask *task;

    if (manager == NULL ||
        manager->locked ||
        name == NULL) {
        return -1;
    }

    task = speedpy_task_find(manager, name);

    if (task == NULL) {
        return -2;
    }

    task->retry_enabled = enabled;
    task->max_retries = max_retries;

    if (!enabled) {
        task->retries_used = 0;
    }

    manager->generation++;

    return 0;
}

int speedpy_tasks_set_tag(
    SpeedPyTaskManager *manager,
    const char *name,
    const char *tag
)
{
    SpeedPyTask *task;

    if (manager == NULL ||
        manager->locked ||
        name == NULL ||
        tag == NULL) {
        return -1;
    }

    task = speedpy_task_find(manager, name);

    if (task == NULL) {
        return -2;
    }

    strncpy(
        task->tag,
        tag,
        SPEEDPY_TASK_TAG_SIZE - 1
    );

    task->tag[SPEEDPY_TASK_TAG_SIZE - 1] = '\0';

    manager->generation++;

    return 0;
}

SpeedPyTask *speedpy_tasks_get(
    SpeedPyTaskManager *manager,
    const char *name
)
{
    return speedpy_task_find(manager, name);
}

SpeedPyTask *speedpy_tasks_get_id(
    SpeedPyTaskManager *manager,
    uint32_t id
)
{
    return speedpy_task_find_id(manager, id);
}

size_t speedpy_tasks_count(
    const SpeedPyTaskManager *manager
)
{
    if (manager == NULL) {
        return 0;
    }

    return manager->task_count;
}

size_t speedpy_tasks_count_state(
    const SpeedPyTaskManager *manager,
    SpeedPyTaskState state
)
{
    size_t i;
    size_t count = 0;

    if (manager == NULL) {
        return 0;
    }

    for (i = 0; i < manager->task_count; ++i) {
        if (manager->tasks[i].state == state) {
            count++;
        }
    }

    return count;
}

void speedpy_tasks_set_accepting(
    SpeedPyTaskManager *manager,
    bool accepting
)
{
    if (manager != NULL && !manager->locked) {
        manager->accepting = accepting;
        manager->generation++;
    }
}

bool speedpy_tasks_is_accepting(
    const SpeedPyTaskManager *manager
)
{
    return manager != NULL && manager->accepting;
}

uint64_t speedpy_tasks_generation(
    const SpeedPyTaskManager *manager
)
{
    if (manager == NULL) {
        return 0;
    }

    return manager->generation;
}

uint64_t speedpy_tasks_created_count(
    const SpeedPyTaskManager *manager
)
{
    if (manager == NULL) {
        return 0;
    }

    return manager->created_count;
}

uint64_t speedpy_tasks_completed_count(
    const SpeedPyTaskManager *manager
)
{
    if (manager == NULL) {
        return 0;
    }

    return manager->completed_count;
}

uint64_t speedpy_tasks_failed_count(
    const SpeedPyTaskManager *manager
)
{
    if (manager == NULL) {
        return 0;
    }

    return manager->failed_count;
}

uint64_t speedpy_tasks_cancelled_count(
    const SpeedPyTaskManager *manager
)
{
    if (manager == NULL) {
        return 0;
    }

    return manager->cancelled_count;
}

void speedpy_tasks_lock(
    SpeedPyTaskManager *manager
)
{
    if (manager != NULL) {
        manager->locked = true;
    }
}

void speedpy_tasks_unlock(
    SpeedPyTaskManager *manager
)
{
    if (manager != NULL) {
        manager->locked = false;
    }
}

bool speedpy_tasks_is_locked(
    const SpeedPyTaskManager *manager
)
{
    return manager != NULL && manager->locked;
}

void speedpy_tasks_set_user_data(
    SpeedPyTaskManager *manager,
    void *user_data
)
{
    if (manager != NULL) {
        manager->user_data = user_data;
    }
}

void *speedpy_tasks_get_user_data(
    const SpeedPyTaskManager *manager
)
{
    if (manager == NULL) {
        return NULL;
    }

    return manager->user_data;
}

const char *speedpy_tasks_name(
    const SpeedPyTaskManager *manager
)
{
    if (manager == NULL) {
        return NULL;
    }

    return manager->name;
}

void speedpy_tasks_reset(
    SpeedPyTaskManager *manager
)
{
    if (manager == NULL || manager->locked) {
        return;
    }

    memset(
        manager->tasks,
        0,
        sizeof(manager->tasks)
    );

    manager->task_count = 0;
    manager->next_task_id = 1;
    manager->created_count = 0;
    manager->queued_count = 0;
    manager->completed_count = 0;
    manager->cancelled_count = 0;
    manager->failed_count = 0;

    manager->generation++;
}