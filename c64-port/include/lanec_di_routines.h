/* SPDX-License-Identifier: MIT */
#ifndef LANEC_DI_ROUTINES_H
#define LANEC_DI_ROUTINES_H
/* DISPLAY_INTERFACE_ROUTINES (Luminary099 and Comanche055) as native
 * routines of the control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md). Every
 * entry and every point where a callee returns into the file that a Lane C
 * suite verifies is registered at its physical address
 * (src/lanec_di_routines.inc, tools/lanec/di_routines.py) and runs that
 * module's lanec_di_*_dispatch; LANEC_OUT_OF_SCOPE is EXEC_OUT_OF_SCOPE.
 * The modules use the superbank, the interrupt-enable state and output
 * channel 11 (MAKEPLAY's KEY REL light): the LM routines take a p66_bias_io
 * and channel 11 from program->hardware (refusing when it is NULL), the CM
 * routines a cm_native_io. */
#include "agc_transfer.h"
extern const agc_routine_table lanec_di_lm_routines;
extern const agc_routine_table lanec_di_cm_routines;
/* The same points, one table per module, for a program that registers only
 * some of them. */
extern const agc_routine_table lanec_di_lm_dsp, lanec_di_lm_mark, lanec_di_lm_play, lanec_di_lm_sleep,
    lanec_di_lm_idle, lanec_di_lm_nvdsp;
extern const agc_routine_table lanec_di_cm_dsp, lanec_di_cm_mark, lanec_di_cm_play, lanec_di_cm_sleep,
    lanec_di_cm_idle, lanec_di_cm_nvdsp;
#endif
