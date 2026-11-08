/* SPDX-License-Identifier: MIT */
/* The CM's RCS digital autopilot, readable C of Comanche055 (CM).
 *
 * MYSUBS.agc (pp. 999-1001), mysubs.c: two arithmetic subroutines of the
 * DAP ("COUNT 21/DAPMS"), on the double word KMPAC, KMPAC +1:
 *     SMALLMP  a single word times KMPAC: KMPAC A, double length.
 *     DPADD    A,L added into KMPAC, the sum taken round the circle when
 *              it overflows (KMPAC is an angle).
 * Each is called by TC and returns by TC Q. The faithful C is
 * c64-port/src/cm_mysubs.c.
 *
 * The file also gives SPCOS and SPSIN (SINGLE_PRECISION_SUBROUTINES.agc,
 * src/single_precision) the DAP's names for them: SPCOS1 and SPCOS2 EQUALS
 * SPCOS, SPSIN1 and SPSIN2 EQUALS SPSIN.
 *
 * RCS-CSM_DIGITAL_AUTOPILOT.agc (pp. 1002-1024, bank 21), "T5 INTERRUPT
 * PROGRAM FOR THE RCS-CSM AUTOPILOT", c64-port/src/cm_rcs_dap.c, written
 * part by part:
 *
 *   REDORCS, RCSATT to DELOOP (rcsatt.c): phase 1 of the T5 program, every
 *     100 ms: the autopilot's state checked (G&N control, free mode, the
 *     IMU), the rate filter ("DRHO", "ADOT": the body rates from the CDU
 *     angles), the one-second call of the gimbal-to-body matrix update
 *     (KMATRIX) and, in an automatic maneuver, the desired angles stepped
 *     on by their increments (DELOOP).
 *   DACNDLS, NEEDLER, NEEDLES, FDAIDSP2 (needles.c): the attitude errors
 *     on the FDAI needles, through the IMU error counters and their DACs,
 *     and the errors they show.
 *   REINIT, FRESHDAP, REDAP, ZEROJET (initialize.c): the autopilot started
 *     again: the crew's DAP data decoded, its words and the jets zeroed,
 *     the rate filter primed, and the T5 program begun.
 *   T5PHASE2 to SPNDXCHK, FREEFUNC, STICKCHK, KALUPDT (manual.c): the
 *     start of phase 2, the RHC's manual rotation commands, as commanded
 *     rates in WBODY and the attitude errors they build up, for the jet
 *     selection logic, or in free mode as jet firing times.
 *
 * AUTOMATIC_MANEUVERS.agc (pp. 1025-1036, bank 21), the rest of phase 2,
 * c64-port/src/cm_auto_man.c, written part by part:
 *
 *   AHFNOROT, FREECONT, ATTHOLD, HOLDFUNC, GRABANG (attitude_hold.c): with
 *     no manual rotation, free mode's minimum impulses, or the attitude
 *     held and its errors.
 *   JETS to NJ22 (switching.c): the start of the jet switching logic, each
 *     axis' rate and attitude errors placed in the phase plane.
 *
 * All basic code. Each function runs from a word the program can come
 * back to (an entry, the word after a call out of the file) until control
 * leaves its file's code (the RCS DAP's to RESUME, NOVAC, IBNKCALL,
 * SMALLMP, DPADD, TC Q, AHFNOROT, JETS; AUTOMATIC_MANEUVERS's to REINIT,
 * STICKCHK, T6PROGM), or reaches a word where the faithful port's run
 * stops: a key of its own, run by the function named for it. */
#ifndef READABLE_CM_DAP_H
#define READABLE_CM_DAP_H

#include "../agc/agc.h"

void smallmp(void);
void dpadd(void);

#if COMANCHE055

#include "../tvc/tvc.h" /* value_of, tc_to, tc_q */
#include "../agc/names.h"

/* The loops' indexes, as INDEX reads them: SPNDX, the axis (2, 1, 0), and
 * DPNDX, twice it, for the double words. */
static inline word spndx(void)
{
    return fetch(ADDRESS_OF(SPNDX));
}
static inline word dpndx(void)
{
    return fetch(ADDRESS_OF(DPNDX));
}

/* ---- INDEX in the DAP files (indexing.c) ---- */

/* The words of fixed memory a file's code reads under INDEX, as the
 * faithful C copies them: whether an address is one. */
