/* SPDX-License-Identifier: MIT */
#ifndef INTERPRETER_NATIVE_CHECK_H
#define INTERPRETER_NATIVE_CHECK_H
#include "interpreter_native_cases.h"
uint32_t in_digest(const in_run *run,enum exec_status status);
int in_checkpoint(const in_run *run,unsigned number,unsigned step,enum exec_status status);
#endif
