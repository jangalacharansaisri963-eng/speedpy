#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define SPEEDPY_MEMORY_MAX_BLOCKS 2048
#define SPEEDPY_MEMORY_NAME_SIZE 96

typedef struct {
    void *pointer;
    size_t size;

    uint64_t id;

    char name[SPEEDPY_MEMORY_NAME_SIZE];

    bool active;
    bool zeroed;

    uint64_t allocation_tick;
} SpeedPyMemoryBlock;

typedef struct {
    SpeedPyMemoryBlock blocks[SPEEDPY_MEMORY_MAX_BLOCKS];

    size_t block_count;
    size_t active_blocks;

    uint64_t next_id;

    size_t current_bytes;
    size_t peak_bytes;
    size_t total_allocated;
    size_t total_freed;

    uint64_t allocation_count;
    uint64_t free_count;
    uint64_t failed_allocations;

    size_t allocation_limit;

    bool enabled;
    bool locked;

    uint64_t generation;

    void *user_data;
} SpeedPyMemoryManager;

static SpeedPyMemoryBlock *speedpy_memory_find_pointer(
    SpeedPyMemoryManager *manager,
    void *pointer
)
{
    size_t i;

    if (manager == NULL || pointer == NULL) {
        return NULL;
    }

    for (i = 0; i < manager->block_count; ++i) {
        if (manager->blocks[i].active &&
            manager->blocks[i].pointer == pointer) {
            return &manager->blocks[i];
        }
    }

    return NULL;
}

static SpeedPyMemoryBlock *speedpy_memory_find_id(
    SpeedPyMemoryManager *manager,
    uint64_t id
)
{
    size_t i;

    if (manager == NULL || id == 0) {
        return NULL;
    }

    for (i = 0; i < manager->block_count; ++i) {
        if (manager->blocks[i].active &&
            manager->blocks[i].id == id) {
            return &manager->blocks[i];
        }
    }

    return NULL;
}

static SpeedPyMemoryBlock *speedpy_memory_find_free_slot(
    SpeedPyMemoryManager *manager
)
{
    size_t i;

    if (manager == NULL) {
        return NULL;
    }

    for (i = 0; i < manager->block_count; ++i) {
        if (!manager->blocks[i].active) {
            return &manager->blocks[i];
        }
    }

    if (manager->block_count >= SPEEDPY_MEMORY_MAX_BLOCKS) {
        return NULL;
    }

    return &manager->blocks[manager->block_count++];
}

SpeedPyMemoryManager *speedpy_memory_create(void)
{
    SpeedPyMemoryManager *manager;

    manager = (SpeedPyMemoryManager *)calloc(
        1,
        sizeof(SpeedPyMemoryManager)
    );

    if (manager == NULL) {
        return NULL;
    }

    manager->next_id = 1;
    manager->enabled = true;
    manager->generation = 1;

    return manager;
}

void speedpy_memory_destroy(
    SpeedPyMemoryManager *manager
)
{
    size_t i;

    if (manager == NULL) {
        return;
    }

    for (i = 0; i < manager->block_count; ++i) {
        if (manager->blocks[i].active &&
            manager->blocks[i].pointer != NULL) {
            free(manager->blocks[i].pointer);
            manager->blocks[i].pointer = NULL;
            manager->blocks[i].active = false;
        }
    }

    free(manager);
}

void *speedpy_memory_allocate(
    SpeedPyMemoryManager *manager,
    size_t size,
    const char *name,
    uint64_t tick
)
{
    void *pointer;
    SpeedPyMemoryBlock *block;

    if (manager == NULL ||
        manager->locked ||
        !manager->enabled ||
        size == 0) {
        return NULL;
    }

    if (manager->allocation_limit != 0 &&
        size > manager->allocation_limit - manager->current_bytes) {
        manager->failed_allocations++;
        return NULL;
    }

    block = speedpy_memory_find_free_slot(manager);

    if (block == NULL) {
        manager->failed_allocations++;
        return NULL;
    }

    pointer = malloc(size);

    if (pointer == NULL) {
        manager->failed_allocations++;
        return NULL;
    }

    memset(block, 0, sizeof(SpeedPyMemoryBlock));

    block->pointer = pointer;
    block->size = size;
    block->id = manager->next_id++;
    block->active = true;
    block->allocation_tick = tick;

    if (name != NULL) {
        strncpy(
            block->name,
            name,
            SPEEDPY_MEMORY_NAME_SIZE - 1
        );

        block->name[SPEEDPY_MEMORY_NAME_SIZE - 1] = '\0';
    }

    manager->active_blocks++;
    manager->current_bytes += size;
    manager->total_allocated += size;
    manager->allocation_count++;

    if (manager->current_bytes > manager->peak_bytes) {
        manager->peak_bytes = manager->current_bytes;
    }

    manager->generation++;

    return pointer;
}

