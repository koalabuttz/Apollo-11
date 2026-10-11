/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_LENGTH_PHASED_H
#define GUIDANCE_LENGTH_PHASED_H
#include "guidance_unit.h"
/* Basic ABVAL overlay boundaries, with exclusive canonical ownership spanning
 * all three calls. These internal C returns do not admit an Executive job.
 * Square stops at basic PC03202; root at03210; finish returns to the caller's
 * DANZIG with its pending paired operation unchanged. PUSHLOC is retained. */
enum p66_norm_result p66_length_square_phase(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *);
enum p66_norm_result p66_length_root_phase(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *);
enum p66_norm_result p66_length_finish_phase(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *);
#endif
