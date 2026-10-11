/* SPDX-License-Identifier: MIT */
#ifndef CM_STARTUP_CASES_H
#define CM_STARTUP_CASES_H
#include "cm_startup.h"
#define CM_STARTUP_CASE_COUNT 640u
#define CM_STARTUP_ROW_WORDS 2063u
void cm_startup_prepare(apollo_overlay_state *,cm_native_io *,cm_startup_ports *,uint16_t);
void cm_startup_words(const apollo_overlay_state *,const cm_native_io *,const cm_startup_ports *,enum cm_startup_status,void (*)(void *,agc_word),void *);
#endif
