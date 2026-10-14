/* SPDX-License-Identifier: MIT */
#ifndef LM_INTERBANK_DAP_H
#define LM_INTERBANK_DAP_H
/* IBNKCALL (04674) and ISWRETRN (04707) alone, as lm_interbank_routines runs
 * them (src/lm_interbank.c): every RCS DAP pass calls TJETLAW through them,
 * so the flight store's DAP excursion carries this table (RIDE in
 * tools/flight_store_build.py) instead of loading the whole interbank module.
 * The program and the resolver keep lm_interbank_routines. io is a
 * p66_bias_io. tests/test_lm_interbank_dap.c checks both entries against
 * lm_interbank_routines' on random states. */
#include "agc_transfer.h"
extern const agc_routine_table lm_interbank_dap_hot_routines;
#endif
