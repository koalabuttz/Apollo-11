/* SPDX-License-Identifier: MIT */
#ifndef CM_EXECUTIVE_CASES_H
#define CM_EXECUTIVE_CASES_H
#include "cm_executive.h"
#define CM_EXEC_CASE_COUNT 1536
#define CM_EXEC_STATE_WORDS 312
#define CM_EXEC_TARGET_COUNT 40
typedef struct {
 cm_executive state; agc_word priority,location,bank;
 uint8_t needs_vac,complete; const char *name;
} cm_scenario;
void cm_case_prepare(cm_scenario *state,uint16_t n);
#endif
