#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define SPEEDPY_CONTAINER_INITIAL_CAPACITY 16
#define SPEEDPY_CONTAINER_MAX_CAPACITY 1048576

typedef struct {
    unsigned char *data;
    size_t element_size;
    size_t count;
    size_t capacity;
    bool locked;
    uint64_t generation;
    void *user_data;
} SpeedPyVector;

typedef struct {
    unsigned char *data;
    size_t element_size;
    size_t count;
    size_t capacity;
    size_t head;
    size_t tail;
    bool locked;
    uint64_t generation;
    void *user_data;
} SpeedPyQueue;

static bool speedpy_vector_valid(
    const SpeedPyVector *vector
) {
    return vector != NULL &&
           vector->data != NULL &&
           vector->element_size > 0 &&
           vector->capacity > 0 &&
           vector->count <= vector->capacity;
}

static bool speedpy_queue_valid(
    const SpeedPyQueue *queue
) {
    return queue != NULL &&
           queue->data != NULL &&
           queue->element_size > 0 &&
           queue->capacity > 0 &&
           queue->count <= queue->capacity;
}

static size_t speedpy_container_next_capacity(
    size_t current,
    size_t required
) {
    size_t capacity = current;

    if (capacity == 0) {
        capacity = SPEEDPY_CONTAINER_INITIAL_CAPACITY;
    }

    while (capacity < required) {
        if (capacity >= SPEEDPY_CONTAINER_MAX_CAPACITY / 2) {
            capacity = SPEEDPY_CONTAINER_MAX_CAPACITY;
            break;
        }

        capacity *= 2;
    }

    return capacity;
}

SpeedPyVector *speedpy_vector_create(
    size_t element_size
) {
    SpeedPyVector *vector;

    if (element_size == 0) {
        return NULL;
    }

    vector =
        (SpeedPyVector *)calloc(1, sizeof(SpeedPyVector));

    if (vector == NULL) {
        return NULL;
    }

    vector->capacity =
        SPEEDPY_CONTAINER_INITIAL_CAPACITY;

    if (element_size >
        SIZE_MAX / vector->capacity) {
        free(vector);
        return NULL;
    }

    vector->data = (unsigned char *)calloc(
        vector->capacity,
        element_size
    );

    if (vector->data == NULL) {
        free(vector);
        return NULL;
    }

    vector->element_size = element_size;
    vector->generation = 1;

    return vector;
}

void speedpy_vector_destroy(
    SpeedPyVector *vector
) {
    if (vector == NULL) {
        return;
    }

    free(vector->data);
    vector->data = NULL;

    free(vector);
}

bool speedpy_vector_reserve(
    SpeedPyVector *vector,
    size_t capacity
) {
    unsigned char *new_data;
    size_t bytes;

    if (!speedpy_vector_valid(vector) ||
        vector->locked) {
        return false;
    }

    if (capacity <= vector->capacity) {
        return true;
    }

    if (capacity > SPEEDPY_CONTAINER_MAX_CAPACITY) {
        return false;
    }

    if (vector->element_size >
        SIZE_MAX / capacity) {
        return false;
    }

    bytes = capacity * vector->element_size;

    new_data =
        (unsigned char *)realloc(
            vector->data,
            bytes
        );

    if (new_data == NULL) {
        return false;
    }

    memset(
        new_data +
        vector->capacity * vector->element_size,
        0,
        bytes -
        vector->capacity * vector->element_size
    );

    vector->data = new_data;
    vector->capacity = capacity;
    vector->generation++;

    return true;
}

bool speedpy_vector_push(
    SpeedPyVector *vector,
    const void *element
) {
    size_t required;
    size_t capacity;

    if (!speedpy_vector_valid(vector) ||
        vector->locked ||
        element == NULL) {
        return false;
    }

    if (vector->count >=
        SPEEDPY_CONTAINER_MAX_CAPACITY) {
        return false;
    }

    required = vector->count + 1;

    if (required > vector->capacity) {
        capacity = speedpy_container_next_capacity(
            vector->capacity,
            required
        );

        if (!speedpy_vector_reserve(
                vector,
                capacity)) {
            return false;
        }
    }

    memcpy(
        vector->data +
        vector->count * vector->element_size,
        element,
        vector->element_size
    );

    vector->count++;
    vector->generation++;

    return true;
}

bool speedpy_vector_pop(
    SpeedPyVector *vector,
    void *output
) {
    unsigned char *element;

    if (!speedpy_vector_valid(vector) ||
        vector->locked ||
        vector->count == 0) {
        return false;
    }

    element =
        vector->data +
        (vector->count - 1) *
        vector->element_size;

    if (output != NULL) {
        memcpy(
            output,
            element,
            vector->element_size
        );
    }

    memset(
        element,
        0,
        vector->element_size
    );

    vector->count--;
    vector->generation++;

    return true;
}

