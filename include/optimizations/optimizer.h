#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "backend/ir/irTypes.h"

typedef struct OptimizationPass {
    const char *name;
    bool (*run)(IRProgram *program);
} OptimizationPass;

typedef struct OptimizationPipeline {
    DynamicArray passes;
    bool enabled;
} OptimizationPipeline;

OptimizationPipeline *optimization_pipeline_create(void);
void optimization_pipeline_destroy(OptimizationPipeline *pipeline);
void optimization_pipeline_add_pass(OptimizationPipeline *pipeline, OptimizationPass *pass);

bool optimization_pipeline_run(OptimizationPipeline *pipeline, IRProgram *program);
bool optimization_constant_folding(IRProgram *program);
bool optimization_dead_code_elimination(IRProgram *program);
bool optimization_copy_propagation(IRProgram *program);
OptimizationPipeline *optimization_pipeline_create_default(void);
bool optimization_optimize_program(IRProgram *program);

#endif 