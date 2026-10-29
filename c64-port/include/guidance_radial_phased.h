/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_RADIAL_PHASED_H
#define GUIDANCE_RADIAL_PHASED_H
#include "guidance_radial.h"
#include "guidance_unit_phased.h"
/* After the source UNIT completes (use UNIT's three internal overlay phases),
 * explicit small P66 phases reach genuine original completed-pair boundaries.
 * Canonical ownership/exclusion and legal VAC/PUSHLOCbase required. Ordinary
 * words only; no decoder or scheduling. These functions do not repeat UNIT. */
enum p66_radial_status p66_radial_store_unit(p66_preparation *,p66_norm_scratch *,p66_pushdown *); /*LOC03375*/
enum p66_radial_status p66_radial_dot_velocity(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *); /*03377*/
enum p66_radial_status p66_radial_store_rate(p66_preparation *,p66_norm_scratch *,p66_pushdown *,p66_radial_state *); /*03401*/
enum p66_radial_status p66_radial_predict_distance(p66_preparation *,p66_scale_scratch *,p66_norm_scratch *,p66_pushdown *,const p66_radial_state *); /*03404*/
enum p66_radial_status p66_radial_subtract_radius(p66_preparation *,p66_scale_scratch *,p66_elapsed_state *,p66_norm_scratch *,p66_pushdown *,const agc_dp *landing_radius); /*03407*/
enum p66_radial_status p66_radial_store_altitude(p66_preparation *,p66_norm_scratch *,p66_pushdown *,p66_radial_state *); /*03411*/
#endif
