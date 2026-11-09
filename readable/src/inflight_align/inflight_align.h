/* SPDX-License-Identifier: MIT */
/* INFLIGHT_ALIGNMENT_ROUTINES.agc, both missions' (Luminary099 pp.
 * 1249-1257, Comanche055 pp. 1355-1363): CALCGTA, ARCTRIG, CALCGA and
 * AXISGEN, in inflight_align.c. The binding is
 * src/binding/inflight_align_routines.c. */
#ifndef READABLE_INFLIGHT_ALIGN_H
#define READABLE_INFLIGHT_ALIGN_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#ifndef COUNT
#define COUNT(array) (sizeof(array) / sizeof((array)[0]))
#endif

/* A fixed label's bank, and its address in the bank, from its I_ value
 * (bank * 02000 + its place in the bank). */
#ifndef CODE_BANK
#define CODE_BANK(i) ((unsigned)((i) >> 10))
#define CODE_Z(i)    ((word)((i) % 02000u + 02000u))
#endif

/* CALCGTA to GIMLOCK1's EXIT; after GIMLOCK1's TC INTPRET to AXISGEN's
 * end; the constants QTSN45 and .166... */
extern const struct interpretive_code CALCGTA_CODE, AXISGEN_CODE, INFLIGHT_CONSTANTS;

/* The basic code after GIMLOCK1's EXIT. */
void gimlock1_after_exit(void);   /* GIMLOCK1 +1: TC ALARM */
void gimlock1_after_alarm(void);  /* GIMLOCK1 +3: TC UPFLAG */
void gimlock1_after_upflag(void); /* GIMLOCK1 +5: TC INTPRET */

#endif
