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
 *   DACNDLS, NEEDLER, NEEDLES (needles.c): the attitude errors on the FDAI
 *     needles, through the IMU error counters and their DACs.
 *
 * All basic code. Each function runs from a word the program can come
 * back to (an entry, the word after a call out of the file) until control
 * leaves the file's code (RESUME, NOVAC, SMALLMP, DPADD, TC Q), or reaches
 * a word where the faithful port's run stops: a key of its own, run by the
 * function named for it. */
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

/* ---- INDEX in the RCS DAP (indexing.c) ---- */

/* INDEX, then an order that reads the word at its address field plus the
 * index (CA, CS, MP, AD): the address. Not modelled (the faithful port
 * refuses them too): a sum outside 0-07777 or at Z, or a word of fixed
 * memory other than the file's tables read by INDEX (the gains, DACLIMIT,
 * FREETAU, MANTABLE). */
unsigned index_read(unsigned field, word index);

/* INDEX, then DCA or DCS, whose address field is the double word's
 * address +1: A,L the double word (DCS complemented). */
void dca_indexed(unsigned address, word index);
void dcs_indexed(unsigned address, word index);

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

#endif

#endif
