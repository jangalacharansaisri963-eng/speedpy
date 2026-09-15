#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define SPEEDPY_UTILS_MAX_BUFFER_SIZE 1048576

typedef struct {
    uint64_t operations;
    uint64_t comparisons;
    uint64_t copies;
    uint64_t allocations;
    uint64_t failures;
    uint64_t generation;

    bool enabled;
    bool locked;

    void *user_data;
} SpeedPyUtilsState;

static SpeedPyUtilsState speedpy_utils_state = {
    0,
    0,
    0,
    0,
    0,
    1,
    true,
    false,
    NULL
};

static bool speedpy_utils_ready(void) {
    return speedpy_utils_state.enabled &&
           !speedpy_utils_state.locked;
}

static void speedpy_utils_operation(void) {
    if (speedpy_utils_state.operations <
        UINT64_MAX) {
        speedpy_utils_state.operations++;
    }

    if (speedpy_utils_state.generation <
        UINT64_MAX) {
        speedpy_utils_state.generation++;
    }
}

void *speedpy_utils_alloc(size_t size) {
    void *memory;

    if (!speedpy_utils_ready() ||
        size == 0 ||
        size > SPEEDPY_UTILS_MAX_BUFFER_SIZE) {
        speedpy_utils_state.failures++;
        return NULL;
    }

    memory = malloc(size);

    if (memory == NULL) {
        speedpy_utils_state.failures++;
        return NULL;
    }

    speedpy_utils_state.allocations++;
    speedpy_utils_operation();

    return memory;
}

void *speedpy_utils_calloc(
    size_t count,
    size_t size
) {
    void *memory;

    if (!speedpy_utils_ready() ||
        count == 0 ||
        size == 0 ||
        count > SIZE_MAX / size ||
        count * size > SPEEDPY_UTILS_MAX_BUFFER_SIZE) {
        speedpy_utils_state.failures++;
        return NULL;
    }

    memory = calloc(count, size);

    if (memory == NULL) {
        speedpy_utils_state.failures++;
        return NULL;
    }

    speedpy_utils_state.allocations++;
    speedpy_utils_operation();

    return memory;
}

void *speedpy_utils_realloc(
    void *memory,
    size_t size
) {
    void *new_memory;

    if (!speedpy_utils_ready() ||
        size == 0 ||
        size > SPEEDPY_UTILS_MAX_BUFFER_SIZE) {
        speedpy_utils_state.failures++;
        return NULL;
    }

    new_memory = realloc(memory, size);

    if (new_memory == NULL) {
        speedpy_utils_state.failures++;
        return NULL;
    }

    speedpy_utils_state.allocations++;
    speedpy_utils_operation();

    return new_memory;
}

void speedpy_utils_free(void *memory) {
    if (memory == NULL) {
        return;
    }

    free(memory);
    speedpy_utils_operation();
}

bool speedpy_utils_copy(
    void *destination,
    const void *source,
    size_t size
) {
    if (!speedpy_utils_ready() ||
        destination == NULL ||
        source == NULL ||
        size > SPEEDPY_UTILS_MAX_BUFFER_SIZE) {
        speedpy_utils_state.failures++;
        return false;
    }

    if (size > 0) {
        memcpy(destination, source, size);
    }

    speedpy_utils_state.copies++;
    speedpy_utils_operation();

    return true;
}

bool speedpy_utils_move(
    void *destination,
    const void *source,
    size_t size
) {
    if (!speedpy_utils_ready() ||
        destination == NULL ||
        source == NULL ||
        size > SPEEDPY_UTILS_MAX_BUFFER_SIZE) {
        speedpy_utils_state.failures++;
        return false;
    }

    if (size > 0) {
        memmove(destination, source, size);
    }

    speedpy_utils_state.copies++;
    speedpy_utils_operation();

    return true;
}

bool speedpy_utils_zero(
    void *memory,
    size_t size
) {
    if (!speedpy_utils_ready() ||
        memory == NULL ||
        size > SPEEDPY_UTILS_MAX_BUFFER_SIZE) {
        speedpy_utils_state.failures++;
        return false;
    }

    memset(memory, 0, size);

    speedpy_utils_operation();

    return true;
}

bool speedpy_utils_fill(
    void *memory,
    int value,
    size_t size
) {
    if (!speedpy_utils_ready() ||
        memory == NULL ||
        size > SPEEDPY_UTILS_MAX_BUFFER_SIZE) {
        speedpy_utils_state.failures++;
        return false;
    }

    memset(memory, value, size);

    speedpy_utils_operation();

    return true;
}

