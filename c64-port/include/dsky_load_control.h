/* SPDX-License-Identifier: MIT */
#ifndef DSKY_LOAD_CONTROL_H
#define DSKY_LOAD_CONTROL_H
#include "dsky_display_release.h"
/* Actual VBPROC/VBTERM/VBRESEQ/PROCKEY through RECALTST and genuine
 * ENDOFJOB, or direct RECALTST. Source sleeping-job admissions and both real
 * RELDSP calls compose unchanged JOBWAKE. Includes real no-match LOCCTR=-1
 * fixed indexed addresses, signed-zero CADRs and LOADSTAT edge branches.
 * Selected wakes require documented wake domains; unused Executive words
 * impose no guard. Source bank40, all EB/SUPER100..160, IRQ0/1. */
enum p66_bias_status dsky_load_control(apollo_overlay_state *,dsky_charin_io *);
enum exec_status cm_load_control(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channel11);
#endif
