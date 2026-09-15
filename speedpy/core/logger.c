#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define SPEEDPY_LOGGER_MAX_ENTRIES 1024
#define SPEEDPY_LOGGER_MESSAGE_SIZE 512
#define SPEEDPY_LOGGER_SOURCE_SIZE 96
#define SPEEDPY_LOGGER_NAME_SIZE 96

typedef enum {
    SPEEDPY_LOG_TRACE = 0,
    SPEEDPY_LOG_DEBUG = 1,
    SPEEDPY_LOG_INFO = 2,
    SPEEDPY_LOG_WARNING = 3,
    SPEEDPY_LOG_ERROR = 4,
    SPEEDPY_LOG_CRITICAL = 5
} SpeedPyLogLevel;

typedef struct {
    uint64_t id;
    uint64_t tick;

    SpeedPyLogLevel level;

    char source[SPEEDPY_LOGGER_SOURCE_SIZE];
    char message[SPEEDPY_LOGGER_MESSAGE_SIZE];

    bool active;
} SpeedPyLogEntry;

typedef struct {
    char name[SPEEDPY_LOGGER_NAME_SIZE];

    SpeedPyLogEntry entries[SPEEDPY_LOGGER_MAX_ENTRIES];

    size_t entry_count;
    size_t write_index;

    uint64_t next_id;

    uint64_t total_count;
    uint64_t trace_count;
    uint64_t debug_count;
    uint64_t info_count;
    uint64_t warning_count;
    uint64_t error_count;
    uint64_t critical_count;

    SpeedPyLogLevel minimum_level;

    bool enabled;
    bool locked;
    bool circular;

    uint64_t generation;

    void *user_data;
} SpeedPyLogger;

static bool speedpy_logger_valid_name(const char *name)
{
    return name != NULL && name[0] != '\0';
}

static bool speedpy_logger_valid_level(
    SpeedPyLogLevel level
)
{
    return level >= SPEEDPY_LOG_TRACE &&
           level <= SPEEDPY_LOG_CRITICAL;
}

static void speedpy_logger_count_level(
    SpeedPyLogger *logger,
    SpeedPyLogLevel level
)
{
    if (logger == NULL) {
        return;
    }

    switch (level) {
        case SPEEDPY_LOG_TRACE:
            logger->trace_count++;
            break;

        case SPEEDPY_LOG_DEBUG:
            logger->debug_count++;
            break;

        case SPEEDPY_LOG_INFO:
            logger->info_count++;
            break;

        case SPEEDPY_LOG_WARNING:
            logger->warning_count++;
            break;

        case SPEEDPY_LOG_ERROR:
            logger->error_count++;
            break;

        case SPEEDPY_LOG_CRITICAL:
            logger->critical_count++;
            break;

        default:
            break;
    }
}

static SpeedPyLogEntry *speedpy_logger_next_entry(
    SpeedPyLogger *logger
)
{
    SpeedPyLogEntry *entry;

    if (logger == NULL) {
        return NULL;
    }

    if (logger->entry_count < SPEEDPY_LOGGER_MAX_ENTRIES) {
        entry = &logger->entries[logger->entry_count];
        logger->entry_count++;
        return entry;
    }

    if (!logger->circular) {
        return NULL;
    }

    entry = &logger->entries[logger->write_index];

    logger->write_index++;

    if (logger->write_index >= SPEEDPY_LOGGER_MAX_ENTRIES) {
        logger->write_index = 0;
    }

    return entry;
}

SpeedPyLogger *speedpy_logger_create(
    const char *name
)
{
    SpeedPyLogger *logger;

    if (!speedpy_logger_valid_name(name)) {
        return NULL;
    }

    logger = (SpeedPyLogger *)calloc(
        1,
        sizeof(SpeedPyLogger)
    );

    if (logger == NULL) {
        return NULL;
    }

    strncpy(
        logger->name,
        name,
        SPEEDPY_LOGGER_NAME_SIZE - 1
    );

    logger->name[SPEEDPY_LOGGER_NAME_SIZE - 1] = '\0';

    logger->next_id = 1;
    logger->minimum_level = SPEEDPY_LOG_INFO;
    logger->enabled = true;
    logger->circular = true;
    logger->generation = 1;

    return logger;
}

