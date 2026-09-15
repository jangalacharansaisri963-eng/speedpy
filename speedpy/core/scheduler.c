#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define SPEEDPY_SCHEDULER_MAX_TASKS 512
#define SPEEDPY_SCHEDULER_NAME_SIZE 96

typedef enum {
    SPEEDPY_SCHEDULED = 0,
    SPEEDPY_READY = 1,
    SPEEDPY_RUNNING = 2,
    SPEEDPY_PAUSED = 3,
    SPEEDPY_COMPLETED = 4,
    SPEEDPY_CANCELLED = 5,
    SPEEDPY_FAILED = 6
} SpeedPyScheduledState;

typedef enum {
    SPEEDPY_PRIORITY_LOW = 0,
    SPEEDPY_PRIORITY_NORMAL = 1,
    SPEEDPY_PRIORITY_HIGH = 2,
    SPEEDPY_PRIORITY_CRITICAL = 3
} SpeedPySchedulePriority;

typedef struct {
    uint32_t id;
    char name[SPEEDPY_SCHEDULER_NAME_SIZE];

    uint64_t execute_at;
    uint64_t interval;
    uint64_t deadline;

    uint32_t priority;

    SpeedPyScheduledState state;

    bool repeat;
    bool enabled;

    uint64_t run_count;
    uint64_t failure_count;
    uint64_t missed_count;

    uint64_t last_run;
    uint64_t next_run;

    void *user_data;
} SpeedPyScheduledTask;

typedef struct {
    char name[SPEEDPY_SCHEDULER_NAME_SIZE];

    SpeedPyScheduledTask tasks[SPEEDPY_SCHEDULER_MAX_TASKS];

    size_t task_count;
    uint32_t next_task_id;

    uint64_t current_tick;

    uint64_t dispatch_count;
    uint64_t completed_count;
    uint64_t cancelled_count;
    uint64_t failed_count;
    uint64_t missed_count;

    uint32_t max_dispatch_per_tick;

    bool running;
    bool paused;
    bool locked;

    uint64_t generation;

    void *user_data;
} SpeedPyScheduler;

static bool speedpy_scheduler_valid_name(const char *name)
{
    return name != NULL && name[0] != '\0';
}

static SpeedPyScheduledTask *speedpy_scheduler_find_task(
    SpeedPyScheduler *scheduler,
    const char *name
)
{
    size_t i;

    if (scheduler == NULL || name == NULL) {
        return NULL;
    }

    for (i = 0; i < scheduler->task_count; ++i) {
        if (strcmp(scheduler->tasks[i].name, name) == 0) {
            return &scheduler->tasks[i];
        }
    }

    return NULL;
}

static SpeedPyScheduledTask *speedpy_scheduler_find_task_id(
    SpeedPyScheduler *scheduler,
    uint32_t id
)
{
    size_t i;

    if (scheduler == NULL || id == 0) {
        return NULL;
    }

    for (i = 0; i < scheduler->task_count; ++i) {
        if (scheduler->tasks[i].id == id) {
            return &scheduler->tasks[i];
        }
    }

    return NULL;
}

static bool speedpy_scheduler_task_due(
    const SpeedPyScheduledTask *task,
    uint64_t tick
)
{
    if (task == NULL ||
        !task->enabled ||
        task->state == SPEEDPY_COMPLETED ||
        task->state == SPEEDPY_CANCELLED ||
        task->state == SPEEDPY_FAILED) {
        return false;
    }

    return task->next_run <= tick;
}

static int speedpy_scheduler_compare(
    const SpeedPyScheduledTask *a,
    const SpeedPyScheduledTask *b
)
{
    if (a->priority > b->priority) {
        return -1;
    }

    if (a->priority < b->priority) {
        return 1;
    }

    if (a->next_run < b->next_run) {
        return -1;
    }

    if (a->next_run > b->next_run) {
        return 1;
    }

    if (a->id < b->id) {
        return -1;
    }

    if (a->id > b->id) {
        return 1;
    }

    return 0;
}

