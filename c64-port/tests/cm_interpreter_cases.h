/* SPDX-License-Identifier: MIT */
#ifndef CM_INTERPRETER_CASES_H
#define CM_INTERPRETER_CASES_H
#include "cm_dispatch.h"
#define CM_INTERPRETER_COUNT 512
typedef struct { cm_executive state; exec_address address;
 exec_interpreter control; agc_word q,superbank; } cm_interpreter_case;
void cm_interpreter_prepare(cm_interpreter_case *,unsigned);
void cm_interpreter_row(unsigned,const cm_interpreter_case *,enum exec_status,exec_transfer);
#endif
