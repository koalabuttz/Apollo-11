/* SPDX-License-Identifier: MIT */
#ifndef SCENARIOS_H
#define SCENARIOS_H
#include "executive.h"
#define SCENARIO_COUNT 26
typedef struct {
    executive state;
    agc_word priority, location, bank;
    uint8_t needs_vac, complete;
    const char *name;
} scenario;
void scenario_prepare(scenario *s, uint8_t number);
int scenario_expected(uint8_t number, const executive *s, int result);
#endif
