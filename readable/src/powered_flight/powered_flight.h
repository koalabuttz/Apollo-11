/* SPDX-License-Identifier: MIT */
/* POWERED_FLIGHT_SUBROUTINES.agc (Luminary099 pp. 1259-1267, Comanche055
 * pp. 1365-1372): the sines and cosines of the CDU angles, and the
 * transformations between the navigation base and the stable member, in
 * bank 23. The LM's FLESHPOT and the CM's SMCDURES are still the faithful
 * port's (c64-port/src/lm_powered_subs.c and cm_powered_subs.c).
 *
 * Most of the file is basic code. Each function below runs from a place
 * the flight program enters (a key of the faithful tables, or a return
 * into the file) until control leaves the file: by a TC to a routine
 * outside it (USPRCADR, MPACVBUF, SPSIN, SPCOS, the CM's DMPSUB), back
 * into the interpreter (INTPRET), or back to a caller. */
#ifndef READABLE_POWERED_FLIGHT_H
#define READABLE_POWERED_FLIGHT_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#ifndef COUNT
#define COUNT(a) (sizeof(a) / sizeof((a)[0]))
#endif
#ifndef CODE_BANK
#define CODE_BANK(fcadr) ((unsigned)(((fcadr) >> 10) & 037u))
#define CODE_Z(fcadr)    ((word)(02000u + ((fcadr) & 01777u)))
#endif

/* The interpretive words: each entry's EXIT, and the RVQs and VLOAD RVQ
 * the interface routines go back to the interpreter for. */
extern const struct interpretive_code CDUTRIG_CODE, CDUTRIG_RVQ_CODE, CD_STAR_TR_STAR_G_CODE,
    CD_STAR_TR_STAR_G_RVQ_CODE, CDU_STAR_SMNB_CODE, TRG_STAR_SMNB_CODE, C_STAR_MM_STAR_N2_CODE,
    CDU_STAR_NBSM_CODE, TRG_STAR_NBSM_CODE, STAR_SMNB_STAR_CODE, STAR_NBSM_STAR_CODE,
    AX_STAR_SR_STAR_T_CONSTANTS;
#if COMANCHE055
extern const struct interpretive_code NBSM_CODE;
#endif

/* cdutrig.c: CDUTRIG, CD*TR*G, CDUTRIGS and CD*TR*GS. */
void cdutrig_plus1(void);                         /* CDUTRIG +1: TC CDUTRIGS */
void cdutrig_plus2(void);                         /* CDUTRIG +2: TC INTPRET */
void cd_star_tr_star_g_plus1(void);               /* CD*TR*G +1: TC CD*TR*GS */
void cd_star_tr_star_g_plus2(void);               /* CD*TR*G +2: TC INTPRET */
void cdutrigs(void);                              /* CDUTRIGS */
void cd_star_tr_star_gs(void);                    /* CD*TR*GS */
void tr_star_gl_star_star_p_after_cdulogic(void); /* TR*GL**P +010 */
void tr_star_gl_star_star_p_after_cosine(void);   /* TR*GL**P +016 */
void tr_star_gl_star_star_p_after_sine(void);     /* TR*GL**P +026 */

/* quictrig.c: QUICTRIG. */
void quictrig(void);             /* the whole routine, for an RTB */
void quictrig_to_spsin(void);    /* QUICTRIG, to its first TC SPSIN */
void quictrig_after_spsin(void); /* QUICTRIG +011 */
void quictrig_after_spcos(void); /* QUICTRIG +020 */

/* transforms.c: the interface routines, from the interpreter to AX*SR*T. */
void cdu_star_smnb_plus1(void);     /* CDU*SMNB +1: TC CDUTRIGS */
void cdu_star_smnb_plus2(void);     /* CDU*SMNB +2: TCF C*MM*N1 */
void trg_star_smnb_plus1(void);     /* TRG*SMNB +1: TC CD*TR*GS */
void c_star_mm_star_n1(void);       /* C*MM*N1: TC MPACVBUF */
void c_star_mm_star_n1_plus1(void); /* C*MM*N1 +1: CS THREE */
void c_star_mm_star_n2_plus1(void); /* C*MM*N2 +1: TC INTPRET */
void cdu_star_nbsm_plus1(void);     /* CDU*NBSM +1: TC CDUTRIGS */
void cdu_star_nbsm_plus2(void);     /* CDU*NBSM +2: TCF C*MM*N3 */
void trg_star_nbsm_plus1(void);     /* TRG*NBSM +1: TC CD*TR*GS */
void c_star_mm_star_n3(void);       /* C*MM*N3: TC MPACVBUF */
void c_star_mm_star_n3_plus1(void); /* C*MM*N3 +1: CA THREE */
void star_smnb_star_plus1(void);    /* *SMNB* +1: TCF C*MM*N1 */
void star_nbsm_star_plus1(void);    /* *NBSM* +1: TCF C*MM*N3 */

/* ax_sr_t.c: AX*SR*T. */
void ax_star_sr_star_t(void);          /* AX*SR*T */
void loop1_after_sine_product(void);   /* LOOP1 +5 (the CM's DMPSUB's return) */
void loop1_after_cosine_product(void); /* LOOP1 +023 (the same) */

#endif
