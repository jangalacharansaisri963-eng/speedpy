#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define SPEEDPY_EVENT_MAX_EVENTS 512
#define SPEEDPY_EVENT_MAX_LISTENERS 1024
#define SPEEDPY_EVENT_NAME_SIZE 96
#define SPEEDPY_EVENT_MAX_PAYLOAD 256

typedef uint64_t SpeedPyEventId;

typedef bool (*SpeedPyEventCallback)(
    SpeedPyEventId event_id,
    const char *event_name,
    const void *payload,
    size_t payload_size,
    void *user_data
);

typedef struct {
    SpeedPyEventId id;
    char name[SPEEDPY_EVENT_NAME_SIZE];

    bool active;

    uint64_t dispatch_count;
    uint64_t listener_count;
} SpeedPyEvent;

typedef struct {
    uint64_t id;

    SpeedPyEventId event_id;

    SpeedPyEventCallback callback;
    void *user_data;

    int32_t priority;

    bool active;
    bool once;
} SpeedPyEventListener;

typedef struct {
    SpeedPyEvent events[SPEEDPY_EVENT_MAX_EVENTS];
    SpeedPyEventListener listeners[
        SPEEDPY_EVENT_MAX_LISTENERS
    ];

    size_t event_count;
    size_t listener_count;

    SpeedPyEventId next_event_id;
    uint64_t next_listener_id;
    uint64_t generation;

    bool enabled;
    bool locked;
} SpeedPyEventSystem;

static SpeedPyEvent *speedpy_event_find(
    SpeedPyEventSystem *system,
    const char *name
)
{
    size_t i;

    if (system == NULL || name == NULL) {
        return NULL;
    }

    for (i = 0; i < system->event_count; i++) {
        if (system->events[i].active &&
            strcmp(system->events[i].name, name) == 0) {
            return &system->events[i];
        }
    }

    return NULL;
}

static SpeedPyEvent *speedpy_event_find_id(
    SpeedPyEventSystem *system,
    SpeedPyEventId id
)
{
    size_t i;

    if (system == NULL || id == 0) {
        return NULL;
    }

    for (i = 0; i < system->event_count; i++) {
        if (system->events[i].active &&
            system->events[i].id == id) {
            return &system->events[i];
        }
    }

    return NULL;
}

static SpeedPyEventListener *speedpy_listener_find(
    SpeedPyEventSystem *system,
    uint64_t listener_id
)
{
    size_t i;

    if (system == NULL || listener_id == 0) {
        return NULL;
    }

    for (i = 0; i < system->listener_count; i++) {
        if (system->listeners[i].active &&
            system->listeners[i].id == listener_id) {
            return &system->listeners[i];
        }
    }

    return NULL;
}

SpeedPyEventSystem *speedpy_events_create(void)
{
    SpeedPyEventSystem *system;

    system = (SpeedPyEventSystem *)calloc(
        1,
        sizeof(SpeedPyEventSystem)
    );

    if (system == NULL) {
        return NULL;
    }

    system->event_count = 0;
    system->listener_count = 0;

    system->next_event_id = 1;
    system->next_listener_id = 1;

    system->generation = 1;

    system->enabled = true;
    system->locked = false;

    return system;
}

void speedpy_events_destroy(
    SpeedPyEventSystem *system
)
{
    if (system == NULL) {
        return;
    }

    free(system);
}

SpeedPyEventId speedpy_event_create(
    SpeedPyEventSystem *system,
    const char *name
)
{
    SpeedPyEvent *event;
    size_t length;

    if (system == NULL ||
        name == NULL ||
        system->locked) {
        return 0;
    }

    length = strlen(name);

    if (length == 0 ||
        length >= SPEEDPY_EVENT_NAME_SIZE) {
        return 0;
    }

    if (speedpy_event_find(system, name) != NULL) {
        return 0;
    }

    if (system->event_count >= SPEEDPY_EVENT_MAX_EVENTS) {
        return 0;
    }

    event = &system->events[system->event_count];

    memset(event, 0, sizeof(SpeedPyEvent));

    memcpy(event->name, name, length);
    event->name[length] = '\0';

    event->id = system->next_event_id++;
    event->active = true;
    event->dispatch_count = 0;
    event->listener_count = 0;

    system->event_count++;
    system->generation++;

    return event->id;
}

bool speedpy_event_destroy(
    SpeedPyEventSystem *system,
    SpeedPyEventId event_id
)
{
    SpeedPyEvent *event;
    size_t i;

    if (system == NULL ||
        event_id == 0 ||
        system->locked) {
        return false;
    }

    event = speedpy_event_find_id(system, event_id);

    if (event == NULL) {
        return false;
    }

    for (i = 0; i < system->listener_count; i++) {
        if (system->listeners[i].active &&
            system->listeners[i].event_id == event_id) {
            system->listeners[i].active = false;
        }
    }

    event->active = false;

    system->generation++;

    return true;
}

SpeedPyEventId speedpy_event_find_id_by_name(
    const SpeedPyEventSystem *system,
    const char *name
)
{
    size_t i;

    if (system == NULL || name == NULL) {
        return 0;
    }

    for (i = 0; i < system->event_count; i++) {
        if (system->events[i].active &&
            strcmp(system->events[i].name, name) == 0) {
            return system->events[i].id;
        }
    }

    return 0;
}

