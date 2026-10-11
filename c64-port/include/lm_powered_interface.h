/* SPDX-License-Identifier: MIT */
#ifndef LM_POWERED_INTERFACE_H
#define LM_POWERED_INTERFACE_H
#include "overlay_runtime.h"
#include "agc_word.h"
enum lm_powered_interface_status {LM_POWERED_INTERFACE_OK=0,LM_POWERED_INTERFACE_OUT_OF_SCOPE=1};
/* Compiled original interpreted wrappers: selectors0..7 = CDUTRIG,CD*TR*G,
 * CDU*SMNB,TRG*SMNB,CDU*NBSM,TRG*NBSM,*SMNB*,*NBSM*. Calls on canonical
 * source states with no private continuation. Internal returns are not
 * scheduling checks; source VLOAD return retains pending RVQ inEDOP0161. */
enum lm_powered_interface_status lm_powered_interface_start(apollo_overlay_state *,uint8_t selector);
enum lm_powered_interface_status lm_powered_interface_axis(apollo_overlay_state *);
enum lm_powered_interface_status lm_powered_interface_reentry(apollo_overlay_state *,agc_word *interrupt_enabled);
enum lm_powered_interface_status lm_powered_interface_load(apollo_overlay_state *);
enum lm_powered_interface_status lm_powered_interface_return(apollo_overlay_state *);
#endif