static void speedpy_scheduler_sort(
    SpeedPyScheduler *scheduler
)
{
    size_t i;
    size_t j;

    if (scheduler == NULL || scheduler->task_count < 2) {
        return;
    }

    for (i = 0; i < scheduler->task_count - 1; ++i) {
        for (j = i + 1; j < scheduler->task_count; ++j) {
            if (speedpy_scheduler_compare(
                    &scheduler->tasks[j],
                    &scheduler->tasks[i]
                ) < 0) {
                SpeedPyScheduledTask temporary;

                temporary = scheduler->tasks[i];
                scheduler->tasks[i] = scheduler->tasks[j];
                scheduler->tasks[j] = temporary;
            }
        }
    }
}

SpeedPyScheduler *speedpy_scheduler_create(
    const char *name
)
{
    SpeedPyScheduler *scheduler;

    if (!speedpy_scheduler_valid_name(name)) {
        return NULL;
    }

    scheduler = (SpeedPyScheduler *)calloc(
        1,
        sizeof(SpeedPyScheduler)
    );

    if (scheduler == NULL) {
        return NULL;
    }

    strncpy(
        scheduler->name,
        name,
        SPEEDPY_SCHEDULER_NAME_SIZE - 1
    );

    scheduler->name[SPEEDPY_SCHEDULER_NAME_SIZE - 1] = '\0';

    scheduler->next_task_id = 1;
    scheduler->max_dispatch_per_tick = 64;
    scheduler->generation = 1;

    return scheduler;
}

void speedpy_scheduler_destroy(
    SpeedPyScheduler *scheduler
)
{
    if (scheduler == NULL) {
        return;
    }

    free(scheduler);
}

int speedpy_scheduler_add_task(
    SpeedPyScheduler *scheduler,
    const char *name,
    uint64_t execute_at,
    uint64_t interval,
    uint32_t priority,
    bool repeat,
    void *user_data
)
{
    SpeedPyScheduledTask *task;

    if (scheduler == NULL ||
        scheduler->locked ||
        !speedpy_scheduler_valid_name(name) ||
        scheduler->task_count >= SPEEDPY_SCHEDULER_MAX_TASKS) {
        return -1;
    }

    if (speedpy_scheduler_find_task(scheduler, name) != NULL) {
        return -2;
    }

    if (priority > SPEEDPY_PRIORITY_CRITICAL) {
        return -3;
    }

    if (repeat && interval == 0) {
        return -4;
    }

    task = &scheduler->tasks[scheduler->task_count];

    memset(task, 0, sizeof(SpeedPyScheduledTask));

    strncpy(
        task->name,
        name,
        SPEEDPY_SCHEDULER_NAME_SIZE - 1
    );

    task->name[SPEEDPY_SCHEDULER_NAME_SIZE - 1] = '\0';

    task->id = scheduler->next_task_id++;
    task->execute_at = execute_at;
    task->interval = interval;
    task->next_run = execute_at;
    task->priority = priority;
    task->repeat = repeat;
    task->enabled = true;
    task->state = SPEEDPY_SCHEDULED;
    task->user_data = user_data;

    scheduler->task_count++;
    scheduler->generation++;

    speedpy_scheduler_sort(scheduler);

    return 0;
}

int speedpy_scheduler_remove_task(
    SpeedPyScheduler *scheduler,
    const char *name
)
{
    size_t i;
    SpeedPyScheduledTask *task;

    if (scheduler == NULL ||
        scheduler->locked ||
        name == NULL) {
        return -1;
    }

    task = speedpy_scheduler_find_task(scheduler, name);

    if (task == NULL) {
        return -2;
    }

    i = (size_t)(task - scheduler->tasks);

    if (i + 1 < scheduler->task_count) {
        memmove(
            &scheduler->tasks[i],
            &scheduler->tasks[i + 1],
            (scheduler->task_count - i - 1) *
                sizeof(SpeedPyScheduledTask)
        );
    }

    scheduler->task_count--;
    scheduler->generation++;

    return 0;
}

