/* SPDX-License-Identifier: MIT */
#ifndef CM_KEYBOARD_IRQ_H
#define CM_KEYBOARD_IRQ_H
#include "cm_admission_image.h"
/* Actual Comanche KEYCOM07,3702 / KEYRUPT107,3673 through RESUME5333,
 * successful original NOVAC admission to actual CHARIN2000/060101, seven
 * cores. These routines stop at RESUME; complete-frame entry below also
 * executes actual source RESUME and clears in_interrupt. Source-valid
 * admission state with an available core required; unsupported leaves all
 * state/I/O unchanged. Original BRUPT remains data, never native opcodes. */
enum exec_status cm_keyboard_keycom(apollo_overlay_state *,cm_native_io *);
enum exec_status cm_keyboard_keyrupt(apollo_overlay_state *,cm_native_io *,uint16_t channel15);
enum exec_status cm_keyboard_interrupt(apollo_overlay_state *,cm_native_io *,uint16_t *in_interrupt,uint16_t channel15);
#endif