void speedpy_logger_destroy(
    SpeedPyLogger *logger
)
{
    if (logger == NULL) {
        return;
    }

    free(logger);
}

int speedpy_logger_write(
    SpeedPyLogger *logger,
    SpeedPyLogLevel level,
    const char *source,
    const char *message,
    uint64_t tick
)
{
    SpeedPyLogEntry *entry;

    if (logger == NULL ||
        logger->locked ||
        !logger->enabled ||
        !speedpy_logger_valid_level(level) ||
        source == NULL ||
        message == NULL) {
        return -1;
    }

    if (level < logger->minimum_level) {
        return 0;
    }

    entry = speedpy_logger_next_entry(logger);

    if (entry == NULL) {
        return -2;
    }

    memset(entry, 0, sizeof(SpeedPyLogEntry));

    entry->id = logger->next_id++;
    entry->tick = tick;
    entry->level = level;
    entry->active = true;

    strncpy(
        entry->source,
        source,
        SPEEDPY_LOGGER_SOURCE_SIZE - 1
    );

    entry->source[SPEEDPY_LOGGER_SOURCE_SIZE - 1] = '\0';

    strncpy(
        entry->message,
        message,
        SPEEDPY_LOGGER_MESSAGE_SIZE - 1
    );

    entry->message[SPEEDPY_LOGGER_MESSAGE_SIZE - 1] = '\0';

    logger->total_count++;
    speedpy_logger_count_level(logger, level);

    logger->generation++;

    return 0;
}

int speedpy_logger_trace(
    SpeedPyLogger *logger,
    const char *source,
    const char *message,
    uint64_t tick
)
{
    return speedpy_logger_write(
        logger,
        SPEEDPY_LOG_TRACE,
        source,
        message,
        tick
    );
}

int speedpy_logger_debug(
    SpeedPyLogger *logger,
    const char *source,
    const char *message,
    uint64_t tick
)
{
    return speedpy_logger_write(
        logger,
        SPEEDPY_LOG_DEBUG,
        source,
        message,
        tick
    );
}

int speedpy_logger_info(
    SpeedPyLogger *logger,
    const char *source,
    const char *message,
    uint64_t tick
)
{
    return speedpy_logger_write(
        logger,
        SPEEDPY_LOG_INFO,
        source,
        message,
        tick
    );
}

int speedpy_logger_warning(
    SpeedPyLogger *logger,
    const char *source,
    const char *message,
    uint64_t tick
)
{
    return speedpy_logger_write(
        logger,
        SPEEDPY_LOG_WARNING,
        source,
        message,
        tick
    );
}

int speedpy_logger_error(
    SpeedPyLogger *logger,
    const char *source,
    const char *message,
    uint64_t tick
)
{
    return speedpy_logger_write(
        logger,
        SPEEDPY_LOG_ERROR,
        source,
        message,
        tick
    );
}

int speedpy_logger_critical(
    SpeedPyLogger *logger,
    const char *source,
    const char *message,
    uint64_t tick
)
{
    return speedpy_logger_write(
        logger,
        SPEEDPY_LOG_CRITICAL,
        source,
        message,
        tick
    );
}

SpeedPyLogEntry *speedpy_logger_get_entry(
    SpeedPyLogger *logger,
    size_t index
)
{
    if (logger == NULL ||
        index >= logger->entry_count) {
        return NULL;
    }

    return &logger->entries[index];
}

SpeedPyLogEntry *speedpy_logger_find_id(
    SpeedPyLogger *logger,
    uint64_t id
)
{
    size_t i;

    if (logger == NULL || id == 0) {
        return NULL;
    }

    for (i = 0; i < logger->entry_count; ++i) {
        if (logger->entries[i].active &&
            logger->entries[i].id == id) {
            return &logger->entries[i];
        }
    }

    return NULL;
}

