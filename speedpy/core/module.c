#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define SPEEDPY_MODULE_NAME_SIZE 128
#define SPEEDPY_MODULE_VERSION_SIZE 32
#define SPEEDPY_MODULE_DESCRIPTION_SIZE 256
#define SPEEDPY_MODULE_MAX_MODULES 256
#define SPEEDPY_MODULE_MAX_DEPENDENCIES 32
#define SPEEDPY_MODULE_DEPENDENCY_SIZE 128

typedef enum {
    SPEEDPY_MODULE_CREATED = 0,
    SPEEDPY_MODULE_REGISTERED = 1,
    SPEEDPY_MODULE_INITIALIZED = 2,
    SPEEDPY_MODULE_RUNNING = 3,
    SPEEDPY_MODULE_PAUSED = 4,
    SPEEDPY_MODULE_STOPPED = 5,
    SPEEDPY_MODULE_FAILED = 6,
    SPEEDPY_MODULE_DESTROYED = 7
} SpeedPyModuleState;

typedef struct {
    char name[SPEEDPY_MODULE_NAME_SIZE];
    char version[SPEEDPY_MODULE_VERSION_SIZE];
    char description[SPEEDPY_MODULE_DESCRIPTION_SIZE];

    uint64_t id;

    SpeedPyModuleState state;

    bool enabled;
    bool required;

    char dependencies[
        SPEEDPY_MODULE_MAX_DEPENDENCIES
    ][SPEEDPY_MODULE_DEPENDENCY_SIZE];

    size_t dependency_count;

    uint64_t initialize_count;
    uint64_t start_count;
    uint64_t stop_count;
    uint64_t update_count;
} SpeedPyModule;

typedef struct {
    SpeedPyModule modules[SPEEDPY_MODULE_MAX_MODULES];

    size_t count;
    uint64_t next_id;
    uint64_t generation;

    bool initialized;
} SpeedPyModuleManager;

static bool speedpy_module_valid_name(const char *name)
{
    size_t length;

    if (name == NULL) {
        return false;
    }

    length = strlen(name);

    return length > 0 && length < SPEEDPY_MODULE_NAME_SIZE;
}

static SpeedPyModule *speedpy_module_find(
    SpeedPyModuleManager *manager,
    const char *name
)
{
    size_t i;

    if (manager == NULL || name == NULL) {
        return NULL;
    }

    for (i = 0; i < manager->count; i++) {
        if (strcmp(manager->modules[i].name, name) == 0) {
            return &manager->modules[i];
        }
    }

    return NULL;
}

static const SpeedPyModule *speedpy_module_find_const(
    const SpeedPyModuleManager *manager,
    const char *name
)
{
    size_t i;

    if (manager == NULL || name == NULL) {
        return NULL;
    }

    for (i = 0; i < manager->count; i++) {
        if (strcmp(manager->modules[i].name, name) == 0) {
            return &manager->modules[i];
        }
    }

    return NULL;
}

SpeedPyModuleManager *speedpy_module_manager_create(void)
{
    SpeedPyModuleManager *manager;

    manager = (SpeedPyModuleManager *)calloc(
        1,
        sizeof(SpeedPyModuleManager)
    );

    if (manager == NULL) {
        return NULL;
    }

    manager->count = 0;
    manager->next_id = 1;
    manager->generation = 1;
    manager->initialized = true;

    return manager;
}

void speedpy_module_manager_destroy(
    SpeedPyModuleManager *manager
)
{
    if (manager == NULL) {
        return;
    }

    free(manager);
}

bool speedpy_module_register(
    SpeedPyModuleManager *manager,
    const char *name,
    const char *version,
    const char *description,
    bool required
)
{
    SpeedPyModule *module;
    size_t length;

    if (manager == NULL ||
        !speedpy_module_valid_name(name)) {
        return false;
    }

    if (speedpy_module_find(manager, name) != NULL) {
        return false;
    }

    if (manager->count >= SPEEDPY_MODULE_MAX_MODULES) {
        return false;
    }

    module = &manager->modules[manager->count];

    memset(module, 0, sizeof(SpeedPyModule));

    length = strlen(name);
    memcpy(module->name, name, length);
    module->name[length] = '\0';

    if (version != NULL) {
        length = strlen(version);

        if (length >= SPEEDPY_MODULE_VERSION_SIZE) {
            length = SPEEDPY_MODULE_VERSION_SIZE - 1;
        }

        memcpy(module->version, version, length);
        module->version[length] = '\0';
    }

    if (description != NULL) {
        length = strlen(description);

        if (length >= SPEEDPY_MODULE_DESCRIPTION_SIZE) {
            length = SPEEDPY_MODULE_DESCRIPTION_SIZE - 1;
        }

        memcpy(module->description, description, length);
        module->description[length] = '\0';
    }

    module->id = manager->next_id++;
    module->state = SPEEDPY_MODULE_CREATED;
    module->enabled = true;
    module->required = required;

    manager->count++;
    manager->generation++;

    return true;
}

