#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define SPEEDPY_STRING_INITIAL_CAPACITY 32
#define SPEEDPY_STRING_MAX_CAPACITY 1048576

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
    bool locked;
    uint64_t generation;
} SpeedPyString;

static bool speedpy_string_valid(const SpeedPyString *string) {
    return string != NULL &&
           string->data != NULL &&
           string->capacity > 0 &&
           string->length < string->capacity;
}

static size_t speedpy_string_next_capacity(size_t current, size_t required) {
    size_t capacity = current;

    if (capacity == 0) {
        capacity = SPEEDPY_STRING_INITIAL_CAPACITY;
    }

    while (capacity < required) {
        if (capacity >= SPEEDPY_STRING_MAX_CAPACITY / 2) {
            capacity = SPEEDPY_STRING_MAX_CAPACITY;
            break;
        }

        capacity *= 2;
    }

    return capacity;
}

SpeedPyString *speedpy_string_create(void) {
    SpeedPyString *string =
        (SpeedPyString *)calloc(1, sizeof(SpeedPyString));

    if (string == NULL) {
        return NULL;
    }

    string->capacity = SPEEDPY_STRING_INITIAL_CAPACITY;
    string->data = (char *)calloc(string->capacity, sizeof(char));

    if (string->data == NULL) {
        free(string);
        return NULL;
    }

    string->generation = 1;

    return string;
}

SpeedPyString *speedpy_string_create_from(const char *value) {
    SpeedPyString *string;
    size_t length;
    size_t capacity;

    if (value == NULL) {
        return NULL;
    }

    length = strlen(value);

    if (length >= SPEEDPY_STRING_MAX_CAPACITY) {
        return NULL;
    }

    capacity = speedpy_string_next_capacity(
        SPEEDPY_STRING_INITIAL_CAPACITY,
        length + 1
    );

    string = (SpeedPyString *)calloc(1, sizeof(SpeedPyString));

    if (string == NULL) {
        return NULL;
    }

    string->data = (char *)calloc(capacity, sizeof(char));

    if (string->data == NULL) {
        free(string);
        return NULL;
    }

    memcpy(string->data, value, length);

    string->length = length;
    string->capacity = capacity;
    string->generation = 1;

    return string;
}

void speedpy_string_destroy(SpeedPyString *string) {
    if (string == NULL) {
        return;
    }

    free(string->data);
    string->data = NULL;

    string->length = 0;
    string->capacity = 0;

    free(string);
}

bool speedpy_string_reserve(SpeedPyString *string, size_t capacity) {
    char *new_data;

    if (!speedpy_string_valid(string) || string->locked) {
        return false;
    }

    if (capacity <= string->capacity) {
        return true;
    }

    if (capacity > SPEEDPY_STRING_MAX_CAPACITY) {
        return false;
    }

    new_data = (char *)realloc(string->data, capacity);

    if (new_data == NULL) {
        return false;
    }

    memset(
        new_data + string->capacity,
        0,
        capacity - string->capacity
    );

    string->data = new_data;
    string->capacity = capacity;
    string->generation++;

    return true;
}

bool speedpy_string_resize(SpeedPyString *string, size_t length) {
    if (!speedpy_string_valid(string) || string->locked) {
        return false;
    }

    if (length >= SPEEDPY_STRING_MAX_CAPACITY) {
        return false;
    }

    if (!speedpy_string_reserve(string, length + 1)) {
        return false;
    }

    if (length > string->length) {
        memset(
            string->data + string->length,
            0,
            length - string->length
        );
    }

    string->length = length;
    string->data[length] = '\0';
    string->generation++;

    return true;
}

bool speedpy_string_clear(SpeedPyString *string) {
    if (!speedpy_string_valid(string) || string->locked) {
        return false;
    }

    string->length = 0;
    string->data[0] = '\0';
    string->generation++;

    return true;
}

bool speedpy_string_set(SpeedPyString *string, const char *value) {
    size_t length;

    if (!speedpy_string_valid(string) ||
        string->locked ||
        value == NULL) {
        return false;
    }

    length = strlen(value);

    if (length >= SPEEDPY_STRING_MAX_CAPACITY) {
        return false;
    }

    if (!speedpy_string_reserve(string, length + 1)) {
        return false;
    }

    memcpy(string->data, value, length);
    string->data[length] = '\0';
    string->length = length;
    string->generation++;

    return true;
}

