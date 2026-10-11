/* SPDX-License-Identifier: MIT */
#ifndef CM_EPHEMERIS_SOLAR_H
#define CM_EPHEMERIS_SOLAR_H
#include "cm_ephemeris_time.h"
/* LSPOS solar path through actual RESA frontier or SOLPOS through declared
 * fixed caller return. 0 invalid context,1 completed,2 sourceSQRTABRT. */
uint8_t cm_ephemeris_solar(apollo_overlay_state *,p66_bias_io *);
/* Explicit original computation phases; sole persistent state is canonical. */
uint8_t cm_ephemeris_solar_begin(apollo_overlay_state *,p66_bias_io *);
uint8_t cm_ephemeris_solar_vector(apollo_overlay_state *,p66_bias_io *);
uint8_t cm_ephemeris_solar_unit(apollo_overlay_state *,p66_bias_io *);
uint8_t cm_ephemeris_solar_plane(apollo_overlay_state *,p66_bias_io *);
uint8_t cm_ephemeris_solar_finish(apollo_overlay_state *,p66_bias_io *);
#endif