int speedpy_scheduler_cancel_task(
    SpeedPyScheduler *scheduler,
    const char *name
)
{
    SpeedPyScheduledTask *task;

    if (scheduler == NULL || name == NULL) {
        return -1;
    }

    task = speedpy_scheduler_find_task(scheduler, name);

    if (task == NULL) {
        return -2;
    }

    if (task->state == SPEEDPY_CANCELLED) {
        return 0;
    }

    task->state = SPEEDPY_CANCELLED;
    task->enabled = false;

    scheduler->cancelled_count++;
    scheduler->generation++;

    return 0;
}

int speedpy_scheduler_enable_task(
    SpeedPyScheduler *scheduler,
    const char *name
)
{
    SpeedPyScheduledTask *task;

    if (scheduler == NULL || scheduler->locked) {
        return -1;
    }

    task = speedpy_scheduler_find_task(scheduler, name);

    if (task == NULL) {
        return -2;
    }

    if (task->state == SPEEDPY_CANCELLED ||
        task->state == SPEEDPY_COMPLETED) {
        return -3;
    }

    task->enabled = true;

    if (task->state == SPEEDPY_PAUSED) {
        task->state = SPEEDPY_SCHEDULED;
    }

    scheduler->generation++;

    return 0;
}

int speedpy_scheduler_disable_task(
    SpeedPyScheduler *scheduler,
    const char *name
)
{
    SpeedPyScheduledTask *task;

    if (scheduler == NULL || scheduler->locked) {
        return -1;
    }

    task = speedpy_scheduler_find_task(scheduler, name);

    if (task == NULL) {
        return -2;
    }

    task->enabled = false;
    task->state = SPEEDPY_PAUSED;

    scheduler->generation++;

    return 0;
}

int speedpy_scheduler_set_priority(
    SpeedPyScheduler *scheduler,
    const char *name,
    uint32_t priority
)
{
    SpeedPyScheduledTask *task;

    if (scheduler == NULL ||
        scheduler->locked ||
        priority > SPEEDPY_PRIORITY_CRITICAL) {
        return -1;
    }

    task = speedpy_scheduler_find_task(scheduler, name);

    if (task == NULL) {
        return -2;
    }

    task->priority = priority;
    scheduler->generation++;

    speedpy_scheduler_sort(scheduler);

    return 0;
}

int speedpy_scheduler_set_deadline(
    SpeedPyScheduler *scheduler,
    const char *name,
    uint64_t deadline
)
{
    SpeedPyScheduledTask *task;

    if (scheduler == NULL || scheduler->locked) {
        return -1;
    }

    task = speedpy_scheduler_find_task(scheduler, name);

    if (task == NULL) {
        return -2;
    }

    task->deadline = deadline;
    scheduler->generation++;

    return 0;
}

int speedpy_scheduler_reschedule(
    SpeedPyScheduler *scheduler,
    const char *name,
    uint64_t next_run
)
{
    SpeedPyScheduledTask *task;

    if (scheduler == NULL || scheduler->locked) {
        return -1;
    }

    task = speedpy_scheduler_find_task(scheduler, name);

    if (task == NULL) {
        return -2;
    }

    if (task->state == SPEEDPY_CANCELLED) {
        return -3;
    }

    task->next_run = next_run;
    task->execute_at = next_run;
    task->state = SPEEDPY_SCHEDULED;
    task->enabled = true;

    scheduler->generation++;

    speedpy_scheduler_sort(scheduler);

    return 0;
}

SpeedPyScheduledTask *speedpy_scheduler_get_task(
    SpeedPyScheduler *scheduler,
    const char *name
)
{
    return speedpy_scheduler_find_task(scheduler, name);
}

SpeedPyScheduledTask *speedpy_scheduler_get_task_id(
    SpeedPyScheduler *scheduler,
    uint32_t id
)
{
    return speedpy_scheduler_find_task_id(scheduler, id);
}

size_t speedpy_scheduler_task_count(
    const SpeedPyScheduler *scheduler
)
{
    if (scheduler == NULL) {
        return 0;
    }

    return scheduler->task_count;
}