size_t speedpy_logger_count(
    const SpeedPyLogger *logger
)
{
    if (logger == NULL) {
        return 0;
    }

    return logger->entry_count;
}

uint64_t speedpy_logger_total_count(
    const SpeedPyLogger *logger
)
{
    if (logger == NULL) {
        return 0;
    }

    return logger->total_count;
}

uint64_t speedpy_logger_count_level(
    const SpeedPyLogger *logger,
    SpeedPyLogLevel level
)
{
    if (logger == NULL) {
        return 0;
    }

    switch (level) {
        case SPEEDPY_LOG_TRACE:
            return logger->trace_count;

        case SPEEDPY_LOG_DEBUG:
            return logger->debug_count;

        case SPEEDPY_LOG_INFO:
            return logger->info_count;

        case SPEEDPY_LOG_WARNING:
            return logger->warning_count;

        case SPEEDPY_LOG_ERROR:
            return logger->error_count;

        case SPEEDPY_LOG_CRITICAL:
            return logger->critical_count;

        default:
            return 0;
    }
}

void speedpy_logger_set_minimum_level(
    SpeedPyLogger *logger,
    SpeedPyLogLevel level
)
{
    if (logger == NULL ||
        logger->locked ||
        !speedpy_logger_valid_level(level)) {
        return;
    }

    logger->minimum_level = level;
    logger->generation++;
}

SpeedPyLogLevel speedpy_logger_minimum_level(
    const SpeedPyLogger *logger
)
{
    if (logger == NULL) {
        return SPEEDPY_LOG_INFO;
    }

    return logger->minimum_level;
}

void speedpy_logger_set_enabled(
    SpeedPyLogger *logger,
    bool enabled
)
{
    if (logger == NULL || logger->locked) {
        return;
    }

    logger->enabled = enabled;
    logger->generation++;
}

bool speedpy_logger_is_enabled(
    const SpeedPyLogger *logger
)
{
    return logger != NULL && logger->enabled;
}

void speedpy_logger_set_circular(
    SpeedPyLogger *logger,
    bool circular
)
{
    if (logger == NULL || logger->locked) {
        return;
    }

    logger->circular = circular;
    logger->generation++;
}

bool speedpy_logger_is_circular(
    const SpeedPyLogger *logger
)
{
    return logger != NULL && logger->circular;
}

void speedpy_logger_clear(
    SpeedPyLogger *logger
)
{
    if (logger == NULL || logger->locked) {
        return;
    }

    memset(
        logger->entries,
        0,
        sizeof(logger->entries)
    );

    logger->entry_count = 0;
    logger->write_index = 0;

    logger->generation++;
}

void speedpy_logger_reset_statistics(
    SpeedPyLogger *logger
)
{
    if (logger == NULL || logger->locked) {
        return;
    }

    logger->total_count = 0;
    logger->trace_count = 0;
    logger->debug_count = 0;
    logger->info_count = 0;
    logger->warning_count = 0;
    logger->error_count = 0;
    logger->critical_count = 0;

    logger->generation++;
}

void speedpy_logger_lock(
    SpeedPyLogger *logger
)
{
    if (logger != NULL) {
        logger->locked = true;
    }
}

void speedpy_logger_unlock(
    SpeedPyLogger *logger
)
{
    if (logger != NULL) {
        logger->locked = false;
    }
}

bool speedpy_logger_is_locked(
    const SpeedPyLogger *logger
)
{
    return logger != NULL && logger->locked;
}

uint64_t speedpy_logger_generation(
    const SpeedPyLogger *logger
)
{
    if (logger == NULL) {
        return 0;
    }

    return logger->generation;
}

const char *speedpy_logger_name(
    const SpeedPyLogger *logger
)
{
    if (logger == NULL) {
        return NULL;
    }

    return logger->name;
}

void speedpy_logger_set_user_data(
    SpeedPyLogger *logger,
    void *user_data
)
{
    if (logger != NULL) {
        logger->user_data = user_data;
    }
}

void *speedpy_logger_get_user_data(
    const SpeedPyLogger *logger
)
{
    if (logger == NULL) {
        return NULL;
    }

    return logger->user_data;
}