bool speedpy_string_append(SpeedPyString *string, const char *value) {
    size_t length;
    size_t required;

    if (!speedpy_string_valid(string) ||
        string->locked ||
        value == NULL) {
        return false;
    }

    length = strlen(value);

    if (length > SPEEDPY_STRING_MAX_CAPACITY - string->length - 1) {
        return false;
    }

    required = string->length + length + 1;

    if (!speedpy_string_reserve(string, required)) {
        return false;
    }

    memcpy(
        string->data + string->length,
        value,
        length
    );

    string->length += length;
    string->data[string->length] = '\0';
    string->generation++;

    return true;
}

bool speedpy_string_prepend(SpeedPyString *string, const char *value) {
    size_t length;
    size_t required;

    if (!speedpy_string_valid(string) ||
        string->locked ||
        value == NULL) {
        return false;
    }

    length = strlen(value);

    if (length > SPEEDPY_STRING_MAX_CAPACITY - string->length - 1) {
        return false;
    }

    required = string->length + length + 1;

    if (!speedpy_string_reserve(string, required)) {
        return false;
    }

    memmove(
        string->data + length,
        string->data,
        string->length + 1
    );

    memcpy(string->data, value, length);

    string->length += length;
    string->generation++;

    return true;
}

bool speedpy_string_insert(
    SpeedPyString *string,
    size_t position,
    const char *value
) {
    size_t length;
    size_t required;

    if (!speedpy_string_valid(string) ||
        string->locked ||
        value == NULL ||
        position > string->length) {
        return false;
    }

    length = strlen(value);

    if (length > SPEEDPY_STRING_MAX_CAPACITY - string->length - 1) {
        return false;
    }

    required = string->length + length + 1;

    if (!speedpy_string_reserve(string, required)) {
        return false;
    }

    memmove(
        string->data + position + length,
        string->data + position,
        string->length - position + 1
    );

    memcpy(
        string->data + position,
        value,
        length
    );

    string->length += length;
    string->generation++;

    return true;
}

bool speedpy_string_remove(
    SpeedPyString *string,
    size_t position,
    size_t count
) {
    if (!speedpy_string_valid(string) ||
        string->locked ||
        position >= string->length) {
        return false;
    }

    if (count == 0) {
        return true;
    }

    if (count > string->length - position) {
        count = string->length - position;
    }

    memmove(
        string->data + position,
        string->data + position + count,
        string->length - position - count + 1
    );

    string->length -= count;
    string->generation++;

    return true;
}

int speedpy_string_compare(
    const SpeedPyString *a,
    const SpeedPyString *b
) {
    if (!speedpy_string_valid(a) || !speedpy_string_valid(b)) {
        return 0;
    }

    return strcmp(a->data, b->data);
}

bool speedpy_string_equal(
    const SpeedPyString *a,
    const SpeedPyString *b
) {
    if (!speedpy_string_valid(a) || !speedpy_string_valid(b)) {
        return false;
    }

    if (a->length != b->length) {
        return false;
    }

    return memcmp(a->data, b->data, a->length) == 0;
}

bool speedpy_string_starts_with(
    const SpeedPyString *string,
    const char *prefix
) {
    size_t length;

    if (!speedpy_string_valid(string) || prefix == NULL) {
        return false;
    }

    length = strlen(prefix);

    if (length > string->length) {
        return false;
    }

    return memcmp(string->data, prefix, length) == 0;
}

bool speedpy_string_ends_with(
    const SpeedPyString *string,
    const char *suffix
) {
    size_t length;

    if (!speedpy_string_valid(string) || suffix == NULL) {
        return false;
    }

    length = strlen(suffix);

    if (length > string->length) {
        return false;
    }

    return memcmp(
        string->data + string->length - length,
        suffix,
        length
    ) == 0;
}

int64_t speedpy_string_find(
    const SpeedPyString *string,
    const char *value
) {
    const char *found;

    if (!speedpy_string_valid(string) || value == NULL) {
        return -1;
    }

    found = strstr(string->data, value);

    if (found == NULL) {
        return -1;
    }

    return (int64_t)(found - string->data);
}

int64_t speedpy_string_rfind(
    const SpeedPyString *string,
    const char *value
) {
    size_t value_length;
    size_t position;

    if (!speedpy_string_valid(string) || value == NULL) {
        return -1;
    }

    value_length = strlen(value);

    if (value_length == 0) {
        return (int64_t)string->length;
    }

    if (value_length > string->length) {
        return -1;
    }

    position = string->length - value_length;

    for (;;) {
        if (memcmp(
                string->data + position,
                value,
                value_length
            ) == 0) {
            return (int64_t)position;
        }

        if (position == 0) {
            break;
        }

        position--;
    }

    return -1;
}

