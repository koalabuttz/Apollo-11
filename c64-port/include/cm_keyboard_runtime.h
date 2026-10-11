/* SPDX-License-Identifier: MIT */
#ifndef CM_KEYBOARD_RUNTIME_H
#define CM_KEYBOARD_RUNTIME_H
#include "cm_keyboard_irq.h"
#include "cm_idle_dispatch.h"
#include "cm_basic_finish.h"
#include "cm_charin_nv.h"
#include "cm_charin_number.h"
enum cm_keyboard_stage {CM_KEYBOARD_IRQ=1,CM_KEYBOARD_SELECT=2,CM_KEYBOARD_NV=3,CM_KEYBOARD_NUMBER=4,CM_KEYBOARD_FINISH=5};
/* Compiled source stages only: actual ISR/RESUME, idle selection, native key
 * body and genuine Executive completion. Each call returns before the next
 * stage or disk load; only image/io persist. Caller excludes producers and
 * adopts returned positive transfer metadata. No source opcode is fetched.
 * Unsupported selector is unchanged. Unsupported stage domain follows the
 * independently qualified body contract; no fabricated ENDJOB is emitted. */
enum exec_status cm_keyboard_runtime_entry(apollo_overlay_state *,cm_native_io *,uint16_t *in_interrupt,uint16_t channel15,uint16_t selector,exec_transfer *,uint16_t *wrote_channel11);
#endif
