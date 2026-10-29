/* SPDX-License-Identifier: MIT */
#ifndef LM_SPS_RCS_H
#define LM_SPS_RCS_H
#include "overlay_runtime.h"
#include "agc_word.h"
enum lm_sps_rcs_status {LM_SPS_RCS_OK=0,LM_SPS_RCS_OUT_OF_SCOPE=1};
/* Original SPSRCS21,3727/E6. AXISCTR=-1/0/+1 (eitherzero) selectsP/U/V;
 * fixed basic savedQ02000..07777, REGZERO0 and coherentbanks. Excludes
 * source input/scratch producers; IRQ retained externally. Completes source
 * TCQ and saved return instruction with PC=savedQ,Q3; no Executive check. */
enum lm_sps_rcs_status lm_sps_rcs(apollo_overlay_state *);
uint8_t lm_sps_rcs_constant_read(uint16_t,uint16_t *);
#endif