bool speedpy_string_trim(SpeedPyString *string) {
    size_t start = 0;
    size_t end;

    if (!speedpy_string_valid(string) || string->locked) {
        return false;
    }

    end = string->length;

    while (start < end) {
        char c = string->data[start];

        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            break;
        }

        start++;
    }

    while (end > start) {
        char c = string->data[end - 1];

        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            break;
        }

        end--;
    }

    if (start > 0 && end > start) {
        memmove(
            string->data,
            string->data + start,
            end - start
        );
    }

    string->length = end - start;
    string->data[string->length] = '\0';
    string->generation++;

    return true;
}

bool speedpy_string_lower(SpeedPyString *string) {
    size_t i;

    if (!speedpy_string_valid(string) || string->locked) {
        return false;
    }

    for (i = 0; i < string->length; i++) {
        if (string->data[i] >= 'A' &&
            string->data[i] <= 'Z') {
            string->data[i] =
                (char)(string->data[i] - 'A' + 'a');
        }
    }

    string->generation++;
    return true;
}

bool speedpy_string_upper(SpeedPyString *string) {
    size_t i;

    if (!speedpy_string_valid(string) || string->locked) {
        return false;
    }

    for (i = 0; i < string->length; i++) {
        if (string->data[i] >= 'a' &&
            string->data[i] <= 'z') {
            string->data[i] =
                (char)(string->data[i] - 'a' + 'A');
        }
    }

    string->generation++;
    return true;
}

bool speedpy_string_substring(
    const SpeedPyString *string,
    size_t start,
    size_t length,
    char *output,
    size_t output_capacity
) {
    if (!speedpy_string_valid(string) ||
        output == NULL ||
        output_capacity == 0 ||
        start > string->length) {
        return false;
    }

    if (length > string->length - start) {
        length = string->length - start;
    }

    if (length + 1 > output_capacity) {
        return false;
    }

    memcpy(output, string->data + start, length);
    output[length] = '\0';

    return true;
}

bool speedpy_string_replace(
    SpeedPyString *string,
    const char *old_value,
    const char *new_value
) {
    int64_t position;
    size_t old_length;
    size_t new_length;

    if (!speedpy_string_valid(string) ||
        string->locked ||
        old_value == NULL ||
        new_value == NULL) {
        return false;
    }

    old_length = strlen(old_value);
    new_length = strlen(new_value);

    if (old_length == 0) {
        return false;
    }

    position = speedpy_string_find(string, old_value);

    if (position < 0) {
        return false;
    }

    if (new_length > old_length) {
        size_t extra = new_length - old_length;

        if (extra > SPEEDPY_STRING_MAX_CAPACITY - string->length - 1) {
            return false;
        }

        if (!speedpy_string_reserve(
                string,
                string->length + extra + 1)) {
            return false;
        }
    }

    if (new_length != old_length) {
        memmove(
            string->data + position + new_length,
            string->data + position + old_length,
            string->length -
            (size_t)position -
            old_length + 1
        );

        string->length =
            string->length - old_length + new_length;
    }

    memcpy(
        string->data + position,
        new_value,
        new_length
    );

    string->generation++;

    return true;
}

const char *speedpy_string_data(
    const SpeedPyString *string
) {
    if (!speedpy_string_valid(string)) {
        return NULL;
    }

    return string->data;
}

size_t speedpy_string_length(
    const SpeedPyString *string
) {
    if (!speedpy_string_valid(string)) {
        return 0;
    }

    return string->length;
}

size_t speedpy_string_capacity(
    const SpeedPyString *string
) {
    if (!speedpy_string_valid(string)) {
        return 0;
    }

    return string->capacity;
}

uint64_t speedpy_string_generation(
    const SpeedPyString *string
) {
    if (!speedpy_string_valid(string)) {
        return 0;
    }

    return string->generation;
}

bool speedpy_string_lock(SpeedPyString *string) {
    if (!speedpy_string_valid(string)) {
        return false;
    }

    string->locked = true;
    return true;
}

bool speedpy_string_unlock(SpeedPyString *string) {
    if (!speedpy_string_valid(string)) {
        return false;
    }

    string->locked = false;
    return true;
}

bool speedpy_string_is_locked(
    const SpeedPyString *string
) {
    if (!speedpy_string_valid(string)) {
        return false;
    }

    return string->locked;
}