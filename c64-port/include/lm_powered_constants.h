/* SPDX-License-Identifier: MIT */
#ifndef LM_POWERED_CONSTANTS_H
#define LM_POWERED_CONSTANTS_H
#include <stdbool.h>
#include <stdint.h>
/* Byte-original SINSLOC/INDEXI and XNBECADR, historical physical fixedaddress. */
bool lm_powered_constant_read(uint16_t physical_word,uint16_t *out);
#endif
