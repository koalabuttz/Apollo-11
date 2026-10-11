/* SPDX-License-Identifier: MIT */
#ifndef CM_POWERED_CONSTANTS_H
#define CM_POWERED_CONSTANTS_H
#include <stdbool.h>
#include <stdint.h>
/* Original POWERED file SINESLOC/INDEXI five raw fixed words, physical addresses. */
bool cm_powered_constant_read(uint16_t physical_word,uint16_t *out);
#endif
