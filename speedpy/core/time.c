#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    uint64_t ticks;
    uint64_t frequency;
    uint64_t start_ticks;
    uint64_t elapsed_ticks;
    uint64_t delta_ticks;
    uint64_t frame_count;
    uint64_t update_count;
    uint64_t fixed_steps;
    uint64_t fixed_step_ticks;
    uint64_t accumulator;
    bool running;
    bool paused;
    bool locked;
    uint64_t generation;
    void *user_data;
} SpeedPyClock;

typedef struct {
    uint64_t total_ticks;
    uint64_t delta_ticks;
    uint64_t frame_count;
    uint64_t update_count;
    uint64_t fixed_steps;
    uint64_t frequency;
} SpeedPyTimeSnapshot;

static bool speedpy_clock_valid(
    const SpeedPyClock *clock
) {
    return clock != NULL &&
           clock->frequency > 0;
}

SpeedPyClock *speedpy_clock_create(
    uint64_t frequency
) {
    SpeedPyClock *clock;

    if (frequency == 0) {
        return NULL;
    }

    clock =
        (SpeedPyClock *)calloc(
            1,
            sizeof(SpeedPyClock)
        );

    if (clock == NULL) {
        return NULL;
    }

    clock->frequency = frequency;
    clock->generation = 1;

    return clock;
}

void speedpy_clock_destroy(
    SpeedPyClock *clock
) {
    if (clock == NULL) {
        return;
    }

    free(clock);
}

bool speedpy_clock_start(
    SpeedPyClock *clock,
    uint64_t current_ticks
) {
    if (!speedpy_clock_valid(clock) ||
        clock->locked) {
        return false;
    }

    clock->ticks = current_ticks;
    clock->start_ticks = current_ticks;
    clock->elapsed_ticks = 0;
    clock->delta_ticks = 0;
    clock->accumulator = 0;
    clock->frame_count = 0;
    clock->update_count = 0;
    clock->fixed_steps = 0;
    clock->running = true;
    clock->paused = false;
    clock->generation++;

    return true;
}

bool speedpy_clock_stop(
    SpeedPyClock *clock
) {
    if (!speedpy_clock_valid(clock) ||
        clock->locked) {
        return false;
    }

    clock->running = false;
    clock->paused = false;
    clock->generation++;

    return true;
}

bool speedpy_clock_pause(
    SpeedPyClock *clock
) {
    if (!speedpy_clock_valid(clock) ||
        clock->locked ||
        !clock->running) {
        return false;
    }

    clock->paused = true;
    clock->generation++;

    return true;
}

bool speedpy_clock_resume(
    SpeedPyClock *clock
) {
    if (!speedpy_clock_valid(clock) ||
        clock->locked ||
        !clock->running) {
        return false;
    }

    clock->paused = false;
    clock->generation++;

    return true;
}

bool speedpy_clock_update(
    SpeedPyClock *clock,
    uint64_t current_ticks
) {
    uint64_t delta;

    if (!speedpy_clock_valid(clock) ||
        clock->locked ||
        !clock->running) {
        return false;
    }

    if (current_ticks < clock->ticks) {
        delta = 0;
    } else {
        delta = current_ticks - clock->ticks;
    }

    clock->ticks = current_ticks;

    if (clock->paused) {
        clock->delta_ticks = 0;
        return true;
    }

    clock->delta_ticks = delta;

    if (UINT64_MAX - clock->elapsed_ticks < delta) {
        clock->elapsed_ticks = UINT64_MAX;
    } else {
        clock->elapsed_ticks += delta;
    }

    clock->frame_count++;
    clock->update_count++;

    if (clock->fixed_step_ticks > 0) {
        if (UINT64_MAX - clock->accumulator < delta) {
            clock->accumulator = UINT64_MAX;
        } else {
            clock->accumulator += delta;
        }

        while (clock->accumulator >=
               clock->fixed_step_ticks) {
            clock->accumulator -=
                clock->fixed_step_ticks;

            if (clock->fixed_steps <
                UINT64_MAX) {
                clock->fixed_steps++;
            }
        }
    }

    clock->generation++;

    return true;
}

bool speedpy_clock_advance(
    SpeedPyClock *clock,
    uint64_t delta_ticks
) {
    if (!speedpy_clock_valid(clock) ||
        clock->locked ||
        !clock->running) {
        return false;
    }

    if (UINT64_MAX - clock->ticks < delta_ticks) {
        clock->ticks = UINT64_MAX;
    } else {
        clock->ticks += delta_ticks;
    }

    return speedpy_clock_update(
        clock,
        clock->ticks
    );
}

bool speedpy_clock_set_fixed_step(
    SpeedPyClock *clock,
    uint64_t fixed_step_ticks
) {
    if (!speedpy_clock_valid(clock) ||
        clock->locked) {
        return false;
    }

    clock->fixed_step_ticks =
        fixed_step_ticks;

    clock->accumulator = 0;
    clock->generation++;

    return true;
}