bool speedpy_module_unregister(
    SpeedPyModuleManager *manager,
    const char *name
)
{
    size_t index;
    size_t i;

    if (manager == NULL || name == NULL) {
        return false;
    }

    index = manager->count;

    for (i = 0; i < manager->count; i++) {
        if (strcmp(manager->modules[i].name, name) == 0) {
            index = i;
            break;
        }
    }

    if (index == manager->count) {
        return false;
    }

    if (manager->modules[index].state == SPEEDPY_MODULE_RUNNING) {
        return false;
    }

    for (i = index; i + 1 < manager->count; i++) {
        manager->modules[i] = manager->modules[i + 1];
    }

    memset(
        &manager->modules[manager->count - 1],
        0,
        sizeof(SpeedPyModule)
    );

    manager->count--;
    manager->generation++;

    return true;
}

bool speedpy_module_add_dependency(
    SpeedPyModuleManager *manager,
    const char *module_name,
    const char *dependency
)
{
    SpeedPyModule *module;
    size_t length;

    if (manager == NULL ||
        module_name == NULL ||
        dependency == NULL) {
        return false;
    }

    module = speedpy_module_find(manager, module_name);

    if (module == NULL) {
        return false;
    }

    if (!speedpy_module_valid_name(dependency)) {
        return false;
    }

    if (module->dependency_count >=
        SPEEDPY_MODULE_MAX_DEPENDENCIES) {
        return false;
    }

    if (strcmp(module_name, dependency) == 0) {
        return false;
    }

    for (length = 0;
         length < module->dependency_count;
         length++) {
        if (strcmp(
            module->dependencies[length],
            dependency
        ) == 0) {
            return false;
        }
    }

    length = strlen(dependency);

    if (length >= SPEEDPY_MODULE_DEPENDENCY_SIZE) {
        return false;
    }

    memcpy(
        module->dependencies[module->dependency_count],
        dependency,
        length
    );

    module->dependencies[module->dependency_count][length] = '\0';

    module->dependency_count++;
    manager->generation++;

    return true;
}

bool speedpy_module_remove_dependency(
    SpeedPyModuleManager *manager,
    const char *module_name,
    const char *dependency
)
{
    SpeedPyModule *module;
    size_t index;
    size_t i;

    if (manager == NULL ||
        module_name == NULL ||
        dependency == NULL) {
        return false;
    }

    module = speedpy_module_find(manager, module_name);

    if (module == NULL) {
        return false;
    }

    index = module->dependency_count;

    for (i = 0; i < module->dependency_count; i++) {
        if (strcmp(
            module->dependencies[i],
            dependency
        ) == 0) {
            index = i;
            break;
        }
    }

    if (index == module->dependency_count) {
        return false;
    }

    for (i = index;
         i + 1 < module->dependency_count;
         i++) {
        memcpy(
            module->dependencies[i],
            module->dependencies[i + 1],
            SPEEDPY_MODULE_DEPENDENCY_SIZE
        );
    }

    memset(
        module->dependencies[
            module->dependency_count - 1
        ],
        0,
        SPEEDPY_MODULE_DEPENDENCY_SIZE
    );

    module->dependency_count--;
    manager->generation++;

    return true;
}

bool speedpy_module_initialize(
    SpeedPyModuleManager *manager,
    const char *name
)
{
    SpeedPyModule *module;

    if (manager == NULL || name == NULL) {
        return false;
    }

    module = speedpy_module_find(manager, name);

    if (module == NULL || !module->enabled) {
        return false;
    }

    if (module->state != SPEEDPY_MODULE_CREATED &&
        module->state != SPEEDPY_MODULE_STOPPED) {
        return false;
    }

    module->state = SPEEDPY_MODULE_INITIALIZED;
    module->initialize_count++;

    manager->generation++;

    return true;
}

