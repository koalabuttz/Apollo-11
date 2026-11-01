/* SPDX-License-Identifier: MIT */
/* RADAR_LEADIN_ROUTINES.agc (Luminary099 pp. 490-491): the radar sampling
 * loop of the LM's radar tests (R04, verb 63, and R77, verb 64's LR test).
 * The faithful C is c64-port/src/lm_radar_leadin_routines.c.
 *
 * All basic code, in bank 25. The entries below are the words the program
 * comes to: RADSAMP (a WAITLIST task), the words WAITLIST and NOVAC come
 * back to, DORSAMP (the NOVAC job), the word SWCALL comes back to after
 * VARADAR, RADSTALL's bad return, DORSAMP2 (its good one) and VARADAR. The
 * binding is src/binding/radar_leadin_routines.c. */
#ifndef READABLE_RADAR_LEADIN_H
#define READABLE_RADAR_LEADIN_H

#include "../agc/agc.h"

#if LUMINARY099
void radsamp(void);
void radsamp_after_waitlist(void); /* RADSAMP +6 */
void radsamp_after_novac(void);    /* RADSAMP +012 */
void dorsamp(void);
void dorsamp_after_varadar(void); /* DORSAMP +1 */
void dorsamp_bad_return(void);    /* DORSAMP +3: RADSTALL's bad return */
void dorsamp2(void);
void varadar(void);
#endif

#endif