bool speedpy_vector_insert(
    SpeedPyVector *vector,
    size_t index,
    const void *element
) {
    size_t required;
    size_t capacity;

    if (!speedpy_vector_valid(vector) ||
        vector->locked ||
        element == NULL ||
        index > vector->count) {
        return false;
    }

    if (vector->count >=
        SPEEDPY_CONTAINER_MAX_CAPACITY) {
        return false;
    }

    required = vector->count + 1;

    if (required > vector->capacity) {
        capacity = speedpy_container_next_capacity(
            vector->capacity,
            required
        );

        if (!speedpy_vector_reserve(
                vector,
                capacity)) {
            return false;
        }
    }

    memmove(
        vector->data +
        (index + 1) * vector->element_size,
        vector->data +
        index * vector->element_size,
        (vector->count - index) *
        vector->element_size
    );

    memcpy(
        vector->data +
        index * vector->element_size,
        element,
        vector->element_size
    );

    vector->count++;
    vector->generation++;

    return true;
}

bool speedpy_vector_remove(
    SpeedPyVector *vector,
    size_t index,
    void *output
) {
    unsigned char *element;

    if (!speedpy_vector_valid(vector) ||
        vector->locked ||
        index >= vector->count) {
        return false;
    }

    element =
        vector->data +
        index * vector->element_size;

    if (output != NULL) {
        memcpy(
            output,
            element,
            vector->element_size
        );
    }

    memmove(
        element,
        element + vector->element_size,
        (vector->count - index - 1) *
        vector->element_size
    );

    vector->count--;

    memset(
        vector->data +
        vector->count * vector->element_size,
        0,
        vector->element_size
    );

    vector->generation++;

    return true;
}

void *speedpy_vector_get(
    SpeedPyVector *vector,
    size_t index
) {
    if (!speedpy_vector_valid(vector) ||
        index >= vector->count) {
        return NULL;
    }

    return vector->data +
           index * vector->element_size;
}

const void *speedpy_vector_get_const(
    const SpeedPyVector *vector,
    size_t index
) {
    if (!speedpy_vector_valid(vector) ||
        index >= vector->count) {
        return NULL;
    }

    return vector->data +
           index * vector->element_size;
}

bool speedpy_vector_set(
    SpeedPyVector *vector,
    size_t index,
    const void *element
) {
    if (!speedpy_vector_valid(vector) ||
        vector->locked ||
        element == NULL ||
        index >= vector->count) {
        return false;
    }

    memcpy(
        vector->data +
        index * vector->element_size,
        element,
        vector->element_size
    );

    vector->generation++;

    return true;
}

bool speedpy_vector_clear(
    SpeedPyVector *vector
) {
    if (!speedpy_vector_valid(vector) ||
        vector->locked) {
        return false;
    }

    memset(
        vector->data,
        0,
        vector->count * vector->element_size
    );

    vector->count = 0;
    vector->generation++;

    return true;
}

size_t speedpy_vector_count(
    const SpeedPyVector *vector
) {
    if (!speedpy_vector_valid(vector)) {
        return 0;
    }

    return vector->count;
}

size_t speedpy_vector_capacity(
    const SpeedPyVector *vector
) {
    if (!speedpy_vector_valid(vector)) {
        return 0;
    }

    return vector->capacity;
}

size_t speedpy_vector_element_size(
    const SpeedPyVector *vector
) {
    if (!speedpy_vector_valid(vector)) {
        return 0;
    }

    return vector->element_size;
}

uint64_t speedpy_vector_generation(
    const SpeedPyVector *vector
) {
    if (!speedpy_vector_valid(vector)) {
        return 0;
    }

    return vector->generation;
}

bool speedpy_vector_lock(
    SpeedPyVector *vector
) {
    if (!speedpy_vector_valid(vector)) {
        return false;
    }

    vector->locked = true;
    return true;
}

bool speedpy_vector_unlock(
    SpeedPyVector *vector
) {
    if (!speedpy_vector_valid(vector)) {
        return false;
    }

    vector->locked = false;
    return true;
}

bool speedpy_vector_is_locked(
    const SpeedPyVector *vector
) {
    if (!speedpy_vector_valid(vector)) {
        return false;
    }

    return vector->locked;
}

void speedpy_vector_set_user_data(
    SpeedPyVector *vector,
    void *user_data
) {
    if (!speedpy_vector_valid(vector)) {
        return;
    }

    vector->user_data = user_data;
}

void *speedpy_vector_get_user_data(
    const SpeedPyVector *vector
) {
    if (!speedpy_vector_valid(vector)) {
        return NULL;
    }

    return vector->user_data;
}

SpeedPyQueue *speedpy_queue_create(
    size_t element_size
) {
    SpeedPyQueue *queue;

    if (element_size == 0) {
        return NULL;
    }

    queue =
        (SpeedPyQueue *)calloc(1, sizeof(SpeedPyQueue));

    if (queue == NULL) {
        return NULL;
    }

    queue->capacity =
        SPEEDPY_CONTAINER_INITIAL_CAPACITY;

    if (element_size >
        SIZE_MAX / queue->capacity) {
        free(queue);
        return NULL;
    }

    queue->data = (unsigned char *)calloc(
        queue->capacity,
        element_size
    );

    if (queue->data == NULL) {
        free(queue);
        return NULL;
    }

    queue->element_size = element_size;
    queue->generation = 1;

    return queue;
}