int speedpy_utils_compare(
    const void *a,
    const void *b,
    size_t size
) {
    int result;

    if (!speedpy_utils_ready() ||
        a == NULL ||
        b == NULL ||
        size > SPEEDPY_UTILS_MAX_BUFFER_SIZE) {
        speedpy_utils_state.failures++;
        return 0;
    }

    result = memcmp(a, b, size);

    speedpy_utils_state.comparisons++;
    speedpy_utils_operation();

    return result;
}

bool speedpy_utils_equal(
    const void *a,
    const void *b,
    size_t size
) {
    if (!speedpy_utils_ready() ||
        a == NULL ||
        b == NULL ||
        size > SPEEDPY_UTILS_MAX_BUFFER_SIZE) {
        speedpy_utils_state.failures++;
        return false;
    }

    speedpy_utils_state.comparisons++;
    speedpy_utils_operation();

    return memcmp(a, b, size) == 0;
}

size_t speedpy_utils_min_size(
    size_t a,
    size_t b
) {
    speedpy_utils_operation();

    return a < b ? a : b;
}

size_t speedpy_utils_max_size(
    size_t a,
    size_t b
) {
    speedpy_utils_operation();

    return a > b ? a : b;
}

uint64_t speedpy_utils_min_u64(
    uint64_t a,
    uint64_t b
) {
    speedpy_utils_operation();

    return a < b ? a : b;
}

uint64_t speedpy_utils_max_u64(
    uint64_t a,
    uint64_t b
) {
    speedpy_utils_operation();

    return a > b ? a : b;
}

int64_t speedpy_utils_min_i64(
    int64_t a,
    int64_t b
) {
    speedpy_utils_operation();

    return a < b ? a : b;
}

int64_t speedpy_utils_max_i64(
    int64_t a,
    int64_t b
) {
    speedpy_utils_operation();

    return a > b ? a : b;
}

size_t speedpy_utils_clamp_size(
    size_t value,
    size_t minimum,
    size_t maximum
) {
    speedpy_utils_operation();

    if (minimum > maximum) {
        size_t temporary = minimum;
        minimum = maximum;
        maximum = temporary;
    }

    if (value < minimum) {
        return minimum;
    }

    if (value > maximum) {
        return maximum;
    }

    return value;
}

uint64_t speedpy_utils_clamp_u64(
    uint64_t value,
    uint64_t minimum,
    uint64_t maximum
) {
    speedpy_utils_operation();

    if (minimum > maximum) {
        uint64_t temporary = minimum;
        minimum = maximum;
        maximum = temporary;
    }

    if (value < minimum) {
        return minimum;
    }

    if (value > maximum) {
        return maximum;
    }

    return value;
}

int64_t speedpy_utils_clamp_i64(
    int64_t value,
    int64_t minimum,
    int64_t maximum
) {
    speedpy_utils_operation();

    if (minimum > maximum) {
        int64_t temporary = minimum;
        minimum = maximum;
        maximum = temporary;
    }

    if (value < minimum) {
        return minimum;
    }

    if (value > maximum) {
        return maximum;
    }

    return value;
}

bool speedpy_utils_is_power_of_two(
    size_t value
) {
    speedpy_utils_operation();

    if (value == 0) {
        return false;
    }

    return (value & (value - 1)) == 0;
}

size_t speedpy_utils_next_power_of_two(
    size_t value
) {
    size_t result = 1;

    speedpy_utils_operation();

    if (value <= 1) {
        return 1;
    }

    while (result < value) {
        if (result > SIZE_MAX / 2) {
            return 0;
        }

        result *= 2;
    }

    return result;
}

size_t speedpy_utils_previous_power_of_two(
    size_t value
) {
    size_t result = 1;

    speedpy_utils_operation();

    if (value == 0) {
        return 0;
    }

    while (result <= value / 2) {
        result *= 2;
    }

    return result;
}

bool speedpy_utils_add_overflow(
    size_t a,
    size_t b,
    size_t *result
) {
    if (result == NULL) {
        return true;
    }

    speedpy_utils_operation();

    if (a > SIZE_MAX - b) {
        *result = 0;
        return true;
    }

    *result = a + b;
    return false;
}

