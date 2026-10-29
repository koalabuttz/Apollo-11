/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_UNIT_PHASED_H
#define GUIDANCE_UNIT_PHASED_H
#include "guidance_unit.h"
/* Source UNIT split solely to permit native code overlays. Exclusive canonical
 * ownership must span all three calls; neither internal return admits an
 * Executive job. No pointers/private state survive a call. Ordinary objects
 * are the same physical scratch and VAC as UNIT. Root and finish may return
 * terminal UNIT_OVERFLOW or ALARM_1302; stop subsequent phases on either.
 * Entry source UNIT; intermediate basicPC03031 after VSQSUB and03043 after
 * normalized SQRTSUB; final DANZIG before source next interpreted pair. */
enum p66_unit_result p66_unit_square_phase(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *);
enum p66_unit_result p66_unit_root_phase(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *);
enum p66_unit_result p66_unit_finish_phase(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *);
#endif
