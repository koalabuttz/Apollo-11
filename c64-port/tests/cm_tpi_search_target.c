/* SPDX-License-Identifier: MIT */
/* The program the cm-tpi-search cases run: Comanche055 TPI_SEARCH routines alone,
 * entered at NEWOPS (CM only). */
#include "agc_transfer.h"
#include "cm_tpi_search.h"
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program cm={&cm_tpi_search_basic,1,&cm_tpi_search_interpretive,1,0,0,
 (uint16_t)(2048u+06037u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;return &cm;}
