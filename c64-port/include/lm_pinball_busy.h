/* SPDX-License-Identifier: MIT */
#ifndef LM_PINBALL_BUSY_H
#define LM_PINBALL_BUSY_H
#include "p66_bias.h"
#include "agc_word.h"
enum lm_busy_status {LM_BUSY_SLEEP=0,LM_BUSY_OUT_OF_SCOPE=1,LM_BUSY_ALARM=01206};
/* Actual fixed NVSUBUSY4442 -> POSTJUMP/NVSUBSY1 -> JOBSLEEP5133 or
 * POODOO5652 with the original inline alarm01206. A signextended ordinary
 * wakeCADR, normalbanks/SUPER, ordinarychannel11, DSPLIST not-0 (free+0).
 * Exact source rawregisters/scratch; no decoder. Source sleep/alarm services
 * are actual following boundaries. Unsupported allobjectsunchanged. Caller
 * owns exclusion; current Executive image retained until JOBSLEEP executes. */
enum lm_busy_status lm_pinball_busy(apollo_overlay_state *,p66_bias_io *,agc_word *channel11);
#endif
