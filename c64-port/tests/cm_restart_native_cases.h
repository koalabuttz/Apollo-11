/* SPDX-License-Identifier: MIT */
#ifndef CM_RESTART_NATIVE_CASES_H
#define CM_RESTART_NATIVE_CASES_H
#include "cm_restart_native.h"
#include "cm_restart_cases.h"
#define CM_RN_CASE_COUNT 64u
#define CM_RN_MAX_ROWS 14u
#define CM_RN_ROW_WORDS 434u
void cm_rn_prepare(cm_restart_case *,uint8_t);
exec_address cm_rn_entry(uint8_t);
void cm_rn_words(const cm_restart_case *,enum exec_status,exec_address,
 uint8_t,void (*)(void *,agc_word),void *);
#ifndef CM_RESTART_NATIVE_INPUT_ONLY
typedef struct { cm_restart_case state; cm_exec_native runtime;
 uint8_t callbacks,active,failures; } cm_rn_run;
void cm_rn_init(cm_rn_run *,uint8_t);
enum exec_status cm_rn_step(cm_rn_run *,uint8_t);
#endif
#endif
