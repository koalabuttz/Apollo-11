/* SPDX-License-Identifier: MIT */
#ifndef LM_POWERED_QUICK_H
#define LM_POWERED_QUICK_H
#include "overlay_runtime.h"
#include "agc_word.h"
enum lm_powered_quick_status { LM_POWERED_QUICK_OK=0,LM_POWERED_QUICK_OUT_OF_SCOPE=1 };
/* Source QUICTRIG23,3615 and actual TC A23,3645, fixed saved Q02000..07777.
 * IRQ is sourceINHINT/RELINT; all canonical scratch committed, noExecutivecheck. */
enum lm_powered_quick_status lm_powered_quick(apollo_overlay_state *,agc_word *irq);
enum lm_powered_quick_status lm_powered_quick_return(apollo_overlay_state *);
#endif