bool speedpy_module_start(
    SpeedPyModuleManager *manager,
    const char *name
)
{
    SpeedPyModule *module;

    if (manager == NULL || name == NULL) {
        return false;
    }

    module = speedpy_module_find(manager, name);

    if (module == NULL ||
        !module->enabled ||
        module->state != SPEEDPY_MODULE_INITIALIZED) {
        return false;
    }

    module->state = SPEEDPY_MODULE_RUNNING;
    module->start_count++;

    manager->generation++;

    return true;
}

bool speedpy_module_pause(
    SpeedPyModuleManager *manager,
    const char *name
)
{
    SpeedPyModule *module;

    if (manager == NULL || name == NULL) {
        return false;
    }

    module = speedpy_module_find(manager, name);

    if (module == NULL ||
        module->state != SPEEDPY_MODULE_RUNNING) {
        return false;
    }

    module->state = SPEEDPY_MODULE_PAUSED;

    manager->generation++;

    return true;
}

bool speedpy_module_resume(
    SpeedPyModuleManager *manager,
    const char *name
)
{
    SpeedPyModule *module;

    if (manager == NULL || name == NULL) {
        return false;
    }

    module = speedpy_module_find(manager, name);

    if (module == NULL ||
        module->state != SPEEDPY_MODULE_PAUSED) {
        return false;
    }

    module->state = SPEEDPY_MODULE_RUNNING;

    manager->generation++;

    return true;
}

bool speedpy_module_stop(
    SpeedPyModuleManager *manager,
    const char *name
)
{
    SpeedPyModule *module;

    if (manager == NULL || name == NULL) {
        return false;
    }

    module = speedpy_module_find(manager, name);

    if (module == NULL) {
        return false;
    }

    if (module->state != SPEEDPY_MODULE_RUNNING &&
        module->state != SPEEDPY_MODULE_PAUSED &&
        module->state != SPEEDPY_MODULE_INITIALIZED) {
        return false;
    }

    module->state = SPEEDPY_MODULE_STOPPED;
    module->stop_count++;

    manager->generation++;

    return true;
}

bool speedpy_module_enable(
    SpeedPyModuleManager *manager,
    const char *name
)
{
    SpeedPyModule *module;

    if (manager == NULL || name == NULL) {
        return false;
    }

    module = speedpy_module_find(manager, name);

    if (module == NULL) {
        return false;
    }

    module->enabled = true;
    manager->generation++;

    return true;
}

bool speedpy_module_disable(
    SpeedPyModuleManager *manager,
    const char *name
)
{
    SpeedPyModule *module;

    if (manager == NULL || name == NULL) {
        return false;
    }

    module = speedpy_module_find(manager, name);

    if (module == NULL) {
        return false;
    }

    if (module->state == SPEEDPY_MODULE_RUNNING ||
        module->state == SPEEDPY_MODULE_PAUSED) {
        return false;
    }

    module->enabled = false;
    manager->generation++;

    return true;
}

bool speedpy_module_is_enabled(
    const SpeedPyModuleManager *manager,
    const char *name
)
{
    const SpeedPyModule *module;

    module = speedpy_module_find_const(manager, name);

    if (module == NULL) {
        return false;
    }

    return module->enabled;
}

bool speedpy_module_update(
    SpeedPyModuleManager *manager,
    const char *name
)
{
    SpeedPyModule *module;

    if (manager == NULL || name == NULL) {
        return false;
    }

    module = speedpy_module_find(manager, name);

    if (module == NULL ||
        module->state != SPEEDPY_MODULE_RUNNING) {
        return false;
    }

    module->update_count++;

    return true;
}

SpeedPyModule *speedpy_module_get(
    SpeedPyModuleManager *manager,
    const char *name
)
{
    return speedpy_module_find(manager, name);
}

const SpeedPyModule *speedpy_module_get_const(
    const SpeedPyModuleManager *manager,
    const char *name
)
{
    return speedpy_module_find_const(manager, name);
}

size_t speedpy_module_count(
    const SpeedPyModuleManager *manager
)
{
    if (manager == NULL) {
        return 0;
    }

    return manager->count;
}

uint64_t speedpy_module_generation(
    const SpeedPyModuleManager *manager
)
{
    if (manager == NULL) {
        return 0;
    }

    return manager->generation;
}

bool speedpy_module_exists(
    const SpeedPyModuleManager *manager,
    const char *name
)
{
    return speedpy_module_find_const(manager, name) != NULL;
}