bool speedpy_utils_mul_overflow(
    size_t a,
    size_t b,
    size_t *result
) {
    if (result == NULL) {
        return true;
    }

    speedpy_utils_operation();

    if (a != 0 && b > SIZE_MAX / a) {
        *result = 0;
        return true;
    }

    *result = a * b;
    return false;
}

bool speedpy_utils_add_u64_overflow(
    uint64_t a,
    uint64_t b,
    uint64_t *result
) {
    if (result == NULL) {
        return true;
    }

    speedpy_utils_operation();

    if (a > UINT64_MAX - b) {
        *result = UINT64_MAX;
        return true;
    }

    *result = a + b;
    return false;
}

bool speedpy_utils_mul_u64_overflow(
    uint64_t a,
    uint64_t b,
    uint64_t *result
) {
    if (result == NULL) {
        return true;
    }

    speedpy_utils_operation();

    if (a != 0 && b > UINT64_MAX / a) {
        *result = UINT64_MAX;
        return true;
    }

    *result = a * b;
    return false;
}

size_t speedpy_utils_align_up(
    size_t value,
    size_t alignment
) {
    size_t remainder;

    speedpy_utils_operation();

    if (alignment == 0) {
        return value;
    }

    remainder = value % alignment;

    if (remainder == 0) {
        return value;
    }

    if (value >
        SIZE_MAX - (alignment - remainder)) {
        return 0;
    }

    return value + (alignment - remainder);
}

size_t speedpy_utils_align_down(
    size_t value,
    size_t alignment
) {
    speedpy_utils_operation();

    if (alignment == 0) {
        return value;
    }

    return value -
           (value % alignment);
}

bool speedpy_utils_is_aligned(
    size_t value,
    size_t alignment
) {
    speedpy_utils_operation();

    if (alignment == 0) {
        return false;
    }

    return value % alignment == 0;
}

uint64_t speedpy_utils_hash_bytes(
    const void *data,
    size_t size
) {
    const unsigned char *bytes;
    uint64_t hash;

    if (!speedpy_utils_ready() ||
        data == NULL ||
        size > SPEEDPY_UTILS_MAX_BUFFER_SIZE) {
        speedpy_utils_state.failures++;
        return 0;
    }

    bytes = (const unsigned char *)data;

    hash = UINT64_C(14695981039346656037);

    for (size_t i = 0; i < size; i++) {
        hash ^= (uint64_t)bytes[i];
        hash *= UINT64_C(1099511628211);
    }

    speedpy_utils_operation();

    return hash;
}

uint64_t speedpy_utils_hash_string(
    const char *string
) {
    if (string == NULL) {
        speedpy_utils_state.failures++;
        return 0;
    }

    return speedpy_utils_hash_bytes(
        string,
        strlen(string)
    );
}

size_t speedpy_utils_string_length(
    const char *string
) {
    if (!speedpy_utils_ready() ||
        string == NULL) {
        speedpy_utils_state.failures++;
        return 0;
    }

    speedpy_utils_operation();

    return strlen(string);
}

bool speedpy_utils_string_equal(
    const char *a,
    const char *b
) {
    if (!speedpy_utils_ready() ||
        a == NULL ||
        b == NULL) {
        speedpy_utils_state.failures++;
        return false;
    }

    speedpy_utils_state.comparisons++;
    speedpy_utils_operation();

    return strcmp(a, b) == 0;
}

bool speedpy_utils_string_copy(
    char *destination,
    size_t destination_size,
    const char *source
) {
    size_t length;

    if (!speedpy_utils_ready() ||
        destination == NULL ||
        source == NULL ||
        destination_size == 0) {
        speedpy_utils_state.failures++;
        return false;
    }

    length = strlen(source);

    if (length >= destination_size) {
        destination[0] = '\0';
        speedpy_utils_state.failures++;
        return false;
    }

    memcpy(
        destination,
        source,
        length + 1
    );

    speedpy_utils_state.copies++;
    speedpy_utils_operation();

    return true;
}

void speedpy_utils_swap(
    void *a,
    void *b,
    size_t size
) {
    unsigned char *first;
    unsigned char *second;
    unsigned char *temporary;

    if (!speedpy_utils_ready() ||
        a == NULL ||
        b == NULL ||
        size == 0 ||
        size > SPEEDPY_UTILS_MAX_BUFFER_SIZE) {
        speedpy_utils_state.failures++;
        return;
    }

    if (a == b) {
        return;
    }

    temporary =
        (unsigned char *)malloc(size);

    if (temporary == NULL) {
        speedpy_utils_state.failures++;
        return;
    }

    first = (unsigned char *)a;
    second = (unsigned char *)b;

    memcpy(temporary, first, size);
    memcpy(first, second, size);
    memcpy(second, temporary, size);

    free(temporary);

    speedpy_utils_state.copies += 3;
    speedpy_utils_operation();
}

