/* SPDX-License-Identifier: MIT */
/* The listing's names the readable routines use, for one mission.
 *
 * Build with LUMINARY099=1 (the LM) or COMANCHE055=1 (the CM). The
 * addresses come from the mission's assembled listing through
 * <m>_names.inc, which readable/tools/listing_names.py generates and
 * checks against the listing.
 *
 * A name means what it means in the listing (STYLE.md, Naming):
 *   - an erasable name is the word itself: NEWJOB, LOCCTR, LST1(3);
 *     ADDRESS_OF(NEWJOB) is its address, for INDEX and for store();
 *   - a constant in fixed memory is its value: EXECBANK, COREINC;
 *   - a label on code is its address: NOVAC, TASKOVER, EJSCAN. */
#ifndef READABLE_NAMES_H
#define READABLE_NAMES_H

#include "agc.h"

#if LUMINARY099
#include "lm_names.inc"
#elif COMANCHE055
#include "cm_names.inc"
#else
#error "build with LUMINARY099=1 or COMANCHE055=1"
#endif

#define ADDRESS_OF(name) K_##name

/* ---- Erasable words (ERASABLE_ASSIGNMENTS.agc) ---- */

/* The Executive's own (unswitched). */
#define NEWPRIO   ERASABLE(K_NEWPRIO)
#define LOCCTR    ERASABLE(K_LOCCTR)
#define NEWJOB    ERASABLE(K_NEWJOB)
#define EXECTEM1  ERASABLE(K_EXECTEM1)
#define EXECTEM2  ERASABLE(K_EXECTEM2)
#define FIXLOC    ERASABLE(K_FIXLOC)
#define OVFIND    ERASABLE(K_OVFIND)
#define INTBIT15  ERASABLE(K_INTBIT15)
#define EDOP      ERASABLE(K_EDOP)
#define NEWLOC(n) ERASABLE(K_NEWLOC + (n))
#define BUF(n)    ERASABLE(K_BUF + (n))

/* The running job's core set: core set 0. Core set k (k = 0-6, 0-7 in the
 * LM) is the same twelve words 12k further on: MPAC +0-6, MODE, LOC,
 * BANKSET, PUSHLOC, PRIORITY. LOCCTR and NEWJOB hold such offsets (12k). */
#define CORE_SET_WORDS 12u
#define MPAC(n)        ERASABLE(K_MPAC + (n))
#define LOC            ERASABLE(K_LOC)
#define BANKSET        ERASABLE(K_BANKSET)
#define PUSHLOC        ERASABLE(K_PUSHLOC)
#define PRIORITY       ERASABLE(K_PRIORITY)
/* A word of the core set at offset k (INDEX k on the name). */
#define IN_CORE_SET(name, k) ERASABLE(indexed(K_##name, (k)))

/* The VAC areas' use words: VAC1USE, then every 054 words up to VAC5USE.
 * A free area's word holds its own address; a used one's holds +0. */
#define VAC_AREAS          5u
#define VACNUSE_ADDRESS(n) (K_VAC1USE + 054u * ((n) - 1u))

/* WAITLIST's (unswitched). WAITEXIT holds a 2CADR (WAITEXIT +1 is WAITBANK). */
#define WAITEXIT(n) ERASABLE(K_WAITEXIT + (n))
#define WAITADR     ERASABLE(K_WAITADR)
#define WAITTEMP    ERASABLE(K_WAITTEMP)
#define WAITBANK    ERASABLE(K_WAITBANK)
#define TIME3       ERASABLE(K_TIME3)
#define RUPTAGN     ERASABLE(K_RUPTAGN)
/* The task lists, in the switched bank EBANK=LST1 names. */
#define LST1(n) ERASABLE(K_LST1 + (n))
#define LST2(n) ERASABLE(K_LST2 + (n))
/* LONGCALL's. */
#define LONGCADR(n) ERASABLE(K_LONGCADR + (n))
#define LONGTIME(n) ERASABLE(K_LONGTIME + (n))
#define LONGEXIT(n) ERASABLE(K_LONGEXIT + (n))

/* INTER-BANK_COMMUNICATION's. BUF2 and BUF2 +1 hold the return of a
 * BANKCALL or SWCALL (its address, then the caller's FBANK); RUPTREG3 and
 * RUPTREG4 the same for IBNKCALL and ISWCALL, which interrupt routines use. */
#define BUF2(n)  ERASABLE(K_BUF2 + (n))
#define RUPTREG3 ERASABLE(K_RUPTREG3)
#define RUPTREG4 ERASABLE(K_RUPTREG4)
#define MPTEMP   ERASABLE(K_MPTEMP)

/* The interrupt save area (ARUPT +1 is LRUPT). */
#define ARUPT    ERASABLE(K_ARUPT)
#define QRUPT    ERASABLE(K_QRUPT)
#define ZRUPT    ERASABLE(K_ZRUPT)
#define BANKRUPT ERASABLE(K_BANKRUPT)

/* Read by SVCT3 and the idle loop. */
#define FLAGWRD2 ERASABLE(K_FLAGWRD2)
#define IMUCADR  ERASABLE(K_IMUCADR)
#define SELFRET  ERASABLE(K_SELFRET)

/* ---- Constants in fixed memory ---- */

