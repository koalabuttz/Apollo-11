/* SPDX-License-Identifier: MIT */
#ifndef RTB_AGREEMENT_H
#define RTB_AGREEMENT_H
#include "rtb_angles.h"
enum rtb_agreement_entry {RTB_SGN_AGREE=0,RTB_SIGN_MPAC=1,RTB_TRANSPOSE1=2,RTB_TRANSPOSE2=3};
/* Nonnull canonical image/io, ordinary15-bit words/raw16-bit A/L; source
 * bank10LM/22CM and actual entry, coherent EB/BB, REGZERO0. Exclude producers
 * through the basic body. Stops before actual DANZIG. SGNAGREE performs actual
 * TPAGREE and returns via L/Q2; SIGNMPAC retains Q and declares DP MODE.
 * Transposes are CM-only, leave EB5/BB updated and source ECADR in A;
 * no Executive check occurs before the returned DANZIG entry. */
enum rtb_status rtb_agreement(apollo_overlay_state *,p66_bias_io *,enum rtb_mission,enum rtb_agreement_entry);
uint8_t rtb_agreement_constant_read(enum rtb_mission,uint16_t physical_fixed_word,uint16_t *);
#endif
