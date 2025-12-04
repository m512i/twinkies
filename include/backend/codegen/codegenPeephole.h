#ifndef CODEGEN_PEEPHOLE_H
#define CODEGEN_PEEPHOLE_H

#include "backend/ir/irTypes.h"

void codegen_peephole_optimize_function(IRFunction *func);

void codegen_peephole_optimize_program(IRProgram *program);

#endif