#define ZERO     ((word)0)
#define ONE      ((word)1)
#define TWO      ((word)2)
#define TWO_K    ((word)02000) /* 2K */
#define NEG2     ((word)KW_NEG2)
#define NEG1_2   ((word)KW_NEG1_2) /* NEG1/2 */
#define POS1_2   ((word)020000)    /* POS1/2 */
#define POSMAX   ((word)KW_POSMAX)
#define NEGMAX   ((word)040000)
#define BIT8     ((word)KW_BIT8)
#define BIT14    ((word)KW_BIT14)
#define LOW7     ((word)KW_LOW7)
#define LOW9     ((word)KW_LOW9)
#define LOW10    ((word)KW_LOW10)
#define FBANKMSK ((word)076000)
#define OCT40001 ((word)KW_OCT40001)
#define OCT40201 ((word)KW_OCT40201)

#define EXECBANK    ((word)KW_EXECBANK) /* CADR FINDVAC2 */
#define FAKEPRET    ((word)KW_FAKEPRET) /* ADRES MPAC -36D */
#define NO_CORES    ((word)KW_NO_CORES) /* NO.CORES: core sets less one */
#define COREINC     ((word)KW_COREINC)  /* 12 words per core set */
#define MINUS_CCSPR ((word)KW_MCCSPR)   /* -CCSPR: -(CCS PRIORITY) */
#define SELFBANK    ((word)KW_SELFBANK)
#define WAITBB      ((word)KW_WAITBB)
#define DELAYEX     ((word)KW_DELAYEX)
#define PRIO35      ((word)KW_PRIO35)
#define TSKOVCDR    ((word)KW_TSKOVCDR)
#define T3RPTBB     ((word)KW_T3RPTBB)
/* Two-word constants: ENDTASK (-2CADR SVCT3), DPBIT14 and LGCL2CDR. */
static const word ENDTASK[2] = KW_ENDTASK;
static const word DPBIT14[2] = KW_DPBIT14;
static const word LGCL2CDR[2] = KW_LGCL2CDR;

/* ---- Code addresses ---- */

#define NOVAC        ((word)K_NOVAC)
#define FINDVAC2     ((word)K_FINDVAC2)
#define NOVAC3       ((word)K_NOVAC3)
#define SPECTEST     ((word)K_SPECTEST)
#define NEXTCORE     ((word)K_NEXTCORE)
#define ENDPRCHG     ((word)K_ENDPRCHG)
#define JOBWAKE4     ((word)K_JOBWAKE4)
#define EJSCAN       ((word)K_EJSCAN)
#define EJ1          ((word)K_EJ1)
#define EJ2          ((word)K_EJ2)
#define ADVAN        ((word)K_ADVAN)
#define SUPDXCHZ     ((word)K_SUPDXCHZ)
#define NEG          ((word)K_NEG)
#define CCSHOLE      ((word)K_CCSHOLE)
#define NEWOPS       ((word)K_NEWOPS)
#define LVWTLIST     ((word)K_LVWTLIST)
#define WTLST5       ((word)K_WTLST5)
#define WTABORT      ((word)K_WTABORT)
#define FIXDELAY     ((word)K_FIXDELAY)
#define SVCT3        ((word)K_SVCT3)
#define SVCT3X       ((word)K_SVCT3X)
#define TASKOVER     ((word)K_TASKOVER)
#define T3RUPT2      ((word)K_T3RUPT2)
#define WAITLIST     ((word)K_WAITLIST)
#define LONGCALL     ((word)K_LONGCALL)
#define LONGCYCL     ((word)K_LONGCYCL)
#define LONGRTRN     ((word)K_LONGRTRN)
#define LASTTIME     ((word)K_LASTTIME)
#define MUCHTIME     ((word)K_MUCHTIME)
#define GETCADR      ((word)K_GETCADR)
#define BANKCALL     ((word)K_BANKCALL)
#define SWCALL       ((word)K_SWCALL)
#define SWRETURN     ((word)K_SWRETURN)
#define POSTJUMP     ((word)K_POSTJUMP)
#define BANKJUMP     ((word)K_BANKJUMP)
#define Q_PLUS_10000 ((word)K_QP10000) /* Q+10000 */
#define PRIO12       ((word)K_PRIO12)
#define MAKECADR     ((word)K_MAKECADR)
#define SUPDACAL     ((word)K_SUPDACAL)
#define IBNKCALL     ((word)K_IBNKCALL)
#define ISWCALL      ((word)K_ISWCALL)
#define ISWRETRN     ((word)K_ISWRETRN)
#define USPRCADR     ((word)K_USPRCADR)
#define SUPERSW      ((word)K_SUPERSW)
#if LUMINARY099
#define BAILOUT1 ((word)K_BAILOUT1)
#define POODOO1  ((word)K_POODOO1)
#define FILLED   ((word)K_FILLED)
#define LONGPOOH ((word)K_LONGPOOH)
#define WAITPOOH ((word)K_WAITPOOH)
#else
#define BAILOUT       ((word)K_BAILOUT)
#define POODOO        ((word)K_POODOO)
#define WATLST0_MINUS ((word)K_WATLST0M) /* WATLST0- */
#endif

/* NO.CORES + 1 core sets. */
#define CORE_SETS (NO_CORES + 1u)

#endif
