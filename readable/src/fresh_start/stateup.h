/* SPDX-License-Identifier: MIT */
/* FRESH_START_AND_RESTART.agc's interpretive code (stateup.c): STATEUP and
 * THISVINT in bank 13, and R00's (the CM's ROC's) CALL INTSTALL and EXIT in
 * bank 04, as the words the interpreter reads. */
#ifndef READABLE_STATEUP_H
#define READABLE_STATEUP_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

extern const struct interpretive_code STATEUP_CODE, R00_CODE;

#endif