void speedpy_queue_destroy(
    SpeedPyQueue *queue
) {
    if (queue == NULL) {
        return;
    }

    free(queue->data);
    queue->data = NULL;

    free(queue);
}

bool speedpy_queue_grow(
    SpeedPyQueue *queue,
    size_t capacity
) {
    unsigned char *new_data;
    size_t bytes;
    size_t i;

    if (!speedpy_queue_valid(queue) ||
        queue->locked ||
        capacity <= queue->capacity ||
        capacity > SPEEDPY_CONTAINER_MAX_CAPACITY) {
        return false;
    }

    if (queue->element_size >
        SIZE_MAX / capacity) {
        return false;
    }

    bytes = capacity * queue->element_size;

    new_data =
        (unsigned char *)calloc(1, bytes);

    if (new_data == NULL) {
        return false;
    }

    for (i = 0; i < queue->count; i++) {
        size_t source_index =
            (queue->head + i) % queue->capacity;

        memcpy(
            new_data +
            i * queue->element_size,
            queue->data +
            source_index * queue->element_size,
            queue->element_size
        );
    }

    free(queue->data);

    queue->data = new_data;
    queue->capacity = capacity;
    queue->head = 0;
    queue->tail = queue->count;
    queue->generation++;

    return true;
}

bool speedpy_queue_push(
    SpeedPyQueue *queue,
    const void *element
) {
    size_t capacity;

    if (!speedpy_queue_valid(queue) ||
        queue->locked ||
        element == NULL) {
        return false;
    }

    if (queue->count >=
        SPEEDPY_CONTAINER_MAX_CAPACITY) {
        return false;
    }

    if (queue->count == queue->capacity) {
        capacity = speedpy_container_next_capacity(
            queue->capacity,
            queue->count + 1
        );

        if (!speedpy_queue_grow(queue, capacity)) {
            return false;
        }
    }

    memcpy(
        queue->data +
        queue->tail * queue->element_size,
        element,
        queue->element_size
    );

    queue->tail =
        (queue->tail + 1) % queue->capacity;

    queue->count++;
    queue->generation++;

    return true;
}

bool speedpy_queue_pop(
    SpeedPyQueue *queue,
    void *output
) {
    if (!speedpy_queue_valid(queue) ||
        queue->locked ||
        queue->count == 0) {
        return false;
    }

    if (output != NULL) {
        memcpy(
            output,
            queue->data +
            queue->head * queue->element_size,
            queue->element_size
        );
    }

    memset(
        queue->data +
        queue->head * queue->element_size,
        0,
        queue->element_size
    );

    queue->head =
        (queue->head + 1) % queue->capacity;

    queue->count--;
    queue->generation++;

    return true;
}

const void *speedpy_queue_peek(
    const SpeedPyQueue *queue
) {
    if (!speedpy_queue_valid(queue) ||
        queue->count == 0) {
        return NULL;
    }

    return queue->data +
           queue->head * queue->element_size;
}

bool speedpy_queue_clear(
    SpeedPyQueue *queue
) {
    if (!speedpy_queue_valid(queue) ||
        queue->locked) {
        return false;
    }

    memset(
        queue->data,
        0,
        queue->capacity * queue->element_size
    );

    queue->count = 0;
    queue->head = 0;
    queue->tail = 0;
    queue->generation++;

    return true;
}

size_t speedpy_queue_count(
    const SpeedPyQueue *queue
) {
    if (!speedpy_queue_valid(queue)) {
        return 0;
    }

    return queue->count;
}

size_t speedpy_queue_capacity(
    const SpeedPyQueue *queue
) {
    if (!speedpy_queue_valid(queue)) {
        return 0;
    }

    return queue->capacity;
}

bool speedpy_queue_lock(
    SpeedPyQueue *queue
) {
    if (!speedpy_queue_valid(queue)) {
        return false;
    }

    queue->locked = true;
    return true;
}

bool speedpy_queue_unlock(
    SpeedPyQueue *queue
) {
    if (!speedpy_queue_valid(queue)) {
        return false;
    }

    queue->locked = false;
    return true;
}

bool speedpy_queue_is_locked(
    const SpeedPyQueue *queue
) {
    if (!speedpy_queue_valid(queue)) {
        return false;
    }

    return queue->locked;
}

uint64_t speedpy_queue_generation(
    const SpeedPyQueue *queue
) {
    if (!speedpy_queue_valid(queue)) {
        return 0;
    }

    return queue->generation;
}

void speedpy_queue_set_user_data(
    SpeedPyQueue *queue,
    void *user_data
) {
    if (!speedpy_queue_valid(queue)) {
        return;
    }

    queue->user_data = user_data;
}

void *speedpy_queue_get_user_data(
    const SpeedPyQueue *queue
) {
    if (!speedpy_queue_valid(queue)) {
        return NULL;
    }

    return queue->user_data;
}