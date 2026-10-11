/* SPDX-License-Identifier: MIT */
#ifndef EXECUTIVE_SERVICE_CASES_H
#define EXECUTIVE_SERVICE_CASES_H
#include "executive.h"
#define EXEC_SERVICE_CASE_COUNT 768u
#define EXEC_SERVICE_SPVAC_COUNT 256u
typedef struct {
    executive state;
    exec_address address;
    exec_interpreter control;
    agc_word q, superbank;
} exec_service_case;
void exec_service_prepare(exec_service_case *input, unsigned number);
void exec_service_row(unsigned number, const exec_service_case *input,
                      enum exec_status status, exec_transfer transfer);
#endif
