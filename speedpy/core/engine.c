#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef enum {
    SPEEDPY_ENGINE_CREATED = 0,
    SPEEDPY_ENGINE_INITIALIZED = 1,
    SPEEDPY_ENGINE_RUNNING = 2,
    SPEEDPY_ENGINE_PAUSED = 3,
    SPEEDPY_ENGINE_STOPPING = 4,
    SPEEDPY_ENGINE_STOPPED = 5,
    SPEEDPY_ENGINE_DESTROYED = 6
} SpeedPyEngineState;

typedef struct {
    uint64_t ticks;
    uint64_t frames;
    uint64_t updates;
    uint64_t starts;
    uint64_t stops;
    uint64_t pauses;
    uint64_t resumes;
} SpeedPyEngineStats;

typedef struct {
    SpeedPyEngineState state;
    bool initialized;
    bool running;
    bool paused;

    uint64_t tick_count;
    uint64_t frame_count;
    uint64_t update_count;

    size_t module_count;
    size_t active_module_count;

    uint32_t tick_rate;
    uint32_t max_modules;

    char name[128];

    SpeedPyEngineStats stats;
} SpeedPyEngine;

static void speedpy_engine_reset_stats(SpeedPyEngine *engine)
{
    memset(&engine->stats, 0, sizeof(SpeedPyEngineStats));
}

SpeedPyEngine *speedpy_engine_create(void)
{
    SpeedPyEngine *engine;

    engine = (SpeedPyEngine *)calloc(1, sizeof(SpeedPyEngine));

    if (engine == NULL) {
        return NULL;
    }

    engine->state = SPEEDPY_ENGINE_CREATED;
    engine->initialized = false;
    engine->running = false;
    engine->paused = false;

    engine->tick_count = 0;
    engine->frame_count = 0;
    engine->update_count = 0;

    engine->module_count = 0;
    engine->active_module_count = 0;

    engine->tick_rate = 60;
    engine->max_modules = 1024;

    engine->name[0] = '\0';

    speedpy_engine_reset_stats(engine);

    return engine;
}

bool speedpy_engine_initialize(SpeedPyEngine *engine)
{
    if (engine == NULL) {
        return false;
    }

    if (engine->state == SPEEDPY_ENGINE_DESTROYED) {
        return false;
    }

    if (engine->initialized) {
        return false;
    }

    engine->initialized = true;
    engine->running = false;
    engine->paused = false;

    engine->tick_count = 0;
    engine->frame_count = 0;
    engine->update_count = 0;

    speedpy_engine_reset_stats(engine);

    engine->state = SPEEDPY_ENGINE_INITIALIZED;

    return true;
}

bool speedpy_engine_start(SpeedPyEngine *engine)
{
    if (engine == NULL) {
        return false;
    }

    if (!engine->initialized) {
        return false;
    }

    if (engine->state == SPEEDPY_ENGINE_RUNNING) {
        return false;
    }

    if (engine->state == SPEEDPY_ENGINE_PAUSED) {
        return false;
    }

    if (engine->state == SPEEDPY_ENGINE_STOPPING ||
        engine->state == SPEEDPY_ENGINE_DESTROYED) {
        return false;
    }

    engine->running = true;
    engine->paused = false;
    engine->state = SPEEDPY_ENGINE_RUNNING;

    engine->stats.starts++;

    return true;
}

bool speedpy_engine_pause(SpeedPyEngine *engine)
{
    if (engine == NULL) {
        return false;
    }

    if (!engine->initialized ||
        !engine->running ||
        engine->paused) {
        return false;
    }

    engine->paused = true;
    engine->state = SPEEDPY_ENGINE_PAUSED;

    engine->stats.pauses++;

    return true;
}

bool speedpy_engine_resume(SpeedPyEngine *engine)
{
    if (engine == NULL) {
        return false;
    }

    if (!engine->initialized ||
        !engine->running ||
        !engine->paused) {
        return false;
    }

    engine->paused = false;
    engine->state = SPEEDPY_ENGINE_RUNNING;

    engine->stats.resumes++;

    return true;
}

bool speedpy_engine_tick(SpeedPyEngine *engine)
{
    if (engine == NULL) {
        return false;
    }

    if (!engine->initialized ||
        !engine->running ||
        engine->paused) {
        return false;
    }

    engine->tick_count++;
    engine->update_count++;

    engine->stats.ticks++;
    engine->stats.updates++;

    return true;
}

bool speedpy_engine_frame(SpeedPyEngine *engine)
{
    if (engine == NULL) {
        return false;
    }

    if (!engine->initialized ||
        !engine->running ||
        engine->paused) {
        return false;
    }

    engine->frame_count++;

    engine->stats.frames++;

    return true;
}

bool speedpy_engine_stop(SpeedPyEngine *engine)
{
    if (engine == NULL) {
        return false;
    }

    if (!engine->initialized ||
        !engine->running) {
        return false;
    }

    engine->state = SPEEDPY_ENGINE_STOPPING;

    engine->running = false;
    engine->paused = false;

    engine->state = SPEEDPY_ENGINE_STOPPED;

    engine->stats.stops++;

    return true;
}

