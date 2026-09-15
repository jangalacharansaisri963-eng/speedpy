#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define SPEEDPY_PIPELINE_MAX_STAGES 128
#define SPEEDPY_PIPELINE_NAME_SIZE 96
#define SPEEDPY_PIPELINE_STAGE_NAME_SIZE 96

typedef enum {
    SPEEDPY_STAGE_CREATED = 0,
    SPEEDPY_STAGE_ENABLED = 1,
    SPEEDPY_STAGE_DISABLED = 2,
    SPEEDPY_STAGE_FAILED = 3
} SpeedPyStageState;

typedef struct {
    char name[SPEEDPY_PIPELINE_STAGE_NAME_SIZE];
    uint32_t id;
    uint32_t order;
    uint32_t priority;

    bool enabled;
    SpeedPyStageState state;

    uint64_t execution_count;
    uint64_t failure_count;
    uint64_t input_count;
    uint64_t output_count;

    size_t input_size;
    size_t output_size;

    void *user_data;
} SpeedPyPipelineStage;

typedef struct {
    char name[SPEEDPY_PIPELINE_NAME_SIZE];

    SpeedPyPipelineStage stages[SPEEDPY_PIPELINE_MAX_STAGES];

    size_t stage_count;
    uint32_t next_stage_id;

    bool running;
    bool paused;
    bool locked;

    uint64_t execution_count;
    uint64_t failure_count;
    uint64_t generation;

    size_t total_input;
    size_t total_output;

    void *user_data;
} SpeedPyPipeline;

static bool speedpy_pipeline_valid_name(const char *name)
{
    return name != NULL && name[0] != '\0';
}

static SpeedPyPipelineStage *speedpy_pipeline_find_stage(
    SpeedPyPipeline *pipeline,
    const char *name
)
{
    size_t i;

    if (pipeline == NULL || name == NULL) {
        return NULL;
    }

    for (i = 0; i < pipeline->stage_count; ++i) {
        if (strcmp(pipeline->stages[i].name, name) == 0) {
            return &pipeline->stages[i];
        }
    }

    return NULL;
}

static SpeedPyPipelineStage *speedpy_pipeline_find_stage_id(
    SpeedPyPipeline *pipeline,
    uint32_t id
)
{
    size_t i;

    if (pipeline == NULL || id == 0) {
        return NULL;
    }

    for (i = 0; i < pipeline->stage_count; ++i) {
        if (pipeline->stages[i].id == id) {
            return &pipeline->stages[i];
        }
    }

    return NULL;
}

static void speedpy_pipeline_reorder(SpeedPyPipeline *pipeline)
{
    size_t i;
    size_t j;

    if (pipeline == NULL || pipeline->stage_count < 2) {
        return;
    }

    for (i = 0; i < pipeline->stage_count - 1; ++i) {
        for (j = i + 1; j < pipeline->stage_count; ++j) {
            SpeedPyPipelineStage *a = &pipeline->stages[i];
            SpeedPyPipelineStage *b = &pipeline->stages[j];

            if (b->order < a->order ||
                (b->order == a->order && b->priority > a->priority)) {
                SpeedPyPipelineStage temporary = *a;
                *a = *b;
                *b = temporary;
            }
        }
    }
}

SpeedPyPipeline *speedpy_pipeline_create(const char *name)
{
    SpeedPyPipeline *pipeline;

    if (!speedpy_pipeline_valid_name(name)) {
        return NULL;
    }

    pipeline = (SpeedPyPipeline *)calloc(1, sizeof(SpeedPyPipeline));

    if (pipeline == NULL) {
        return NULL;
    }

    strncpy(
        pipeline->name,
        name,
        SPEEDPY_PIPELINE_NAME_SIZE - 1
    );

    pipeline->name[SPEEDPY_PIPELINE_NAME_SIZE - 1] = '\0';
    pipeline->next_stage_id = 1;
    pipeline->generation = 1;

    return pipeline;
}

void speedpy_pipeline_destroy(SpeedPyPipeline *pipeline)
{
    if (pipeline == NULL) {
        return;
    }

    free(pipeline);
}