uint64_t speedpy_scheduler_current_tick(
    const SpeedPyScheduler *scheduler
)
{
    if (scheduler == NULL) {
        return 0;
    }

    return scheduler->current_tick;
}

void speedpy_scheduler_set_tick(
    SpeedPyScheduler *scheduler,
    uint64_t tick
)
{
    if (scheduler == NULL) {
        return;
    }

    scheduler->current_tick = tick;
}

uint32_t speedpy_scheduler_set_dispatch_limit(
    SpeedPyScheduler *scheduler,
    uint32_t limit
)
{
    uint32_t previous;

    if (scheduler == NULL || limit == 0) {
        return 0;
    }

    previous = scheduler->max_dispatch_per_tick;
    scheduler->max_dispatch_per_tick = limit;

    return previous;
}

uint32_t speedpy_scheduler_dispatch_limit(
    const SpeedPyScheduler *scheduler
)
{
    if (scheduler == NULL) {
        return 0;
    }

    return scheduler->max_dispatch_per_tick;
}

size_t speedpy_scheduler_count_due(
    SpeedPyScheduler *scheduler,
    uint64_t tick
)
{
    size_t i;
    size_t count = 0;

    if (scheduler == NULL) {
        return 0;
    }

    for (i = 0; i < scheduler->task_count; ++i) {
        if (speedpy_scheduler_task_due(
                &scheduler->tasks[i],
                tick
            )) {
            count++;
        }
    }

    return count;
}

int speedpy_scheduler_begin_task(
    SpeedPyScheduler *scheduler,
    const char *name,
    uint64_t tick
)
{
    SpeedPyScheduledTask *task;

    if (scheduler == NULL || name == NULL) {
        return -1;
    }

    task = speedpy_scheduler_find_task(scheduler, name);

    if (task == NULL) {
        return -2;
    }

    if (!speedpy_scheduler_task_due(task, tick)) {
        return -3;
    }

    if (task->deadline != 0 && tick > task->deadline) {
        task->missed_count++;
        scheduler->missed_count++;
        task->state = SPEEDPY_SCHEDULED;

        if (!task->repeat) {
            task->enabled = false;
        }

        return -4;
    }

    task->state = SPEEDPY_RUNNING;
    task->last_run = tick;

    scheduler->dispatch_count++;

    return 0;
}

int speedpy_scheduler_complete_task(
    SpeedPyScheduler *scheduler,
    const char *name,
    bool failed,
    uint64_t tick
)
{
    SpeedPyScheduledTask *task;

    if (scheduler == NULL || name == NULL) {
        return -1;
    }

    task = speedpy_scheduler_find_task(scheduler, name);

    if (task == NULL) {
        return -2;
    }

    if (task->state != SPEEDPY_RUNNING) {
        return -3;
    }

    task->run_count++;

    if (failed) {
        task->failure_count++;
        task->state = SPEEDPY_FAILED;
        scheduler->failed_count++;
        return 0;
    }

    if (task->repeat) {
        task->next_run = tick + task->interval;
        task->state = SPEEDPY_SCHEDULED;
    } else {
        task->state = SPEEDPY_COMPLETED;
        task->enabled = false;
        scheduler->completed_count++;
    }

    return 0;
}

int speedpy_scheduler_tick(
    SpeedPyScheduler *scheduler,
    uint64_t tick
)
{
    size_t i;
    uint32_t dispatched = 0;

    if (scheduler == NULL) {
        return -1;
    }

    scheduler->current_tick = tick;

    if (!scheduler->running || scheduler->paused) {
        return 0;
    }

    speedpy_scheduler_sort(scheduler);

    for (i = 0;
         i < scheduler->task_count &&
         dispatched < scheduler->max_dispatch_per_tick;
         ++i) {
        SpeedPyScheduledTask *task = &scheduler->tasks[i];

        if (!speedpy_scheduler_task_due(task, tick)) {
            continue;
        }

        if (task->deadline != 0 &&
            tick > task->deadline) {
            task->missed_count++;
            scheduler->missed_count++;

            if (!task->repeat) {
                task->enabled = false;
            }

            continue;
        }

        task->state = SPEEDPY_RUNNING;
        task->last_run = tick;
        dispatched++;
        scheduler->dispatch_count++;
    }

    return (int)dispatched;
}