uint64_t speedpy_clock_consume_fixed_step(
    SpeedPyClock *clock
) {
    uint64_t steps;

    if (!speedpy_clock_valid(clock) ||
        clock->fixed_step_ticks == 0) {
        return 0;
    }

    steps = clock->fixed_steps;
    clock->fixed_steps = 0;

    return steps;
}

uint64_t speedpy_clock_ticks(
    const SpeedPyClock *clock
) {
    if (!speedpy_clock_valid(clock)) {
        return 0;
    }

    return clock->ticks;
}

uint64_t speedpy_clock_delta(
    const SpeedPyClock *clock
) {
    if (!speedpy_clock_valid(clock)) {
        return 0;
    }

    return clock->delta_ticks;
}

uint64_t speedpy_clock_elapsed(
    const SpeedPyClock *clock
) {
    if (!speedpy_clock_valid(clock)) {
        return 0;
    }

    return clock->elapsed_ticks;
}

uint64_t speedpy_clock_frequency(
    const SpeedPyClock *clock
) {
    if (!speedpy_clock_valid(clock)) {
        return 0;
    }

    return clock->frequency;
}

uint64_t speedpy_clock_frame_count(
    const SpeedPyClock *clock
) {
    if (!speedpy_clock_valid(clock)) {
        return 0;
    }

    return clock->frame_count;
}

uint64_t speedpy_clock_update_count(
    const SpeedPyClock *clock
) {
    if (!speedpy_clock_valid(clock)) {
        return 0;
    }

    return clock->update_count;
}

uint64_t speedpy_clock_fixed_step_count(
    const SpeedPyClock *clock
) {
    if (!speedpy_clock_valid(clock)) {
        return 0;
    }

    return clock->fixed_steps;
}

uint64_t speedpy_clock_fixed_step_size(
    const SpeedPyClock *clock
) {
    if (!speedpy_clock_valid(clock)) {
        return 0;
    }

    return clock->fixed_step_ticks;
}

bool speedpy_clock_is_running(
    const SpeedPyClock *clock
) {
    if (!speedpy_clock_valid(clock)) {
        return false;
    }

    return clock->running;
}

bool speedpy_clock_is_paused(
    const SpeedPyClock *clock
) {
    if (!speedpy_clock_valid(clock)) {
        return false;
    }

    return clock->paused;
}

bool speedpy_clock_snapshot(
    const SpeedPyClock *clock,
    SpeedPyTimeSnapshot *snapshot
) {
    if (!speedpy_clock_valid(clock) ||
        snapshot == NULL) {
        return false;
    }

    snapshot->total_ticks =
        clock->elapsed_ticks;

    snapshot->delta_ticks =
        clock->delta_ticks;

    snapshot->frame_count =
        clock->frame_count;

    snapshot->update_count =
        clock->update_count;

    snapshot->fixed_steps =
        clock->fixed_steps;

    snapshot->frequency =
        clock->frequency;

    return true;
}

double speedpy_clock_delta_seconds(
    const SpeedPyClock *clock
) {
    if (!speedpy_clock_valid(clock)) {
        return 0.0;
    }

    return (double)clock->delta_ticks /
           (double)clock->frequency;
}

double speedpy_clock_elapsed_seconds(
    const SpeedPyClock *clock
) {
    if (!speedpy_clock_valid(clock)) {
        return 0.0;
    }

    return (double)clock->elapsed_ticks /
           (double)clock->frequency;
}

uint64_t speedpy_clock_generation(
    const SpeedPyClock *clock
) {
    if (!speedpy_clock_valid(clock)) {
        return 0;
    }

    return clock->generation;
}

bool speedpy_clock_lock(
    SpeedPyClock *clock
) {
    if (!speedpy_clock_valid(clock)) {
        return false;
    }

    clock->locked = true;
    return true;
}

bool speedpy_clock_unlock(
    SpeedPyClock *clock
) {
    if (!speedpy_clock_valid(clock)) {
        return false;
    }

    clock->locked = false;
    return true;
}

bool speedpy_clock_is_locked(
    const SpeedPyClock *clock
) {
    if (!speedpy_clock_valid(clock)) {
        return false;
    }

    return clock->locked;
}

void speedpy_clock_set_user_data(
    SpeedPyClock *clock,
    void *user_data
) {
    if (!speedpy_clock_valid(clock)) {
        return;
    }

    clock->user_data = user_data;
}

void *speedpy_clock_get_user_data(
    const SpeedPyClock *clock
) {
    if (!speedpy_clock_valid(clock)) {
        return NULL;
    }

    return clock->user_data;
}

uint64_t speedpy_time_ticks_to_units(
    uint64_t ticks,
    uint64_t frequency
) {
    if (frequency == 0) {
        return 0;
    }

    return ticks / frequency;
}

uint64_t speedpy_time_units_to_ticks(
    uint64_t units,
    uint64_t frequency
) {
    if (frequency == 0) {
        return 0;
    }

    if (units >
        UINT64_MAX / frequency) {
        return UINT64_MAX;
    }

    return units * frequency;
}