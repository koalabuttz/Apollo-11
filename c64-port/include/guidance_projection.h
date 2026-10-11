/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_PROJECTION_H
#define GUIDANCE_PROJECTION_H
#include "guidance_rotation.h"
#include "guidance_push.h"
enum p66_projection_status {P66_PROJECTION_OK=0,P66_PROJECTION_OUT_OF_SCOPE=1};
/* P66 PDVL UNITX/CALL CDU*NBSM through STOVL /AFC/,DELVROD. Entry
 * scalar MPAC total acceleration, legal owned VAC/PUSHLOCbase, half unitR
 * VAC14..19. CDU words are live stable X/Y/Z counters, exclusively sampled.
 * Computes source trig/rotation, DOT14, STORE22 and saved-acceleration BDDV.
 * AFC is raw erasable output03463/4; DELVROD loads from preparation words.
 * SQRT-free: source division overflow continues to the final store/load.
 * Producer exclusion/distinct owned objects required. Invalid phase shape
 * rejects unchanged. The three entry points below end at original EDOP0
 * scheduler checks, with source LOC03427,03430,03433 respectively. They do
 * not dispatch an interpreter, job, interrupt or historical address. */
enum p66_projection_status p66_projection_dot(p66_preparation *,p66_scale_scratch *,
    p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *,
    p66_trig_scratch *,p66_rotation_scratch *,const agc_word cdu_xyz[3]);
enum p66_projection_status p66_projection_store(p66_preparation *,p66_norm_scratch *,p66_pushdown *);
enum p66_projection_status p66_projection_acceleration(p66_preparation *,p66_scale_scratch *,
    p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *,agc_dp *afc);
enum p66_projection_status p66_projection(p66_preparation *,p66_scale_scratch *,
    p66_elapsed_state *,p66_divide_scratch *,p66_norm_scratch *,p66_pushdown *,
    p66_trig_scratch *,p66_rotation_scratch *,const agc_word cdu_xyz[3],agc_dp *afc);
#endif
