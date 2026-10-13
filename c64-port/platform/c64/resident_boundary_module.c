/* SPDX-License-Identifier: MIT */
/* The boundary-word data module (task slot, RK_MODULE_BOUNDARY): the rope
 * word at every fixed point where the flight resident stops between
 * routines (platform/c64/resident_boundary_words_<m>.h, tools/resident_boundary_words.py).
 * The resident runs it once at boot for the table's place in the image and
 * then reads the table from the REU store without loading it.
 * RK_BOUNDARY_HEADER names another generated table (the flight store's,
 * tools/flight_store_build.py, whose resolver records add keys).
 * Value: the address of an rk_boundary_table. */
#include "resident_flight.h"
#ifdef RK_BOUNDARY_HEADER
#include RK_BOUNDARY_HEADER
#elif RK_LM
#include "resident_boundary_words_lm.h"
#else
#include "resident_boundary_words_cm.h"
#endif
static const rk_boundary_table table={rk_boundary_words,RK_BOUNDARY_WORDS};
uint16_t apollo_ov2_main(apollo_ov2_frame *f,uint16_t selector){(void)selector;
 f->value=(uint16_t)(uintptr_t)&table;return APOLLO_OV2_DONE;}