typedef bool fixed_tables(unsigned address);
bool rcs_dap_tables(unsigned address);  /* the gains, DACLIMIT, FREETAU, MANTABLE */
bool maneuver_tables(unsigned address); /* MINTAU, BIT13 on, OCT01400 on */

/* INDEX, then an order that reads the word at its address field plus the
 * index (CA, CS, MP, AD): the address. Not modelled (the faithful port
 * refuses them too): a sum outside 0-07777 or at Z, or a word of fixed
 * memory other than the file's tables. */
unsigned index_read_in(fixed_tables *tables, unsigned field, word index);

/* INDEX, then DCA or DCS, whose address field is the double word's
 * address +1: A,L the double word (DCS complemented). */
void dca_indexed_in(fixed_tables *tables, unsigned address, word index);
void dcs_indexed_in(fixed_tables *tables, unsigned address, word index);

/* RCS-CSM_DIGITAL_AUTOPILOT's reads. */
static inline unsigned index_read(unsigned field, word index)
{
    return index_read_in(rcs_dap_tables, field, index);
}
static inline void dca_indexed(unsigned address, word index)
{
    dca_indexed_in(rcs_dap_tables, address, index);
}
static inline void dcs_indexed(unsigned address, word index)
{
    dcs_indexed_in(rcs_dap_tables, address, index);
}

/* AUTOMATIC_MANEUVERS's. */
static inline unsigned maneuver_index_read(unsigned field, word index)
{
    return index_read_in(maneuver_tables, field, index);
}
static inline void maneuver_dca_indexed(unsigned address, word index)
{
    dca_indexed_in(maneuver_tables, address, index);
}
static inline void maneuver_dcs_indexed(unsigned address, word index)
{
    dcs_indexed_in(maneuver_tables, address, index);
}

/* INDEX, then ADS or LXCH, which write the word at their address: the
 * address. Not modelled (the faithful port refuses them too): one outside
 * erasable memory or among the central registers (0-07). */
unsigned index_store(unsigned field, word index);

/* INDEX, then DAS or DXCH, whose address field is the double word's
 * address +1, held to the same rule. */
void das_indexed(unsigned address, word index);
void dxch_indexed(unsigned address, word index);

/* INDEX, then TS: the address. Not modelled: one outside erasable memory,
 * or 4 to 6 (the bank registers and Z). */
unsigned index_ts(unsigned field, word index);

/* ---- RCS-CSM_DIGITAL_AUTOPILOT.agc, phase 1 -- rcsatt.c ---- */
void redorcs(void);                 /* REDORCS */
void rcsatt(void);                  /* RCSATT */
void rcsatt_after_zerojet(void);    /* RCSATT +032, after TCR ZEROJET */
void drholoop(void);                /* DRHOLOOP */
void drholoop_drho(void);           /* DRHOLOOP +3: INDEX DPNDX; CS DRHO */
void drholoop_drho_unindexed(void); /* DRHOLOOP +4: CS DRHO */
void drholoop_das(void);            /* DRHOLOOP +010: INDEX DPNDX; DAS DRHO */
void drholoop_after_smallmp(void);  /* DRHOLOOP +020 */
void adotloop(void);                /* ADOTLOOP */
void adotloop_drho(void);           /* ADOTLOOP +6 */
void adotloop_merrorx(void);        /* ADOTLOOP +013 */
void kmatrix(void);                 /* KMATRIX */
void kmatrix_after_novac(void);     /* KMATRIX +010 */
void deloop(void);                  /* DELOOP */
void deloop_delcdux(void);          /* DELOOP +6 */
void deloop_dpadd(void);            /* DELOOP +012: TC DPADD */
void deloop_after_dpadd(void);      /* DELOOP +013 */
void deloop_next(void);             /* DELOOP +021: CCS SPNDX */

