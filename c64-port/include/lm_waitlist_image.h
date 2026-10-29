/* SPDX-License-Identifier: MIT */
#ifndef LM_WAITLIST_IMAGE_H
#define LM_WAITLIST_IMAGE_H
#include "p66_bias.h"
#include "waitlist.h"
enum lm_waitlist_entry {LM_WAITLIST_ENTRY,LM_TWIDDLE_ENTRY};
/* Literal WAITLIST/TWIDDLE with an explicit compiled task address. TWIDDLE
 * takes its minor bank word from the canonical caller banks and SUPER.
 * Runs through actual caller return or BAILOUT1/POODOO1 alarm entry;
 * source IRQ remains inhibited. The caller owns source producers. */
enum wl_status lm_waitlist_image(apollo_overlay_state *,p66_bias_io *,enum lm_waitlist_entry,exec_address);
#endif