int speedpy_pipeline_add_stage(
    SpeedPyPipeline *pipeline,
    const char *name,
    uint32_t order,
    uint32_t priority,
    void *user_data
)
{
    SpeedPyPipelineStage *stage;

    if (pipeline == NULL ||
        !speedpy_pipeline_valid_name(name) ||
        pipeline->locked ||
        pipeline->stage_count >= SPEEDPY_PIPELINE_MAX_STAGES) {
        return -1;
    }

    if (speedpy_pipeline_find_stage(pipeline, name) != NULL) {
        return -2;
    }

    stage = &pipeline->stages[pipeline->stage_count];

    memset(stage, 0, sizeof(SpeedPyPipelineStage));

    strncpy(
        stage->name,
        name,
        SPEEDPY_PIPELINE_STAGE_NAME_SIZE - 1
    );

    stage->name[SPEEDPY_PIPELINE_STAGE_NAME_SIZE - 1] = '\0';

    stage->id = pipeline->next_stage_id++;
    stage->order = order;
    stage->priority = priority;
    stage->enabled = true;
    stage->state = SPEEDPY_STAGE_ENABLED;
    stage->user_data = user_data;

    pipeline->stage_count++;
    pipeline->generation++;

    speedpy_pipeline_reorder(pipeline);

    return 0;
}

int speedpy_pipeline_remove_stage(
    SpeedPyPipeline *pipeline,
    const char *name
)
{
    size_t i;
    SpeedPyPipelineStage *stage;

    if (pipeline == NULL ||
        name == NULL ||
        pipeline->locked) {
        return -1;
    }

    stage = speedpy_pipeline_find_stage(pipeline, name);

    if (stage == NULL) {
        return -2;
    }

    i = (size_t)(stage - pipeline->stages);

    if (i + 1 < pipeline->stage_count) {
        memmove(
            &pipeline->stages[i],
            &pipeline->stages[i + 1],
            (pipeline->stage_count - i - 1) *
                sizeof(SpeedPyPipelineStage)
        );
    }

    pipeline->stage_count--;
    pipeline->generation++;

    return 0;
}

int speedpy_pipeline_enable_stage(
    SpeedPyPipeline *pipeline,
    const char *name
)
{
    SpeedPyPipelineStage *stage;

    if (pipeline == NULL || pipeline->locked) {
        return -1;
    }

    stage = speedpy_pipeline_find_stage(pipeline, name);

    if (stage == NULL) {
        return -2;
    }

    stage->enabled = true;
    stage->state = SPEEDPY_STAGE_ENABLED;
    pipeline->generation++;

    return 0;
}

int speedpy_pipeline_disable_stage(
    SpeedPyPipeline *pipeline,
    const char *name
)
{
    SpeedPyPipelineStage *stage;

    if (pipeline == NULL || pipeline->locked) {
        return -1;
    }

    stage = speedpy_pipeline_find_stage(pipeline, name);

    if (stage == NULL) {
        return -2;
    }

    stage->enabled = false;
    stage->state = SPEEDPY_STAGE_DISABLED;
    pipeline->generation++;

    return 0;
}

int speedpy_pipeline_set_stage_order(
    SpeedPyPipeline *pipeline,
    const char *name,
    uint32_t order
)
{
    SpeedPyPipelineStage *stage;

    if (pipeline == NULL || pipeline->locked) {
        return -1;
    }

    stage = speedpy_pipeline_find_stage(pipeline, name);

    if (stage == NULL) {
        return -2;
    }

    stage->order = order;
    pipeline->generation++;

    speedpy_pipeline_reorder(pipeline);

    return 0;
}

int speedpy_pipeline_set_stage_priority(
    SpeedPyPipeline *pipeline,
    const char *name,
    uint32_t priority
)
{
    SpeedPyPipelineStage *stage;

    if (pipeline == NULL || pipeline->locked) {
        return -1;
    }

    stage = speedpy_pipeline_find_stage(pipeline, name);

    if (stage == NULL) {
        return -2;
    }

    stage->priority = priority;
    pipeline->generation++;

    speedpy_pipeline_reorder(pipeline);

    return 0;
}

SpeedPyPipelineStage *speedpy_pipeline_get_stage(
    SpeedPyPipeline *pipeline,
    const char *name
)
{
    return speedpy_pipeline_find_stage(pipeline, name);
}

SpeedPyPipelineStage *speedpy_pipeline_get_stage_id(
    SpeedPyPipeline *pipeline,
    uint32_t id
)
{
    return speedpy_pipeline_find_stage_id(pipeline, id);
}

size_t speedpy_pipeline_stage_count(
    const SpeedPyPipeline *pipeline
)
{
    if (pipeline == NULL) {
        return 0;
    }

    return pipeline->stage_count;
}

uint64_t speedpy_pipeline_execution_count(
    const SpeedPyPipeline *pipeline
)
{
    if (pipeline == NULL) {
        return 0;
    }

    return pipeline->execution_count;
}