const char *speedpy_event_get_name(
    const SpeedPyEventSystem *system,
    SpeedPyEventId event_id
)
{
    size_t i;

    if (system == NULL || event_id == 0) {
        return NULL;
    }

    for (i = 0; i < system->event_count; i++) {
        if (system->events[i].active &&
            system->events[i].id == event_id) {
            return system->events[i].name;
        }
    }

    return NULL;
}

uint64_t speedpy_event_subscribe(
    SpeedPyEventSystem *system,
    SpeedPyEventId event_id,
    SpeedPyEventCallback callback,
    void *user_data,
    int32_t priority,
    bool once
)
{
    SpeedPyEvent *event;
    SpeedPyEventListener *listener;

    if (system == NULL ||
        event_id == 0 ||
        callback == NULL ||
        system->locked) {
        return 0;
    }

    event = speedpy_event_find_id(system, event_id);

    if (event == NULL) {
        return 0;
    }

    if (system->listener_count >=
        SPEEDPY_EVENT_MAX_LISTENERS) {
        return 0;
    }

    listener = &system->listeners[system->listener_count];

    memset(
        listener,
        0,
        sizeof(SpeedPyEventListener)
    );

    listener->id = system->next_listener_id++;
    listener->event_id = event_id;
    listener->callback = callback;
    listener->user_data = user_data;
    listener->priority = priority;
    listener->active = true;
    listener->once = once;

    system->listener_count++;
    event->listener_count++;

    system->generation++;

    return listener->id;
}

bool speedpy_event_unsubscribe(
    SpeedPyEventSystem *system,
    uint64_t listener_id
)
{
    SpeedPyEventListener *listener;
    SpeedPyEvent *event;

    if (system == NULL ||
        listener_id == 0 ||
        system->locked) {
        return false;
    }

    listener = speedpy_listener_find(
        system,
        listener_id
    );

    if (listener == NULL) {
        return false;
    }

    event = speedpy_event_find_id(
        system,
        listener->event_id
    );

    if (event != NULL &&
        event->listener_count > 0) {
        event->listener_count--;
    }

    listener->active = false;

    system->generation++;

    return true;
}

bool speedpy_event_dispatch(
    SpeedPyEventSystem *system,
    SpeedPyEventId event_id,
    const void *payload,
    size_t payload_size
)
{
    SpeedPyEvent *event;
    size_t i;
    size_t j;
    size_t count;
    bool result;

    if (system == NULL ||
        event_id == 0 ||
        !system->enabled) {
        return false;
    }

    if (payload_size > SPEEDPY_EVENT_MAX_PAYLOAD) {
        return false;
    }

    event = speedpy_event_find_id(system, event_id);

    if (event == NULL) {
        return false;
    }

    event->dispatch_count++;

    count = system->listener_count;

    for (i = 0; i < count; i++) {
        SpeedPyEventListener *listener;
        int32_t best_priority;

        listener = NULL;
        best_priority = -2147483647;

        for (j = 0; j < count; j++) {
            SpeedPyEventListener *candidate;

            candidate = &system->listeners[j];

            if (!candidate->active ||
                candidate->event_id != event_id) {
                continue;
            }

            if (candidate->priority > best_priority) {
                listener = candidate;
                best_priority = candidate->priority;
            }
        }

        if (listener == NULL) {
            break;
        }

        listener->active = false;

        result = listener->callback(
            event->id,
            event->name,
            payload,
            payload_size,
            listener->user_data
        );

        if (event->listener_count > 0) {
            event->listener_count--;
        }

        if (!result) {
            listener->active = false;
        }

        if (listener->once) {
            continue;
        }

        listener->active = true;
    }

    system->generation++;

    return true;
}

uint64_t speedpy_event_dispatch_count(
    const SpeedPyEventSystem *system,
    SpeedPyEventId event_id
)
{
    size_t i;

    if (system == NULL || event_id == 0) {
        return 0;
    }

    for (i = 0; i < system->event_count; i++) {
        if (system->events[i].active &&
            system->events[i].id == event_id) {
            return system->events[i].dispatch_count;
        }
    }

    return 0;
}

size_t speedpy_event_listener_count(
    const SpeedPyEventSystem *system,
    SpeedPyEventId event_id
)
{
    size_t i;

    if (system == NULL || event_id == 0) {
        return 0;
    }

    for (i = 0; i < system->event_count; i++) {
        if (system->events[i].active &&
            system->events[i].id == event_id) {
            return system->events[i].listener_count;
        }
    }

    return 0;
}

size_t speedpy_event_count(
    const SpeedPyEventSystem *system
)
{
    if (system == NULL) {
        return 0;
    }

    return system->event_count;
}

uint64_t speedpy_events_generation(
    const SpeedPyEventSystem *system
)
{
    if (system == NULL) {
        return 0;
    }

    return system->generation;
}

bool speedpy_events_enable(
    SpeedPyEventSystem *system
)
{
    if (system == NULL) {
        return false;
    }

    system->enabled = true;

    return true;
}

bool speedpy_events_disable(
    SpeedPyEventSystem *system
)
{
    if (system == NULL) {
        return false;
    }

    system->enabled = false;

    return true;
}

bool speedpy_events_is_enabled(
    const SpeedPyEventSystem *system
)
{
    if (system == NULL) {
        return false;
    }

    return system->enabled;
}

bool speedpy_events_lock(
    SpeedPyEventSystem *system
)
{
    if (system == NULL) {
        return false;
    }

    system->locked = true;

    return true;
}

bool speedpy_events_unlock(
    SpeedPyEventSystem *system
)
{
    if (system == NULL) {
        return false;
    }

    system->locked = false;

    return true;
}