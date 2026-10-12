/* SPDX-License-Identifier: MIT */
#include "lm_ephemeris_routines.h"
#define ROUTE(name,call) enum exec_status lm_ephemeris_route_##name(const agc_native_program *p,apollo_overlay_state *s,void *io){\
 (void)p;if(!s||!io)return EXEC_OUT_OF_SCOPE;return next(call);}
/* A step that leaves a NEWOPS frontier (the next phase, the caller or the
 * abort frontier) hands control back to the trampoline. */
static enum exec_status next(uint8_t phase){return phase<LME_FAILED?EXEC_OK:EXEC_OUT_OF_SCOPE;}
/* LSPOS and LUNVEL share LME_ENTRY, which reads LOC; only an admitted entry
 * image runs. */
enum exec_status lm_ephemeris_route_lspos(const agc_native_program *p,apollo_overlay_state *s,void *io){
 (void)p;if(!s||!io||!lm_ephemeris_admitted(s,io))return EXEC_OUT_OF_SCOPE;return next(lme_entry(s,io));
}
ROUTE(posita,lme_posita(s,io))
ROUTE(positd,lme_positd(s))
ROUTE(positf,lme_positf(s))
ROUTE(moon_vdef,lme_moon_vdef(s))
ROUTE(moon_unit,lme_moon_unit(s,io))
ROUTE(sun_angles,lme_sun_angles(s))
ROUTE(sun_vdef,lme_sun_vdef(s))
ROUTE(sun_unit,lme_sun_unit(s,io))
