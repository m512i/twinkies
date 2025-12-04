#include "optimizations/optimizer.h"
#include "backend/ir/irCore.h"
#include "backend/ir/irOps.h"
#include "backend/ir/irinstructions.h"
#include "common/common.h"

extern bool debug_enabled;

extern bool optimization_constant_folding(IRProgram *program);
extern bool optimization_dead_code_elimination(IRProgram *program);
extern bool optimization_copy_propagation(IRProgram *program);

OptimizationPipeline *optimization_pipeline_create(void)
{
    OptimizationPipeline *pipeline = safe_malloc(sizeof(OptimizationPipeline));
    array_init(&pipeline->passes, 4);
    pipeline->enabled = true;
    return pipeline;
}

void optimization_pipeline_destroy(OptimizationPipeline *pipeline)
{
    if (!pipeline)
        return;
    array_free(&pipeline->passes);
    safe_free(pipeline);
}

void optimization_pipeline_add_pass(OptimizationPipeline *pipeline, OptimizationPass *pass)
{
    if (!pipeline || !pass)
        return;
    array_push(&pipeline->passes, pass);
}

bool optimization_pipeline_run(OptimizationPipeline *pipeline, IRProgram *program)
{
    if (!pipeline || !program || !pipeline->enabled)
        return false;

    if (debug_enabled)
    {
        printf("[DEBUG] Running optimization pipeline with %zu passes\n", pipeline->passes.size);
    }

    bool changed = false;
    for (size_t i = 0; i < pipeline->passes.size; i++)
    {
        OptimizationPass *pass = (OptimizationPass *)array_get(&pipeline->passes, i);
        if (pass && pass->run)
        {
            if (debug_enabled)
            {
                printf("[DEBUG] Running optimization pass: %s\n", pass->name);
            }
            if (pass->run(program))
            {
                changed = true;
                if (debug_enabled)
                {
                    printf("[DEBUG] Pass %s made changes\n", pass->name);
                }
            }
        }
    }

    return changed;
}

OptimizationPipeline *optimization_pipeline_create_default(void)
{
    OptimizationPipeline *pipeline = optimization_pipeline_create();
    
    static OptimizationPass constant_folding_pass = {
        .name = "constant_folding",
        .run = optimization_constant_folding
    };
    
    static OptimizationPass dead_code_pass = {
        .name = "dead_code_elimination",
        .run = optimization_dead_code_elimination
    };
    
    static OptimizationPass copy_propagation_pass = {
        .name = "copy_propagation",
        .run = optimization_copy_propagation
    };
    
    optimization_pipeline_add_pass(pipeline, &constant_folding_pass);
    optimization_pipeline_add_pass(pipeline, &copy_propagation_pass);
    optimization_pipeline_add_pass(pipeline, &dead_code_pass);
    
    return pipeline;
}

bool optimization_optimize_program(IRProgram *program)
{
    if (!program)
        return false;
    
    OptimizationPipeline *pipeline = optimization_pipeline_create_default();
    
    bool changed = false;
    int iterations = 0;
    const int max_iterations = 10;
    
    do {
        bool iteration_changed = optimization_pipeline_run(pipeline, program);
        if (iteration_changed)
        {
            changed = true;
            iterations++;
            if (debug_enabled)
            {
                printf("[DEBUG] Optimization iteration %d made changes, running again\n", iterations);
            }
        }
        else
        {
            break;
        }
    } while (iterations < max_iterations);
    
    if (debug_enabled && changed)
    {
        printf("[DEBUG] Optimizations completed after %d iterations\n", iterations);
        fflush(stdout);
    }
    
    optimization_pipeline_destroy(pipeline);
    
    if (debug_enabled)
    {
        printf("[DEBUG] optimization_optimize_program: Returning, changed=%d\n", changed);
        fflush(stdout);
    }
    
    return changed;
}