uint8_t speedpy_utils_get_bit(
    uint64_t value,
    unsigned int bit
) {
    speedpy_utils_operation();

    if (bit >= 64) {
        return 0;
    }

    return (uint8_t)((value >> bit) & 1U);
}

uint64_t speedpy_utils_set_bit(
    uint64_t value,
    unsigned int bit
) {
    speedpy_utils_operation();

    if (bit >= 64) {
        return value;
    }

    return value | (UINT64_C(1) << bit);
}

uint64_t speedpy_utils_clear_bit(
    uint64_t value,
    unsigned int bit
) {
    speedpy_utils_operation();

    if (bit >= 64) {
        return value;
    }

    return value &
           ~(UINT64_C(1) << bit);
}

uint64_t speedpy_utils_toggle_bit(
    uint64_t value,
    unsigned int bit
) {
    speedpy_utils_operation();

    if (bit >= 64) {
        return value;
    }

    return value ^
           (UINT64_C(1) << bit);
}

bool speedpy_utils_test_bit(
    uint64_t value,
    unsigned int bit
) {
    speedpy_utils_operation();

    if (bit >= 64) {
        return false;
    }

    return
        (value &
         (UINT64_C(1) << bit)) != 0;
}

uint64_t speedpy_utils_rotate_left(
    uint64_t value,
    unsigned int amount
) {
    amount %= 64;

    speedpy_utils_operation();

    if (amount == 0) {
        return value;
    }

    return
        (value << amount) |
        (value >> (64 - amount));
}

uint64_t speedpy_utils_rotate_right(
    uint64_t value,
    unsigned int amount
) {
    amount %= 64;

    speedpy_utils_operation();

    if (amount == 0) {
        return value;
    }

    return
        (value >> amount) |
        (value << (64 - amount));
}

unsigned int speedpy_utils_count_bits(
    uint64_t value
) {
    unsigned int count = 0;

    speedpy_utils_operation();

    while (value != 0) {
        value &= value - 1;
        count++;
    }

    return count;
}

bool speedpy_utils_get_enabled(void) {
    return speedpy_utils_state.enabled;
}

void speedpy_utils_set_enabled(
    bool enabled
) {
    speedpy_utils_state.enabled = enabled;

    if (speedpy_utils_state.generation <
        UINT64_MAX) {
        speedpy_utils_state.generation++;
    }
}

bool speedpy_utils_lock(void) {
    if (speedpy_utils_state.locked) {
        return false;
    }

    speedpy_utils_state.locked = true;
    return true;
}

bool speedpy_utils_unlock(void) {
    if (!speedpy_utils_state.locked) {
        return false;
    }

    speedpy_utils_state.locked = false;
    return true;
}

bool speedpy_utils_is_locked(void) {
    return speedpy_utils_state.locked;
}

uint64_t speedpy_utils_operations(void) {
    return speedpy_utils_state.operations;
}

uint64_t speedpy_utils_comparisons(void) {
    return speedpy_utils_state.comparisons;
}

uint64_t speedpy_utils_copies(void) {
    return speedpy_utils_state.copies;
}

uint64_t speedpy_utils_allocations(void) {
    return speedpy_utils_state.allocations;
}

uint64_t speedpy_utils_failures(void) {
    return speedpy_utils_state.failures;
}

uint64_t speedpy_utils_generation(void) {
    return speedpy_utils_state.generation;
}

void speedpy_utils_reset_statistics(void) {
    speedpy_utils_state.operations = 0;
    speedpy_utils_state.comparisons = 0;
    speedpy_utils_state.copies = 0;
    speedpy_utils_state.allocations = 0;
    speedpy_utils_state.failures = 0;

    if (speedpy_utils_state.generation <
        UINT64_MAX) {
        speedpy_utils_state.generation++;
    }
}

void speedpy_utils_set_user_data(
    void *user_data
) {
    speedpy_utils_state.user_data =
        user_data;
}

void *speedpy_utils_get_user_data(void) {
    return speedpy_utils_state.user_data;
}