uint64_t speedpy_pipeline_failure_count(
    const SpeedPyPipeline *pipeline
)
{
    if (pipeline == NULL) {
        return 0;
    }

    return pipeline->failure_count;
}

uint64_t speedpy_pipeline_generation(
    const SpeedPyPipeline *pipeline
)
{
    if (pipeline == NULL) {
        return 0;
    }

    return pipeline->generation;
}

int speedpy_pipeline_record_stage_execution(
    SpeedPyPipeline *pipeline,
    const char *stage_name,
    size_t input_size,
    size_t output_size,
    bool failed
)
{
    SpeedPyPipelineStage *stage;

    if (pipeline == NULL || stage_name == NULL) {
        return -1;
    }

    stage = speedpy_pipeline_find_stage(pipeline, stage_name);

    if (stage == NULL) {
        return -2;
    }

    if (!stage->enabled) {
        return -3;
    }

    stage->execution_count++;
    stage->input_count++;
    stage->input_size += input_size;
    stage->output_size += output_size;
    stage->output_count++;

    pipeline->execution_count++;
    pipeline->total_input += input_size;
    pipeline->total_output += output_size;

    if (failed) {
        stage->failure_count++;
        stage->state = SPEEDPY_STAGE_FAILED;
        pipeline->failure_count++;
    } else {
        stage->state = SPEEDPY_STAGE_ENABLED;
    }

    return 0;
}

int speedpy_pipeline_start(SpeedPyPipeline *pipeline)
{
    if (pipeline == NULL || pipeline->running) {
        return -1;
    }

    pipeline->running = true;
    pipeline->paused = false;

    return 0;
}

int speedpy_pipeline_pause(SpeedPyPipeline *pipeline)
{
    if (pipeline == NULL || !pipeline->running) {
        return -1;
    }

    pipeline->paused = true;

    return 0;
}

int speedpy_pipeline_resume(SpeedPyPipeline *pipeline)
{
    if (pipeline == NULL || !pipeline->running) {
        return -1;
    }

    pipeline->paused = false;

    return 0;
}

int speedpy_pipeline_stop(SpeedPyPipeline *pipeline)
{
    if (pipeline == NULL) {
        return -1;
    }

    pipeline->running = false;
    pipeline->paused = false;

    return 0;
}

bool speedpy_pipeline_is_running(
    const SpeedPyPipeline *pipeline
)
{
    return pipeline != NULL && pipeline->running;
}

bool speedpy_pipeline_is_paused(
    const SpeedPyPipeline *pipeline
)
{
    return pipeline != NULL && pipeline->paused;
}

void speedpy_pipeline_lock(SpeedPyPipeline *pipeline)
{
    if (pipeline != NULL) {
        pipeline->locked = true;
    }
}

void speedpy_pipeline_unlock(SpeedPyPipeline *pipeline)
{
    if (pipeline != NULL) {
        pipeline->locked = false;
    }
}

bool speedpy_pipeline_is_locked(
    const SpeedPyPipeline *pipeline
)
{
    return pipeline != NULL && pipeline->locked;
}

void speedpy_pipeline_set_user_data(
    SpeedPyPipeline *pipeline,
    void *user_data
)
{
    if (pipeline != NULL) {
        pipeline->user_data = user_data;
    }
}

void *speedpy_pipeline_get_user_data(
    const SpeedPyPipeline *pipeline
)
{
    if (pipeline == NULL) {
        return NULL;
    }

    return pipeline->user_data;
}

const char *speedpy_pipeline_name(
    const SpeedPyPipeline *pipeline
)
{
    if (pipeline == NULL) {
        return NULL;
    }

    return pipeline->name;
}

void speedpy_pipeline_reset_stats(SpeedPyPipeline *pipeline)
{
    size_t i;

    if (pipeline == NULL || pipeline->locked) {
        return;
    }

    pipeline->execution_count = 0;
    pipeline->failure_count = 0;
    pipeline->total_input = 0;
    pipeline->total_output = 0;

    for (i = 0; i < pipeline->stage_count; ++i) {
        pipeline->stages[i].execution_count = 0;
        pipeline->stages[i].failure_count = 0;
        pipeline->stages[i].input_count = 0;
        pipeline->stages[i].output_count = 0;
        pipeline->stages[i].input_size = 0;
        pipeline->stages[i].output_size = 0;

        if (pipeline->stages[i].enabled) {
            pipeline->stages[i].state = SPEEDPY_STAGE_ENABLED;
        }
    }

    pipeline->generation++;
}