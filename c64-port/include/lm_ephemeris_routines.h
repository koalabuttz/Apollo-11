/* SPDX-License-Identifier: MIT */
#ifndef LM_EPHEMERIS_ROUTINES_H
#define LM_EPHEMERIS_ROUTINES_H
/* Luminary099 LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES as native interpretive
 * routines of the control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md). io is a
 * p66_bias_io. Each routine is keyed by the physical address of LOC under
 * BANKSET (bank 15) at an original NEWOPS frontier and runs one step of the
 * lm_ephemeris chain:
 *
 *   LSPOS (=LUNPOS) 03664 and LUNVEL 03775, the two entries (admitted as
 *   lm_ephemeris admits them); POSITA 03700, POSITD 03722, POSITF 03743, the
 *   two COS VDEF frontiers 03752 and 03763, the two MXV KONMAT UNIT frontiers
 *   03753 and 03764, and DLOAD KONMAT +2 03756.
 *
 * Each leaves the image at the next NEWOPS frontier, so the trampoline
 * continues at the routine for the next LOC, and after LUNVEL's RVQ or
 * STORE VSUN; RVQ at the caller's NEWOPS frontier selected by QPRET (or at
 * UNIT's SQRT abort frontier). A step outside the qualified primitive
 * contracts is EXEC_OUT_OF_SCOPE; as with lm_ephemeris, the image may already
 * have changed in that case. */
#include "agc_transfer.h"
#include "lm_ephemeris.h"
extern const agc_routine_table lm_ephemeris_routines;
/* The routines themselves, for programs that link only some phases (the C64
 * phase groups). */
enum exec_status lm_ephemeris_route_lspos(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status lm_ephemeris_route_posita(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status lm_ephemeris_route_positd(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status lm_ephemeris_route_positf(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status lm_ephemeris_route_moon_vdef(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status lm_ephemeris_route_moon_unit(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status lm_ephemeris_route_sun_angles(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status lm_ephemeris_route_sun_vdef(const agc_native_program *,apollo_overlay_state *,void *);
enum exec_status lm_ephemeris_route_sun_unit(const agc_native_program *,apollo_overlay_state *,void *);
/* Physical keys (agc_fixed_physical): FB bank 015, LOC 03664 ... 03775. */
#define LM_EPHEMERIS_KEY(loc) ((uint16_t)(015u*1024u+((loc)-02000u)))
#endif
