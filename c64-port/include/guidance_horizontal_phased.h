/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_HORIZONTAL_PHASED_H
#define GUIDANCE_HORIZONTAL_PHASED_H
#include "guidance_horizontal.h"
#include "guidance_unit_phased.h"
#include "guidance_length_phased.h"
/* UNIT's generic phases retain actual PUSHLOC=FIXLOC+6. This suffix resumes
 * the pending source VXSC after UNIT, and completes genuine LOC03524. The
 * caller separately retains UNIT's overflow observation across overlays. */
enum p66_horizontal_status p66_horizontal_radial_product(p66_preparation *,p66_scale_scratch *,p66_norm_scratch *,p66_pushdown *,const agc_dp *);
#endif