/* ---- RCS-CSM_DIGITAL_AUTOPILOT.agc, the needles -- needles.c ---- */
void dacndls(void);                /* DACNDLS */
void dacndls_ccs(enum sign entry); /* DACNDLS, entered by a CCS */
void kresume1(void);               /* KRESUME1 */
void needler(void);                /* NEEDLER */
void needler_first_pass(void);     /* NEEDLER +5 */
void needle11(void);               /* NEEDLE11 */
void needler2(void);               /* NEEDLER2 */
void needler2_reset(void);         /* NEEDLER2 +3 */
void needler2_mask(void);          /* NEEDLER2 +4 */
void needles3(void);               /* NEEDLES3 */
void needles3_test(void);          /* NEEDLES3 +3: EXTEND; BZF NEEDLER +5 */
void needles(void);                /* NEEDLES */
void dacloop(void);                /* DACLOOP */
void dacloop_limit(void);          /* DACLOOP +6: CCS A */
void dacloop_return(void);         /* DACLOOP +034: TC Q */
void fdaidsp2(void);               /* FDAIDSP2 */
void fdaidsp2_ak(void);            /* FDAIDSP2 +011: DXCH AK */
void fdaitotl(void);               /* FDAITOTL */
void wrtn22(void);                 /* WRTN22 */
void wrtn22_wtemp(void);           /* WRTN22 +2: DXCH WTEMP */
void getaks(void);                 /* GETAKS */
void getaks_amgb5(void);           /* GETAKS +026: MP AMGB5 */
void wrtn17(void);                 /* WRTN17 */

/* ---- RCS-CSM_DIGITAL_AUTOPILOT.agc, starting the DAP -- initialize.c ---- */
void reinit(void);              /* REINIT */
void freshdap(void);            /* FRESHDAP */
void redap(void);               /* REDAP: TC IBNKCALL (S41.2) */
void redap_s40_14(void);        /* REDAP +2: TC IBNKCALL (S40.14) */
void redap_zerot5(void);        /* REDAP +4 */
void redap_after_zerojet(void); /* REDAP +014, after TCR ZEROJET */
void redap_chantemp(void);      /* REDAP +015: TS CHANTEMP */
void redap_imu(void);           /* REDAP +035: CS IMODES33 */
void imuaok_after_novac(void);  /* IMUAOK +4 */
void zerojet(void);             /* ZEROJET */
void zerojet_return(void);      /* ZEROJET +026: TC Q */

/* ---- RCS-CSM_DIGITAL_AUTOPILOT.agc, manual rotation -- manual.c ---- */
void t5phase2(void);                   /* T5PHASE2 */
void t5phase2_hiauto(void);            /* NOHIAUTO -1: CS ONE */
void nohiauto(void);                   /* NOHIAUTO */
void nohiauto_damping(void);           /* NOHIAUTO +025: MASK OCT16000 */
void nochange(void);                   /* NOCHANGE */
void nochange_after_stickchk(void);    /* NOCHANGE +6 */
void setwbody(void);                   /* SETWBODY */
void setwbody_dpndx(void);             /* SETWBODY +2: TS DPNDX */
void setwbody_rmanndx(void);           /* SETWBODY +3: INDEX SPNDX; CA RMANNDX */
void setwbody_rmanndx_unindexed(void); /* SETWBODY +4: CA RMANNDX */
void setwbody_wbody(void);             /* SETWBODY +015: INDEX DPNDX; DXCH WBODY */
void zeroer(void);                     /* ZEROER */
void norate(void);                     /* NORATE */
void merupdat(void);                   /* MERUPDAT */
void spndxchk(void);                   /* SPNDXCHK */
void spndxchk_errorx(void);            /* SPNDXCHK +2: INDEX SPNDX; TS ERRORX */
void freefunc(void);                   /* FREEFUNC */
void t6progm(void);                    /* T6PROGM */
void stickchk(void);                   /* STICKCHK */
void kalupdt(void);                    /* KALUPDT */
void kresume2_t5phase(void);           /* KRESUME2 +1: TS T5PHASE */

/* ---- AUTOMATIC_MANEUVERS.agc, attitude hold -- attitude_hold.c ---- */
void ahfnorot(void);                /* AHFNOROT */
void freecont_after_stickchk(void); /* FREECONT +012 */
void atthold(void);                 /* ATTHOLD */
void atthold_bias(void);            /* ATTHOLD +041: CA BIAS */
void atthold_bias_errorx(void);     /* ATTHOLD +042: ADS ERRORX */
void holdfunc(void);                /* HOLDFUNC */
void grabang(void);                 /* GRABANG */

/* ---- AUTOMATIC_MANEUVERS.agc, jet switching logic -- switching.c ---- */
void jets(void);    /* JETS */
void jloop(void);   /* JLOOP */
void inhold(void);  /* INHOLD */
void signck1(void); /* SIGNCK1 */
void posvel(void);  /* POSVEL */
void negvel(void);  /* NEGVEL */

#endif

#endif