void *speedpy_memory_callocate(
    SpeedPyMemoryManager *manager,
    size_t count,
    size_t size,
    const char *name,
    uint64_t tick
)
{
    size_t total;
    void *pointer;

    if (manager == NULL ||
        count == 0 ||
        size == 0) {
        return NULL;
    }

    if (size > SIZE_MAX / count) {
        manager->failed_allocations++;
        return NULL;
    }

    total = count * size;

    pointer = speedpy_memory_allocate(
        manager,
        total,
        name,
        tick
    );

    if (pointer != NULL) {
        memset(pointer, 0, total);

        {
            SpeedPyMemoryBlock *block =
                speedpy_memory_find_pointer(
                    manager,
                    pointer
                );

            if (block != NULL) {
                block->zeroed = true;
            }
        }
    }

    return pointer;
}

int speedpy_memory_free(
    SpeedPyMemoryManager *manager,
    void *pointer
)
{
    SpeedPyMemoryBlock *block;

    if (manager == NULL ||
        manager->locked ||
        pointer == NULL) {
        return -1;
    }

    block = speedpy_memory_find_pointer(
        manager,
        pointer
    );

    if (block == NULL) {
        return -2;
    }

    free(block->pointer);

    if (manager->current_bytes >= block->size) {
        manager->current_bytes -= block->size;
    } else {
        manager->current_bytes = 0;
    }

    manager->total_freed += block->size;
    manager->free_count++;

    if (manager->active_blocks > 0) {
        manager->active_blocks--;
    }

    block->pointer = NULL;
    block->size = 0;
    block->active = false;
    block->zeroed = false;

    manager->generation++;

    return 0;
}

int speedpy_memory_reallocate(
    SpeedPyMemoryManager *manager,
    void *pointer,
    size_t new_size,
    uint64_t tick
)
{
    SpeedPyMemoryBlock *block;
    void *new_pointer;
    size_t old_size;

    if (manager == NULL ||
        manager->locked ||
        pointer == NULL ||
        new_size == 0) {
        return -1;
    }

    block = speedpy_memory_find_pointer(
        manager,
        pointer
    );

    if (block == NULL) {
        return -2;
    }

    old_size = block->size;

    if (manager->allocation_limit != 0) {
        size_t projected = manager->current_bytes;

        if (new_size > old_size) {
            size_t increase = new_size - old_size;

            if (increase > manager->allocation_limit -
                manager->current_bytes) {
                manager->failed_allocations++;
                return -3;
            }

            projected += increase;
        }

        if (projected > manager->allocation_limit) {
            manager->failed_allocations++;
            return -3;
        }
    }

    new_pointer = realloc(pointer, new_size);

    if (new_pointer == NULL) {
        manager->failed_allocations++;
        return -4;
    }

    block->pointer = new_pointer;
    block->size = new_size;
    block->allocation_tick = tick;

    if (new_size >= old_size) {
        manager->current_bytes += new_size - old_size;
        manager->total_allocated += new_size - old_size;
    } else {
        manager->current_bytes -= old_size - new_size;
        manager->total_freed += old_size - new_size;
    }

    if (manager->current_bytes > manager->peak_bytes) {
        manager->peak_bytes = manager->current_bytes;
    }

    manager->generation++;

    return 0;
}

SpeedPyMemoryBlock *speedpy_memory_get(
    SpeedPyMemoryManager *manager,
    void *pointer
)
{
    return speedpy_memory_find_pointer(
        manager,
        pointer
    );
}

