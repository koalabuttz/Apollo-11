/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_THROTTLE_SERVICE_H
#define GUIDANCE_THROTTLE_SERVICE_H
#include "guidance_throttle.h"
typedef struct {
    agc_dp fcodd,fp,pif;
    agc_word fc,fcold,pifpset,pseudo55,tthrot,rtnhold,thrust,channel14;
    agc_word lowcrit,highcrit,modreg,phase3[2],q,bbank,superbank,interrupt_enabled;
} p66_throttle_service_state;
/* Basic THROTTLE+3 through THDUMP before saved-return dispatch. Entry A/L is
 * measured acceleration, AFC is desired acceleration; MASS and pad-loaded
 * LOWCRIT/HIGHCRIT are stable. TIME1 aliases preparation.time.low. Writes
 * pulse command THRUST/channel14, FCOLD, FWEIGHT, TTHROT and FASTCHNG PHSNAME3;
 * retains exact source conversion/saturation, register and numerical scratch.
 * Any ordinary MODE is retained. Bank31/EB7 and absent SUPER selection are required; Q is a12-bit saved
 * caller return, interrupt flag0/1 retained. Invalid ownership rejects all
 * objects unchanged. Distinct ordinary-word objects and producer exclusion
 * are caller obligations. No hardware I/O dispatch or Executive scheduling. */
uint8_t p66_throttle_service(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,
    p66_divide_scratch *,p66_norm_scratch *,p66_throttle_feedback *,
    const p66_throttle_inputs *,p66_throttle_service_state *);
#endif