void speedpy_scheduler_start(
    SpeedPyScheduler *scheduler
)
{
    if (scheduler == NULL) {
        return;
    }

    scheduler->running = true;
    scheduler->paused = false;
}

void speedpy_scheduler_pause(
    SpeedPyScheduler *scheduler
)
{
    if (scheduler == NULL || !scheduler->running) {
        return;
    }

    scheduler->paused = true;
}

void speedpy_scheduler_resume(
    SpeedPyScheduler *scheduler
)
{
    if (scheduler == NULL || !scheduler->running) {
        return;
    }

    scheduler->paused = false;
}

void speedpy_scheduler_stop(
    SpeedPyScheduler *scheduler
)
{
    if (scheduler == NULL) {
        return;
    }

    scheduler->running = false;
    scheduler->paused = false;
}

bool speedpy_scheduler_is_running(
    const SpeedPyScheduler *scheduler
)
{
    return scheduler != NULL && scheduler->running;
}

bool speedpy_scheduler_is_paused(
    const SpeedPyScheduler *scheduler
)
{
    return scheduler != NULL && scheduler->paused;
}

uint64_t speedpy_scheduler_dispatch_count(
    const SpeedPyScheduler *scheduler
)
{
    if (scheduler == NULL) {
        return 0;
    }

    return scheduler->dispatch_count;
}

uint64_t speedpy_scheduler_completed_count(
    const SpeedPyScheduler *scheduler
)
{
    if (scheduler == NULL) {
        return 0;
    }

    return scheduler->completed_count;
}

uint64_t speedpy_scheduler_failed_count(
    const SpeedPyScheduler *scheduler
)
{
    if (scheduler == NULL) {
        return 0;
    }

    return scheduler->failed_count;
}

uint64_t speedpy_scheduler_missed_count(
    const SpeedPyScheduler *scheduler
)
{
    if (scheduler == NULL) {
        return 0;
    }

    return scheduler->missed_count;
}

uint64_t speedpy_scheduler_generation(
    const SpeedPyScheduler *scheduler
)
{
    if (scheduler == NULL) {
        return 0;
    }

    return scheduler->generation;
}

const char *speedpy_scheduler_name(
    const SpeedPyScheduler *scheduler
)
{
    if (scheduler == NULL) {
        return NULL;
    }

    return scheduler->name;
}

void speedpy_scheduler_lock(
    SpeedPyScheduler *scheduler
)
{
    if (scheduler != NULL) {
        scheduler->locked = true;
    }
}

void speedpy_scheduler_unlock(
    SpeedPyScheduler *scheduler
)
{
    if (scheduler != NULL) {
        scheduler->locked = false;
    }
}

bool speedpy_scheduler_is_locked(
    const SpeedPyScheduler *scheduler
)
{
    return scheduler != NULL && scheduler->locked;
}

void speedpy_scheduler_set_user_data(
    SpeedPyScheduler *scheduler,
    void *user_data
)
{
    if (scheduler != NULL) {
        scheduler->user_data = user_data;
    }
}

void *speedpy_scheduler_get_user_data(
    const SpeedPyScheduler *scheduler
)
{
    if (scheduler == NULL) {
        return NULL;
    }

    return scheduler->user_data;
}

void speedpy_scheduler_reset_stats(
    SpeedPyScheduler *scheduler
)
{
    size_t i;

    if (scheduler == NULL || scheduler->locked) {
        return;
    }

    scheduler->dispatch_count = 0;
    scheduler->completed_count = 0;
    scheduler->cancelled_count = 0;
    scheduler->failed_count = 0;
    scheduler->missed_count = 0;

    for (i = 0; i < scheduler->task_count; ++i) {
        scheduler->tasks[i].run_count = 0;
        scheduler->tasks[i].failure_count = 0;
        scheduler->tasks[i].missed_count = 0;
        scheduler->tasks[i].last_run = 0;
    }

    scheduler->generation++;
}