SpeedPyMemoryBlock *speedpy_memory_get_id(
    SpeedPyMemoryManager *manager,
    uint64_t id
)
{
    return speedpy_memory_find_id(
        manager,
        id
    );
}

size_t speedpy_memory_current_bytes(
    const SpeedPyMemoryManager *manager
)
{
    if (manager == NULL) {
        return 0;
    }

    return manager->current_bytes;
}

size_t speedpy_memory_peak_bytes(
    const SpeedPyMemoryManager *manager
)
{
    if (manager == NULL) {
        return 0;
    }

    return manager->peak_bytes;
}

size_t speedpy_memory_total_allocated(
    const SpeedPyMemoryManager *manager
)
{
    if (manager == NULL) {
        return 0;
    }

    return manager->total_allocated;
}

size_t speedpy_memory_total_freed(
    const SpeedPyMemoryManager *manager
)
{
    if (manager == NULL) {
        return 0;
    }

    return manager->total_freed;
}

size_t speedpy_memory_active_blocks(
    const SpeedPyMemoryManager *manager
)
{
    if (manager == NULL) {
        return 0;
    }

    return manager->active_blocks;
}

uint64_t speedpy_memory_allocation_count(
    const SpeedPyMemoryManager *manager
)
{
    if (manager == NULL) {
        return 0;
    }

    return manager->allocation_count;
}

uint64_t speedpy_memory_free_count(
    const SpeedPyMemoryManager *manager
)
{
    if (manager == NULL) {
        return 0;
    }

    return manager->free_count;
}

uint64_t speedpy_memory_failed_allocations(
    const SpeedPyMemoryManager *manager
)
{
    if (manager == NULL) {
        return 0;
    }

    return manager->failed_allocations;
}

void speedpy_memory_set_limit(
    SpeedPyMemoryManager *manager,
    size_t limit
)
{
    if (manager == NULL || manager->locked) {
        return;
    }

    manager->allocation_limit = limit;
    manager->generation++;
}

size_t speedpy_memory_limit(
    const SpeedPyMemoryManager *manager
)
{
    if (manager == NULL) {
        return 0;
    }

    return manager->allocation_limit;
}

size_t speedpy_memory_available(
    const SpeedPyMemoryManager *manager
)
{
    if (manager == NULL ||
        manager->allocation_limit == 0) {
        return 0;
    }

    if (manager->current_bytes >=
        manager->allocation_limit) {
        return 0;
    }

    return manager->allocation_limit -
           manager->current_bytes;
}

void speedpy_memory_set_enabled(
    SpeedPyMemoryManager *manager,
    bool enabled
)
{
    if (manager == NULL || manager->locked) {
        return;
    }

    manager->enabled = enabled;
    manager->generation++;
}

bool speedpy_memory_is_enabled(
    const SpeedPyMemoryManager *manager
)
{
    return manager != NULL && manager->enabled;
}

uint64_t speedpy_memory_generation(
    const SpeedPyMemoryManager *manager
)
{
    if (manager == NULL) {
        return 0;
    }

    return manager->generation;
}

void speedpy_memory_lock(
    SpeedPyMemoryManager *manager
)
{
    if (manager != NULL) {
        manager->locked = true;
    }
}

void speedpy_memory_unlock(
    SpeedPyMemoryManager *manager
)
{
    if (manager != NULL) {
        manager->locked = false;
    }
}

bool speedpy_memory_is_locked(
    const SpeedPyMemoryManager *manager
)
{
    return manager != NULL && manager->locked;
}

void speedpy_memory_set_user_data(
    SpeedPyMemoryManager *manager,
    void *user_data
)
{
    if (manager != NULL) {
        manager->user_data = user_data;
    }
}

void *speedpy_memory_get_user_data(
    const SpeedPyMemoryManager *manager
)
{
    if (manager == NULL) {
        return NULL;
    }

    return manager->user_data;
}

void speedpy_memory_reset_statistics(
    SpeedPyMemoryManager *manager
)
{
    if (manager == NULL || manager->locked) {
        return;
    }

    manager->total_allocated = 0;
    manager->total_freed = 0;
    manager->allocation_count = 0;
    manager->free_count = 0;
    manager->failed_allocations = 0;
    manager->peak_bytes = manager->current_bytes;

    manager->generation++;
}