bool speedpy_engine_restart(SpeedPyEngine *engine)
{
    if (engine == NULL) {
        return false;
    }

    if (!engine->initialized ||
        engine->state == SPEEDPY_ENGINE_DESTROYED) {
        return false;
    }

    if (engine->running) {
        if (!speedpy_engine_stop(engine)) {
            return false;
        }
    }

    return speedpy_engine_start(engine);
}

bool speedpy_engine_is_running(const SpeedPyEngine *engine)
{
    if (engine == NULL) {
        return false;
    }

    return engine->running && !engine->paused;
}

bool speedpy_engine_is_paused(const SpeedPyEngine *engine)
{
    if (engine == NULL) {
        return false;
    }

    return engine->paused;
}

bool speedpy_engine_is_initialized(const SpeedPyEngine *engine)
{
    if (engine == NULL) {
        return false;
    }

    return engine->initialized;
}

SpeedPyEngineState speedpy_engine_get_state(
    const SpeedPyEngine *engine
)
{
    if (engine == NULL) {
        return SPEEDPY_ENGINE_DESTROYED;
    }

    return engine->state;
}

uint64_t speedpy_engine_get_tick_count(
    const SpeedPyEngine *engine
)
{
    if (engine == NULL) {
        return 0;
    }

    return engine->tick_count;
}

uint64_t speedpy_engine_get_frame_count(
    const SpeedPyEngine *engine
)
{
    if (engine == NULL) {
        return 0;
    }

    return engine->frame_count;
}

uint64_t speedpy_engine_get_update_count(
    const SpeedPyEngine *engine
)
{
    if (engine == NULL) {
        return 0;
    }

    return engine->update_count;
}

size_t speedpy_engine_get_module_count(
    const SpeedPyEngine *engine
)
{
    if (engine == NULL) {
        return 0;
    }

    return engine->module_count;
}

size_t speedpy_engine_get_active_module_count(
    const SpeedPyEngine *engine
)
{
    if (engine == NULL) {
        return 0;
    }

    return engine->active_module_count;
}

bool speedpy_engine_set_tick_rate(
    SpeedPyEngine *engine,
    uint32_t tick_rate
)
{
    if (engine == NULL || tick_rate == 0) {
        return false;
    }

    engine->tick_rate = tick_rate;

    return true;
}

uint32_t speedpy_engine_get_tick_rate(
    const SpeedPyEngine *engine
)
{
    if (engine == NULL) {
        return 0;
    }

    return engine->tick_rate;
}

bool speedpy_engine_set_max_modules(
    SpeedPyEngine *engine,
    uint32_t max_modules
)
{
    if (engine == NULL || max_modules == 0) {
        return false;
    }

    if (engine->module_count > max_modules) {
        return false;
    }

    engine->max_modules = max_modules;

    return true;
}

uint32_t speedpy_engine_get_max_modules(
    const SpeedPyEngine *engine
)
{
    if (engine == NULL) {
        return 0;
    }

    return engine->max_modules;
}

bool speedpy_engine_set_name(
    SpeedPyEngine *engine,
    const char *name
)
{
    size_t length;

    if (engine == NULL || name == NULL) {
        return false;
    }

    length = strlen(name);

    if (length >= sizeof(engine->name)) {
        return false;
    }

    memcpy(engine->name, name, length);
    engine->name[length] = '\0';

    return true;
}

const char *speedpy_engine_get_name(
    const SpeedPyEngine *engine
)
{
    if (engine == NULL) {
        return NULL;
    }

    return engine->name;
}

bool speedpy_engine_add_module(SpeedPyEngine *engine)
{
    if (engine == NULL) {
        return false;
    }

    if (engine->module_count >= engine->max_modules) {
        return false;
    }

    engine->module_count++;
    engine->active_module_count++;

    return true;
}

bool speedpy_engine_remove_module(SpeedPyEngine *engine)
{
    if (engine == NULL || engine->module_count == 0) {
        return false;
    }

    engine->module_count--;

    if (engine->active_module_count > 0) {
        engine->active_module_count--;
    }

    return true;
}

const SpeedPyEngineStats *speedpy_engine_get_stats(
    const SpeedPyEngine *engine
)
{
    if (engine == NULL) {
        return NULL;
    }

    return &engine->stats;
}

void speedpy_engine_reset_counters(SpeedPyEngine *engine)
{
    if (engine == NULL) {
        return;
    }

    engine->tick_count = 0;
    engine->frame_count = 0;
    engine->update_count = 0;

    engine->stats.ticks = 0;
    engine->stats.frames = 0;
    engine->stats.updates = 0;
}

void speedpy_engine_destroy(SpeedPyEngine *engine)
{
    if (engine == NULL) {
        return;
    }

    engine->running = false;
    engine->paused = false;
    engine->initialized = false;

    engine->state = SPEEDPY_ENGINE_DESTROYED;

    free(engine);
}