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

/* The listing's "nD" in basic code: word n (decimal) of the job's work
 * area, an offset from FIXLOC (INDEX FIXLOC; TS 10D). */
#define D(n) ((unsigned)(n))

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

/* PHASE_TABLE_MAINTENANCE's. MODREG is the major mode on the DSKY. The
 * restart groups 1-6 each keep a phase, its complement (-PHASE), a TBASE, a
 * priority or delta time (PHSPRDT) and a 2CADR (PHSNAME), the words of
 * group g at offset 2(g-1) from group 1's: the listing indexes them with
 * TEMPG (2g) from PHASE1 -2 and the like. TEMPG through TEMPBBCN are the
 * listing's names for ITEMP1-6 and RUPTREG1-4 while a phase changes. */
#define MODREG   ERASABLE(K_MODREG)
#define RUPTREG1 ERASABLE(K_RUPTREG1)
#define TEMPG    ERASABLE(K_TEMPG)
#define TEMPP    ERASABLE(K_TEMPP)
#define TEMPNM   ERASABLE(K_TEMPNM)
#define TEMPBB   ERASABLE(K_TEMPBB)
#define TEMPSW   ERASABLE(K_TEMPSW)
#define TEMPSW2  ERASABLE(K_TEMPSW2)
#define TEMPPR   ERASABLE(K_TEMPPR)
#define TEMPG2   ERASABLE(K_TEMPG2)
#define TEMPP2   ERASABLE(K_TEMPP2)
#define TEMPBBCN ERASABLE(K_TEMPBBCN)
#define TIME1    ERASABLE(K_TIME1)
#define TIME2    ERASABLE(K_TIME2)
/* A group's word: INDEX TEMPG (2g) on the name -2. */
#define OF_GROUP(name, twice_g) (ADDRESS_OF(name) - 2u + (twice_g))
#define PHSPRDT1_OF(twice_g)    ERASABLE(OF_GROUP(PHSPRDT1, twice_g))

/* RESTARTS_ROUTINE's. It runs as a job (GOPROG starts it) and borrows the
 * job's MPAC: TEMPPHS (the group's phase), TEMP2G (the group, doubled),
 * POINTER (into the restart tables) and TEMPSWCH (a return). GOLOC -1 to
 * GOLOC +2 hold the call it makes, run by TC GOLOC -1: TC FINDVAC, NOVAC,
 * WAITLIST or LONGCALL; the 2CADR to start; and where to go after. */
#define TEMPPHS  ERASABLE(K_TEMPPHS)
#define TEMP2G   ERASABLE(K_TEMP2G)
#define POINTER  ERASABLE(K_POINTER)
#define TEMPSWCH ERASABLE(K_TEMPSWCH)
#define GOLOC(n) ERASABLE(K_GOLOC + (unsigned)(n))
/* The editing registers CYR (cycle right) and SR (shift right). */
#define CYR_ADDRESS ((unsigned)K_CYR)
#define SR_ADDRESS  ((unsigned)K_SR)

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

#define ZERO     ((word)KW_ZERO)
#define ONE      ((word)KW_ONE)
#define TWO      ((word)KW_TWO)
#define TWO_K    ((word)KW_2K) /* 2K */
#define NEG2     ((word)KW_NEG2)
#define NEG1_2   ((word)KW_NEG1_2) /* NEG1/2 */
#define POS1_2   ((word)KW_POS1_2) /* POS1/2 */
#define POSMAX   ((word)KW_POSMAX)
#define NEGMAX   ((word)KW_NEGMAX)
#define BIT8     ((word)KW_BIT8)
#define BIT14    ((word)KW_BIT14)
#define LOW7     ((word)KW_LOW7)
#define LOW9     ((word)KW_LOW9)
#define LOW10    ((word)KW_LOW10)
#define OCT7     ((word)KW_OCT7)
#define OCT17770 ((word)KW_OCT17770)
#define OCT14000 ((word)KW_OCT14000)
#define OCT60000 ((word)KW_OCT60000)
#define BIT7     ((word)KW_BIT7)
#define BIT12    ((word)KW_BIT12)
#define BIT15    ((word)KW_BIT15)
#define PRIO30   ((word)KW_PRIO30)
#define BIT14COM ((word)KW_BIT14COM)
static const word SECS600[2] = KW_600SECS; /* 600SECS: 2DEC 60000 */
#define PRIO14   ((word)KW_PRIO14)
#define OCT1400  ((word)KW_OCT1400)
#define OCT177   ((word)KW_OCT177)
#define OCT37776 ((word)KW_OCT37776)
#define BIT10    ((word)KW_BIT10)
#define THREE    ((word)KW_THREE)
#define MINUS2   ((word)KW_MINUS2)
/* GENADRs of the routines RESTARTS calls or returns through. */
#define PHS2CADR ((word)KW_PHS2CADR) /* GENADR PHSPART2 */
#define PRT2CADR ((word)KW_PRT2CADR) /* GENADR GETPART2 */
#define LGCLCADR ((word)KW_LGCLCADR) /* GENADR LONGCALL */
#define FVACCADR ((word)KW_FVACCADR) /* GENADR FINDVAC */
#define WTLTCADR ((word)KW_WTLTCADR) /* GENADR WAITLIST */
#define NOVACADR ((word)KW_NOVACADR) /* GENADR NOVAC */
/* CA RTRNCADR reads the instruction there, TC SWRETURN, as a word: the
 * address SWRETURN (TC's code is 0). */
#define RTRNCADR_WORD ((word)KW_RTRNCADR)
#define CON2ADR       ((word)KW_CON2ADR)     /* GENADR CON2 */
#define SETUPDSP_CADR ((word)KW_MMDSPLAYP3)  /* MMDSPLAY +3: CADR SETUPDSP */
static const word ADRPCHN2[2] = KW_ADRPCHN2; /* 2CADR PHSCHNG2 */
#define FBANKMSK ((word)KW_FBANKMSK)
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
#define TWIDDLE      ((word)K_TWIDDLE)
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
#define CHECKMM      ((word)K_CHECKMM)
#define TCQ          ((word)K_TCQ)      /* = Q+2 +1, a TC Q in fixed-fixed */
#define SETUPDSP     ((word)K_SETUPDSP) /* bank 10 */
#define TOCON2       ((word)K_TOCON2)
#define BELOW4       ((word)K_BELOW4) /* bank 10 */
#define Q_PLUS_1     ((word)K_QP1)    /* Q+1 and Q+2, in INTERPRETER */
#define Q_PLUS_2     ((word)K_QP2)
/* ADRPCHN2, the 2CADR PHSCHNG2 that PHASJUMP's DCA reads; BIT14COM, the
 * constant in bank 10 that BELOW2 runs into if it ever got there. */
#define ADRPCHN2_ADDRESS ((word)K_ADRPCHN2)
#define BIT14COM_ADDRESS ((word)K_BIT14COM)
/* RESTARTS_ROUTINE (bank 01, but ITSINDIR and ITSLGCL1 in fixed-fixed). */
#define RESTARTS ((word)K_RESTARTS)
#define GETPART2 ((word)K_GETPART2)
#define PHSPART2 ((word)K_PHSPART2)
#define RTRNCADR ((word)K_RTRNCADR)
#define CHKNOVAC ((word)K_CHKNOVAC)
#define ITSNOVAC ((word)K_ITSNOVAC)
#define IMEDIATE ((word)K_IMEDIATE)
#define LONGCLCL ((word)K_LONGCLCL)
#define FINDVAC  ((word)K_FINDVAC)
/* The restart tables (RESTART_TABLES.agc, bank 01): from PRDTTAB, three
 * words per phase, a priority or delta time and then (at CADRTAB) a 2CADR.
 * SIZETAB +2g -2 and -1 locate group g's even and odd tables. */
#define PRDTTAB ((word)K_PRDTTAB)
#define CADRTAB ((word)K_CADRTAB)
#define SIZETAB ((word)K_SIZETAB)
#if COMANCHE055
#define NEWPHASE  ((word)K_NEWPHASE)
#define NUFAZ_P10 ((word)K_NUFAZP10) /* NUFAZ+10 */
#endif
#if LUMINARY099
#define FILLED   ((word)K_FILLED)
#define LONGPOOH ((word)K_LONGPOOH)
#define WAITPOOH ((word)K_WAITPOOH)
#else
#define WATLST0_MINUS ((word)K_WATLST0M) /* WATLST0- */
#endif

/* ---- The interpreter (INTERPRETER.agc) ---- */

/* Its erasables. POLISH holds the order pair being decoded and ADDRWD the
 * address of the current order; MODE (in the core set) says what MPAC holds:
 * +1 a triple, +0 a double, -1 a vector. CYR and CYL are the editing
 * registers cycle right and cycle left. */
#define ADDRWD      ERASABLE(K_ADDRWD)
#define POLISH      ERASABLE(K_POLISH)
#define MODE        ERASABLE(K_MODE)
#define INDEXLOC    ERASABLE(K_INDEXLOC)
#define QPRET       ERASABLE(K_QPRET)
#define MAXDVSW     ERASABLE(K_MAXDVSW)
#define DVNORMCT    ERASABLE(K_DVNORMCT)
#define DVSIGN      ERASABLE(K_DVSIGN)
#define ESCAPE      ERASABLE(K_ESCAPE)
#define ESCAPE2     ERASABLE(K_ESCAPE2)
#define POLYRET     ERASABLE(K_POLYRET)
#define POLYCNT     ERASABLE(K_POLYCNT)
#define MATINC      ERASABLE(K_MATINC)
#define DOTRET      ERASABLE(K_DOTRET)
#define DOTINC      ERASABLE(K_DOTINC)
#define SWBIT       ERASABLE(K_SWBIT)
#define SWWORD      ERASABLE(K_SWWORD)
#define TEM1        ERASABLE(K_TEM1)
#define DMPNTEMP    ERASABLE(K_DMPNTEMP)
#define CYL_ADDRESS ((unsigned)K_CYL)

/* Its constants (the name is the value, as everywhere). Some of them sit in
 * the slots after a CCS and so would also run as instructions: LOW7, LBUF2,
 * OCT176, SRDDV, B5TOBB, BIASHI and BIASLO as TC to their own value, 5B10 as
 * TCF 2000 and SLOPEHI as LXCH 650. */
#define B12T14           ((word)KW_B12T14)
#define B3TOB4           ((word)KW_B3TOB4)
#define B5TOBB           ((word)KW_B5TOBB)
#define BANKMASK         ((word)KW_BANKMASK)
#define BIASHI           ((word)KW_BIASHI)
#define BIASLO           ((word)KW_BIASLO)
#define BIT1             ((word)KW_BIT1)
#define BIT5             ((word)KW_BIT5)
#define BIT6             ((word)KW_BIT6)
#define BIT11            ((word)KW_BIT11)
#define BIT13            ((word)KW_BIT13)
#define CALLCODE         ((word)KW_CALLCODE)
#define DEC45            ((word)KW_DEC45)
#define DLOADCOD         ((word)KW_DLOADCOD)
#define VLOADCOD         ((word)KW_VLOADCOD)
#define DLOAD_STAR       ((word)KW_DLOAD_STAR) /* DLOAD*: the order code itself */
#define VLOAD_STAR       ((word)KW_VLOAD_STAR) /* VLOAD* */
#define FOUR             ((word)KW_FOUR)
#define FIVE             ((word)KW_FIVE)
#define SIX              ((word)KW_SIX)
#define SEVEN            ((word)KW_SEVEN)
#define TEN              ((word)KW_TEN)
#define THIRTEEN         ((word)KW_THIRTEEN)
#define FOURTEEN         ((word)KW_FOURTEEN)
#define FIVE_B10         ((word)KW_5B10) /* 5B10 */
#define HALF             ((word)KW_HALF)
#define QUARTER          ((word)KW_QUARTER)
#define HIGH4            ((word)KW_HIGH4)
#define HIGH9            ((word)KW_HIGH9)
#define LOW2             ((word)KW_LOW2)
#define LOW3             ((word)KW_LOW3)
#define LOW4             ((word)KW_LOW4)
#define LOW8             ((word)KW_LOW8)
#define LOW11            ((word)KW_LOW11)
#define LOW7_PLUS_2K     ((word)KW_LOW7P2K)  /* LOW7+2K */
#define MINUS_1_2_PLUS_2 ((word)KW_M1_2P2)   /* -1/2+2 */
#define MINUS_ENDERAS    ((word)KW_MENDERAS) /* -ENDERAS */
#define MINUS_ENDVAC     ((word)KW_MENDVAC)  /* -ENDVAC */
#define MINUS_OCT10      ((word)KW_MOCT10)   /* -OCT10 */
#define NEG4             ((word)KW_NEG4)
#define NEG12            ((word)KW_NEG12)
#define NEGONE           ((word)KW_NEGONE)
#define OCT23            ((word)KW_OCT23)
#define OCT176           ((word)KW_OCT176)
#define SLOPEHI          ((word)KW_SLOPEHI)
#define SLOPELO          ((word)KW_SLOPELO)
#define SRDDV            ((word)KW_SRDDV)
#define LBUF2            ((word)KW_LBUF2) /* ADRES BUF2 */
#define LVBUF            ((word)KW_LVBUF) /* ADRES VBUF */
/* The words that hold a TCF, read by INDEX and compared against ESCAPE. */
#define LASINEX ((word)KW_LASINEX) /* TCF ASINEX */
#define LDANZIG ((word)KW_LDANZIG) /* TCF DANZIG */
#define TCSUBTR ((word)KW_TCSUBTR) /* TCF SUBTR */
/* Words of code that run under an INDEX. ITR15 +1 and ITR11 +1 are the
 * words 7 INDJUMP -1 and 3 INDJUMP -1 that INDEX CYR makes TCF INDJUMP + OP; VAD +4, +11 and +18 are
 * the extracode READs that INDEX ADDRWD turns into DCA or DCS of a vector
 * component (their C names say which READ: HISCALAR, CHAN5, LCHAN). */
#define ITR_7_INDJUMP ((word)KW_ITR15P1) /* ITR15 +1 */
#define ITR_3_INDJUMP ((word)KW_ITR11P1) /* ITR11 +1 */
#define READ_HISCALAR ((word)KW_VADP4)   /* VAD +4 */
#define READ_CHAN5    ((word)KW_VADP11)  /* VAD +11 */
#define READ_LCHAN    ((word)KW_VADP18)  /* VAD +18 */

/* Its code labels. */
#define INTPRET        ((word)K_INTPRET)
#define DANZIG         ((word)K_DANZIG)
#define TLOAD          ((word)K_TLOAD)
#define NOIBNKSW       ((word)K_NOIBNKSW)
#define CHANG2         ((word)K_CHANG2)
#define DIRADRES       ((word)K_DIRADRES)
#define FIFTEEN_BITADR ((word)K_15BITADR) /* 15BITADR */
#define INDJUMP        ((word)K_INDJUMP)
#define STORJUMP       ((word)K_STORJUMP)
#define DODLOAD        ((word)K_DODLOAD)
#define DODLOAD_STAR   ((word)K_DODLOAD_STAR) /* DODLOAD* */
#define DOVLOAD        ((word)K_DOVLOAD)
#define DOVLOAD_STAR   ((word)K_DOVLOAD_STAR) /* DOVLOAD* */
#define GOTO           ((word)K_GOTO)
#define EXIT           ((word)K_EXIT)
#define SETOVF         ((word)K_SETOVF)
#define VAD            ((word)K_VAD)
#define BVSU           ((word)K_BVSU)
#define DOT            ((word)K_DOT)
#define DOTSUB         ((word)K_DOTSUB)
#define ENDDOT         ((word)K_ENDDOT)
#define VXM_MXV        ((word)K_VXM_MXV) /* VXM/MXV */
#define VVXSC          ((word)K_VVXSC)
#define DVXSC          ((word)K_DVXSC)
#define VPROJ          ((word)K_VPROJ)
#define VXV            ((word)K_VXV)
#define DMP1           ((word)K_DMP1)
#define DMPR           ((word)K_DMPR)
#define TPAGREE        ((word)K_TPAGREE)
#define RIGHTR         ((word)K_RIGHTR)
#define ROUND          ((word)K_ROUND)
#define VSSR           ((word)K_VSSR)
#define VSSL           ((word)K_VSSL)
#define TSLC2          ((word)K_TSLC2)
#define GENSHIFT       ((word)K_GENSHIFT)
#define VRIGHT2        ((word)K_VRIGHT2)
#define GENSCR         ((word)K_GENSCR)
#define BUFZERO        ((word)K_BUFZERO)
#define DVOVF          ((word)K_DVOVF)
#define DVNORM         ((word)K_DVNORM)
#define MAXTEST        ((word)K_MAXTEST)
#define V_SC2          ((word)K_V_SC2) /* V/SC2 */
#define SLASH_NORM     ((word)K__NORM) /* /NORM */
#define VECAGREE       ((word)K_VECAGREE)
#define UNIT           ((word)K_UNIT)
#define LARGE2         ((word)K_LARGE2)
#define DSQ            ((word)K_DSQ)
#define ABS            ((word)K_ABS)
#define ABVAL          ((word)K_ABVAL)
#define SQRT           ((word)K_SQRT)
#define SQRTABRT       ((word)K_SQRTABRT)
#define VSQ            ((word)K_VSQ)
#define VSQSUB         ((word)K_VSQSUB)
#define COSINE         ((word)K_COSINE)
#define SN1            ((word)K_SN1)
#define POLY           ((word)K_POLY)
#define POLY2          ((word)K_POLY2)
#define ARCCOS         ((word)K_ARCCOS)
#define ACOS_EQ_0      ((word)K_ACOS_EQ_0) /* ACOS=0 */
#define ACOSST2        ((word)K_ACOSST2)
#define ACOS3          ((word)K_ACOS3)
#define ACOSSHR        ((word)K_ACOSSHR)
#define ACOSABRT       ((word)K_ACOSABRT)
#define SUBTR          ((word)K_SUBTR)
#define AXT            ((word)K_AXT)
#define AXC            ((word)K_AXC)
#define LXA            ((word)K_LXA)
#define LXC            ((word)K_LXC)
#define SXA            ((word)K_SXA)
#define XCHX           ((word)K_XCHX)
#define XAD            ((word)K_XAD)
#define XSU            ((word)K_XSU)
#define INCR           ((word)K_INCR)
#define TIX            ((word)K_TIX)
#define RTB            ((word)K_RTB)
#define BZE_GOTO       ((word)K_BZE_GOTO) /* BZE/GOTO */
#define BPL            ((word)K_BPL)
#define BPL_BMN        ((word)K_BPL_BMN)  /* BPL/BMN */
#define CALL_ITA       ((word)K_CALL_ITA) /* CALL/ITA */
#define TEST           ((word)K_TEST)

/* ---- Alarms and aborts (ALARM_AND_ABORT.agc) ---- */

/* ALMCADR and ALMCADR +1: the 2CADR of the last alarm's caller. ITEMP1
 * keeps the return meanwhile. FAILREG +0, +1 and +2: the first, second and
 * latest alarm codes. DSPTAB +11D: the display word that holds the PROG
 * light. */
#define ALMCADR(n)           ERASABLE(K_ALMCADR + (n))
#define ITEMP1               ERASABLE(K_ITEMP1)
#define FAILREG(n)           ERASABLE(K_FAILREG + (n))
#define DSPTAB(n)            ERASABLE(K_DSPTAB + (n))
#define FLAGWRD3             ERASABLE(K_FLAGWRD3)
#define FLAGWRD7             ERASABLE(K_FLAGWRD7)
#define FLGWRD10             ERASABLE(K_FLGWRD10)
#define BRUPT                ERASABLE(K_BRUPT)
#define MINUS_PHASE4_ADDRESS ((unsigned)K_MPHASE4) /* -PHASE4, then PHASE4 */
#if COMANCHE055
#define LOCALARM ERASABLE(K_LOCALARM)
#define BANKALRM ERASABLE(K_BANKALRM)
#endif

/* Its constants. */
#define OCT40400 ((word)KW_OCT40400)
#define OCT1103  ((word)KW_OCT1103)
#define V05N09   ((word)KW_V05N09)
#define V37FLBIT ((word)KW_V37FLBIT)
#define NODOBIT  ((word)KW_NODOBIT)
#if LUMINARY099
#define OCT35    ((word)KW_OCT35)
#define BBSERVDL ((word)KW_BBSERVDL) /* BBCON SERVIDLE */
#define STATEBIT ((word)KW_STATEBIT)
#define REINTBIT ((word)KW_REINTBIT)
/* ADR40400 and ADR77770 are instructions, TCF OCT40400 and TCF OCT77770,
 * read as words. */
#define ADR40400 ((word)KW_ADR40400)
#define ADR77770 ((word)KW_ADR77770)
#endif

/* Its labels. PRIOLARM is in bank 10 (with PRIODSPR, which it goes on to)
 * and the LM's FLAGS in bank 13; the rest are in fixed-fixed memory. */
#define ALARM    ((word)K_ALARM)
#define ALARM2   ((word)K_ALARM2)
#define BORTENT  ((word)K_BORTENT)
#define PRIOENT  ((word)K_PRIOENT)
#define LARMENT  ((word)K_LARMENT)
#define CHKFAIL1 ((word)K_CHKFAIL1)
#define MULTEXIT ((word)K_MULTEXIT)
#define PRIOLARM ((word)K_PRIOLARM)
#define PRIODSPR ((word)K_PRIODSPR)
#define BAILOUT  ((word)K_BAILOUT)
#define WHIMPER  ((word)K_WHIMPER)
#define POODOO   ((word)K_POODOO)
#define ABORT2   ((word)K_ABORT2)
#define CURTAINS ((word)K_CURTAINS)
#define VARALARM ((word)K_VARALARM)
#if LUMINARY099
#define GOPOODOO ((word)K_GOPOODOO)
#define STRTIDLE ((word)K_STRTIDLE)
#define BAILOUT1 ((word)K_BAILOUT1)
#define BOTHABRT ((word)K_BOTHABRT)
#define POODOO1  ((word)K_POODOO1)
#define ALARM1   ((word)K_ALARM1)
#define ALMNCADR ((word)K_ALMNCADR)
#define FLAGS    ((word)K_FLAGS)
#endif
#define DOWNFLAG ((word)K_DOWNFLAG)

/* ---- Single precision sine and cosine (SINGLE_PRECISION_SUBROUTINES.agc) ---- */

/* TEMK holds the argument as it is reduced, then the result; SQ its
 * square. The CM shares them with other routines (TEMK EQUALS HALFY, SQ
 * EQUALS ROOTRET). */
#define TEMK ERASABLE(K_TEMK)
#define SQ   ERASABLE(K_SQ)
/* The polynomial's coefficients, halved (C1/2 is pi/4). */
#define C1_2   ((word)KW_C1_2) /* C1/2 */
#define C3_2   ((word)KW_C3_2) /* C3/2 */
#define C5_2   ((word)KW_C5_2) /* C5/2 */
#define SPCOS  ((word)K_SPCOS)
#define SPSIN  ((word)K_SPSIN)
#define POLLEY ((word)K_POLLEY)
#define ARG90  ((word)K_ARG90)

/* ---- Service routines (SERVICE_ROUTINES.agc) ---- */

/* The flag words, FLAGWRD0 on: FLAGWRD0 +n is flag word n. ITEMP1-3 are
 * the scratch words of routines that run with interrupts inhibited (ITEMP1
 * is also WAITEXIT). DELAYLOC +n holds the job DELAYJOB put to sleep in
 * slot n. EXTVBACT: the extended verb in progress (bit 5 set by TESTXACT). */
#define FLAGWRD0(n) ERASABLE(K_FLAGWRD0 + (n))
#define FLAGWRD4    ERASABLE(K_FLAGWRD4)
#define ITEMP2      ERASABLE(K_ITEMP2)
#define ITEMP3      ERASABLE(K_ITEMP3)
#define DELAYLOC(n) ERASABLE(K_DELAYLOC + (n))
#define EXTVBACT    ERASABLE(K_EXTVBACT)
#if COMANCHE055
#define OPTMODES ERASABLE(K_OPTMODES)
#endif

/* Its constants. DELAYNUM is the last DELAYLOC slot. TCSLEEP and WAKECAD
 * are GENADRs: TCGETCAD -2 and WAKER. OCT40200 is the tracker light (bit
 * 8) and bit 15 of a display word. */
#define OCT77770 ((word)KW_OCT77770)
#define DELAYNUM ((word)KW_DELAYNUM)
#define TCSLEEP  ((word)KW_TCSLEEP)
#define WAKECAD  ((word)KW_WAKECAD)
#define OCT40200 ((word)KW_OCT40200)
/* BIT15 and the bits after it, BIT14 ... BIT1, in the constant pool. */
#define BIT15_ADDRESS ((unsigned)K_BIT15)

/* Its labels. DELAYJOB, TCGETCAD and WAKER are in bank 0, the LM's UPENT2
 * and DOWNENT2 in bank 10; the rest are in fixed-fixed memory. */
#define UPENT2   ((word)K_UPENT2)
#define DOWNENT2 ((word)K_DOWNENT2)
#define UPFLAG   ((word)K_UPFLAG)
#define COMFLAG  ((word)K_COMFLAG)
#define DEBIT    ((word)K_DEBIT)
#define DELAYJOB ((word)K_DELAYJOB)
#define OK2DELAY ((word)K_OK2DELAY)
#define TCGETCAD ((word)K_TCGETCAD)
#define WAKER    ((word)K_WAKER)
#define GENTRAN  ((word)K_GENTRAN)
#define B5OFF    ((word)K_B5OFF)
#if COMANCHE055
#define TRFAILOF ((word)K_TRFAILOF)
#define TRFAILON ((word)K_TRFAILON)
#endif
/* Where they go on to: the Executive, WAITLIST, and the aborts. */
#define JOBSLEEP ((word)K_JOBSLEEP)
#define JOBWAKE  ((word)K_JOBWAKE)
#define ENDOFJOB ((word)K_ENDOFJOB)
#define DLY2     ((word)K_DLY2)

/* ---- Down-telemetry (DOWN-TELEMETRY_PROGRAM.agc) ---- */

/* Its erasable words. DNLSTCOD numbers the downlist being sent, DUMPCNT
 * how many erasable dumps V74 asked for. DNTMGOTO holds where the next
 * DOWNRUPT goes (run there as TC). The listing gives three words two names
 * each: CTLIST (LDATALST) points into the control list, SUBLIST (DNQ) into
 * a sublist, and DNECADR (TMINDEX, DUMPLOC) holds the DNADR being sent,
 * the snapshot buffer's index, or the erasable dump's place. DNTMBUFF +n
 * is the snapshot buffer. */
#define DNLSTCOD    ERASABLE(K_DNLSTCOD)
#define DUMPCNT     ERASABLE(K_DUMPCNT)
#define DNTMGOTO    ERASABLE(K_DNTMGOTO)
#define CTLIST      ERASABLE(K_CTLIST)
#define SUBLIST     ERASABLE(K_SUBLIST)
#define DNECADR     ERASABLE(K_DNECADR)
#define TMINDEX     ERASABLE(K_TMINDEX)
#define DUMPLOC     ERASABLE(K_DUMPLOC)
#define DNTMBUFF(n) ERASABLE(K_DNTMBUFF + (n))

/* Its constants. MINTIME2 is the -1DNADR of TIME2, DNADRDCR (OCT 74001)
 * takes one from a DNADR's count and two from its address, MINB1314
 * (-030000) and MINB12 (-04000, -1/8) tell a DNADR, a DNPTR and a DNCHAN
 * apart. LDNPHAS2, LDNDUMP and LDNDUMP1 hold the addresses DNTMGOTO gets.
 * ERASID and LOWIDCOD are the dump's ID word and the downlink's sync
 * bits. MINTIME2, DNADRDCR, MINB1314 and LDNPHAS2 are also labels: CCS
 * lands on them and runs them as instructions (ADDRESS_OF gives where). */
#define NEG0     ((word)KW_NEG0)
#define ERASID   ((word)KW_ERASID)
#define PRIO34   ((word)KW_PRIO34)
#define MINB12   ((word)KW_MINB12)
#define LOWIDCOD ((word)KW_LOWIDCOD)
#define MINTIME2 ((word)KW_MINTIME2)
#define DNADRDCR ((word)KW_DNADRDCR)
#define MINB1314 ((word)KW_MINB1314)
#define LDNPHAS2 ((word)KW_LDNPHAS2)
#define LDNDUMP  ((word)KW_LDNDUMP)
#define LDNDUMP1 ((word)KW_LDNDUMP1)

/* Its labels, all in bank 05 with the downlists (DNTABLE is the table of
 * the downlists' control lists). */
#define DODOWNTM ((word)K_DODOWNTM)
#if LUMINARY099
#define WO1    ((word)K_WO1)
#define WOTEST ((word)K_WOTEST)
#endif
#define DNPHASE1 ((word)K_DNPHASE1)
#define DNPHASE2 ((word)K_DNPHASE2)
#define DODNADR  ((word)K_DODNADR)
#define SETWO    ((word)K_SETWO)
#define DNDUMPI  ((word)K_DNDUMPI)
#define DNDUMP   ((word)K_DNDUMP)
#define DNDUMP1  ((word)K_DNDUMP1)
#define DNDUMP2  ((word)K_DNDUMP2)
#define SENDID   ((word)K_SENDID)
#define DNTABLE  ((word)K_DNTABLE)
#define RESUME   ((word)K_RESUME)

/* The channels: CHAN13 (bit 7 is the downlink's word order code), and
 * DNTM1 and DNTM2, the two words each DOWNRUPT sends. */
#define CHAN13 013u
#define DNTM1  034u
#define DNTM2  035u

/* ---- Fresh start and restart (FRESH_START_AND_RESTART.agc) ---- */

/* Its erasable words, by the listing's names (ADDRESS_OF gives each one's
 * address for the instructions). The restart groups' phase pairs:
 * -PHASEn and PHASEn, the phase complemented and the phase. */
#define K_MINUS_PHASE1 K_MPHASE1
#define K_MINUS_PHASE2 K_MPHASE2
#define K_MINUS_PHASE3 K_MPHASE3
#define K_MINUS_PHASE4 K_MPHASE4
#define K_MINUS_PHASE5 K_MPHASE5
#define K_MINUS_PHASE6 K_MPHASE6
#if LUMINARY099
#define K_MINUS_RATEDB K_MRATEDB /* -RATEDB */
#endif
#define STATE(n) ERASABLE(K_STATE + (n)) /* STATE +n: flag word n */

/* Its channels: CHAN5 and CHAN6 (the RCS jets), DSALMOUT (11), CHAN12,
 * CHAN14, CHAN33, MNKEYIN (15, the main DSKY's keys) and NAVKEYIN (16, the
 * navigation DSKY's keys and the MARK buttons); SUPERBNK is channel 7 and
 * LCHAN (1) register L. */
#define CHAN5    ((unsigned)K_CHAN5)
#define CHAN6    ((unsigned)K_CHAN6)
#define DSALMOUT ((unsigned)K_DSALMOUT)
#define CHAN12   ((unsigned)K_CHAN12)
#define CHAN14   ((unsigned)K_CHAN14)
#define CHAN33   ((unsigned)K_CHAN33)
#define MNKEYIN  ((unsigned)K_MNKEYIN)
#define NAVKEYIN ((unsigned)K_NAVKEYIN)

/* Its constants. PRIO12 is the label of an instruction (TCF 10000) the
 * fresh start loads as the number 12000. SWINIT +n is flag word n's fresh
 * start value. */
#define PRIO12_WORD ((word)KW_PRIO12)
#define BITS4_6     ((word)KW_BITS4_6) /* BITS4&6 */
#define BITS_9_6_4  ((word)KW_9_6_4)   /* 9,6,4: bits 9, 6 and 4 */
#define BIT3        ((word)KW_BIT3)
#define BIT4        ((word)KW_BIT4)
#define HI5         ((word)KW_HI5)
#define EBANK3      ((word)KW_EBANK3)
#define EBANK6      ((word)KW_EBANK6)
#define NUMGRPS     ((word)KW_NUMGRPS)
#define MINUS_ELR   ((word)KW_MELR) /* -ELR: minus the ERROR LIGHT RESET key */
#define IM30INIF    ((word)KW_IM30INIF)
#define IM30INIR    ((word)KW_IM30INIR)
#define IM33INIT    ((word)KW_IM33INIT)
#define IFAILINH    ((word)KW_IFAILINH)
#define INTFLBIT    ((word)KW_INTFLBIT)
#define ENGONBIT    ((word)KW_ENGONBIT)
#define RACTCADR    ((word)KW_RACTCADR)
#define LDNPHAS1    ((word)KW_LDNPHAS1)
#define STARTEB     ((word)KW_STARTEB)
#define VAC1ADRC    ((word)KW_VAC1ADRC)
#define LTHVACA     ((word)KW_LTHVACA)
#define NOUTCON     ((word)KW_NOUTCON)
#define LESCHK      ((word)KW_LESCHK)
#define VD1         ((word)KW_VD1)
static const word SWINIT[12] = {KW_SWINIT,   KW_SWINITP1, KW_SWINITP2,  KW_SWINITP3,
                                KW_SWINITP4, KW_SWINITP5, KW_SWINITP6,  KW_SWINITP7,
                                KW_SWINITP8, KW_SWINITP9, KW_SWINITP10, KW_SWINITP11};
#if LUMINARY099
#define MAXDB     ((word)KW_MAXDB)
#define BOOLSTRT  ((word)KW_BOOLSTRT)
#define STIKSTRT  ((word)KW_STIKSTRT)
#define RATESTRT  ((word)KW_RATESTRT)
#define FULLAPS   ((word)KW_FULLAPS)
#define OCT_77001 ((word)KW_77001OCT) /* 77001OCT */
#define DEC_60    ((word)KW_60DEC)    /* 60DEC */
#define REFSMBIT  ((word)KW_REFSMBIT)
#define SURFFBIT  ((word)KW_SURFFBIT)
#define CMOONBIT  ((word)KW_CMOONBIT)
#define LMOONBIT  ((word)KW_LMOONBIT)
#define APSFLBIT  ((word)KW_APSFLBIT)
#define RSFLGBTS  ((word)KW_RSFLGBTS)
#define DIDFLBIT  ((word)KW_DIDFLBIT)
#define RODFLBIT  ((word)KW_RODFLBIT)
#define P21FLBIT  ((word)KW_P21FLBIT)
#define RMODINIT  ((word)KW_RMODINIT)
#define ACCSOKAY  ((word)KW_ACCSOKAY)
#define OCT30001  ((word)KW_OCT30001)
#define READRBIT  ((word)KW_READRBIT)
#define NR29FBIT  ((word)KW_NR29FBIT)
#define OCT32001  ((word)KW_OCT32001)
#define OCT27470  ((word)KW_OCT27470)
#define NORRMBIT  ((word)KW_NORRMBIT)
#define R77FLBIT  ((word)KW_R77FLBIT)
#define OCT74160  ((word)KW_OCT74160)
#define OCT33     ((word)KW_OCT33)
static const word IDLEADR[2] = KW_IDLEADR; /* 2CADR DAPIDLER */
#else
#define OPTINITF ((word)KW_OPTINITF)
#define OPTINITR ((word)KW_OPTINITR)
#define NOP01BIT ((word)KW_NOP01BIT)
#define OCT6200  ((word)KW_OCT6200)
#define INTMASK  ((word)KW_INTMASK)
#define OCT37    ((word)KW_OCT37)
#define OCT77603 ((word)KW_OCT77603)
#define OCT74777 ((word)KW_OCT74777)
#define SKIPVBIT ((word)KW_SKIPVBIT)
/* T5IDLER: four 2CADRs, for the T5RUPT owner FLAGWRD6 names. */
static const word T5IDLER[4][2] = {KW_T5IDLER, KW_T5IDLERP2, KW_T5IDLERP4, KW_T5IDLERP6};
#endif
#define HALF_SEC ((word)KW__5SEC) /* .5SEC */
#define PRIO25 ((word)KW_PRIO25)

/* Its labels, and the routines elsewhere it calls. */
#define MMDSPLAY ((word)K_MMDSPLAY)
#define PHASCHNG ((word)K_PHASCHNG)
#define SLAP1    ((word)K_SLAP1)
#define STARTSW  ((word)K_STARTSW)
#define SKIPSIM  ((word)K_SKIPSIM)
#define DOFSTART ((word)K_DOFSTART)
#define ENDRSTRT ((word)K_ENDRSTRT)
#define MR_KLEAN ((word)K_MR_KLEAN) /* MR.KLEAN */
#define P00KLEAN ((word)K_P00KLEAN)
#define V37KLEAN ((word)K_V37KLEAN)
#define GOPROG   ((word)K_GOPROG)
#define BUTTONS  ((word)K_BUTTONS)
#define ENEMA    ((word)K_ENEMA)
#define GOPROG2  ((word)K_GOPROG2)
#define GOPROG3  ((word)K_GOPROG3)
#define PCLOOP   ((word)K_PCLOOP)
#define NXTRST   ((word)K_NXTRST)
#define PINACT   ((word)K_PINACT)
#define PTBAD    ((word)K_PTBAD)
#define LIGHTSET ((word)K_LIGHTSET)
#define NONAVKEY ((word)K_NONAVKEY)
#define STARTSUB ((word)K_STARTSUB)
#define STARTSB2 ((word)K_STARTSB2)
#define GOTOPOOH ((word)K_GOTOPOOH)
#define NOQRSM   ((word)K_NOQRSM)
#if LUMINARY099
#define DOFSTRT1 ((word)K_DOFSTRT1)
#define DORSTART ((word)K_DORSTART)
#define SETINFL  ((word)K_SETINFL)
#define GOPROG2A ((word)K_GOPROG2A)
#define STARTSB1 ((word)K_STARTSB1)
#else
#define ELRSKIP  ((word)K_ELRSKIP)
#define NOCOARSE ((word)K_NOCOARSE)
#define T5IDLOC  ((word)K_T5IDLOC)
#define STOPRATE ((word)K_STOPRATE)
#endif

/* ---- Verb 37 and P00 (FRESH_START_AND_RESTART.agc, bank 04) ---- */

/* Its erasable words: ADDRESS_OF(MMNUMBER) and the like. MMTEMP is
 * PHSPRDT3 and BASETEMP is TBASE4, by EQUALS. Its constants: */
#define V37N99   ((word)KW_V37N99) /* VN 3799: flash V37 N99 */
#define NOV37MM  ((word)KW_NOV37MM)
#define R00AD    ((word)KW_R00AD)
#define V37QCAD  ((word)KW_V37QCAD)
#define DNLADP00 ((word)KW_DNLADP00)
#define RNDVZBIT ((word)KW_RNDVZBIT)
#define PRIO5    ((word)KW_PRIO5)
#define PRIO37   ((word)KW_PRIO37)
#define OCT24    ((word)KW_OCT24)
#define OCT120   ((word)KW_OCT120)
#define ELEVEN   ((word)KW_ELEVEN)
#if LUMINARY099
#define DEC70    ((word)KW_DEC70)
#define V37RETAD ((word)KW_V37RETAD)
#define P25FLBIT ((word)KW_P25FLBIT)
#define OCT37667 ((word)KW_OCT37667)
#define OCT40072 ((word)KW_OCT40072)
#define LRBYBIT  ((word)KW_LRBYBIT)
#define OCT700   ((word)KW_OCT700)
#define OCT26    ((word)KW_OCT26)
#define OCT500   ((word)KW_OCT500)
#define P20REG   ((word)KW_P20REG)
#define OCT305   ((word)KW_OCT305)
#define NEG2     ((word)KW_NEG2)
static const word P70CADR[2] = KW_P70CADR;
#else
#define THREE_1SEC ((word)KW_3_1SEC) /* 3.1SEC */
#define AVEGBIT    ((word)KW_AVEGBIT)
#define OCT44571   ((word)KW_OCT44571)
#define OCT600     ((word)KW_OCT600)
#define BIT7_8     ((word)KW_BIT7_8) /* BIT7-8 */
#define OCT01120   ((word)KW_OCT01120)
#define MAXDBBIT   ((word)KW_MAXDBBIT)
#define TWELVE     ((word)KW_TWELVE)
#define EIGHTEEN   ((word)KW_EIGHTEEN)
#endif

/* Its labels, and the routines elsewhere it calls. */
#define GOP00FIX ((word)K_GOP00FIX)
#define V37      ((word)K_V37)
#define V37BAD   ((word)K_V37BAD)
#define ISSERVON ((word)K_ISSERVON)
#define CANV37   ((word)K_CANV37)
#define DUMMYAD  ((word)K_DUMMYAD)
#define POOH     ((word)K_POOH)
#define SEUDOP00 ((word)K_SEUDOP00)
#define GOMOD    ((word)K_GOMOD)
#define GOGOPROG ((word)K_GOGOPROG)
#define NOUVEAU  ((word)K_NOUVEAU)
#define V37NONO  ((word)K_V37NONO)
#define V37XEQ   ((word)K_V37XEQ)
#define V37XEQC  ((word)K_V37XEQC)
#define CLEARMRK ((word)K_CLEARMRK)
#define RELDSP   ((word)K_RELDSP)
#define FALTON   ((word)K_FALTON)
#define SPVAC    ((word)K_SPVAC)
#define NEWMODEA ((word)K_NEWMODEA)
#if LUMINARY099
#define CANTROD  ((word)K_CANTROD)
#define V37RET   ((word)K_V37RET)
#define R00      ((word)K_R00)
#define RESET22  ((word)K_RESET22)
#define KILL2    ((word)K_KILL2)
#define SETUP70  ((word)K_SETUP70)
#define PHSCHNGA ((word)K_PHSCHNGA)
#define CLRADMOD ((word)K_CLRADMOD)
#else
#define CANTR00  ((word)K_CANTR00)
#define R00TOP00 ((word)K_R00TOP00)
#define ROC      ((word)K_ROC)
#define GROUPKIL ((word)K_GROUPKIL)
#define INITSUB  ((word)K_INITSUB)
#define SETMAXER ((word)K_SETMAXER)
#define RAKE     ((word)K_RAKE)
#define VAC5STOR ((word)K_VAC5STOR)
#define V5OUT1   ((word)K_V5OUT1)
#endif

/* ---- The keyboard and uplink interrupts (KEYRUPT_UPRUPT.agc) ---- */

/* Its erasable words (ADDRESS_OF gives each one's address): INLINK, the
 * uplink's register, which holds the word the ground sent; KEYTEMP1 (the
 * same word as RUPTREG4) and KEYTEMP2, the word as it is tested; FLAGWRD5
 * (DSKYFLAG, bit 15) and FLAGWRD7 (UPLOCKFL, bit 4). */

/* Its constants. CHRPRIO is CHARIN's job priority, ELRCODE the ERROR RESET
 * key's code. HI10 (OCT 77740, -037) is also a label: CCS lands on it and
 * runs it as an instruction. The LM's listing names the flag bits
 * DSKYFBIT (BIT15) and UPLOCBIT (BIT4); the CM's writes BIT15 and BIT4. */
#define LOW5    ((word)KW_LOW5)
#define CHRPRIO ((word)KW_CHRPRIO)
#define HI10    ((word)KW_HI10)
#define ELRCODE ((word)KW_ELRCODE)
#if LUMINARY099
#define DSKYFBIT ((word)KW_DSKYFBIT)
#define UPLOCBIT ((word)KW_UPLOCBIT)
#endif

/* Its labels, in bank 04 (LM) or 07 (CM), and LODSAMPT, PINBALL's routine
 * that takes the time of a key, in fixed-fixed memory. */
#define KEYRUPT1 ((word)K_KEYRUPT1)
#define KEYCOM   ((word)K_KEYCOM)
#define ACCEPTUP ((word)K_ACCEPTUP)
#define UPRUPT   ((word)K_UPRUPT)
#define UPRPT1   ((word)K_UPRPT1)
#define UPOK     ((word)K_UPOK)
#define CLUPLOCK ((word)K_CLUPLOCK)
#define TMFAIL2  ((word)K_TMFAIL2)
#define UPTEST   ((word)K_UPTEST)
#define LODSAMPT ((word)K_LODSAMPT)

/* ---- The interrupt lead-ins (INTERRUPT_LEAD_INS.agc) ---- */

/* The BBCONs the lead-ins load, each interrupt's bank and erasable bank.
 * UPRPTBB is KEYRPTBB, by EQUALS. The LM's T6RUPT jumps by a 2CADR in
 * fixed memory (T6ADR); its T5RUPT and the CM's T6RUPT and T5RUPT by a
 * 2CADR in erasable (T5ADR, T6LOC, T5LOC), which the program changes. */
#define GOBB     ((word)KW_GOBB)
#define T4RPTBB  ((word)KW_T4RPTBB)
#define KEYRPTBB ((word)KW_KEYRPTBB)
#define MKRUPTBB ((word)KW_MKRUPTBB)
#define UPRPTBB  ((word)KW_UPRPTBB)
#define DWNRPTBB ((word)KW_DWNRPTBB)
#define RDRPTBB  ((word)KW_RDRPTBB)
#if LUMINARY099
#define RUPT10BB ((word)KW_RUPT10BB)
static const word T6ADR[2] = KW_T6ADR; /* 2CADR DOT6RUPT */
#else
#define HCRUPTBB ((word)KW_HCRUPTBB)
#endif

/* The handlers they go to, in other files, and the CM's T5RUPT, the end
 * of its T5RUPT lead-in. NOQBRSM is the last of WAITLIST's interrupt
 * exits (RESUME, NOQRSM, NOQBRSM). */
#define T4RUPT   ((word)K_T4RUPT)
#define MARKRUPT ((word)K_MARKRUPT)
#if LUMINARY099
#define RADAREAD ((word)K_RADAREAD)
#define PITFALL  ((word)K_PITFALL)
#else
#define VHFREAD ((word)K_VHFREAD)
#define T5RUPT  ((word)K_T5RUPT)
#endif
#define NOQBRSM ((word)K_NOQBRSM)

/* ---- The LM's T6 interrupt (T6-RUPT_PROGRAMS.agc) ---- */

/* Its erasable words: TIME6, the clock (ADDRESS_OF(TIME6)), and in EBANK 6
 * the queue of the next two RCS jet firings: T6NEXT and T6FURTHA, each a
 * time and a writer's offset (NXT6ADR after T6NEXT), and NEXTP, NEXTU and
 * NEXTV, the jets to fire. Its labels, in bank 17 (T6JOBCHK, DOT6RUPT) or
 * fixed-fixed memory (the writers), and its one constant, 00314OCT, the
 * channel 5 bits of the V jets. */
#if LUMINARY099
#define T6JOBCHK  ((word)K_T6JOBCHK)
#define T6JOB     ((word)K_T6JOB)
#define ENABLET6  ((word)K_ENABLET6)
#define GOCH56    ((word)K_GOCH56)
#define DOT6RUPT  ((word)K_DOT6RUPT)
#define WRITEP    ((word)K_WRITEP)
#define WRITEU    ((word)K_WRITEU)
#define WRITEV    ((word)K_WRITEV)
#define OCT_00314 ((word)KW_00314OCT) /* 00314OCT */
#endif

/* ---- T4RUPT (T4RUPT_PROGRAM.agc): the DSKY's relays ---- */

/* Its erasable words (ADDRESS_OF gives each one's address): DSRUPTSW, the
 * phase count; DSPTAB, the display table (+11D the lights' relay word);
 * NOUT, the count of display words to send; DSPCNT, where the scan is;
 * DSRUPTEM, the scan's pass; TIME4, the clock; IMODES33, the PRO key's
 * last state (bit 14). OUT0 (channel 10) takes a relay word, CHAN32
 * bit 14 is the PRO key. */
#define OUT0   ((unsigned)K_OUT0)
#define CHAN32 ((unsigned)K_CHAN32)

/* Its constants: TIME4 counts for 20 and 120 ms (20MRUPT, 120MRUPT, the
 * latter a word in the code), TABLNTH (OCT 12, also in the code), BIT9,
 * OCT37737 (in the code, where it runs as an instruction), and RELTAB,
 * the relay word codes of DSPTAB's eleven words and the lights' word
 * (RELTAB11). The CM names 14,11,9 11,14,9. */
#define TWENTY_MS_RUPT    ((word)KW_20MRUPT)  /* 20MRUPT */
#define HUNDRED20_MS_RUPT ((word)KW_120MRUPT) /* 120MRUPT */
#define TABLNTH           ((word)KW_TABLNTH)
#define BIT9              ((word)KW_BIT9)
#if LUMINARY099
#define BITS_14_11_9 ((word)KW_14_11_9) /* 14,11,9 */
#else
#define BITS_14_11_9 ((word)KW_11_14_9) /* 11,14,9 */
#endif
static const word RELTAB[12] = {KW_RELTAB,   KW_RELTABP1, KW_RELTABP2,  KW_RELTABP3,
                                KW_RELTABP4, KW_RELTABP5, KW_RELTABP6,  KW_RELTABP7,
                                KW_RELTABP8, KW_RELTABP9, KW_RELTABP10, KW_RELTAB11};
#define RELTAB11 ((word)KW_RELTAB11)

/* Its labels, in bank 06, and where T4JUMP goes, in other files. */
#define NORMT4   ((word)K_NORMT4)
#define CDRVE    ((word)K_CDRVE)
#define DSPOUTSB ((word)K_DSPOUTSB)
#define DSPSCAN  ((word)K_DSPSCAN)
#define DSPLAY   ((word)K_DSPLAY)
#define DSPOUT   ((word)K_DSPOUT)
#define HANG20   ((word)K_HANG20)
#define SETTIME4 ((word)K_SETTIME4)
#define PROCEEDE ((word)K_PROCEEDE)
#define T4JUMP   ((word)K_T4JUMP)
#define NODSPOUT ((word)K_NODSPOUT)
#define QUIKDSP  ((word)K_QUIKDSP)
#define QUIKRUPT ((word)K_QUIKRUPT)
#define NODSPY   ((word)K_NODSPY)
#define SYNCT4   ((word)K_SYNCT4)
#define QUIKOFF  ((word)K_QUIKOFF)
#define IMUMON   ((word)K_IMUMON)
#if LUMINARY099
#define RCSMONIT ((word)K_RCSMONIT)
#define RRAUTCHK ((word)K_RRAUTCHK)
#define DAPT4S   ((word)K_DAPT4S)
#else
#define OPTTEST ((word)K_OPTTEST)
#define OPTMON  ((word)K_OPTMON)
#endif

/* ---- T4RUPT, part 2: the IMU monitor (IMUMON to LAMPTEST) ---- */

/* Its erasable words (ADDRESS_OF gives each one's address): IMODES30 and
 * IMODES33, the IMU's and channel 33's bits as last sampled, with the
 * monitor's own flags; RUPTREG1 and RUPTREG2, the bit scan's position and
 * the changes left; CDUZ, the middle gimbal angle; CDUXCMD, CDUYCMD,
 * CDUZCMD and GYROCMD, the drive counters; ITEMP6, SETISSW's return over
 * VARALARM; the LM's RADMODES (the radar's), FLAGWRD5 (ENGONFLG) and
 * 1/PIPADT. Channel 30 holds the IMU's status bits, channel 33 the PIPA
 * fail and telemetry rate flip-flops. */
#define CHAN30 ((unsigned)K_CHAN30)

/* Its constants: the read masks of channels 30 and 33 (30RDMSK, a word
 * inside the IFAILJMP table, and 33RDMSK, which EQUALS PRIO16), the masks
 * and times of the listing's page "JUMP TABLES AND CONSTANTS", and the
 * flag bits it tests: IMUSEBIT and IMUSEFLG (BIT8 of FLAGWRD0 in the LM,
 * of STATE in the CM) and the LM's DRFTBIT (FLAGWRD2 bit 15). The CM
 * names BITS6&15 BIT15+6. */
#define K30RDMSK     ((word)KW_30RDMSK) /* 30RDMSK */
#define K33RDMSK     ((word)KW_33RDMSK) /* 33RDMSK */
#define OCT15        ((word)KW_OCT15)
#define OCT15000     ((word)KW_OCT15000)
#define OC40010      ((word)KW_OC40010)
#define OCT54        ((word)KW_OCT54)
#define OCT75        ((word)KW_OCT75)
#define OCT272       ((word)KW_OCT272)
#define BITS7_8      ((word)KW_BITS7_8) /* BITS7&8 */
#define OCT1720      ((word)KW_OCT1720)
#define OCT740       ((word)KW_OCT740)
#define OCT77000     ((word)KW_OCT77000)
#define MINUS_BIT10  ((word)KW_MBIT10)  /* -BIT10 */
#define K90SECS      ((word)KW_90SECS)  /* 90SECS */
#define K4SECS       ((word)KW_4SECS)   /* 4SECS */
#define MINUS_70DEGS ((word)KW_M70DEGS) /* -70DEGS */
#define MINUS_15DEGS ((word)KW_M15DEGS) /* -15DEGS */
#define IMUSEFLG     ((word)KW_IMUSEFLG)
#define BIT2         ((word)KW_BIT2)
#define BITS4_5      ((word)KW_BITS4_5) /* BITS4&5 */
#define OCT37737     ((word)KW_OCT37737)
#if LUMINARY099
#define BITS6_15 ((word)KW_BITS6_15) /* BITS6&15 */
#define IMUSEBIT ((word)KW_IMUSEBIT)
#define DRFTBIT  ((word)KW_DRFTBIT)
#define RRINIT   ((word)KW_RRINIT)
#else
#define BITS6_15 ((word)KW_BIT15P6) /* BIT15+6 */
#endif

/* Its labels, in bank 06, and the routines it calls in other files. */
#define TNONTEST ((word)K_TNONTEST)
#define NXTIFAIL ((word)K_NXTIFAIL)
#define PROCTNON ((word)K_PROCTNON)
#define RETNON   ((word)K_RETNON)
#define ENDTNON  ((word)K_ENDTNON)
#define ENDTNON2 ((word)K_ENDTNON2)
#define UNZ2     ((word)K_UNZ2)
#define ISSUP    ((word)K_ISSUP)
#define OPONLY   ((word)K_OPONLY)
#define ISSZERO  ((word)K_ISSZERO)
#define C33TEST  ((word)K_C33TEST)
#define NXTFL33  ((word)K_NXTFL33)
#define GLOCKCHK ((word)K_GLOCKCHK)
#define NOGIMRUN ((word)K_NOGIMRUN)
#define GLAMPTST ((word)K_GLAMPTST)
#define TEMPOK   ((word)K_TEMPOK)
#define ITURNON  ((word)K_ITURNON)
#define IMUCAGE  ((word)K_IMUCAGE)
#define IMUOP    ((word)K_IMUOP)
#define PIPFAIL  ((word)K_PIPFAIL)
#define DNTMFAST ((word)K_DNTMFAST)
#define UPTMFAST ((word)K_UPTMFAST)
#define SETISSW  ((word)K_SETISSW)
#define ISSWON   ((word)K_ISSWON)
#define IFAILJMP ((word)K_IFAILJMP)
#define C33JMP   ((word)K_C33JMP)
#define VARDELAY ((word)K_VARDELAY)
#define ZEROICDU ((word)K_ZEROICDU)

/* ---- T4RUPT, part 3: the LM's radar checks and GPMATRIX ---- */

#if LUMINARY099
/* Its erasable words (ADDRESS_OF gives each one's address): RADMODES, the
 * rendezvous radar's modes and its inbits as last sampled; CDUX and CDUZ,
 * the outer and middle gimbal angles; COSMG and M11 ... M32, GPMATRIX's
 * results (in EBANK 6, as T4RUPT runs). Its constants: the RADMODES bits
 * (AUTOMBIT, RCDUFBIT, REPOSBIT) and masks, and the flag bits it tests:
 * NORRMBIT (FLAGWRD5, no angle monitor), AVEGFBIT (FLAGWRD7, SERVICER
 * running), MUNFLBIT (FLAGWRD6) and RNDVZBIT (FLAGWRD0, P20 or P22). */
#define AUTOMBIT ((word)KW_AUTOMBIT)
#define OCT05776 ((word)KW_OCT05776)
#define OCT10001 ((word)KW_OCT10001)
#define RCDUFBIT ((word)KW_RCDUFBIT)
#define AVEGFBIT ((word)KW_AVEGFBIT)
#define MUNFLBIT ((word)KW_MUNFLBIT)
#define OCT32002 ((word)KW_OCT32002)
#define REPOSBIT ((word)KW_REPOSBIT)
#define OCT20002 ((word)KW_OCT20002)

/* Its labels, in bank 06, and the routines it calls in other files. */
#define RRCDUCHK ((word)K_RRCDUCHK)
#define TRKFLCDU ((word)K_TRKFLCDU)
#define RRGIMON  ((word)K_RRGIMON)
#define MONREPOS ((word)K_MONREPOS)
#define GPMATRIX ((word)K_GPMATRIX)
#define SETTRKF  ((word)K_SETTRKF)
#define RRLIMCHK ((word)K_RRLIMCHK)
#endif

/* ---- T4RUPT, part 4: the CM's optics (OPTMON to MAXPLS1) ---- */

#if COMANCHE055
/* Its erasable words (ADDRESS_OF gives each one's address): OPTMODES, the
 * optics' inbits as last sampled and its modes; SWSAMPLE and DESOPMOD, the
 * optics mode switch now and as last acted on (positive CSC, +0 manual,
 * -1 zero optics); ZOPTCNT and WTOPTION, zeroing's counters; OPTIND, the
 * coarse optics state (-1 not working) and OPTDRIVE's index; CDUT and
 * CDUS, the trunnion and shaft CDUs, DESOPTT and DESOPTS their desired
 * angles, COMMANDO their commands and CDUTCMD their drive counters; ZONE,
 * the shaft's stop zone.
 *
 * Its constants, the listing's names: OCT13 (ELEVEN), OCTHIRTY (BITS4&5,
 * the switch's bits), OCT40 (BIT6), 200MS (OCT24); 20DEGS; 13,14,15 (-45
 * degrees); MAXPLS and MAXPLS1, the most pulses one pass sends, as
 * negative numbers; 11,12 (PRIO6), the drive bits of channel 14. */
#define OCT13        ((word)KW_OCT13)
#define OCTHIRTY     ((word)KW_OCTHIRTY)
#define OCT40        ((word)KW_OCT40)
#define K200MS       ((word)KW_200MS)    /* 200MS */
#define K20DEGS      ((word)KW_20DEGS)   /* 20DEGS */
#define MINUS_45DEGS ((word)KW_13_14_15) /* 13,14,15 */
#define MAXPLS       ((word)KW_MAXPLS)
#define MAXPLS1      ((word)KW_MAXPLS1)
#define BITS11_12    ((word)KW_11_12) /* 11,12 */

/* Its labels: in bank 06 (OPTMON to OPFAILOF) and bank 10 (OPTDRIVE to
 * SETBIT). */
#define K33OPTMON ((word)K_33OPTMON) /* 33OPTMON */
#define SETSAMP   ((word)K_SETSAMP)
#define PROCESSW  ((word)K_PROCESSW)
#define ZOPTDES   ((word)K_ZOPTDES)
#define ZTOMAN    ((word)K_ZTOMAN)
#define ZOPALARM  ((word)K_ZOPALARM)
#define ZTOCSC    ((word)K_ZTOCSC)
#define COARSLOK  ((word)K_COARSLOK)
#define ZOPFINI   ((word)K_ZOPFINI)
#define ZOPFIN2   ((word)K_ZOPFIN2)
#define CANZOPT   ((word)K_CANZOPT)
#define MANUDES   ((word)K_MANUDES)
#define MANTOMAN  ((word)K_MANTOMAN)
#define MANTOCSC  ((word)K_MANTOCSC)
#define CSCDES    ((word)K_CSCDES)
#define CSCTOZOP  ((word)K_CSCTOZOP)
#define CSCTOMAN  ((word)K_CSCTOMAN)
#define CANCOARS  ((word)K_CANCOARS)
#define OPTZERO   ((word)K_OPTZERO)
#define SETCNT    ((word)K_SETCNT)
#define SETDESMD  ((word)K_SETDESMD)
#define SETZOEND  ((word)K_SETZOEND)
#define ENDZOPT   ((word)K_ENDZOPT)
#define ZEROPCDU  ((word)K_ZEROPCDU)
#define INITZOPT  ((word)K_INITZOPT)
#define OCDUFTST  ((word)K_OCDUFTST)
#define OPFAILON  ((word)K_OPFAILON)
#define SETOFF    ((word)K_SETOFF)
#define OPFAILOF  ((word)K_OPFAILOF)
#define OPTDRIVE  ((word)K_OPTDRIVE)
#define OZONE     ((word)K_OZONE)
#define CONTDRVE  ((word)K_CONTDRVE)
#define OPT2      ((word)K_OPT2)
#define GETOPCMD  ((word)K_GETOPCMD)
#define STORCMD   ((word)K_STORCMD)
#define CMDSETUP  ((word)K_CMDSETUP)
#define TRUNCMD   ((word)K_TRUNCMD)
#define POSOPCMD  ((word)K_POSOPCMD)
#define NEXTOPT   ((word)K_NEXTOPT)
#define NEGOPCMD  ((word)K_NEGOPCMD)
#define DELOPCMD  ((word)K_DELOPCMD)
#define SENDOCMD  ((word)K_SENDOCMD)
#define SETBIT    ((word)K_SETBIT)
#endif

/* ---- RCS_FAILURE_MONITOR: the LM's jet isolation valves ---- */

#if LUMINARY099
/* Its erasable words (ADDRESS_OF gives each one's address): PVALVEST, the
 * valve states it last acted on (channel 32's low 8 bits, set for a closed
 * valve); CH5MASK and CH6MASK, the jets the DAP may not fire (channels 5
 * and 6 bits). Its constants: PRIO27, 1/ACCJOB's priority; the tables
 * 5FAILTAB and 6FAILTAB, each the word before its eight jet bits (the
 * listing's EQUALS -1), so INDEX L with L 1 to 8 reads one. */
#define PRIO27       ((word)KW_PRIO27)
#define FIVE_FAILTAB ((word)K_5FAILTAB) /* 5FAILTAB */
#define SIX_FAILTAB  ((word)K_6FAILTAB) /* 6FAILTAB */
#define FIVE_FAILTAB_WORDS                                                                         \
    {KW_5FAILTABP1, KW_5FAILTABP2, KW_5FAILTABP3, KW_5FAILTABP4,                                   \
     KW_5FAILTABP5, KW_5FAILTABP6, KW_5FAILTABP7, KW_5FAILTABP8}
#define SIX_FAILTAB_WORDS                                                                          \
    {KW_6FAILTABP1, KW_6FAILTABP2, KW_6FAILTABP3, KW_6FAILTABP4,                                   \
     KW_6FAILTABP5, KW_6FAILTABP6, KW_6FAILTABP7, KW_6FAILTABP8}

/* Its labels, in bank 06; RCSMONEX EQUALS RESUME. */
#define RCSMON     ((word)K_RCSMON)
#define VOPENED    ((word)K_VOPENED)
#define ONE_ACCFIX ((word)K_1_ACCFIX) /* 1/ACCFIX */
#define RCSMONEX   ((word)K_RCSMONEX)
#endif

/* ---- AGS_INITIALIZATION: the LM's R47 ---- */

#if LUMINARY099
/* Its erasable words (ADDRESS_OF gives each one's address): SAMPTIME, the
 * time of the last ENTER; AGSK, the mission time of the AGS clock's zero
 * (in EBANK 4, as R47 runs); DSPTEMX, where the display shows it and V25
 * loads it; AGSWORD, the downlist to go back to. Its constants: the verb
 * and noun words it displays (V06N16, V50N16), the 20 s it waits while
 * the AGS downlist goes out, and LAGSLIST (= ONE), that downlist's code. */
#define V06N16     ((word)KW_V06N16)
#define V50N16     ((word)KW_V50N16)
#define TWENTY_SEC ((word)KW_20SEC) /* 20SEC */
#define LAGSLIST   ((word)KW_LAGSLIST)

/* Its labels, in bank 32, and the routines it calls in other files. */
#define AGSINIT  ((word)K_AGSINIT)
#define NEWAGS   ((word)K_NEWAGS)
#define REDSPTEM ((word)K_REDSPTEM)
#define AGSDISPK ((word)K_AGSDISPK)
#define AGSVCALC ((word)K_AGSVCALC)
#define CKSTALL  ((word)K_CKSTALL)
#define AGSEND   ((word)K_AGSEND)
#define SCALEVEC ((word)K_SCALEVEC)
#define GOMARKF  ((word)K_GOMARKF)
#define EXDSPRET ((word)K_EXDSPRET)
#define IMUZERO  ((word)K_IMUZERO)
#define IMUSTALL ((word)K_IMUSTALL)
#define GOMARK3  ((word)K_GOMARK3)
#endif

/* ---- The sighting marks: AOTMARK (LM) and SXTMARK (CM) ---- */

/* MARKSTAT, the state of the marking: the VAC area of the marks in its
 * low 9 bits, and each mission's own bits above them. MKVACFND, MKRELEAS,
 * VACSTOR and ENDMARKS are labels of both missions' files; ENDMARK,
 * GOMARK4 and GOODEND are PINBALL's and the extended verbs'. */
#define MARKSTAT ERASABLE(K_MARKSTAT)
#define MKVACFND ((word)K_MKVACFND)
#define MKRELEAS ((word)K_MKRELEAS)
#define VACSTOR  ((word)K_VACSTOR)
#define ENDMARKS ((word)K_ENDMARKS)
#define ENDMARK  ((word)K_ENDMARK)
#define GOMARK4  ((word)K_GOMARK4)
#define GOODEND  ((word)K_GOODEND)

#if LUMINARY099
/* AOTMARK's erasable words: MARKSTAT (its VAC area in bits 1-9; bits 10
 * and 11 the X and Y marks wanted or made; bit 12 set to keep MARKRUPT
 * out; bit 13 a mark rejected; bit 14 a pair complete);
 * XYMARK, the mark's kind (BIT10 X, BIT11 Y) or the detent code; MARKCNTR,
 * the pairs made; MKDEX, an index (the verb to display, a mark's place in
 * the VAC area, the star vectors averaged). ADDRESS_OF gives the others:
 * AOTCODE, the detent and star code the crew load; AZ, the COAS's azimuth
 * and elevation; AOTAZ and AOTEL, each detent's; TSIGHT, the last mark's
 * time; CURSOR and SPIRAL, the surface readings. */
#define XYMARK   ERASABLE(K_XYMARK)
#define MARKCNTR ERASABLE(K_MARKCNTR)
#define MKDEX    ERASABLE(K_MKDEX)

/* Its constants: priorities (PRIO15, and PRIO3 and PRIO23, which are
 * also MARKSTAT's bits 10-11 and 10, 11, 14), the verb and noun words it
 * displays, and the key bits of channel 16 it tests (OCT34: the marks and
 * mark reject; OCT140: the rate of descent switch). */
#define PRIO3       ((word)KW_PRIO3)
#define PRIO15      ((word)KW_PRIO15)
#define PRIO23      ((word)KW_PRIO23)
#define NEG7        ((word)KW_NEG7)
#define OCT34       ((word)KW_OCT34)
#define OCT140      ((word)KW_OCT140)
#define V01N71      ((word)KW_V01N71)
#define V06N87_STAR ((word)KW_V06N87_STAR) /* V06N87* */
#define V06N79_STAR ((word)KW_V06N79_STAR) /* V06N79* */
#define MKVB54_STAR ((word)KW_MKVB54_STAR) /* MKVB54* */
/* MKVB54, MKVB53, MKVB52 and MKVB54*: "THE FOUR MKVBS ARE INDEXED -- THEIR
 * ORDER CANNOT BE CHANGED" (ADDRESS_OF(MKVB54) is the table's address). */
#define MKVB54_WORDS {KW_MKVB54, KW_MKVB53, KW_MKVB52, KW_MKVB54_STAR}

/* Its labels, in bank 07 (OANB in 04, SURFSTAR in 15), and the routines
 * it goes to in other files. */
#define AOTMARK      ((word)K_AOTMARK)
#define EXTVBCHK     ((word)K_EXTVBCHK)
#define MKVAC        ((word)K_MKVAC)
#define MKABORT      ((word)K_MKABORT)
#define KILLAOT      ((word)K_KILLAOT)
#define GETDAT       ((word)K_GETDAT)
#define ENTERDAT     ((word)K_ENTERDAT)
#define CODE7        ((word)K_CODE7)
#define COASCODE     ((word)K_COASCODE)
#define OPTAXIS      ((word)K_OPTAXIS)
#define YZCHK        ((word)K_YZCHK)
#define OANB         ((word)K_OANB)
#define SURFSTAR     ((word)K_SURFSTAR)
#define GETMKS       ((word)K_GETMKS)
#define PASTIT       ((word)K_PASTIT)
#define AVESTAR      ((word)K_AVESTAR)
#define AVEIT        ((word)K_AVEIT)
#define MKALARM      ((word)K_MKALARM)
#define FINDKEY      ((word)K_FINDKEY)
#define SOMEKEY      ((word)K_SOMEKEY)
#define XMKRUPT      ((word)K_XMKRUPT)
#define YMKRUPT      ((word)K_YMKRUPT)
#define VERIFYMK     ((word)K_VERIFYMK)
#define FIVE_MKALARM ((word)K_5MKALARM) /* 5MKALARM */
#define MKREJ        ((word)K_MKREJ)
#define REJALM       ((word)K_REJALM)
#define REJECT       ((word)K_REJECT)
#define SURFREJ      ((word)K_SURFREJ)
#define MARKTYPE     ((word)K_MARKTYPE)
#define SURFJOB      ((word)K_SURFJOB)
#define CHANGEVB     ((word)K_CHANGEVB)
#define DSPV6N79     ((word)K_DSPV6N79)
#define DESCBITS     ((word)K_DESCBITS)
#endif

#if COMANCHE055
/* SXTMARK's erasable words: MARKSTAT (its VAC area in bits 1-9; bit 10
 * set by a mark and cleared by a mark reject; bits 12-14 the marks still
 * wanted); MKNDX, the CDU DIFCHK checks; DSPTEM1, the register 1 word of
 * a display; CDUCHKWD, the delay before a mark's CDUs are checked (+0 or
 * less: no check). ADDRESS_OF gives the others: MKCDUX ... MKCDUT, the
 * CDUs at the mark, and MKT2T1, its time; MRKBUF1 and MRKBUF2, R21's
 * marks; MARKDOWN, the mark for the downlink. */
#define MKNDX    ERASABLE(K_MKNDX)
#define DSPTEM1  ERASABLE(K_DSPTEM1)
#define CDUCHKWD ERASABLE(K_CDUCHKWD)

/* Its constants: PRIO32, a job's priority (PRIO34 is also MARKSTAT's
 * bits 12-14); 74K (-3777: with the one a CCS takes off, one mark less in
 * bits 12-14); R1D1 (OCT 16); the verb and noun words V50N25 and VB51.
 * OCT37 is LOW5. */
#define PRIO32         ((word)KW_PRIO32)
#define SEVENTY_FOUR_K ((word)KW_74K) /* 74K */
#define R1D1           ((word)KW_R1D1)
#define V50N25         ((word)KW_V50N25)
#define VB51           ((word)KW_VB51)

/* Its labels, in bank 07 (MARKDIF, MKACPT and DIFCHK in 10). */
#define SXTMARK       ((word)K_SXTMARK)
#define MKABORT       ((word)K_MKABORT)
#define MARKOK        ((word)K_MARKOK)
#define MKRLEES       ((word)K_MKRLEES)
#define MARKRUPT      ((word)K_MARKRUPT)
#define KEYCALL       ((word)K_KEYCALL)
#define MARKCONT      ((word)K_MARKCONT)
#define MARKIT1       ((word)K_MARKIT1)
#define MARKET        ((word)K_MARKET)
#define MARKET3       ((word)K_MARKET3)
#define HUNDRED14_ALM ((word)K_114ALM) /* 114ALM */
#define MARK2         ((word)K_MARK2)
#define MKREJECT      ((word)K_MKREJECT)
#define MRKREJCT      ((word)K_MRKREJCT)
#define REJECT2       ((word)K_REJECT2)
#define REJECT3       ((word)K_REJECT3)
#define MKVB51        ((word)K_MKVB51)
#define TERMSXT       ((word)K_TERMSXT)
#define TERMP03       ((word)K_TERMP03)
#define ENTANSWR      ((word)K_ENTANSWR)
#define JAMIT         ((word)K_JAMIT)
#define MKVB5X        ((word)K_MKVB5X)
#define MKVB50        ((word)K_MKVB50)
#define MARKIT        ((word)K_MARKIT)
#define MARKDIF       ((word)K_MARKDIF)
#define MKACPT        ((word)K_MKACPT)
#define DIFCHK        ((word)K_DIFCHK)
#endif

/* ---- EXTENDED_VERBS, part 1: the front end ---- */

/* Its constant: OC24100, the bits of FLAGWRD4 that say a priority
 * display has the DSKY. Its labels, in bank 43 at the same addresses in
 * both missions (CHKPOOH the LM's; the CM's is later in its file); and
 * PINBALL's NVSUB and ENDEXT (= ENDMARK), which it calls with FALTON. The
 * fan's targets, VBZERO to VERB96, are written in its table
 * (src/extended_verbs/front.c) as K_<label>. */
#define OC24100  ((word)KW_OC24100)
#define GOEXTVB  ((word)K_GOEXTVB)
#define LST2FAN  ((word)K_LST2FAN)
#define TESTXACT ((word)K_TESTXACT)
#define SETXTACT ((word)K_SETXTACT)
#define XACTALM  ((word)K_XACTALM)
#define XACT0    ((word)K_XACT0)
#define ALM_END  ((word)K_ALM_END) /* ALM/END */
#define GOPIN    ((word)K_GOPIN)
#define CHKPOOH  ((word)K_CHKPOOH)
#define NVSUB    ((word)K_NVSUB)
#define ENDEXT   ((word)K_ENDEXT)

/* ---- EXTENDED_VERBS, part 2: the short verbs ---- */

/* Its erasable words are named by ADDRESS_OF: MODECADR, FLAGWRD0-9,
 * DAPBOOLS, SKEEP1-6, SMODE, CPHIX, T5LOC, DAPDATR1, MRKBUF1, LANDMARK.
 * Its constants: the job priorities, the flag bits it tests, and the
 * words it stores (LDNDUMPI, the REMADR of DNDUMPI; S+1 and S+ZERO, the
 * self-check's EQUALS for BIT1 and ZERO; SELFADRS, the ADRES of SELFCHK;
 * ADRS1, the ADRES of SKEEP1; VNCON, VN 0501). */
#define PRIO4       ((word)KW_PRIO4)
#define PRIO7       ((word)KW_PRIO7)
#define PRIO10      ((word)KW_PRIO10)
#define PRIO16      ((word)KW_PRIO16)
#define PRIO20      ((word)KW_PRIO20)
#define PRIO22      ((word)KW_PRIO22)
#define EBANK7      ((word)KW_EBANK7)
#define PINSUPBT    ((word)KW_PINSUPBT)
#define TRACKBIT    ((word)KW_TRACKBIT)
#define LDNDUMPI    ((word)KW_LDNDUMPI)
#define SELFADRS    ((word)KW_SELFADRS)
#define ADRS1       ((word)KW_ADRS1)
#define VNCON       ((word)KW_VNCON)
#define S_PLUS_1    ((word)KW_SP1)    /* S+1 */
#define S_PLUS_ZERO ((word)KW_SPZERO) /* S+ZERO */
#if LUMINARY099
#define PULSES       ((word)KW_PULSES)
#define RENDWBIT     ((word)KW_RENDWBIT)
#define BITS9_PLUS_7 ((word)KW_BITS9P7)  /* BITS9+7 */
static const word V82CON[2] = KW_V82CON; /* 2CADR V82CALL */
#else
#define R23BIT   ((word)KW_R23BIT)
#define R31FLBIT ((word)KW_R31FLBIT)
#define V94FLBIT ((word)KW_V94FLBIT)
#define LANDBANK ((word)KW_LANDBANK) /* ECADR LANDMARK */
static const word IDLECADR[2] = KW_IDLECADR; /* 2CADR T5IDLOC */
#endif

/* Its labels, in bank 43, and where it goes on to. */
#define CKMODCAD ((word)K_CKMODCAD)
#define GOLOADLV ((word)K_GOLOADLV)
#define VB64     ((word)K_VB64)
#define V82PERF  ((word)K_V82PERF)
#define V83PERF  ((word)K_V83PERF)
#define V89PERF  ((word)K_V89PERF)
#define V90PERF  ((word)K_V90PERF)
#define CREWMANU ((word)K_CREWMANU)
#define DNEDUMP  ((word)K_DNEDUMP)
#define LEMVEC   ((word)K_LEMVEC)
#define CSMVEC   ((word)K_CSMVEC)
#define SYSTEST  ((word)K_SYSTEST)
#define WMATRXNG ((word)K_WMATRXNG)
#define SHOWSUM  ((word)K_SHOWSUM)
#define SDISPLAY ((word)K_SDISPLAY)
#define NOKILL   ((word)K_NOKILL)
#define ENDSUMS  ((word)K_ENDSUMS)
#define ATTACHED ((word)K_ATTACHED)
#define VERB96   ((word)K_VERB96)
#define V67      ((word)K_V67)
#define FLASHOFF ((word)K_FLASHOFF) /* PINBALL's */
#define STSHOSUM ((word)K_STSHOSUM) /* the self-check's */
#define NXTBNK   ((word)K_NXTBNK)
#define SELFCHK  ((word)K_SELFCHK)
#define PRIOCHNG ((word)K_PRIOCHNG) /* the Executive's */
#if LUMINARY099
#define DAPATTER ((word)K_DAPATTER)
#define TOTATTER ((word)K_TOTATTER)
#define LRON     ((word)K_LRON)
#define LROFF    ((word)K_LROFF)
#define V47TXACT ((word)K_V47TXACT)
#define ALINTIME ((word)K_ALINTIME)
#define MINIMP   ((word)K_MINIMP)
#define NOMINIMP ((word)K_NOMINIMP)
#define TRMTRACK ((word)K_TRMTRACK)
#define NOUPDOWN ((word)K_NOUPDOWN)
#define UPDATOFF ((word)K_UPDATOFF)
#define DAPDISP  ((word)K_DAPDISP)
#define SNUFFOUT ((word)K_SNUFFOUT)
#define OUTSNUFF ((word)K_OUTSNUFF)
#define VERB85   ((word)K_VERB85)
#else
#define V60      ((word)K_V60)
#define V61      ((word)K_V61)
#define V62      ((word)K_V62)
#define V63      ((word)K_V63)
#define V92CONT  ((word)K_V92CONT)
#define EXDAPOFF ((word)K_EXDAPOFF)
#define CKOPTVB  ((word)K_CKOPTVB)
#define CHAZFOGC ((word)K_CHAZFOGC)
#define ENATMA   ((word)K_ENATMA)
#define STROKON  ((word)K_STROKON)
#define STABLISH ((word)K_STABLISH)
#define V85PERF  ((word)K_V85PERF)
#define GOTOR21  ((word)K_GOTOR21)
#define GOTOR23  ((word)K_GOTOR23)
#define REGR21   ((word)K_REGR21)
#define R22ALARM ((word)K_R22ALARM)
#define V86PERF  ((word)K_V86PERF)
#define LFTFLGON ((word)K_LFTFLGON)
#define SETPRFLG ((word)K_SETPRFLG)
#define RESETPRF ((word)K_RESETPRF)
#define VERB94   ((word)K_VERB94)
#define V52      ((word)K_V52)
#define SETSURF  ((word)K_SETSURF)
#define RESTSRF  ((word)K_RESTSRF)
#define CLEANOUT ((word)K_CLEANOUT) /* part 7's */
#endif

/* ---- EXTENDED_VERBS, part 3: alignment and radar designate ---- */

/* Its erasable words are named by ADDRESS_OF: NOUNREG, MODREG, PRIORITY,
 * FLAGWRD0-7, the LM's FLGWRD11, RADMODES, DESRET and OPTIONX, the CM's
 * OPTCADR, SWSAMPLE, OPTIND, SAC, PAC, DESOPTS, DESOPTT, THETAD and AK.
 * Its constants: the verb-noun codes it shows (VN), the nouns' difference
 * it tests, the flag bits and channel bits it tests and the ones it sets. */
#define VNLODCDU ((word)KW_VNLODCDU) /* VN 2522: flashing V25 N22 */
#define IMUCOARV ((word)KW_IMUCOARV) /* VN 4100: V41 */
#define OPTCOARV IMUCOARV            /* OPTCOARV EQUALS IMUCOARV */
#define VNLODGYR ((word)KW_VNLODGYR) /* VN 2593: flashing V25 N93 */
#define IMUFINEV ((word)KW_IMUFINEV) /* VN 4200: V42 */
#define LGYROBIN ((word)KW_LGYROBIN) /* ECADR OGC */
#if LUMINARY099
#define RRIMUDIF ((word)KW_RRIMUDIF) /* DEC -52 */
#define VNLDRCDU ((word)KW_VNLDRCDU) /* VN 2473: flashing V24 N73 */
#define OCT41000 ((word)KW_OCT41000)
#define ONE_SEC  ((word)KW_1SEC) /* 1SEC */
#define DEC22    ((word)KW_DEC22)
#define BIT13_14 ((word)KW_BIT13_14) /* BIT13-14 */
#define V43K     ((word)KW_V43K)     /* VN 4300: V43 */
#define V04N1272 ((word)KW_V04N1272) /* VN 412: V04 N12 */
#else
#define OPIMDIFF ((word)KW_OPIMDIFF) /* DEC -71 */
#define VNLD0CDU ((word)KW_VNLD0CDU) /* VN 2492: flashing V24 N92 */
#define OCTAL30  ((word)KW_OCTAL30)
#define OCT50    ((word)KW_OCT50)
#define EBANK5   ((word)KW_EBANK5)
#define GRRBKBIT ((word)KW_GRRBKBIT)
#endif

/* Its labels, in bank 43 (the LM's AURLOKON, NOLOKON and AURLKON1 in bank
 * 23), and PINBALL's TERMEXTV and ENDEXTVB (both EQUALS ENDEXT) and
 * BLANKET, where it goes on to. */
#define VBZERO   ((word)K_VBZERO)
#define VBCOARK  ((word)K_VBCOARK)
#define OP_INERT ((word)K_OP_INERT) /* OP/INERT */
#define IMUZEROK ((word)K_IMUZEROK)
#define IMUCOARK ((word)K_IMUCOARK)
#define ICORK2   ((word)K_ICORK2)
#define IMUFINEK ((word)K_IMUFINEK)
#define FINEK2   ((word)K_FINEK2)
#define IMUATTCK ((word)K_IMUATTCK)
#define TERMEXTV ((word)K_TERMEXTV)
#define ENDEXTVB ((word)K_ENDEXTVB)
#if LUMINARY099
#define RRZEROK  ((word)K_RRZEROK)
#define RWAITK   ((word)K_RWAITK)
#define LRPOS2K  ((word)K_LRPOS2K)
#define LRP2ALM  ((word)K_LRP2ALM)
#define RRDESNBK ((word)K_RRDESNBK)
#define RRDESK2  ((word)K_RRDESK2)
#define RRDESEND ((word)K_RRDESEND)
#define RDRUSECK ((word)K_RDRUSECK)
#define CHECKRR  ((word)K_CHECKRR)
#define CKRNDBIT ((word)K_CKRNDBIT)
#define CHECKP22 ((word)K_CHECKP22)
#define AURLOKON ((word)K_AURLOKON) /* bank 23 */
#define NOLOKON  ((word)K_NOLOKON)  /* bank 23 */
#define AURLKON1 ((word)K_AURLKON1) /* bank 23 */
#endif
#define BLANKET ((word)K_BLANKET)
#if !LUMINARY099
#define OPTCOARK ((word)K_OPTCOARK)
#define OPTC1    ((word)K_OPTC1)
#define TRMATTCK ((word)K_TRMATTCK)
#define ATTCK1   ((word)K_ATTCK1)
#define CKLFTBTS ((word)K_CKLFTBTS)
#define XACT0Q   ((word)K_XACT0Q)
#endif

/* ---- EXTENDED_VERBS, part 4: the LM's radar test verbs ---- */

#if LUMINARY099
/* Its erasable words are named by ADDRESS_OF: FLAGWRD3, RADMODES, OPTIONX,
 * DSPTEM1, and in E-bank 4 RSAMPDT, RTSTLOC, RFAILCNT, RTSTDEX, RTSTBASE
 * and RTSTMAX. Its constants: */
#define EBANK4         ((word)KW_EBANK4)
#define ONE_SEC_PLUS_1 ((word)KW_1SECP1)   /* 1SEC+1: DEC 101 */
#define TWO_SECS       ((word)KW_2SECS)    /* 2SECS */
#define MS250_PLUS_1   ((word)KW_250MSP1)  /* 250MS+1 (EQUALS CALLCODE) */
#define LRPOSCAL       ((word)KW_LRPOSCAL) /* OCT 444: RADMODES bits 9, 6 and 3 */
#define R04FLBIT       ((word)KW_R04FLBIT)
#define V04N12X        ((word)KW_V04N12X) /* VN 412: V04 N12 */
#define OCT201_R04     ((word)KW_201R04)  /* 201R04: OCT 00201 */
#define V50N25X        ((word)KW_V50N25X) /* VN 5025: V50 N25 */
#define V16N72         ((word)KW_V16N72)
#define V16N78         ((word)KW_V16N78)
#define V16N66         ((word)KW_V16N66)
#define V16N67         ((word)KW_V16N67)

/* Its labels, in bank 43. */
#define R77    ((word)K_R77)
#define R04    ((word)K_R04)
#define R04Z   ((word)K_R04Z)
#define R04A   ((word)K_R04A)
#define R04X   ((word)K_R04X)
#define R04B   ((word)K_R04B)
#define R04C   ((word)K_R04C)
#define R04RR  ((word)K_R04RR)
#define R04LR  ((word)K_R04LR)
#define R04Y   ((word)K_R04Y)
#define R04K   ((word)K_R04K)
#define R04L   ((word)K_R04L)
#define R04END ((word)K_R04END)
#define R77END ((word)K_R77END)
#endif

/* ---- EXTENDED_VERBS, part 5: the DAP data load (verb 48) ---- */

/* Its erasable words are named by ADDRESS_OF: DAPDATR1, DAPBOOLS,
 * FLGWRD10, FLAGWRD1, FLAGWRD6, CSMMASS, LEMMASS, MASS, RATEINDX, and in
 * E-bank 6 STIKSENS and -RATEDB (MINUS_RATEDB). Its constants: */
#if LUMINARY099
#define BOOLSMSK          ((word)KW_BOOLSMSK) /* OCT 13113: the DAPBOOLS bits shown */
#define DSPLYMSK          ((word)KW_DSPLYMSK) /* OCT 33113: and bit 14, the stage */
#define V01N46            ((word)KW_V01N46)
#define REVCNT            ((word)KW_REVCNT) /* bits 2 and 3: blank R2 and R3 */
#define CSMDOCKD          ((word)KW_CSMDOCKD)
#define ACC4OR2X          ((word)KW_ACC4OR2X)
#define RHCSCALE          ((word)KW_RHCSCALE)
#define NORMAL            ((word)KW_NORMAL)   /* DEC .660214 */
#define FINE              ((word)KW_FINE)     /* DEC .165054 */
#define ONE_TENTH         ((word)KW_1_10)     /* 1/10: DEC .1 */
#define MINUS_0_6_DEG_SEC ((word)KW_M0_6D_S)  /* -0.6D/S: DEC -218 */
#define MINUS_0_3_DEG_SEC ((word)KW_M0_3D_S)  /* -0.3D/S: DEC -109 */
#define V0647             ((word)KW_V0647)    /* VN 0647: V06 N47 */
#define MINLMD            ((word)KW_MINLMD)   /* minus the descent stage's least mass */
#define MINMINLM          ((word)KW_MINMINLM) /* minus the ascent stage's */
#define MINCSM            ((word)KW_MINCSM)
#define V06N48            ((word)KW_V06N48)
#define V50N48            ((word)KW_V50N48)

/* Its labels: DAPDATA1 to DPDAT1 in bank 20, STIKLOAD to TRIMDONE in bank
 * 01. */
#define DAPDATA1 ((word)K_DAPDATA1)
#define CHKDATA1 ((word)K_CHKDATA1)
#define MSKDATR1 ((word)K_MSKDATR1)
#define FORCEONE ((word)K_FORCEONE)
#define DPDAT1   ((word)K_DPDAT1)
#define STIKLOAD ((word)K_STIKLOAD)
#define DAPDATA2 ((word)K_DAPDATA2)
#define ENDR03   ((word)K_ENDR03)
#define DAPDAT2  ((word)K_DAPDAT2)
#define LEMALONE ((word)K_LEMALONE)
#define DAPDATA3 ((word)K_DAPDATA3)
#define DPDAT3   ((word)K_DPDAT3)
#define TRIMDONE ((word)K_TRIMDONE)
#else
#define V04N46   ((word)KW_V04N46)
#define V06N47   ((word)KW_V06N47)
#define V0648    ((word)KW_V0648) /* VN 0648: V06 N48 */

/* Its labels: DAPDISP (the LM's is part 2's) in bank 43, the rest in
 * bank 42. */
#define DAPDISP  ((word)K_DAPDISP)
#define DAPDISP1 ((word)K_DAPDISP1)
#define DONOUN46 ((word)K_DONOUN46)
#define MAXOUT   ((word)K_MAXOUT)
#define DONOUN47 ((word)K_DONOUN47)
#define DONOUN48 ((word)K_DONOUN48)
#define MAXIN    ((word)K_MAXIN)
#endif

/* ---- EXTENDED_VERBS, part 6: the clock update (verb 55) and ATTCK2 ---- */

/* Its erasable words are named by ADDRESS_OF: MPAC, TIME2, DSPTEM2, and
 * the LM's THETAD and CDUXCMD. Its constants and labels: */
#define DEC23    ((word)KW_DEC23)   /* DEC 23: verb 23 */
#define VNLODDT  ((word)KW_VNLODDT) /* VN 2524: V25 N24 */
#define UPDATIME ((word)K_UPDATIME)
#define UPDTMEND ((word)K_UPDTMEND)
#if LUMINARY099
#define ATTSCALE     ((word)KW_ATTSCALE) /* DEC 0.1 */
#define BITS13_14_15 ((word)KW_13_14_15) /* 13,14,15: channel 14's CDU drives */
#define R33          ((word)K_R33)       /* bank 42, with UPDATIME and ATTCK2 */
#define ATTCK2       ((word)K_ATTCK2)
#else
#define ALINTIME ((word)K_ALINTIME) /* bank 43, with UPDATIME */
#endif

/* ---- EXTENDED_VERBS, part 7: tracking and state vector verbs ---- */

/* Its erasable words are named by ADDRESS_OF: FLAGWRD0, FLAGWRD1, MPAC,
 * CDUT, EXTVBACT, FIXLOC, QPRET, and the CM's OPTIND (E-bank 6). Its
 * constants and labels: */
#define OCT51       ((word)KW_OCT51)   /* GENTRAN's count: 42 words */
#define TCPINAD     ((word)KW_TCPINAD) /* CADR TCPIN */
#define TWO_PHSCHNG ((word)K_2PHSCHNG) /* 2PHSCHNG */
#define VERB69      ((word)K_VERB69)
#define ATTACHIT    ((word)K_ATTACHIT)
#define TCPIN       ((word)K_TCPIN) /* the interpretive RTB INTWAKE goes on at, bank 43 */
#if LUMINARY099
#define V16N56   ((word)KW_V16N56)
#define DSPRRLOS ((word)K_DSPRRLOS) /* bank 40, with RRLOSDSP */
#define RRLOSDSP ((word)K_RRLOSDSP)
#define TRMTRAK1 ((word)K_TRMTRAK1) /* bank 42 */
#else
#define TRACKTRM ((word)K_TRACKTRM)
#define SETVHFLG ((word)K_SETVHFLG)
#define RESETVHF ((word)K_RESETVHF)
#define TACHEXIT ((word)K_TACHEXIT)
#define LMTOCMSV ((word)K_LMTOCMSV)
#define LMTOCM   ((word)K_LMTOCM)
#endif

/* ---- The noun table readers (PINBALL_NOUN_TABLES.agc, bank 42) ---- */

/* Their erasable words are named by ADDRESS_OF: NNADTEM, NNTYPTEM, MIXBR,
 * RUTMXTEM, IDAD1TEM-IDAD3TEM, SFTEMP1, SFTEMP2; and the tables' first
 * words (NNADTAB, NNTYPTAB, SFINTAB, SFOUTAB, IDADDTAB, RUTMXTAB) by their
 * addresses in the bank, ADDRESS_OF(NNADTAB), and by their physical
 * addresses, P_NNADTAB. NOUNREG: the noun on the DSKY, which INDEX reads. */
#define NOUNREG  ERASABLE(K_NOUNREG)
#define LODNNTAB ((word)K_LODNNTAB)
#define LODMIXNN ((word)K_LODMIXNN)
#define LODNLV   ((word)K_LODNLV)
#define GTSFOUT  ((word)K_GTSFOUT)
#define SFCOM    ((word)K_SFCOM)
#define GTSFIN   ((word)K_GTSFIN)
#define MIXCON   ((word)KW_OCT50) /* MIXCON = OCT50: "(DEC 40)", the first mixed noun */

/* CDULOGIC, an RTB routine of both missions (RTB_OP_CODES.agc). */
#define CDULOGIC ((word)K_CDULOGIC)

/* ---- The geometry (LEM_GEOMETRY.agc, CSM_GEOMETRY.agc) ---- */

#if LUMINARY099
/* RRANGLES's basic code follows its EXIT, at RRANGLES + 042. Its erasable
 * words are named by ADDRESS_OF: RADMODES, MODEA and MODEB. */
#define RRANGLES ((word)K_RRANGLES)
#define ANTENBIT BIT12 /* ANTENBIT = BIT12: RADMODES's bit for the radar's mode 2 */
#else
/* The basic code among CSM_GEOMETRY's interpretive code: SXTLOGIC (an RTB
 * routine) and LOG's, around its TC POLY. */
#define SXTLOGIC       ((word)K_SXTLOGIC)
#define LOG            ((word)K_LOG)
#define TEN_DEGS_MINUS ((word)KW_10DEGSM) /* 10DEGS-: DEC 3600 */
static const word CLOG2_32[2] = KW_CLOG2_32; /* CLOG2/32: 2DEC .0216608494 */

/* TRANSP1 and TRANSP2, RTB routines (RT8_OP_CODES.agc), and the E-banks
 * of the matrices they transpose: XNB's and XNB1's (ECADRs). */
#define TRANSP1        ((word)K_TRANSP1)
#define TRANSP2        ((word)K_TRANSP2)
#define XNBEB          ((word)KW_XNBEB)
#define XNB1EB         ((word)KW_XNB1EB)
#endif

/* TCDANZIG = ENDT(X) (TIME_OF_FREE_FALL.agc), the basic TC DANZIG a BOVB
 * goes to only to reset OVFIND. */
#define TCDANZIG ((word)K_TCDANZIG)

/* ---- IMU compensation (IMU_COMPENSATION_PACKAGE.agc, bank 06) ---- */

/* Its labels. LGCOMP labels a word, ECADR GCOMP: the E-bank and address
 * of GCOMP, which CAF LGCOMP reads as a constant (LGCOMP_WORD). */
#define ONE_PIPA    ((word)K_1_PIPA)  /* 1/PIPA */
#define ONE_PIPA1   ((word)K_1_PIPA1) /* 1/PIPA1 */
#define IRIGCOMP    ((word)K_IRIGCOMP)
#define IRIG1       ((word)K_IRIG1)
#define IRIGX       ((word)K_IRIGX)
#define IRIGY       ((word)K_IRIGY)
#define IRIGZ       ((word)K_IRIGZ)
#define ONE_GYRO    ((word)K_1_GYRO) /* 1/GYRO */
#define LGCOMP      ((word)K_LGCOMP)
#define LGCOMP_WORD ((word)KW_LGCOMP)
#define GCOMP1      ((word)K_GCOMP1)
#define NBDONLY     ((word)K_NBDONLY)
#define NBD2        ((word)K_NBD2)
#define NBD3        ((word)K_NBD3)
#define LASTBIAS    ((word)K_LASTBIAS)
#if LUMINARY099
#define GCOMPZER ((word)K_GCOMPZER)
#define COMPCHK  ((word)KW_COMPCHK) /* DEC -1 */
#endif

/* Its constants. */
#define PRIO21 ((word)KW_PRIO21)
#define PRIO31 ((word)KW_PRIO31)
#define NEG1   ((word)KW_NEG1)

/* Its erasable words are named by ADDRESS_OF: GCOMP (three double words,
 * the gyro commands of X, Y and Z) and GCOMPSW, the compensation
 * coefficients NBDX..NBDZ, ADIAX..ADIAZ and ADSRAX..ADSRAZ, PIPABIAS and
 * PIPASCF (pairs, by axis), all in E-bank 3; DELVX..DELVZ, the PIPA counts;
 * 1/PIPADT (ONE_PIPADT), the time of the last compensation; PIPTIME (LM)
 * and PIPTIME1 (CM), SERVICER's time of the last PIPA reading. */
#define K_ONE_PIPADT K_1_PIPADT

#if LUMINARY099
/* ---- R63 (LM only) ---- */

/* Its labels in bank 26 and its two verb-noun words. Its erasable
 * words are named by ADDRESS_OF: OPTIONX (+1 the axis chosen), TIME2. */
#define V89CALL  ((word)K_V89CALL)
#define V89RECL  ((word)K_V89RECL)
#define ALINEX   ((word)K_ALINEX)
#define V89CALL1 ((word)K_V89CALL1)
#define ALINEZ   ((word)K_ALINEZ)
#define VB04N12  ((word)KW_VB04N12) /* VN 412 */
#define VB06N18  ((word)KW_VB06N18) /* VN 0618 */

#endif

/* ---- ATTITUDE_MANEUVER_ROUTINE (LM), ANGLFIND (CM) ---- */

/* Their labels in bank 22 the basic code enters or leaves from. */
#define KALCMAN3 ((word)K_KALCMAN3)
#define TRNSPSPD ((word)K_TRNSPSPD)
#define READCDUK ((word)K_READCDUK)
#define NOGO     ((word)K_NOGO)
#if LUMINARY099
#define TOOBADF ((word)K_TOOBADF)
#define TOOBADI ((word)K_TOOBADI)
#else
#define SECAD  ((word)K_SECAD)
#define NOGOM2 ((word)K_NOGOM2)
#endif

/* ---- KALCMANU_STEERING ---- */

/* Its labels (LM: all in bank 22; CM: banks 20, 21 and 22, and STOPRATE
 * in fixed-fixed memory) and its constants. Its erasable words are named
 * by ADDRESS_OF. */
#define NEWDELHI ((word)K_NEWDELHI)
#define NEWANGL  ((word)K_NEWANGL)
#define TMANUCHK ((word)K_TMANUCHK)
#define MANUSTAL ((word)K_MANUSTAL)
#define MANUSTAT ((word)K_MANUSTAT)
#define CONTMANU ((word)K_CONTMANU)
#define MANUCALL ((word)K_MANUCALL)
#define UPDTCALL ((word)K_UPDTCALL)
#define MANUSTOP ((word)K_MANUSTOP)
#define ENDROLL  ((word)K_ENDROLL)
#define PRIO26   ((word)KW_PRIO26)
#if LUMINARY099
#define GOODMANU   ((word)K_GOODMANU)
#define DT_TAU     ((word)KW_DT_TAU)   /* DEC .1 */
#define ONESEK     ((word)K_ONESEK)    /* 2DEC: 0, 100 (one second) */
#define ONESEK_LOW ((word)KW_ONESEKP1) /* ONESEK +1: DEC 100 */
#define BIASCALE   ((word)KW_BIASCALE) /* OCT 75777: -1/16 */
#else
#define ENDMANU  ((word)K_ENDMANU)
#define STOPYZ   ((word)K_STOPYZ)
#define ZEROEROR ((word)K_ZEROEROR)
#define LOADCDUD ((word)K_LOADCDUD)
#define LOADYZ   ((word)K_LOADYZ)
#define QUADROT  ((word)K_QUADROT) /* 2DEC .1 first (MP QUADROT: a tenth) */
#define ONE_SEC  ((word)KW_1SEC)   /* 1SEC: DEC 100 */
#endif

/* 1STO2S, V1STO2S, 2V1STO2S and their 1TO2SUB, LOADTIME, SGNAGREE,
 * TPMODE, DPMODE and NORMUNX1, RTB routines (RTB_OP_CODES.agc). */
#define LOADTIME    ((word)K_LOADTIME)
#define ONE_STO2S   ((word)K_1STO2S)
#define TWO_V1STO2S ((word)K_2V1STO2S)
#define ONE_TO2SUB  ((word)K_1TO2SUB)
#define V1STO2S     ((word)K_V1STO2S)
#define TPMODE      ((word)K_TPMODE)
#define DPMODE      ((word)K_DPMODE)
#define NORMUNX1    ((word)K_NORMUNX1)
#define NORMUNIT    ((word)K_NORMUNIT)
#define SGNAGREE    ((word)K_SGNAGREE)
#define OFFTUNIT    ((word)K_OFFTUNIT)

/* ---- SYSTEM_TEST_STANDARD_LEAD_INS ---- */

/* Its entries: in bank 4 (the LM's, and the CM's E/JOBWAK) and bank 13
 * (the CM's others). EBUF2 and EBUF2 +1, in unswitched erasable, keep an
 * E/BKCALL's or E/CALL's CADR and return. */
#define E_BKCALL ((word)K_E_BKCALL)
#define E_SWITCH ((word)K_E_SWITCH)
#define E_CALL   ((word)K_E_CALL)
#define E_JOBWAK ((word)K_E_JOBWAK)
#define EBUF2(n) ERASABLE(K_EBUF2 + (n))
#if COMANCHE055
/* The CM's system-test entry words, at 33,2000-2002, and where they go. */
#define COMPVER  ((word)K_COMPVER)
#define GTSCPSS1 ((word)K_GTSCPSS1)
#define REDO     ((word)K_REDO)
#define GCOMPVER ((word)K_GCOMPVER)
#define GTSCPSS  ((word)K_GTSCPSS)
#define NEWMODEX ((word)K_NEWMODEX)
#endif

/* ---- IMU_PERFORMANCE_TEST_2 and IMU_PERFORMANCE_TESTS_4 (LM P07) ---- */

#if LUMINARY099
/* Their labels in bank 37, FINETIME in fixed-fixed, and the routines of
 * other files they call. Their erasable words (EBANK 5: POSITON, NDXCTR,
 * QPLACE, ZERONDX and the rest) are named by ADDRESS_OF. */
#define GEOIMUTT    ((word)K_GEOIMUTT)
#define IMUBACK     ((word)K_IMUBACK)
#define NBPOSPL     ((word)K_NBPOSPL)
#define GUESS       ((word)K_GUESS)
#define LATAZCHK    ((word)K_LATAZCHK)
#define POSGMBL     ((word)K_POSGMBL)
#define GOESTIMS    ((word)K_GOESTIMS)
#define TORQUE      ((word)K_TORQUE)
#define PIPACHK     ((word)K_PIPACHK)
#define PIPATASK    ((word)K_PIPATASK)
#define STARTPIP    ((word)K_STARTPIP)
#define PIPJOBB     ((word)K_PIPJOBB)
#define AINGOTN     ((word)K_AINGOTN)
#define VERTDRFT    ((word)K_VERTDRFT)
#define PON2        ((word)K_PON2)
#define PON4        ((word)K_PON4)
#define PON         ((word)K_PON)
#define GUESS1      ((word)K_GUESS1)
#define VALMIS      ((word)K_VALMIS)
#define ENDTEST1    ((word)K_ENDTEST1)
#define OVERFFIX    ((word)K_OVERFFIX)
#define COAALIGN    ((word)K_COAALIGN)
#define ALIGNCOA    ((word)K_ALIGNCOA)
#define IMUSLLLG    ((word)K_IMUSLLLG)
#define FINIMUDD    ((word)K_FINIMUDD)
#define IMUZERR     ((word)K_IMUZERR)
#define CHECKG      ((word)K_CHECKG)
#define CHECKG1     ((word)K_CHECKG1)
#define ENDCHKG     ((word)K_ENDCHKG)
#define ZEROING     ((word)K_ZEROING)
#define ZEROING1    ((word)K_ZEROING1)
#define ERTHRVSE    ((word)K_ERTHRVSE)
#define EARTHR      ((word)K_EARTHR)
#define ERTHR       ((word)K_ERTHR)
#define EARTHR_STAR ((word)K_EARTHR_STAR) /* EARTHR* */
#define SHOW        ((word)K_SHOW)
#define SHOW1       ((word)K_SHOW1)
#define FINETIME    ((word)K_FINETIME)
#define ESTIMS      ((word)K_ESTIMS)
#define VERTSKIP    ((word)K_VERTSKIP)
#define ALLOOP      ((word)K_ALLOOP)
#define SPECSTS     ((word)K_SPECSTS)
#define ALFLT       ((word)K_ALFLT)
#define NORMLOP     ((word)K_NORMLOP)
#define ALCGKK      ((word)K_ALCGKK)
#define ALKCG       ((word)K_ALKCG)
#define ALKCG2      ((word)K_ALKCG2)
#define ALFLT3      ((word)K_ALFLT3)
#define DELMLP      ((word)K_DELMLP)
#define ALILP       ((word)K_ALILP)
#define ALKLP       ((word)K_ALKLP)
#define LOOSE       ((word)K_LOOSE)
#define BOOP        ((word)K_BOOP)
#define PERFERAS    ((word)K_PERFERAS)
#define SETUPER1    ((word)K_SETUPER1)
#define GEOSTRT4    ((word)K_GEOSTRT4)
#define SLEEPIE     ((word)K_SLEEPIE)
#define SOMEERRR    ((word)K_SOMEERRR)
#define SOMERR2     ((word)K_SOMERR2)
#define REDO        ((word)K_REDO)
#define NEWMODEX    ((word)K_NEWMODEX)
#define CHANG1      ((word)K_CHANG1)
/* ATIGINC: the erasable program (EBANK 7) PERFERAS calls. */
#define ATIGINC ((word)K_ATIGINC)
/* The scaler channels FINETIME reads. */
#define LOSCALAR ((unsigned)K_LOSCALAR)
#define HISCALAR ((unsigned)K_HISCALAR)
/* Their constants. */
#define DEC17    ((word)KW_DEC17) /* = ND1 */
#define DEC58    ((word)KW_DEC58)
#define VN0641   ((word)KW_VN0641)   /* VN 0641 */
#define VB06N98  ((word)KW_VB06N98)  /* VN 0698 */
#define DEC3990  ((word)KW_3990DEC)  /* 3990DEC */
#define XNBADR   ((word)KW_XNBADR)   /* GENADR XNB */
#define ESTICADR ((word)KW_ESTICADR) /* CADR ESTIMS */
#define DEC77    ((word)KW_77DECML)  /* 77DECML */
#define ALXXXZ   ((word)KW_ALXXXZ)   /* GENADR ALX1S -1 */
#define OCT1601  ((word)KW_OCT1601)
#define OGCPL    ((word)KW_OGCPL) /* ECADR OGC */
#define EBANK5   ((word)KW_EBANK5)
#endif

/* ---- IMU_CALIBRATION_AND_ALIGNMENT (the CM's) ---- */

#if COMANCHE055
/* Its labels in bank 33, FINETIME in fixed-fixed, and the routines of
 * other files they call. Its erasable words (EBANK 5: POSITON, NDXCTR,
 * QPLACE, ZERONDX1 and the rest) are named by ADDRESS_OF. */
#define IMUTEST     ((word)K_IMUTEST)
#define GEOIMUTT    ((word)K_GEOIMUTT)
#define IMUBACK     ((word)K_IMUBACK)
#define NBPOSPL     ((word)K_NBPOSPL)
#define GUESS       ((word)K_GUESS)
#define GOESTIMS    ((word)K_GOESTIMS)
#define TORQUE      ((word)K_TORQUE)
#define PIPACHK     ((word)K_PIPACHK)
#define PIPATASK    ((word)K_PIPATASK)
#define STARTPIP    ((word)K_STARTPIP)
#define PIPJOBB     ((word)K_PIPJOBB)
#define AINGOTN     ((word)K_AINGOTN)
#define VERTDRFT    ((word)K_VERTDRFT)
#define PONG        ((word)K_PONG)
#define GUESS1      ((word)K_GUESS1)
#define VALMIS      ((word)K_VALMIS)
#define ENDTEST1    ((word)K_ENDTEST1)
#define COAALIGN    ((word)K_COAALIGN)
#define IMUSTLLG    ((word)K_IMUSTLLG)
#define CHECKG      ((word)K_CHECKG)
#define CHECKG1     ((word)K_CHECKG1)
#define ZEROING     ((word)K_ZEROING)
#define OVERFFIX    ((word)K_OVERFFIX)
#define ERTHRVSE    ((word)K_ERTHRVSE)
#define EARTHR      ((word)K_EARTHR)
#define ERTHR       ((word)K_ERTHR)
#define EARTHR_STAR ((word)K_EARTHR_STAR) /* EARTHR* */
#define PROUT       ((word)K_PROUT)
#define DELMLP      ((word)K_DELMLP)
#define ALKLP       ((word)K_ALKLP)
#define BOOP        ((word)K_BOOP)
#define SHOW1       ((word)K_SHOW1)
#define P11OUT      ((word)K_P11OUT)
#define FINETIME    ((word)K_FINETIME)
#define ESTIMS      ((word)K_ESTIMS)
#define RSTGTS1     ((word)K_RSTGTS1)
#define SLEEPIE     ((word)K_SLEEPIE)
#define WTLISTNT    ((word)K_WTLISTNT)
#define WTGTSMPL    ((word)K_WTGTSMPL)
#define RIGHTGTS    ((word)K_RIGHTGTS)
#define ALLOOP      ((word)K_ALLOOP)
#define ALLOOP1     ((word)K_ALLOOP1)
#define SPECSTS     ((word)K_SPECSTS)
#define ALFLT       ((word)K_ALFLT)
#define ALFLT1      ((word)K_ALFLT1)
#define NORMLOP     ((word)K_NORMLOP)
#define SOMEERRR    ((word)K_SOMEERRR)
#define SOMERR2     ((word)K_SOMERR2)
#define STOREDTA    ((word)K_STOREDTA)
#define ANNNNNN     ((word)K_ANNNNNN)
#define CHKCOMED    ((word)K_CHKCOMED)
#define PRELTERM    ((word)K_PRELTERM)
#define SETNBPOS    ((word)K_SETNBPOS)
#define CHANG1      ((word)K_CHANG1)
#define PERFERAS    ((word)K_PERFERAS)
#define SETUPER1    ((word)K_SETUPER1)
#define ONCEMORE    ((word)K_ONCEMORE)
#define TORQINCH    ((word)K_TORQINCH)
#define SETUPGC     ((word)K_SETUPGC)
#define POSN17C     ((word)K_POSN17C)
#define ALWAYSG     ((word)K_ALWAYSG)
#define ENDGTSAL    ((word)K_ENDGTSAL)
#define LASTGTS     ((word)K_LASTGTS)
#define RESTAIER    ((word)K_RESTAIER)
#define RESTEST1    ((word)K_RESTEST1)
#define RESTEST3    ((word)K_RESTEST3)
#define RESCHNG     ((word)K_RESCHNG)
#define SPITGYRO    ((word)K_SPITGYRO)
#define NOAZCHGE    ((word)K_NOAZCHGE)
#define NOCHORLD    ((word)K_NOCHORLD)
#define LATAZCHK    ((word)K_LATAZCHK)
#define OPTDATA     ((word)K_OPTDATA)
#define TAR_EREF    ((word)K_TAR_EREF)
#define CONTIN33    ((word)K_CONTIN33)
#define NEXTBNKS    ((word)K_NEXTBNKS)
#define NEXBNKSS    ((word)K_NEXBNKSS)
#define GCOMP4      ((word)K_GCOMP4)
#define GCOMP5      ((word)K_GCOMP5)
#define GTSOPTCS    ((word)K_GTSOPTCS)
#define TARGDRVE    ((word)K_TARGDRVE)
#define RETARG      ((word)K_RETARG)
#define RETARG1     ((word)K_RETARG1)
#define LITTLSUB    ((word)K_LITTLSUB)
#define AZMTHCG1    ((word)K_AZMTHCG1)
/* LAT(SPL): the erasable program (EBANK 7) PERFERAS runs. */
#define LAT_SPL ((word)K_LAT_SPL)
/* The scaler channels FINETIME reads. */
#define LOSCALAR ((unsigned)K_LOSCALAR)
#define HISCALAR ((unsigned)K_HISCALAR)
/* Their constants. */
#define TESTTIME ((word)KW_TESTTIME)
#define ONE_SECX ((word)KW_1SEC)    /* 1SECX = 1SEC */
#define OC14400  ((word)KW_OC14400)
#define DEC17    ((word)KW_DEC17)   /* = ND1 */
#define DEC57    ((word)KW_VD1)     /* DEC57 = VD1 */
#define DEC19    ((word)KW_VD1)     /* 19DECML = VD1 */
#define DEC25    ((word)KW_OCT31)   /* 25DECML EQUALS OCT31 */
#define DEC77    ((word)KW_77DECML) /* 77DECML */
#define DEC3990  ((word)KW_3990DEC) /* 3990DEC = OMEG/MS */
#define XNBADR   ((word)KW_XNBADR)  /* GENADR XNB */
#define ESTICADR ((word)KW_ESTICADR) /* CADR ESTIMS */
#define ALXXXZ   ((word)KW_ALXXXZ)
#define VB06N98  ((word)KW_VB06N98) /* VN 0698 */
#define GLOKFBIT ((word)KW_GLOKFBIT)
#define EBANK7     ((word)KW_EBANK7)
#define OGCPL      ((word)KW_OGCPL) /* ECADR OGC */
#define XSMADR     ((word)KW_XSMADR) /* GENADR XSM */
#define ERCOMPPL   ((word)KW_ERCOMPPL) /* ECADR ERCOMP */
#define ONE_PIPAGT ((word)KW_1_PIPAGT) /* 1/PIPAGT */
#define TRM03BIT   ((word)KW_TRM03BIT) /* = BIT4 */
#define V05N30E    ((word)KW_V05N30E) /* VN 0530 */
#define VN0641     ((word)KW_VN0641) /* VN 0641 */
#define VNG0641    ((word)KW_VNG0641) /* VN 0641 */
#define V06N93S    ((word)KW_V06N93S) /* VN 0693 */
#define VN0629     ((word)KW_VN0629) /* VN 0629 */

/* ---- P11 (the CM's) ---- */
#define P11       ((word)K_P11)
#define REP11A    ((word)K_REP11A)
#define REP11     ((word)K_REP11)
#define ENDREP11  ((word)K_ENDREP11)
#define MATRXJOB  ((word)K_MATRXJOB)
#define VHHDOT    ((word)K_VHHDOT)
#define ATERTASK  ((word)K_ATERTASK)
#define GETDOWN   ((word)K_GETDOWN)
#define ATERJOB   ((word)K_ATERJOB)
#define ATTDISP   ((word)K_ATTDISP)
#define ATTDISP2  ((word)K_ATTDISP2)
#define STEERSAT  ((word)K_STEERSAT)
#define SATOUT    ((word)K_SATOUT)
#define ATERSET   ((word)K_ATERSET)
#define AKLOAD    ((word)K_AKLOAD)
#define STRSAT    ((word)K_STRSAT)
#define ATRESET   ((word)K_ATRESET)
#define TAKEON    ((word)K_TAKEON)
#define DELSTOR   ((word)K_DELSTOR)
#define RLTST     ((word)K_RLTST)
#define NOPOLY    ((word)K_NOPOLY)
#define RESCALES  ((word)K_RESCALES)
#define SATSTKON  ((word)K_SATSTKON)
#define REDOSAT   ((word)K_REDOSAT)
#define SATSTICK  ((word)K_SATSTICK)
#define S11_1     ((word)K_S11_1) /* S11.1 */
#define ATTDISP1  ((word)K_ATTDISP1) /* (bank 37) */
#define ATTDISPR  ((word)K_ATTDISPR)
#define NOPOLYM   ((word)K_NOPOLYM)
#define MBDYTCTL  ((word)K_MBDYTCTL) /* the body to control axes matrix (bank 34) */
#define POWRSERS  ((word)K_POWRSERS) /* (the LM's are its own) */
/* Its constants. */
#define EBDVCNT      ((word)KW_EBDVCNT)  /* ECADR DVCNTR */
#define EBQPLACE     ((word)KW_EBQPLACE) /* ECADR QPLACES */
#define P11XIT       ((word)KW_P11XIT)   /* GENADR P11OUT */
#define COEFPOLY     ((word)KW_COEFPOLY) /* ADRES POLYLOC */
#define V06N62       ((word)KW_V06N62)   /* VN 0662 */
#define PRIO1        ((word)KW_PRIO1)    /* = BIT10 */
#define STIKBITS     ((word)KW_STIKBITS) /* OCT 00077 */
#define HUNDRED_MST5 ((word)KW_100MST5)  /* 100MST5: DEC 16374 */
#define POWDNCOD     THREE               /* POWDNCOD EQUALS THREE */
#define REVCNT       ((word)KW_REVCNT)   /* OCT 6 */
static const word REP11S[2] = KW_REP11S;     /* 2OCT 7776600011 */
static const word REP11SA[2] = KW_REP11SA;   /* 2OCT 7776400013 */
static const word P11SCADR[2] = KW_P11SCADR; /* 2CADR VHHDOT */
static const word TWO_REDOSAT[2] = KW_2REDOSAT; /* 2REDOSAT: 2CADR REDOSAT */
static const word TWO_SATSTCK[2] = KW_2SATSTCK; /* 2SATSTCK: 2CADR SATSTICK */
#endif

/* ---- PINBALL_GAME_BUTTONS_AND_LIGHTS, part 1: the keys ---- */

/* Its labels in bank 40 and fixed-fixed, both missions, and the routines
 * of the file's later parts the keys go to. Its erasable words (DSPLOCK,
 * CADRSTOR, MPAC, CHAR, DSPCOUNT, DECBRNCH, INREL, CODE, COUNT, VERBREG,
 * XREGLP, CLPASS, REQRET, ENTRET, SGNON, SGNOFF, MIXTEMP, DSEXIT, DSREL,
 * DSMAG, DSPTAB, NOUT) are named by ADDRESS_OF, and so are the tables an
 * INDEX reads (INRELTAB, RELTAB, CRITCON, SGNTAB, SINBLANK, DOUBLK, DSMSK,
 * and R1D1 and BIT7 as the first words of theirs). */
#define CHARIN           ((word)K_CHARIN)
#define CHARIN2          ((word)K_CHARIN2)
#define ENTERJMP         ((word)K_ENTERJMP)
#define EIGHTY_NINE_TEST ((word)K_89TEST) /* 89TEST */
#define NUM              ((word)K_NUM)
#define DECTOBIN         ((word)K_DECTOBIN)
#define ENDNMTST         ((word)K_ENDNMTST)
#define ENDNUM           ((word)K_ENDNUM)
#define ENDALL           ((word)K_ENDALL)
#define DECEND           ((word)K_DECEND)
#define PLUS_DECSGN      ((word)K_PDECSGN) /* +DECSGN */
#define MORNUM           ((word)K_MORNUM)
#define GETINREL         ((word)K_GETINREL)
#define VERB             ((word)K_VERB)
#define NVCOM            ((word)K_NVCOM)
#define NOUN             ((word)K_NOUN)
#define NEGSGN           ((word)K_NEGSGN)
#define BOTHSGN          ((word)K_BOTHSGN)
#define FIXCLPAS         ((word)K_FIXCLPAS)
#define POSGN            ((word)K_POSGN)
#define SGNCOM           ((word)K_SGNCOM)
#define PLUS_ON          ((word)(K_POSGN + 4u))  /* +ON: the word after POSGN's TC BOTHSGN */
#define MINUS_ON         ((word)(K_SGNCOM + 9u)) /* -ON: the word after SGNCOM's TC L */
#define SIGNTEST         ((word)K_SIGNTEST)
#define SGNTST1          ((word)K_SGNTST1)
#define CLEAR            ((word)K_CLEAR)
#define CLPASHI          ((word)K_CLPASHI)
#define CLEAR1           ((word)K_CLEAR1)
#define CLR5             ((word)K_CLR5)
#define LEGALTST         ((word)K_LEGALTST)
#define FIVE_BLANK       ((word)K_5BLANK)  /* 5BLANK */
#define FIVE_BLANK1      ((word)K_5BLANK1) /* 5BLANK1 */
#define TWO_BLANK        ((word)K_2BLANK)  /* 2BLANK */
#define DSPIN            ((word)K_DSPIN)
#define DSPIN1           ((word)K_DSPIN1)
#define DFRNT            ((word)K_DFRNT)
#define DSLV             ((word)K_DSLV)
#define ELEVEN_DSPIN     ((word)K_11DSPIN) /* 11DSPIN */
#define SLEFT5           ((word)K_SLEFT5)
#define RELDSPON         ((word)K_RELDSPON)
#define CHARALRM         ((word)K_CHARALRM)
#define ERROR            ((word)K_ERROR)
#define VBRELDSP         ((word)K_VBRELDSP)
#define UPDATVB          ((word)K_UPDATVB)
#define ENTER            ((word)K_ENTER)
/* 21/22REG: where CHARIN keeps DSPLOCK's old word for ERROR LIGHT RESET. */
#define K_TWENTY_ONE_22REG K_21_22REG
/* Its constants. */
#define ELRCODE1 ((word)KW_ELRCODE1) /* OCT 22: the ERROR LIGHT RESET key */
#define ND1      ((word)KW_ND1)      /* OCT 21: the noun's left digit */
#define R1D1     ((word)KW_R1D1)     /* OCT 16 */
#define R2D1     ((word)KW_R2D1)     /* OCT 11 */
#define R3D1     ((word)KW_R3D1)     /* OCT 4 */
#define ENDINST  ((word)KW_ENDINST)  /* TC ENDOFJOB */
#define BLANKCON ((word)KW_BLANKCON) /* OCT 4000 */
#define BRNCHCON ((word)KW_BRNCHCON) /* OCT 77774 */

/* ---- PINBALL_GAME_BUTTONS_AND_LIGHTS, part 2a: the display words ---- */
/* The routines that put a word on the DSKY in decimal or octal (LM pp.
 * 441-445, CM pp. 359-363): bank 40, but DSPOCTWD, OCTBACK and DSP2BIT in
 * bank 41. The label this transcription spells DSPOCTWO at its line is
 * DSPOCTWD in the assembled listing and in the comment above it. Their
 * erasable words (WDRET, DSPWDRET, WDCNT) are named by ADDRESS_OF. */
#define DSPSIGN  ((word)K_DSPSIGN)
#define DSPRND   ((word)K_DSPRND)
#define DSPDECWD ((word)K_DSPDECWD)
#define DSPDCWD1 ((word)K_DSPDCWD1)
#define TRACE1   ((word)K_TRACE1)
#define TRACE1S  ((word)K_TRACE1S)
#define DECROUND ((word)K_DECROUND)
#define DSPDECNR ((word)K_DSPDECNR)
#define DSPDC2NR ((word)K_DSPDC2NR)
#define DSP2DEC  ((word)K_DSP2DEC)
#define END2DEC  ((word)K_END2DEC)
#define DSPDECVN ((word)K_DSPDECVN)
#define GOVNUPDT ((word)K_GOVNUPDT)
#define DSPOCTWD ((word)K_DSPOCTWD)
#define WDAGAIN  ((word)K_WDAGAIN)
#define OCTBACK  ((word)K_OCTBACK)
#define DSPLV    ((word)K_DSPLV)
#define DSP2BIT  ((word)K_DSP2BIT)
#define DSPOCTIN ((word)K_DSPOCTIN)
#define ENDSPOCT ((word)K_ENDSPOCT)
#define DPOSMAX  ((word)K_DPOSMAX) /* FIXED_FIXED_CONSTANT_POOL: OCT 37777, then POSMAX */
/* Their constants. */
#define BINCON   ((word)KW_BINCON)   /* DEC 10 */
#define VNDSPCON ((word)KW_VNDSPCON) /* OCT 00244: ".01 ROUNDED UP" */
#define DSPMSK   SEVEN               /* DSPMSK = SEVEN */

/* ---- PINBALL_GAME_BUTTONS_AND_LIGHTS, part 2b: ENTER and the verb fan ---- */
/* ENTER, the requests for data and the noun and verb display updates (LM
 * pp. 406-412, CM pp. 323-329), bank 41; and the fixed-fixed services they
 * use (ALMCYCLE, SFRUTMIX, SETNCADR, SETNADD, SETEBANK, RIGHT5, LEFT5,
 * FALTON, FLASHON, FLASHOFF, RELDSP, RELDSP1). Their erasable words
 * (VERBSAVE, NOUNCADR, NOUNADD, ZREG, DECOUNT, NOUNTEM, UPDATRET, RELRET,
 * EXITEM) are named by ADDRESS_OF, and so are the tables an INDEX reads
 * (VERBTAB, DISPLACE, LOW5 as the first word of LOW5, MID5, HI5) and
 * LODNNLOC, the 2CADR of LODNNTAB. ENTEXIT is ENTRET ("ENTEXIT = ENTRET"). */
#define NVSUBB   ((word)K_NVSUBB)
#define LOADLV1  ((word)K_LOADLV1)
#define ENTPASHI ((word)K_ENTPASHI)
#define ACCEPTWD ((word)K_ACCEPTWD)
#define ENTPAS0  ((word)K_ENTPAS0)
#define TESTVB   ((word)K_TESTVB)
#define TESTNN   ((word)K_TESTNN)
#define REQADD   ((word)K_REQADD)
#define USEADD   ((word)K_USEADD)
#define LODNNLOC ((word)K_LODNNLOC)
#define INTMCTBS ((word)K_INTMCTBS)
#define VERBFAN  ((word)K_VERBFAN)
#define VBFANDIR ((word)K_VBFANDIR)
#define MIXNOUN  ((word)K_MIXNOUN)
#define MIXNN1   ((word)K_MIXNN1)
#define MIXNN2   ((word)K_MIXNN2)
#define DPTEST   ((word)K_DPTEST)
#define DPTEST1  ((word)K_DPTEST1)
#define REQDATX  ((word)K_REQDATX)
#define REQDATY  ((word)K_REQDATY)
#define REQDATZ  ((word)K_REQDATZ)
#define REQCOM   ((word)K_REQCOM)
#define ENDRQDAT ((word)K_ENDRQDAT)
#define UPDATNN  ((word)K_UPDATNN)
#define PUTADD   ((word)K_PUTADD)
#define UPDAT1   ((word)K_UPDAT1)
#define GOALMCYC ((word)K_GOALMCYC)
#define GODSPALM ((word)K_GODSPALM)
#define NVSUB1   ((word)K_NVSUB1)
#define LOADLV   ((word)K_LOADLV)
#define DSPALARM ((word)K_DSPALARM)
#define SETNCADR ((word)K_SETNCADR)
#define SETNADD  ((word)K_SETNADD)
#define SETEBANK ((word)K_SETEBANK)
#define RIGHT5   ((word)K_RIGHT5)
#define LEFT5    ((word)K_LEFT5)
#define FALTON   ((word)K_FALTON)
#define FLASHON  ((word)K_FLASHON)
#define RELDSP   ((word)K_RELDSP)
#define RELDSP2  ((word)K_RELDSP2)
#define RELDSP1  ((word)K_RELDSP1)
#define ALMCYCLE ((word)K_ALMCYCLE)
#define SFRUTMIX ((word)K_SFRUTMIX)
#define SFRET1   ((word)K_SFRET1)
#define GOQ      ((word)K_GOQ)
/* Their constants. */
#define MMADREF ((word)KW_MMADREF) /* ADRES MMCHANG +1 */
#define LOWVERB ((word)KW_LOWVERB) /* DEC 28: "LOWER VERB THAT AVOIDS NOUN TEST." */
#define LST2CON ((word)KW_LST2CON) /* DEC 40: "FIRST LIST2 VERB (EXTENDED VERB)" */
#define MIXAD   ((word)KW_MIXAD)   /* TC MIXTEMP */
#define MID5    ((word)KW_MID5)    /* OCT 1740 */

/* ---- PINBALL_GAME_BUTTONS_AND_LIGHTS, part 3: the display verbs ---- */
/* The display verbs and the scale factor routines for display (LM pp.
 * 414-424, CM pp. 331-342): bank 41 (DSPABC to SFOUTABR), bank 40 (the
 * output scale routines, READLO and DSPDPDEC) and bank 42 (HMSOUT, M/SOUT
 * and their subroutines); with the scale factor code subroutines of bank
 * 41 (SFRUTNOR, SFCONUM: LM p. 428) and TPLEFTN (bank 40), TPSL1 and
 * PRSHRTMP (fixed-fixed). A label that starts with a digit, or holds '/',
 * '-' or '+', is spelled in words: 360-CDUO as THREE_SIXTY_CDUO, 2INTOUT as
 * TWO_INTOUT, M/SOUT as M_SOUT, L14/OUT as L14_OUT, +LIMIT as PLUS_LIMIT.
 * 360-CDUO, 360-CDU, 360-CDUE and LRPOSOUT are Luminary099's; OPDEGOUT,
 * BIASCOM, NEGOPT and DPFRACOT Comanche055's. Their erasable words
 * (DISTEM, DECTEM, XREG, TEM4, HITEMOUT, LOTEMOUT, SEPSCRET, SEPMNRET)
 * are named by ADDRESS_OF, and so are the constants a DCA, DMP or INDEX
 * reads (DEGTAB, SECON1, SECON2, MINCON1, HRCON1, RNDCON, COMPICK,
 * SFOUTABR, GTSFOUTL). ENTEXIT is ENTRET. */
#define DSPABC     ((word)K_DSPABC)
#define DSPAB      ((word)K_DSPAB)
#define DSPA       ((word)K_DSPA)
#define DSPCOM1    ((word)K_DSPCOM1)
#define DSPB       ((word)K_DSPB)
#define DSPC       ((word)K_DSPC)
#define DSPCOM2    ((word)K_DSPCOM2)
#define DSPCOM3    ((word)K_DSPCOM3)
#define COMPTEST   ((word)K_COMPTEST)
#define COMPTST1   ((word)K_COMPTST1)
#define NDCMPTST   ((word)K_NDCMPTST)
#define DCOMPTST   ((word)K_DCOMPTST)
#define DECTEST    ((word)K_DECTEST)
#define DCTSTCYC   ((word)K_DCTSTCYC)
#define NOUNTEST   ((word)K_NOUNTEST)
#define TSTFORDP   ((word)K_TSTFORDP)
#define CHANDSP    ((word)K_CHANDSP)
#define COMPICK    ((word)K_COMPICK)
#define GETCOMP    ((word)K_GETCOMP)
#define DECDSP     ((word)K_DECDSP)
#define DSPDCGET   ((word)K_DSPDCGET)
#define DSPDCPUT   ((word)K_DSPDCPUT)
#define DSPSFNOR   ((word)K_DSPSFNOR)
#define DSPDCEND   ((word)K_DSPDCEND)
#define DECDSP3    ((word)K_DECDSP3)
#define DEGOUTSF   ((word)K_DEGOUTSF)
#define SETAUG     ((word)K_SETAUG)
#define FIXRANGE   ((word)K_FIXRANGE)
#define DEGCOM     ((word)K_DEGCOM)
#define ARTOUTSF   ((word)K_ARTOUTSF)
#define SCOUTEND   ((word)K_SCOUTEND)
#define AROUT1SF   ((word)K_AROUT1SF)
#define DP1OUTSF   ((word)K_DP1OUTSF)
#define L14_OUT    ((word)K_L14_OUT)
#define DP2OUTSF   ((word)K_DP2OUTSF)
#define DP3OUTSF   ((word)K_DP3OUTSF)
#define DPOUT      ((word)K_DPOUT)
#define TWO_INTOUT ((word)K_2INTOUT)
#define READLO     ((word)K_READLO)
#define READLO1    ((word)K_READLO1)
#define RDLONOR    ((word)K_RDLONOR)
#define ENDRDLO    ((word)K_ENDRDLO)
#define HMSOUT     ((word)K_HMSOUT)
#define M_SOUT     ((word)K_M_SOUT)
#define M_SLIMIT   ((word)K_M_SLIMIT)
#define LIMITCOM   ((word)K_LIMITCOM)
#define PLUS_LIMIT ((word)K_PLIMIT)
#define M_SNORM    ((word)K_M_SNORM)
#define SEPSEC     ((word)K_SEPSEC)
#define SEPSEC1    ((word)K_SEPSEC1)
#define POSEC      ((word)K_POSEC)
#define SEPSECNR   ((word)K_SEPSECNR)
#define SEPMIN     ((word)K_SEPMIN)
#define ENDSPMIN   ((word)K_ENDSPMIN)
#define DSPDPDEC   ((word)K_DSPDPDEC)
#define ENDDPDEC   ((word)K_ENDDPDEC)
#define SFRUTNOR   ((word)K_SFRUTNOR)
#define SFCONUM    ((word)K_SFCONUM)
#define SFRET      ((word)K_SFRET)
#define CONUMNOR   ((word)K_CONUMNOR)
#define TPLEFTN    ((word)K_TPLEFTN)
#define LEFTNCOM   ((word)K_LEFTNCOM)
#define TPSL1      ((word)K_TPSL1)
#define PRSHRTMP   ((word)K_PRSHRTMP)
#define PREDSPAL   ((word)K_PREDSPAL)
#if LUMINARY099
#define THREE_SIXTY_CDUO ((word)K_360_CDUO)
#define THREE_SIXTY_CDU  ((word)K_360_CDU)
#define THREE_SIXTY_CDUE ((word)K_360_CDUE)
#define LRPOSOUT         ((word)K_LRPOSOUT)
#else
#define OPDEGOUT    ((word)K_OPDEGOUT)
#define BIASCOM     ((word)K_BIASCOM)
#define NEGOPT      ((word)K_NEGOPT)
#define DPFRACOT    ((word)K_DPFRACOT)
#define TWENTY_BIAS ((word)KW_20BIAS) /* OCT 16040: "20 DEG BIAS FOR OPTICS" */
#endif
/* Their constants. */
#define HISECON    ((word)KW_HISECON)   /* OCT 23147: ".6 + 2EXP-14" */
#define HIMINCON   ((word)KW_HIMINCON)  /* OCT 23346: ".6 + 2EXP-7" */
#define M_SCON1    ((word)KW_M_SCON1)   /* OCT 77753: "- HI PART OF (59M58.5S) +1" */
#define M_SCON2    ((word)KW_M_SCON2)   /* OCT 41126: "- LO PART OF (59M58.5S) +1" */
#define NORMADR    ((word)KW_NORMADR)   /* ADRES M/SNORM +1 */
#define M_SCON3    ((word)KW_M_SCON3)   /* OCT 00025: "59M 59.5S", high part */
#define M_SCON3_LO ((word)KW_M_SCON3P1) /* OCT 37016, its low part */
#define MINCON2    ((word)KW_MINCON2)   /* OCT 00011: ".0006 FOR MINUTES DISPLAY" */
#define MINCON2_LO ((word)KW_MINCON2P1) /* OCT 32445, its low part */

/* ---- PINBALL_GAME_BUTTONS_AND_LIGHTS, part 4: the load verbs ---- */
/* The load verbs and the scale factor routines for loading (LM pp.
 * 425-436, CM pp. 343-354): bank 41 (ABCLOAD to SFINTABR), bank 40 (the
 * input scale routines, DEGINSF to TESTOFUF) and bank 42 (HMSIN to
 * FORCEV25). Spelled in words: ALLDC/OC as ALLDC_OC, 2ROUND as TWO_ROUND,
 * 2RNDEND as TWO_RNDEND, RND/TST as RND_TST. SFMIXCAL, SFNORCAL, MIXBACK,
 * NORBACK and ENDSCAL1 are Luminary099's (Comanche055's ENDSCALE goes
 * straight to PUTCOM2); OPTDEGIN, OPDEGIN2 and DPFRACIN Comanche055's.
 * Their erasable words (LOADSTAT, YREG, YREGLP, ZREGLP, DECRET, HITEMIN)
 * are named by ADDRESS_OF, and so are the constants a DMP or INDEX reads
 * (DEGCON1, DEGCON2, WHOLECON, SFINTABR, GTSFINLC). */
#define ABCLOAD    ((word)K_ABCLOAD)
#define PUTXYZ     ((word)K_PUTXYZ)
#define BITSOFF    ((word)K_BITSOFF)
#define BITSOFF1   ((word)K_BITSOFF1)
#define ABLOAD     ((word)K_ABLOAD)
#define PUTXY      ((word)K_PUTXY)
#define ALOAD      ((word)K_ALOAD)
#define BLOAD      ((word)K_BLOAD)
#define CLOAD      ((word)K_CLOAD)
#define ALLDC_OC   ((word)K_ALLDC_OC)
#define PUTCOM     ((word)K_PUTCOM)
#define PUTDPCOM   ((word)K_PUTDPCOM)
#define PUTNORM    ((word)K_PUTNORM)
#define PUTCOM2    ((word)K_PUTCOM2)
#define CHANLOAD   ((word)K_CHANLOAD)
#define PUTDECSF   ((word)K_PUTDECSF)
#define PUTSFNOR   ((word)K_PUTSFNOR)
#define PUTDCSF2   ((word)K_PUTDCSF2)
#define DEGINSF    ((word)K_DEGINSF)
#define DEGINSF2   ((word)K_DEGINSF2)
#define SIGNFIX    ((word)K_SIGNFIX)
#define ENDSCALE   ((word)K_ENDSCALE)
#define NEG180     ((word)K_NEG180)
#define SGNTO1     ((word)K_SGNTO1)
#define ARTHINSF   ((word)K_ARTHINSF)
#define BINROUND   ((word)K_BINROUND)
#define ARTIN1SF   ((word)K_ARTIN1SF)
#define DPINSF     ((word)K_DPINSF)
#define DPINCOM    ((word)K_DPINCOM)
#define DPINORM    ((word)K_DPINORM)
#define DPINSF2    ((word)K_DPINSF2)
#define DPINSF4    ((word)K_DPINSF4)
#define TWO_ROUND  ((word)K_2ROUND)
#define TWO_RNDEND ((word)K_2RNDEND)
#define TESTOFUF   ((word)K_TESTOFUF)
#define HMSIN      ((word)K_HMSIN)
#define RND_TST    ((word)K_RND_TST)
#define MPACTST    ((word)K_MPACTST)
#define SIZETST    ((word)K_SIZETST)
#define ALL3DEC    ((word)K_ALL3DEC)
#define FORCEV25   ((word)K_FORCEV25)
#if LUMINARY099
#define MIXBACK  ((word)K_MIXBACK)
#define NORBACK  ((word)K_NORBACK)
#define ENDSCAL1 ((word)K_ENDSCAL1)
#define SFMIXCAL ((word)K_SFMIXCAL)
#define SFNORCAL ((word)K_SFNORCAL)
#else
#define OPTDEGIN    ((word)K_OPTDEGIN)
#define OPDEGIN2    ((word)K_OPDEGIN2)
#define DPFRACIN    ((word)K_DPFRACIN)
#define NEG_POINT_2 ((word)KW_NEG_2) /* OCT -06250 */
#endif
/* Their constants. */
#define VBSP1LD  ((word)KW_VBSP1LD)  /* DEC 21: "VB21 = ALOAD" */
#define VBSP2LD  ((word)KW_VBSP2LD)  /* DEC 22: "VB22 = BLOAD" */
#define VBSP3LD  ((word)KW_VBSP3LD)  /* DEC 23: "VB23 = CLOAD" */
#define HRCON    ((word)KW_HRCON)    /* OCT 00025: "1 HOUR IN CENTI-SEC", high part */
#define HRCON_LO ((word)KW_HRCONP1)  /* OCT 37100, its low part */
#define MINCON   ((word)KW_MINCON)   /* OCT 13560: "1 MINUTE IN CENTI-SEC" */
#define MIN59    ((word)KW_59MIN)    /* OCT 00073: 59MIN, "59 AS WHOLE" */
#define SEC59_99 ((word)KW_59_99SEC) /* OCT 13557: 59.99SEC, "5999 CENTI-SEC" */
#define OCT34BAR ((word)KW_OCT34BAR) /* OCT 77743 */
#define OCT31    ((word)KW_OCT31)

/* ---- PINBALL_GAME_BUTTONS_AND_LIGHTS, part 5: monitor and the alarms ---- */
/* MONITOR to MONBUSY and DSPFMEM (LM pp. 437-440, CM pp. 355-358) in bank
 * 41, PASTEVB to ENDPASTE in fixed-fixed, and PREDSPAL, DSPALARM and
 * CHARALRM (LM p. 446, CM p. 364) in bank 40. Spelled in words: BIT15/14
 * as BIT15_14. MONSAVE2 (the NVMONOPT options) is named by ADDRESS_OF. */
#define MONITOR  ((word)K_MONITOR)
#define MONIT1   ((word)K_MONIT1)
#define MONIT2   ((word)K_MONIT2)
#define MONREQ   ((word)K_MONREQ)
#define KILLMON  ((word)K_KILLMON)
#define MONDO    ((word)K_MONDO)
#define ENDMONDO ((word)K_ENDMONDO)
#define PASTEVB  ((word)K_PASTEVB)
#define PASTEOPT ((word)K_PASTEOPT)
#define ENDPASTE ((word)K_ENDPASTE)
#define MONBUSY  ((word)K_MONBUSY)
#define DSPFMEM  ((word)K_DSPFMEM)
#define ENDSPF   ((word)K_ENDSPF)
#define BLANKSUB ((word)K_BLANKSUB)
#define KILMONON ((word)K_KILMONON)
#define NVSUBEND ((word)K_NVSUBEND)
/* Their constants. */
#define BIT15_14 ((word)KW_BIT15_14) /* OCT 60000, a label too: MONITOR's CCS runs it */
#define MONDEL   ((word)KW_MONDEL)   /* OCT 144: "FOR 1 SEC MONITOR INTERVALS" */
#define MONREF   ((word)KW_MONREF)   /* OCT 75377: "-DEC10, STARTING IN BIT8" */
#define MONBACK  ((word)KW_MONBACK)  /* ADRES PASTEVB */
#define MID7     ((word)KW_MID7)     /* OCT 37600 */
#define MONADR   ((word)KW_MONADR)   /* GENADR PASTEVB */
#define NVSBENDL ((word)KW_NVSBENDL) /* TC NVSUBEND */

/* ---- PINBALL_GAME_BUTTONS_AND_LIGHTS, part 6: the special verbs ---- */
/* MMCHANG to ENDRQWT (LM pp. 447-448, CM pp. 365-366) in bank 41, VBPROC
 * to UNSUSPEN (LM pp. 448-450, CM pp. 366-368) in bank 40; RECALTST
 * (part 7) and PINBRNCH (DISPLAY_INTERFACE_ROUTINES), which they go to. */
#define MMCHANG  ((word)K_MMCHANG)
#define REQMM    ((word)K_REQMM)
#define VBRQEXEC ((word)K_VBRQEXEC)
#define REQEX1   ((word)K_REQEX1)
#define REQUESTC ((word)K_REQUESTC)
#define SETVAC   ((word)K_SETVAC)
#define VBRQWAIT ((word)K_VBRQWAIT)
#define ENDRQWT  ((word)K_ENDRQWT)
#define VBPROC   ((word)K_VBPROC)
#define VBTERM   ((word)K_VBTERM)
#define PROCKEY  ((word)K_PROCKEY)
#define VBRESEQ  ((word)K_VBRESEQ)
#define TSTLTS4  ((word)K_TSTLTS4)
#define UNSUSPEN ((word)K_UNSUSPEN)
#define RECALTST ((word)K_RECALTST)
#define PINBRNCH ((word)K_PINBRNCH)
/* Their constants: the instructions REQUESTC runs from MPAC. */
#define TCNOVAC  ((word)KW_TCNOVAC)  /* TC NOVAC */
#define TCFINDVC ((word)KW_TCFINDVC) /* TC FINDVAC */
#define TCWAIT   ((word)KW_TCWAIT)   /* TC WAITLIST */

/* ---- PINBALL_GAME_BUTTONS_AND_LIGHTS, part 7: NVSUB and its company ---- */
/* NVSUB to NVSUBEND, ENDIDLE to DSPABORT, JAMTERM, JAMPROC and BLANKSUB
 * (LM pp. 452-456, CM pp. 370-374) in fixed-fixed; BLANKDSP to ENDNVSB1
 * (LM pp. 453-454, CM pp. 371-372) in bank 41; BLNKSUB1, TESTBIT, DSPMMJB
 * and RECALTST to DOPROC (LM pp. 456-459, CM pp. 374-377) in bank 40; DSPMM
 * (LM p. 457, CM p. 375) in bank 04. Spelled in words: ISCADR+0 as
 * ISCADR_PLUS0, 34DEC as THIRTY_FOUR_DEC. NVTEMP, NVQTEM, NVBNKTEM and
 * DSPMMTEM are named by ADDRESS_OF. */
#define NVMONOPT     ((word)K_NVMONOPT)
#define NVSBCOM      ((word)K_NVSBCOM)
#define NVSUBCOM     ((word)K_NVSUBCOM)
#define ENDIDLE      ((word)K_ENDIDLE)
#define ISCADR_PLUS0 ((word)K_ISCADRP0) /* ISCADR+0 */
#define ISLIST_PLUS0 ((word)K_ISLISTP0) /* ISLIST+0 */
#define DSPABORT     ((word)K_DSPABORT)
#define JAMTERM      ((word)K_JAMTERM)
#define JAMPROC      ((word)K_JAMPROC)
#define BLANKDSP     ((word)K_BLANKDSP)
#define ENTSET       ((word)K_ENTSET)
#define NVSUB2       ((word)K_NVSUB2)
#define ENDNVSB1     ((word)K_ENDNVSB1)
#define BLNKSUB1     ((word)K_BLNKSUB1)
#define TESTBIT      ((word)K_TESTBIT)
#define DSPMM        ((word)K_DSPMM)
#define DSPMMJB      ((word)K_DSPMMJB)
#define ENDSPMM      ((word)K_ENDSPMM)
#define RECAL1       ((word)K_RECAL1)
#define RECAL2       ((word)K_RECAL2)
#define RECAL3       ((word)K_RECAL3)
#define DOTERM       ((word)K_DOTERM)
#define DOPROC       ((word)K_DOPROC)
/* Their constants. */
#define NVSBBBNK         ((word)KW_NVSBBBNK) /* BBCON NVSUB1 (PINSUPBT is the same word) */
#define BLNKBBNK         ((word)KW_BLNKBBNK) /* BBCON BLNKSUB1 */
#define THIRTY_FOUR_DEC  ((word)KW_34DEC)    /* 34DEC: DEC 34 */
#define THIRTY_THREE_DEC ((word)KW_33DEC)    /* 33DEC: DEC 33 */
#define MD1              ((word)KW_MD1)      /* OCT 25, after VD1 and ND1 */

/* ---- PINBALL_GAME_BUTTONS_AND_LIGHTS, part 8: the lights test, RSET and NVSUBUSY ---- */
/* FALTOF (LM p. 461, CM p. 379), PRENVBSY, NVSUBUSY, NVSBWAIT and NVSBWT1
 * (LM p. 464, CM p. 382) in fixed-fixed; NVSUBSY1 and ENDNVBSY (LM p.
 * 464, CM p. 382) in bank 04; VBTSTLTS to TSTLTS3 (LM pp. 467-468, CM pp.
 * 385-386) in bank 41; ERROR's TSTAB, ERMINUS, ERPLUS and ERCOM (LM pp.
 * 470-471, CM pp. 388-389) in bank 40. ERCNT (which shares its word
 * with POLISH) and SFAIL are named by ADDRESS_OF. */
#define FALTOF   ((word)K_FALTOF)
#define PRENVBSY ((word)K_PRENVBSY)
#define NVSUBUSY ((word)K_NVSUBUSY)
#define NVSUBSY1 ((word)K_NVSUBSY1)
#define ENDNVBSY ((word)K_ENDNVBSY)
#define NVSBWAIT ((word)K_NVSBWAIT)
#define NVSBWT1  ((word)K_NVSBWT1)
#define VBTSTLTS ((word)K_VBTSTLTS)
#define TSTLTS1  ((word)K_TSTLTS1)
#define TSTLTS2  ((word)K_TSTLTS2)
#define TSTLTS3  ((word)K_TSTLTS3)
#define TSTAB    ((word)K_TSTAB)
#define ERMINUS  ((word)K_ERMINUS)
#define ERPLUS   ((word)K_ERPLUS)
#define ERCOM    ((word)K_ERCOM)
/* Their constants. */
#define TWO_K_PLUS_3  ((word)KW_2KP3)     /* 2K+3: OCT 2003 */
#define FULLDSP       ((word)KW_FULLDSP)  /* "DISPLAY ALL 8:S" */
#define FULLDSP1      ((word)KW_FULLDSP1) /* "DISPLAY ALL 8:S AND +" */
#define TSTCON1       ((word)KW_TSTCON1)  /* channel 11 lights on */
#define TSTCON2       ((word)KW_TSTCON2)  /* DSPTAB +11D lights on */
#define TSTCON3       ((word)KW_TSTCON3)  /* "CHAN 11 BITS 1, 3, 4, 7." */
#define SHOLTS        ((word)KW_SHOLTS)   /* "5 SEC" */
#define BITS_13_11_1  ((word)KW_13_11_1)  /* 13-11,1: OCT 16001 */
#define OCT55000      ((word)KW_OCT55000)
#define ERCON         ((word)KW_ERCON)    /* "CHAN 11 BITS 3,7." */
#define GL_PLUS_NOATT ((word)KW_GLPNOATT) /* GL+NOATT: "NO ATT AND GIMBAL LOCK LAMPS" */
#define NOTBIT12      ((word)KW_NOTBIT12)
#if LUMINARY099
#define RFAILS2 ((word)KW_RFAILS2) /* "RADAR CDU AND DATA FAIL FLAGS." */
#define RFAILS  ((word)KW_RFAILS)  /* the same, for ERROR */
#else
#define PRIO15 ((word)KW_PRIO15) /* the LM's is named with AOTMARK's */
#endif

/* ---- R60_62 ---- */

/* Its labels (the LM's R60 and BALLANGS in bank 26, VECPOINT in 27 and
 * R62 in 23; the CM's all in bank 27), the routines it calls and its
 * constants. Its erasable words (TEMPR60, BALLEXIT, CPHI, TBASE1 or
 * TBASE2, ...) are named by ADDRESS_OF. */
#define REDOMANN ((word)K_REDOMANN)
#define TOBALL   ((word)K_TOBALL)
#define REDOMANC ((word)K_REDOMANC)
#define TOBALLC  ((word)K_TOBALLC)
#define STARTMNV ((word)K_STARTMNV)
#define ENDMANUV ((word)K_ENDMANUV)
#define ENDMANU1 ((word)K_ENDMANU1)
#define CHKLINUS ((word)K_CHKLINUS)
#define RELINUS  ((word)K_RELINUS)
#define R61TEST  ((word)K_R61TEST)
#define GOMOVE   ((word)K_GOMOVE)
#define GOPERF2R ((word)K_GOPERF2R)
#define GODSPR   ((word)K_GODSPR)
#define GOMANUR  ((word)K_GOMANUR)
#define GOFLASH  ((word)K_GOFLASH)
#define LINUS    ((word)K_LINUS)
#define GOTOV56  ((word)K_GOTOV56)
#define V06N18   ((word)KW_V06N18) /* VN 0618 */
#define V06N22   ((word)KW_V06N22) /* VN 0622 */
#define THREE_AXIS_BIT ((word)KW_3AXISBIT) /* 3AXISBIT: FLAGWRD5's 3AXISFLG */
#define PDSPFBIT ((word)KW_PDSPFBIT)       /* FLAGWRD4's PDSPFLAG */
#if LUMINARY099
#define R60LEM   ((word)K_R60LEM)
#define TOBALLA  ((word)K_TOBALLA)
#define AUTOMANV ((word)K_AUTOMANV)
#define RER60    ((word)K_RER60)
#define G_N_AUTO ((word)K_GPN_AUTO) /* G+N,AUTO */
#define ISITAUTO ((word)K_ISITAUTO)
#define BALLANGS ((word)K_BALLANGS)
#define ENDBALL  ((word)K_ENDBALL)
#define R62DISP  ((word)K_R62FLASH) /* R62DISP EQUALS R62FLASH */
#else
#define R60CSM       ((word)K_R60CSM)
#define GOREDO20     ((word)K_GOREDO20)
#define R62DISP      ((word)K_R62DISP)
#define BITS13_14_15 ((word)KW_13_14_15) /* 13,14,15 */
#endif

/* ---- S-BAND_ANTENNA_FOR_LM, S-BAND_ANTENNA_FOR_CM ---- */

/* R05, verb 64 (bank 42 in both missions: FB 32 with the superbank bit). */
#define SBANDANT ((word)K_SBANDANT)
#define V06N51   ((word)KW_V06N51) /* VN 0651 */
#if LUMINARY099
#define SBANDEX ((word)K_SBANDEX)
#endif

/* ---- RADAR_LEADIN_ROUTINES ---- */

#if LUMINARY099
/* The radar sampling loop, in bank 25, and the radar routines RDRLOCS
 * names (bank 25). Its erasable words (RSAMPDT, RTSTLOC, RTSTDEX,
 * RTSTBASE, RTSTMAX, RFAILCNT, RSTACK, SAMPLSUM) are named by ADDRESS_OF. */
#define RADSAMP  ((word)K_RADSAMP)
#define DORSAMP  ((word)K_DORSAMP)
#define DORSAMP2 ((word)K_DORSAMP2)
#define VARADAR  ((word)K_VARADAR)
#define RDRLOCS  ((word)K_RDRLOCS)
#define RADSTALL ((word)K_RADSTALL)
#endif

/* ---- P20-P25 ---- */

#if LUMINARY099
/* Part 1, P20 and P25 (bank 24; ORBCHG3 in bank 32), and LUNSFCHK,
 * R21LEM and R23LEM, which they call. Their erasable words (OPTION2,
 * MARKCTR, R65CNTR, FLAGWRD0-2, FLAGWRD8, MODREG, RADMODES, STATE, MPAC)
 * are named by ADDRESS_OF. */
#define PROG20                ((word)K_PROG20)
#define ORBCHGO               ((word)K_ORBCHGO)
#define ORBCHG1               ((word)K_ORBCHG1)
#define ORBCHG2               ((word)K_ORBCHG2)
#define ORBCHG3               ((word)K_ORBCHG3)
#define PROG20A               ((word)K_PROG20A)
#define P20LEM1               ((word)K_P20LEM1)
#define FIVE_TWENTY_SIX_ALARM ((word)K_526ALARM) /* 526ALARM */
#define P20LEMA               ((word)K_P20LEMA)
#define P20LEMB               ((word)K_P20LEMB)
#define P20LEMB1              ((word)K_P20LEMB1)
#define P20LEMB2              ((word)K_P20LEMB2)
#define P20LEMB3              ((word)K_P20LEMB3)
#define P20LEMB4              ((word)K_P20LEMB4)
#define P20LEMB5              ((word)K_P20LEMB5)
#define P20LEMB6              ((word)K_P20LEMB6)
#define P20LEMC               ((word)K_P20LEMC)
#define P20LEMC1              ((word)K_P20LEMC1)
#define P20LEMC2              ((word)K_P20LEMC2)
#define P20LEMC3              ((word)K_P20LEMC3)
#define P20LEMD               ((word)K_P20LEMD)
#define P20LEMD1              ((word)K_P20LEMD1)
#define P20LEMD2              ((word)K_P20LEMD2)
#define P20LEMF               ((word)K_P20LEMF)
#define P20LEMWT              ((word)K_P20LEMWT)
#define P20LMWT1              ((word)K_P20LMWT1)
#define UPPSV                 ((word)K_UPPSV)
#define PROG25                ((word)K_PROG25)
#define P25LEM1               ((word)K_P25LEM1)
#define P25LEM2               ((word)K_P25LEM2)
#define P25LEMWT              ((word)K_P25LEMWT)
#define P25LMWT1              ((word)K_P25LMWT1)
#define LUNSFCHK              ((word)K_LUNSFCHK)
#define R21LEM                ((word)K_R21LEM)
#define R23LEM                ((word)K_R23LEM)
#define ALRM514               ((word)KW_ALRM514)
#define ALRM526               ((word)KW_ALRM526)
#define OCT201                ((word)KW_OCT201)
#define OCT00012              ((word)KW_OCT00012)
#define P22ONE                ((word)KW_P22ONE)
#define V06N33_STAR           ((word)KW_V06N33_STAR) /* V06N33*: VN 0633 */
#define DEC_250               ((word)KW_250DEC)      /* 250DEC */
#define DEC_1500              ((word)KW_1500DEC)     /* 1500DEC */
#define SIXTY_SCNDS           ((word)KW_60SCNDS)     /* 60SCNDS */
#define RCDU0BIT              ((word)KW_RCDU0BIT)
/* Part 2, R22 and R21 (bank 24), and R24LEM, which R21 calls. */
#define R22LEM   ((word)K_R22LEM)
#define R22LEM1  ((word)K_R22LEM1)
#define R22LEM2  ((word)K_R22LEM2)
#define R22LEM3  ((word)K_R22LEM3)
#define R22LEM93 ((word)K_R22LEM93)
#define R22LEM96 ((word)K_R22LEM96)
#define R22LEM7  ((word)K_R22LEM7)
#define R22LEM44 ((word)K_R22LEM44)
#define R22LEM42 ((word)K_R22LEM42)
#define R22LEM45 ((word)K_R22LEM45)
#define R22WAIT  ((word)K_R22WAIT)
#define R22LEM46 ((word)K_R22LEM46)
#define N49DSP   ((word)K_N49DSP)
#define R22RSTRT ((word)K_R22RSTRT)
#define R21LEM5  ((word)K_R21LEM5)
#define R21LEM6  ((word)K_R21LEM6)
#define R21LEM10 ((word)K_R21LEM10)
#define R21LEM2  ((word)K_R21LEM2)
#define R21LEM1  ((word)K_R21LEM1)
#define R21LEM3  ((word)K_R21LEM3)
#define R21_503  ((word)K_R21_503) /* R21-503 */
#define R21END   ((word)K_R21END)
#define R21SRCH  ((word)K_R21SRCH)
#define R21LEM4  ((word)K_R21LEM4)
#define R21LEM12 ((word)K_R21LEM12)
#define R21LEM13 ((word)K_R21LEM13)
#define R21LEM7  ((word)K_R21LEM7)
#define R21LEM8  ((word)K_R21LEM8)
#define R21LEM9  ((word)K_R21LEM9)
#define R21LEM11 ((word)K_R21LEM11)
#define R21DISP  ((word)K_R21DISP)
#define R24LEM   ((word)K_R24LEM)
#define KILLTASK ((word)K_KILLTASK)
#define ALRM525  ((word)KW_ALRM525)
#define ALRM503  ((word)KW_ALRM503)
#define ALRM530  ((word)KW_ALRM530)
#define V06N05   ((word)KW_V06N05)
#define V06N49NB ((word)KW_V06N49NB)
#define V06N72PV ((word)KW_V06N72PV)
#define MAXTRIES ((word)KW_MAXTRIES)
#define NOUPFBIT ((word)KW_NOUPFBIT)
#define UPDATBIT ((word)KW_UPDATBIT)
/* Part 3, R23 and R24 (bank 24), and CLRADMOD (fixed-fixed). Their
 * erasable words (DATAGOOD, OMEGAD, RADCADR, RADMODES) are named by
 * ADDRESS_OF. */
#define R23LEM1  ((word)K_R23LEM1)
#define R23LEM11 ((word)K_R23LEM11)
#define OUTOFLIM ((word)K_OUTOFLIM)
#define R23LEM2  ((word)K_R23LEM2)
#define R23LEM3  ((word)K_R23LEM3)
#define R24LEM1  ((word)K_R24LEM1)
#define R24LEM2  ((word)K_R24LEM2)
#define R24END   ((word)K_R24END)
#define R24LEM3  ((word)K_R24LEM3)
#define R24LEM4  ((word)K_R24LEM4)
#define OCT501PV ((word)KW_OCT501PV)
#define OCT205   ((word)KW_OCT205)
#define BIT10_15 ((word)KW_BIT10P15) /* BIT10+15 */
#define V16N80   ((word)KW_V16N80)
/* Part 4, R61 and R65 (bank 23), and TMODE (the interpreter's, which
 * READCDUD ends at). GENRET, CPHI, CPSI, CDUXD-CDUZD and R65CNTR are
 * named by ADDRESS_OF. */
#define R61LEM   ((word)K_R61LEM)
#define R65LEM   ((word)K_R65LEM)
#define R61C_L01 ((word)K_R61CPL01) /* R61C+L01 */
#define R61C_L03 ((word)K_R61CPL03) /* R61C+L03 */
#define R61LEM1  ((word)K_R61LEM1)
#define R61LEM2  ((word)K_R61LEM2)
#define R61C_L05 ((word)K_R61CPL05) /* R61C+L05 */
#define R61C_L06 ((word)K_R61CPL06) /* R61C+L06 */
#define R61C_L2  ((word)K_R61CPL2)  /* R61C+L2 */
#define R61C_L04 ((word)K_R61CPL04) /* R61C+L04 */
#define R61C_L4  ((word)K_R61CPL4)  /* R61C+L4 */
#define R65WAIT  ((word)K_R65WAIT)
#define READCDUD ((word)K_READCDUD)
#define TMODE    ((word)K_TMODE)
#define R61FLBIT ((word)KW_R61FLBIT)
#define SIX_SEC  ((word)KW_06SEC) /* 06SEC: DEC 600 */
/* Part 5, the rendezvous radar's subroutines: MAGSUB to FLIP (fixed-
 * fixed) and RRTURNON to RRZ2 (bank 25), with ENDRADAR, RDBADEND and
 * RADNOOP, which they end at. RRRET, RDES, RRINDEX, LASTYCMD and LASTXCMD
 * are named by ADDRESS_OF. */
#define MAGSUB        ((word)K_MAGSUB)
#define MODE2CHK      ((word)K_MODE2CHK)
#define RRLIMOK       ((word)K_RRLIMOK)
#define FLIP          ((word)K_FLIP)
#define RRTURNON      ((word)K_RRTURNON)
#define RRZEROSB      ((word)K_RRZEROSB)
#define DORREPOS      ((word)K_DORREPOS)
#define REPOSRPT      ((word)K_REPOSRPT)
#define SETRRECR      ((word)K_SETRRECR)
#define REMODE        ((word)K_REMODE)
#define RMODINV       ((word)K_RMODINV)
#define RRTONLY       ((word)K_RRTONLY)
#define RRSONLY       ((word)K_RRSONLY)
#define RR1AXIS       ((word)K_RR1AXIS)
#define NXTRR1AX      ((word)K_NXTRR1AX)
#define RR1AX2        ((word)K_RR1AX2)
#define RROUT         ((word)K_RROUT)
#define RROUT2        ((word)K_RROUT2)
#define SETRRCTR      ((word)K_SETRRCTR)
#define RROUTLIM      ((word)K_RROUTLIM)
#define RRZERO        ((word)K_RRZERO)
#define ROADBACK      ((word)K_ROADBACK)
#define RRZ2          ((word)K_RRZ2)
#define RADNOOP       ((word)K_RADNOOP)
#define ENDRADAR      ((word)K_ENDRADAR)
#define RGOODEND      ((word)K_RGOODEND)
#define RDBADEND      ((word)K_RDBADEND)
#define BEGDES        ((word)K_BEGDES)
#define STDESIG       ((word)K_STDESIG)
#define TURNONBT      BIT1                /* RADMODES bit 1: the turn-on sequence running */
#define RRDATABT      BIT4                /* RADMODES bit 4: no rendezvous radar data */
#define DESIGBIT      BIT10               /* RADMODES bit 10: a designate waiting or running */
#define REMODBIT      BIT14               /* RADMODES bit 14: a remode asked for or running */
#define MINUS_BIT12   MINB12              /* -BIT12 EQUALS -1/8 (OCT 73777) */
#define MINUS_45DEGSR ((word)KW_13_14_15) /* -45DEGSR = 13,14,15 */
#define FIVE_5DEGS    ((word)KW_5_5DEGS)  /* 5.5DEGS */
#define EIGHTY2DEGS   ((word)KW_82DEGS)   /* 82DEGS */
#define BITS13_7_2    ((word)KW_13_7_2)   /* 13,7,2: OCT 10102 */
#define MINUS_50DEGSR ((word)KW_M50DEGSR) /* -50DEGSR */
#define MINUS_80DEGSR ((word)KW_M80DEGSR) /* -80DEGSR */
#define RRSPGAIN      ((word)KW_RRSPGAIN)
#define MINUS_RRLIMIT ((word)KW_MRRLIMIT) /* -RRLIMIT: DEC -384 */
#define BIT11_1       ((word)KW_BIT11P1)  /* BIT11+1: OCT 02001 */
#define PRIO6         ((word)KW_PRIO6)
/* Part 6, the designate: RRDESSM to DODESEND (bank 25). DESRET, DESCOUNT,
 * LOSCOUNT, TANG and TANGNB are named by ADDRESS_OF. */
#define RRDESSM      ((word)K_RRDESSM)
#define OKDESSM      ((word)K_OKDESSM)
#define LUNDESCH     ((word)K_LUNDESCH)
#define STARTDES     ((word)K_STARTDES)
#define DESRETRN     ((word)K_DESRETRN)
#define DESRTRN      ((word)K_DESRTRN)
#define NORDSTAL     ((word)K_NORDSTAL)
#define TRYSWS       ((word)K_TRYSWS)
#define NODESSM      ((word)K_NODESSM)
#define RRDESNB      ((word)K_RRDESNB)
#define OKDESNB      ((word)K_OKDESNB)
#define TRYSWN       ((word)K_TRYSWN)
#define NODESNB      ((word)K_NODESNB)
#define RRLIMNB      ((word)K_RRLIMNB)
#define SHAFTLIM     ((word)K_SHAFTLIM)
#define NEGSHAFT     ((word)K_NEGSHAFT)
#define DESLOOP      ((word)K_DESLOOP)
#define STDESIG1     ((word)K_STDESIG1)
#define BADDES       ((word)K_BADDES)
#define MOREDES      ((word)K_MOREDES)
#define DODES        ((word)K_DODES)
#define DONBRD       ((word)K_DONBRD)
#define SIGNLCHK     ((word)K_SIGNLCHK)
#define DGOODCHK     ((word)K_DGOODCHK)
#define RRDESDUN     ((word)K_RRDESDUN)
#define DORROUT      ((word)K_DORROUT)
#define NOTP20       ((word)K_NOTP20)
#define DODESEND     ((word)K_DODESEND)
#define COS1_2DG     ((word)K_COS1_2DG)  /* COS1/2DG: 2DEC .999961923 B-2, the cosine of half a degree */
#define MAXTRYS      ((word)KW_MAXTRYS)  /* DEC 60 */
#define TWENTY_5DEGS ((word)KW_20_5DEGS) /* 20.5DEGS */
#define B14_B2       ((word)KW_B14PB2)   /* B14+B2: OCT 20002 */
#define RDESGAIN     ((word)KW_RDESGAIN)
#define BIT12_14     ((word)KW_BIT12_14) /* BIT12,14 EQUALS PRIO24 */
#define LOKONBIT     ((word)KW_LOKONBIT)
#define LOSCMBIT     ((word)KW_LOSCMBIT)
/* Part 7, the radar data read: LRALT to RGOODEND (bank 25), and the
 * landing radar's lamps, R12LITES to VLIGHT (bank 25). TIMEHOLD, NSAMP,
 * OLDATAGD, DNINDEX, DNRRANGE, RNRAD and HFLSHBIT (an address: INDEX
 * ITEMP5 reads the bit next to it) are named by ADDRESS_OF. */
#define LRALT    ((word)K_LRALT)
#define LRVELZ   ((word)K_LRVELZ)
#define LRVELY   ((word)K_LRVELY)
#define LRVELX   ((word)K_LRVELX)
#define RRRDOT   ((word)K_RRRDOT)
#define RRRANGE  ((word)K_RRRANGE)
#define LRVEL    ((word)K_LRVEL)
#define INITREAD ((word)K_INITREAD)
#define RADAREAD ((word)K_RADAREAD)
#define TRYCOUNT ((word)K_TRYCOUNT)
#define NOMORE   ((word)K_NOMORE)
#define BADRAD   ((word)K_BADRAD)
#define PLENTY   ((word)K_PLENTY)
#define LRPOSCHK ((word)K_LRPOSCHK)
#define VELCHK   ((word)K_VELCHK)
#define GOODRAD  ((word)K_GOODRAD)
#define NOEND    ((word)K_NOEND)
#define RESAMPLE ((word)K_RESAMPLE)
#define LRHEIGHT ((word)K_LRHEIGHT)
#define RENDRAD  ((word)K_RENDRAD)
#define SCALECHK ((word)K_SCALECHK)
#define RADIN    ((word)K_RADIN)
#define DASAMPL  ((word)K_DASAMPL)
#define DGCHECK2 ((word)K_DGCHECK2)
#define SCALCHNG ((word)K_SCALCHNG)
#define R77CHECK ((word)K_R77CHECK)
#define SCALADJ  ((word)K_SCALADJ)
#define LRSCK    ((word)K_LRSCK)
#define DGCHECK  ((word)K_DGCHECK)
#define DATAFAIL ((word)K_DATAFAIL)
#define LRPOS2   ((word)K_LRPOS2)
#define LRPOSNXT ((word)K_LRPOSNXT)
#define LASTLRDT ((word)K_LASTLRDT)
#define LRPOSCAN ((word)K_LRPOSCAN)
#define R12LITES ((word)K_R12LITES)
#define RADLITES ((word)K_RADLITES)
#define HLIGHT   ((word)K_HLIGHT)
#define BOTHLITS ((word)K_BOTHLITS)
#define LITIT    ((word)K_LITIT)
#define ONLITES  ((word)K_ONLITES)
#define VLIGHT   ((word)K_VLIGHT)
#define RDOTBIAS ((word)K_RDOTBIAS)
#define ALLREAD  ((word)KW_ALLREAD) /* OCT 17: the radar select bits, channel 13 bits 1-4 */
#define DGBITS   ((word)KW_DGBITS) /* OCT 230: the data good bits, channel 33 bits 4, 5 and 8 */
#define BITS5_8  ((word)KW_BITS5_8) /* BITS5,8: OCT 220 */
#define HISCALIM ((word)KW_HISCALIM) /* DEC 460 */
#define LVELBIAS ((word)KW_LVELBIAS) /* DEC -12288 */
#define SCABBIT  ((word)KW_SCABBIT) /* FLGWRD11 bit 3: the LR scale discrete bad */
#define LRALTBIT ((word)KW_LRALTBIT) /* RADMODES bit 5: LR altitude data failed */
#define LRPOSBIT ((word)KW_LRPOSBIT) /* RADMODES bit 6: LR position 2 wanted */
#define SIX_SECS ((word)KW_6SECS) /* 6SECS: DEC 600 */
/* Part 8, LPS20.1 (bank 24) and LPS20.2 (bank 25). */
#define LPS20_1 ((word)K_LPS20_1)
#define LPS20_2 ((word)K_LPS20_2)
/* Part 9, LRS22.1 (bank 32) and LRS22.2 (bank 24). LRS22.1X, RDOTMSAV,
 * RANGRDOT, MKTIME, AIG, AMG and AOG are named by ADDRESS_OF. */
#define LRS22_1  ((word)K_LRS22_1)
#define READRDOT ((word)K_READRDOT)
#define NORMEXIT ((word)K_NORMEXIT)
#define OUT22_1  ((word)K_OUT22_1) /* OUT22.1 */
#define CHEXERR  ((word)K_CHEXERR)
#define EREXIT1  ((word)K_EREXIT1)
#define LRS22_2  ((word)K_LRS22_2)
#define OKEXIT   ((word)K_OKEXIT)
#define OUT22_2  ((word)K_OUT22_2) /* OUT22.2 */
#define RNGSCBIT ((word)KW_RNGSCBIT) /* FLAGWRD5 bit 10: the RR range scale changed */
#define RRRSBIT  ((word)KW_RRRSBIT) /* RADMODES bit 3: the RR range scale (channel 33 bit 3) */
/* Part 10, LSR22.3 (bank 26). */
#define LSR22_3 ((word)K_LSR22_3)
#define RANGEBQ ((word)K_RANGEBQ)
#define QOK     ((word)K_QOK)
/* Part 12, the angles. */
#define MXMYMZ  ((word)K_MXMYMZ)
#define AIGBANK ((word)KW_AIGBANK) /* BBCON LSR22.3: bank 26, E bank 7 */
/* Part 13, LSR22.4 and WLINIT. */
#define LSR22_4 ((word)K_LSR22_4) /* LSR22.4 */
#define WLINIT  ((word)K_WLINIT)
#define WBANK   ((word)KW_WBANK) /* BBCON WLINIT: bank 26, E bank 5 (W) */
#define WSIZE   ((word)KW_WSIZE) /* DEC 161: W's last word */
/* Part 14, GETULC. */
#define GETULC ((word)K_GETULC)
/* Part 15, LRS24.1, the RR search. NSRCHPNT, DATAGOOD, RADMODES, CDUT and
 * TANGNB are named by ADDRESS_OF. */
#define LRS24_1     ((word)K_LRS24_1) /* LRS24.1 */
#define CONTDES2    ((word)K_CONTDES2)
#define OMEGCALC    ((word)K_OMEGCALC)
#define CALCXY      ((word)K_CALCXY)
#define CALLDGCH    ((word)K_CALLDGCH)
#define DATGDCHK    ((word)K_DATGDCHK)
#define LIMALARM    ((word)K_LIMALARM)
#define SRCHOBIT    BIT14                /* FLAGWRD2 bit 14: SRCHOPT, the R24 search wanted */
#define CDESBIT     BIT15                /* RADMODES bit 15: a continuous designate wanted */
#define SIX_SECONDS ((word)KW_6SECONDS)  /* 6SECONDS: DEC 600 */
#define ALL1S       ((word)KW_ALL1S)     /* DEC 11111 */
/* Part 16, GOTOV56 (fixed-fixed) and R29's start (bank 33, PREPOS29 bank
 * 25). PIPTIME, PIPCTR, STATE, MPAC, MODE and TIME2 are named by ADDRESS_OF. */
#define R29        ((word)K_R29)
#define R29_LOS    ((word)K_R29_LOS)  /* R29.LOS */
#define R29REMOJ   ((word)K_R29REMOJ) /* R29REM0J in the listing */
#define SETPRPOS   ((word)K_SETPRPOS)
#define PREPOS29   ((word)K_PREPOS29)
#define SETUP29D   ((word)K_SETUP29D)
#define NOR29NOW   ((word)K_NOR29NOW)
#define OLDESBIT   BIT1                /* STATE bit 1: R29's designate loop requested */
#define SUPER110   ((word)KW_SUPER110) /* OCT 140 */
static const word VB56CADR[2] = KW_VB56CADR; /* 2CADR TRMTRACK */
/* Part 17, R29's designate loop: BEGDES29 (bank 24) and R29DODES (bank
 * 32). LOSSM, LOSVDT/4, SAVECDUT, TANG, VBUF, PUSHLOC and FIXLOC are named by
 * ADDRESS_OF. */
#define BEGDES29 ((word)K_BEGDES29)
#define R29DLOOP ((word)K_R29DLOOP)
#define R29DODES ((word)K_R29DODES)
#define R29DPAS2 ((word)K_R29DPAS2)
#define GOTANGLS ((word)K_GOTANGLS)
#define R29LOKON ((word)K_R29LOKON)
#define END29DOD ((word)K_END29DOD)
#define MPACVBUF ((word)K_MPACVBUF)
#define RR29GAIN ((word)KW_RR29GAIN) /* DEC -.53624 */
#define R29FXLOC ((word)KW_R29FXLOC) /* ADRES INTB15+ -34D */
/* Part 18, R29's radar reading: R29READ and R29RDJOB (bank 24). TIMEHOLD,
 * CDUT, CDUY, CDUX, DNRRANGE, RM, MKTIME, TANGNB, AIG, AOG and TRKMKCNT
 * are named by ADDRESS_OF. */
#define R29READ  ((word)K_R29READ)
#define R29RDJOB ((word)K_R29RDJOB)
#define R29RANGE ((word)K_R29RANGE)
#define R29RRR_Q ((word)K_R29RRR_Q) /* R29RRR? */
#define ENDRRD29 ((word)K_ENDRRD29)
/* Part 19, V67, the W-matrix monitor: V67CALL (bank 31) and ADDPOS (bank
 * 30). WWPOS, WWVEL and WWBIAS are named by ADDRESS_OF. */
#define V67CALL  ((word)K_V67CALL)
#define V06N99DS ((word)K_V06N99DS)
#define V6N99PRO ((word)K_V6N99PRO)
#define V06N9933 ((word)K_V06N9933)
#define V67WW    ((word)K_V67WW)
#define ADDPOS   ((word)K_ADDPOS)
#define V67XXX   ((word)K_V67XXX)
#define V67CLRF  ((word)K_V67CLRF)
#define V06N99   ((word)KW_V06N99) /* VN 0699 */
#else
/* The CM's P20-P25 (Comanche055). Part 1, P20 (bank 37). Its erasable words
 * (TRKMKCNT, VHFCNT, R61CNTR, HOLDFLAG, STATE) are named by ADDRESS_OF. */
#define PROG20   ((word)K_PROG20)
#define PIKUP20  ((word)K_PIKUP20)
#define P20R52JB ((word)K_P20R52JB)
/* Part 2, P22 (bank 30). Its erasable words (LANDMARK, MARKINDX, CXOFF,
 * 22SUBSCL, 8KK) are named by ADDRESS_OF. */
#define PROG22   ((word)K_PROG22)
#define PROG22A  ((word)K_PROG22A)
#define CALLR52  ((word)K_CALLR52)
#define PROG22C  ((word)K_PROG22C)
#define P22OVER  ((word)K_P22OVER)
#define S22LSITE ((word)K_S22LSITE)
#define P22SUBRB ((word)K_P22SUBRB)
#define N89ERRX  ((word)K_N89ERRX)
#define S22DSPPA ((word)K_S22DSPPA)
#define S22DSPP  ((word)K_S22DSPP)
#define S22ABDE  ((word)K_S22ABDE)
#define DE_GR_50 ((word)K_DE_GR_50) /* DE-GR-50 */
#define V06N45B  ((word)KW_V06N45B) /* VN 0645 */
#define V06N89B  ((word)KW_V06N89B) /* VN 0689 */
/* Part 3, R57 and R21/R23 for the CSM (bank 37). Their erasable words
 * (EXTVBACT, MRKBUF1, MRKBUF2, TRUNBIAS) are named by ADDRESS_OF. */
#define R57            ((word)K_R57)
#define R57A           ((word)K_R57A)
#define MARKDISP       ((word)K_MARKDISP)
#define R57B           ((word)K_R57B)
#define ENDR57         ((word)K_ENDR57)
#define P23ABRT        ((word)K_P23ABRT)
#define R21CSM         ((word)K_R21CSM)
#define R23CSM         ((word)K_R23CSM)
#define R23CSM1        ((word)K_R23CSM1)
#define R21C1          ((word)K_R21C1)
#define R21CSM1        ((word)K_R21CSM1)
#define R21END         ((word)K_R21END)
#define V59NB          ((word)KW_V59NB)    /* VN 5900 */
#define V06N87NB       ((word)KW_V06N87NB) /* VN 0687 */
#define V51NB          ((word)KW_V51NB)    /* VN 5100 */
#define V53            ((word)KW_V53)      /* VN 5300 */
#define V0694          ((word)KW_V0694)    /* VN 0694 */
#define NINETEEN_77DEG ((word)KW_19_77DEG) /* 19.77DEG: OCT 61740 */
/* Part 4, R61CSM (bank 37), BVECTORS and GETUM (bank 23). GENRET, SAVBNK
 * and R61CNTR are named by ADDRESS_OF. */
#define R61CSM   ((word)K_R61CSM)
#define R61C1    ((word)K_R61C1)
#define R61END   ((word)K_R61END)
#define DECRM61  ((word)K_DECRM61)
#define BVECTORS ((word)K_BVECTORS)
#define GETUM    ((word)K_GETUM)
/* Part 5, R22 up to REND12 (bank 34). */
#define R22     ((word)K_R22)
#define REND1A  ((word)K_REND1A)
#define REND2   ((word)K_REND2)
#define REND3A  ((word)K_REND3A)
#define REND3   ((word)K_REND3)
#define REND4   ((word)K_REND4)
#define REND6   ((word)K_REND6)
#define REND9   ((word)K_REND9)
#define REND10  ((word)K_REND10)
#define REND10A ((word)K_REND10A)
#define NEG3    ((word)KW_NEG3)
/* Part 6, R22 from REND12 (bank 34) and GOTOV56 (fixed-fixed). N49DISP and
 * TEMPOR1 are named by ADDRESS_OF. */
#define REND12   ((word)K_REND12)
#define REND13   ((word)K_REND13)
#define CSMUPP   ((word)K_CSMUPP)
#define REND14   ((word)K_REND14)
#define REND14A  ((word)K_REND14A)
#define REND15   ((word)K_REND15)
#define REND16   ((word)K_REND16)
#define REND17   ((word)K_REND17)
#define REND18   ((word)K_REND18)
#define CMPOS    ((word)K_CMPOS)
#define LMPOS    ((word)K_LMPOS)
#define RENDISP  ((word)K_RENDISP)
#define RENDISP7 ((word)K_RENDISP7)
#define RENDISP2 ((word)K_RENDISP2)
#define RENDISP3 ((word)K_RENDISP3)
#define RENDISP4 ((word)K_RENDISP4)
#define SOURCBIT ((word)KW_SOURCBIT)
#define BUFBANK  ((word)KW_BUFBANK) /* BBCON RENDISP3 */
#define V06N49   ((word)KW_V06N49)
#define PRIO27   ((word)KW_PRIO27)
static const word VB56CADR[2] = KW_VB56CADR; /* 2CADR TRACKTRM */
/* Part 7, R22's subroutines (bank 27). MARKTIME and VHFRANGE are named by
 * ADDRESS_OF. */
#define SETINTG  ((word)K_SETINTG)
#define CNTCHK   ((word)K_CNTCHK)
#define CONTCHK  ((word)K_CONTCHK)
#define REDOR22  ((word)K_REDOR22)
#define WAITONE  ((word)K_WAITONE)
#define RANGERD  ((word)K_RANGERD)
#define RANGERD1 ((word)K_RANGERD1)
#define RANGERD2 ((word)K_RANGERD2)
#define RANGERD3 ((word)K_RANGERD3)
#define RANGERD4 ((word)K_RANGERD4)
#define LIGHTON  ((word)K_LIGHTON)
#define BYPASS   ((word)K_BYPASS)
#define BYPASS1  ((word)K_BYPASS1)
#define VHFGOOD  ((word)K_VHFGOOD)
#define SHIFTNDX ((word)K_SHIFTNDX)
#define SHIFTA   ((word)K_SHIFTA)
#define INITIALW ((word)K_INITIALW)
#define INITA    ((word)K_INITA)
#define INITB    ((word)K_INITB)
#define OCT17    ((word)KW_OCT17)
#define OCT11    ((word)KW_OCT11)
#define UPDATBIT ((word)KW_UPDATBIT)

/* Part 8, CRS61.1 and R63 (bank 34). DTHETASM and SAVEDCDU are named by
 * ADDRESS_OF. */
#define CRS61_1  ((word)K_CRS61_1)
#define AUTOCK   ((word)K_AUTOCK)
#define DAPCK    ((word)K_DAPCK)
#define ASET     ((word)K_ASET)
#define STEP2CK  ((word)K_STEP2CK)
#define CDULOOP  ((word)K_CDULOOP)
#define STKTEST  ((word)K_STKTEST)
#define STEP3CK  ((word)K_STEP3CK)
#define CRS61_2A ((word)K_CRS61_2A)
#define MANUEXIS ((word)K_MANUEXIS)
#define MANUEXIT ((word)K_MANUEXIT)
#define R63      ((word)K_R63)
#define STIKBIT  ((word)KW_STIKBIT)
#define FURST3   ((word)KW_FURST3)

/* Part 9, the start of S22.1 (bank 30) and its subroutines (bank 31). */
#define S22_1    ((word)K_S22_1)
#define S2231X13 ((word)K_S2231X13)

/* Part 11, S22.1's next mark and the incorporation (bank 30). */
#define S22I_EQ_N ((word)K_S22I_EQ_N)
#define S22F244X  ((word)K_S22F244X)
#define S22EXEX   ((word)K_S22EXEX)
#define V06N49EE  ((word)KW_V06N49EE) /* VN 0649 */

/* Part 12, S22.1's end: the landmark's position shown (bank 30). */
#define S22_981X ((word)K_S22_981X)
#define S22GTP   ((word)K_S22GTP)

/* Part 13, the W matrix from nine dimensions to six (bank 30). */
#define NINE_DWTO6DW ((word)K_9DWTO6DW) /* 9DWTO6DW */
#define ROWDOT       ((word)K_ROWDOT)

/* Part 14, V89 (bank 34). Its erasable words are named by ADDRESS_OF:
 * OPTION1 (+1 the axis chosen). */
#define V89CALL ((word)K_V89CALL)
#define V89RECL ((word)K_V89RECL)
#define SETPAF  ((word)K_SETPAF)
#define VB04N06 ((word)KW_VB04N06) /* VN 0406 */
#define VB06N18 ((word)KW_VB06N18) /* VN 0618 */

/* Part 15, P23's start (bank 31). Its erasable words are named by
 * ADDRESS_OF: LANDMARK, HORIZON and STARCODE (the noun 70 entries),
 * PHSPRDT2, TEMPFLSH. */
#define P23        ((word)K_P23)
#define P23_00     ((word)K_P23_00)
#define P23_06     ((word)K_P23_06)
#define P23_15     ((word)K_P23_15)
#define P23_151    ((word)K_P23_151)
#define P23_152    ((word)K_P23_152)
#define P23_16     ((word)K_P23_16)
#define P23_17     ((word)K_P23_17)
#define P23_176    ((word)K_P23_176)
#define P23_18     ((word)K_P23_18)
#define P23_20     ((word)K_P23_20)
#define P23_30     ((word)K_P23_30)
#define P23_55     ((word)K_P23_55)
#define V94ENTER   ((word)K_V94ENTER)
#define R60CALL    ((word)K_R60CALL)
#define R23_10     ((word)K_R23_10)
#define V05N70     ((word)KW_V05N70)  /* VN 0570 */
#define V50N25P    ((word)KW_V50N25P) /* OCT 00202 */
#define R60ADRS    ((word)KW_R60ADRS) /* CADR R60CALL +3 */
#define PRIO13     ((word)KW_PRIO13)
#define NEG100     ((word)KW_NEG100)  /* OCT 77677 */
#define NEG37      ((word)KW_NEG37)   /* DEC -37 B-14 */
#define BITS7_9    ((word)KW_BITS7_9) /* BITS7-9: OCT 700 */
#define BITS4_TO_6 ((word)KW_BITS4TO6) /* BITS4-6: OCT 70 */
/* Part 16, P23's marks and POINTAXS (bank 31). MARKSTAT, MARKDATA,
 * MARKTIME and TRUNION are named by ADDRESS_OF. */
#define P23_60 ((word)K_P23_60)
#define P23_65 ((word)K_P23_65)
#define P23_85 ((word)K_P23_85)
#define R23_2  ((word)K_R23_2)
#define V05N71 ((word)KW_V05N71) /* VN 0571 */
/* Part 17, R23.55 to R23.END, the measurement's incorporation (bank 31).
 * TRUNION and MPAC are named by ADDRESS_OF. */
#define R23_55  ((word)K_R23_55)
#define R23_6   ((word)K_R23_6)
#define R23_7   ((word)K_R23_7)
#define R23_END ((word)K_R23_END)
#define V6N49   ((word)KW_V6N49)   /* VN 0649 */
#define VARSUBL ((word)KW_VARSUBL) /* DEC 0 */

/* Part 18, HORIZ, DIVIDE and RECT.1 (bank 31). */
#define HORIZ  ((word)K_HORIZ)
#define DIVIDE ((word)K_DIVIDE)
#define RECT_1 ((word)K_RECT_1)

/* Part 19, LOWMEMRY and LDPLANET (banks 14 and 16), and V67 (bank 40). The
 * V67 words (WWPOS, WWVEL and WWOPT) are named by ADDRESS_OF. */
#define LOWMEMRY ((word)K_LOWMEMRY)
#define LDPLANET ((word)K_LDPLANET)
#define VNPLAN23 ((word)KW_VNPLAN23) /* VN 0688 */
#define V67CALL  ((word)K_V67CALL)
#define V06N99DS ((word)K_V06N99DS)
#define V06N9933 ((word)K_V06N9933)
#define V67WW    ((word)K_V67WW)
#define V67XXX   ((word)K_V67XXX)
#define V06N99A  ((word)KW_V06N99A) /* VN 0699 */
#endif

/* P30_P37.agc (LM) and P30-P37.agc (CM): P30, the external delta-V
 * targeting program, and S30.1, its computation. VNPOOH, the display
 * routine P30 calls, is P34-35_P74-75.agc's. */
#define P30     ((word)K_P30)
#define PARAM30 ((word)K_PARAM30)
#define S30_1   ((word)K_S30_1) /* S30.1 */
#define VNPOOH  ((word)K_VNPOOH)
#define V06N33  ((word)KW_V06N33) /* VN 0633 */
#define V06N42  ((word)KW_V06N42) /* VN 0642 */
#define V06N81  ((word)KW_V06N81) /* VN 0681 */
/* AVFLAGA, AVFLAGP and P20FLGON, which set the active vehicle and P20's
 * flags, and COMPTGO, which starts the clock display: the CM's in
 * P30-P37.agc, the LM's in P32-P35_P72-P75.agc. */
#define AVFLAGA  ((word)K_AVFLAGA)
#define AVFLAGP  ((word)K_AVFLAGP)
#define P20FLGON ((word)K_P20FLGON)
#define COMPTGO  ((word)K_COMPTGO)
#if LUMINARY099
#define P30N33 ((word)K_P30N33)
/* GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc (LM): P31. */
#define P31 ((word)K_P31)
#endif
/* GROUND_TRACKING_DETERMINATION_PROGRAM.agc (both): P21, and GOPERF4, the
 * please perform display it asks for the vehicle with. */
#define PROG21   ((word)K_PROG21)
#define P21PROG1 ((word)K_P21PROG1)
#define P21DSP   ((word)K_P21DSP)
#define GOPERF4  ((word)K_GOPERF4)
#define V6N34    ((word)KW_V6N34)  /* VN 00634 */
#define V06N43   ((word)KW_V06N43) /* VN 00643 */
/* P34-35_P74-75.agc (both): P34 and P74, the transfer phase initiation
 * programs; DISPLAYE, the elevation angle and central angle display; and
 * P3XORP7X, which tells P3x from P7x; P35 and P75, the mid-course
 * correction programs; KILCLOCK, N45PROC and CLUPDATE, the returns from
 * their verb 16 noun 45. Their erasable words (NORMEX, TBASE5, RTRN,
 * VERBNOUN, KT, ATIGINC, PTIGINC, QSAVED, GDT/2, DELVLVC, DISPDEX) are
 * named by ADDRESS_OF. */
#define P34      ((word)K_P34)
#define P74      ((word)K_P74)
#define P34_P74A ((word)K_P34_P74A)
#define P34_P74E ((word)K_P34_P74E)
#define DISPLAYE ((word)K_DISPLAYE)
#define P3XORP7X ((word)K_P3XORP7X)
#define P35      ((word)K_P35)
#define P75      ((word)K_P75)
#define P35_P75A ((word)K_P35_P75A) /* P35/P75A */
#define V06N58   ((word)KW_V06N58) /* VN 0658 */
#define KILCLOCK ((word)K_KILCLOCK)
#define N45PROC  ((word)K_N45PROC)
#define CLUPDATE ((word)K_CLUPDATE)
#define V06N59   ((word)KW_V06N59) /* VN 0659 */
#define V16N45   ((word)KW_V16N45) /* VN 1645 */
/* R36, the out-of-plane rendezvous display (the CM's R36A and R36P3 in
 * bank 41, after its own R36 in bank 20 asks which vehicle). */
#define R36      ((word)K_R36)
#define ASTROTIM ((word)K_ASTROTIM)
#define LREGCHK  ((word)K_LREGCHK)
#define ENTTIM2  ((word)K_ENTTIM2)
#define V06N16N  ((word)KW_V06N16N) /* VN 00616 */
#define V06N90N  ((word)KW_V06N90N) /* VN 00690 */
#if COMANCHE055
#define R36A     ((word)K_R36A)
#define R36P3    ((word)K_R36P3)
#define OPTION36 ((word)KW_OPTION36) /* VN 0412 */
#endif
/* R31, the rendezvous parameter display (verb 83): the LM's R31CALL and
 * DSPDELAY in bank 40, the rest in its bank 37; the CM's all in bank 31. */
#define R31CALL  ((word)K_R31CALL)
#define DSPDELAY ((word)K_DSPDELAY)
#define DISPN5X  ((word)K_DISPN5X)
#define V83CALL  ((word)K_V83CALL)
#define GETRVN   ((word)K_GETRVN)
#define V16N54   ((word)KW_V16N54) /* VN 1654 */
#if LUMINARY099
#define REV83   ((word)K_REV83)
#define R31SURF ((word)K_R31SURF)
#define MUNG_Q  ((word)K_MUNG_Q) /* MUNG? */
#define ONEBASE ((word)K_ONEBASE)
#else
#define V83    ((word)K_V83)
#define R34LOS ((word)K_R34LOS)
#define V16N53 ((word)KW_V16N53) /* VN 1653 */
#define PRIO3  ((word)KW_PRIO3)
#endif

/* P76, the target delta V program, in bank 13 of both missions, and its
 * two displays (V06N84, NV 0684, and the word after it, NV 0633). */
#define P76           ((word)K_P76)
#define ENDP76        ((word)K_ENDP76)
#define V06N84        ((word)KW_V06N84)   /* NV 0684 */
#define V06N84_PLUS_1 ((word)KW_V06N84P1) /* V06N84 +1: NV 0633 */

/* R30, the orbital parameters display (verb 82), the LM's in bank 22, the
 * CM's in bank 23: its labels, and its two displays (OPTIONVN, VN 0412,
 * and V16N44, VN 1644) and TFFBANK (ECADR TFF). */
#define V82CALL  ((word)K_V82CALL)
#define V82GOFF  ((word)K_V82GOFF)
#define V82GOFLP ((word)K_V82GOFLP)
#define V82STALL ((word)K_V82STALL)
#define FLAGGON  ((word)K_FLAGGON)
#define V82GOFF1 ((word)K_V82GOFF1)
#define OTHSHIP  ((word)K_OTHSHIP)
#define THISSHIP ((word)K_THISSHIP)
#define TICKTPER ((word)K_TICKTPER)
#define TICKTFF  ((word)K_TICKTFF)
#define TICKTEST ((word)K_TICKTEST)
#define DOTICK   ((word)K_DOTICK)
#define TFFTICK  ((word)K_TFFTICK)
#define TPERTICK ((word)K_TPERTICK)
#define V82GON   ((word)K_V82GON)
#define V82REDSP ((word)K_V82REDSP)
#define V82GON1  ((word)K_V82GON1)
#define V82GON3  ((word)K_V82GON3)
#define OPTIONVN ((word)KW_OPTIONVN) /* VN 0412 */
#define V16N44   ((word)KW_V16N44)   /* VN 1644 */
#define TFFBANK  ((word)KW_TFFBANK)  /* ECADR TFF */
#if !LUMINARY099
#define CHANG1  ((word)K_CHANG1) /* (the LM's is with the IMU performance tests') */
#define BOTHPAD ((word)K_BOTHPAD)
#define V82GON2 ((word)K_V82GON2)
#define SPLRET1 ((word)K_SPLRET1)
#define CANDEL  ((word)K_CANDEL)
#endif

/* P38-P39 (STABLE_ORBIT), the stable orbit programs, the LM's in bank 34,
 * the CM's in bank 4: their labels, their displays (V06N33SR ... V06N81SR,
 * VN 0633 ... VN 0681) and the LM's DECTWO (OCT 2). */
#define P38      ((word)K_P38)
#define P78      ((word)K_P78)
#define OPTN1    ((word)K_OPTN1)
#define JUNCTN1  ((word)K_JUNCTN1)
#define RECYCLE  ((word)K_RECYCLE)
#define OPTN2    ((word)K_OPTN2)
#define JUNCTN2  ((word)K_JUNCTN2)
#define MAINRTNE ((word)K_MAINRTNE)
#define TEST3979 ((word)K_TEST3979)
#define P39P79   ((word)K_P39P79)
#define MAINRTN1 ((word)K_MAINRTN1)
#define DSPLY58  ((word)K_DSPLY58)
#define DSPLY81  ((word)K_DSPLY81)
#define P39      ((word)K_P39)
#define P79      ((word)K_P79)
#define P39_P79A ((word)K_P39_P79A) /* P39/P79A */
#define P39_P79B ((word)K_P39_P79B) /* P39/P79B */
#define PREC_TT  ((word)K_PREC_TT)  /* PREC/TT */
#define INTRPVP  ((word)K_INTRPVP)
#define OTHERV   ((word)K_OTHERV)
#define VNDSPLY  ((word)K_VNDSPLY)
#define V06N33SR ((word)KW_V06N33SR) /* VN 0633 */
#define V06N55SR ((word)KW_V06N55SR) /* VN 0655 */
#define V04N06SR ((word)KW_V04N06SR) /* VN 0406 */
#define V06N57SR ((word)KW_V06N57SR) /* VN 0657 */
#define V06N34SR ((word)KW_V06N34SR) /* VN 0634 */
#define V06N58SR ((word)KW_V06N58SR) /* VN 0658 */
#define V06N81SR ((word)KW_V06N81SR) /* VN 0681 */
#if LUMINARY099
#define DECTWO ((word)KW_DECTWO) /* OCT 2 */
#endif
#if LUMINARY099
#define DEG130 ((word)K_130DEG) /* 130DEG, 2DEC .3611111111 */
#else
#define VNBANK ((word)K_VNBANK) /* VNPOOH's TC -5, its bank */
#endif
#if COMANCHE055
/* The CM's: P31, the routines P30 and P31 share, their displays (DISP45,
 * verb 16 noun 45), S31.1 and DELRSPL. */
#define DISPMGA  ((word)K_DISPMGA)
#define DISP45   ((word)K_DISP45)
#define P30PHSI  ((word)K_P30PHSI)
#define END45    ((word)K_END45)
#define P31      ((word)K_P31)
#define P30_P31  ((word)K_P30_P31) /* P30/P31 */
#define CNTNUP30 ((word)K_CNTNUP30)
#define REFTEST  ((word)K_REFTEST)
#define NOTSET   ((word)K_NOTSET)
#define FLASHMGA ((word)K_FLASHMGA)
#define MARSDP   ((word)K_MARSDP) /* OCT 00000, OCT 35100 */
#define S31_1    ((word)K_S31_1)  /* S31.1 */
#define DELRSPL  ((word)K_DELRSPL)
#define EBANK4   ((word)KW_EBANK4)
#endif

#if LUMINARY099
/* P32-P35_P72-P75.agc (LM): CSI (P32 and P72) and CDH (P33 and P73). Their
 * erasable words (RTRN, DISPDEX, CENTANG, NN, TCSI, VERBNOUN, SUBEXIT) are
 * named by ADDRESS_OF. */
#define P32      ((word)K_P32)
#define P72      ((word)K_P72)
#define P32STRT  ((word)K_P32STRT)
#define ALMXIT   ((word)K_ALMXIT)
#define P32_P72A ((word)K_P32_P72A) /* P32/P72A */
#define VN0611   ((word)K_VN0611)
#define VN0655   ((word)K_VN0655)
#define P32_P72F ((word)K_P32_P72F) /* P32/P72F */
#define P33      ((word)K_P33)
#define P73      ((word)K_P73)
#define P33_P73A ((word)K_P33_P73A) /* P33/P73A */
#define P33_P73B ((word)K_P33_P73B) /* P33/P73B */
#define P33_P73F ((word)K_P33_P73F) /* P33/P73F */
#define DISDVLVC ((word)K_DISDVLVC)
#define P30ZERO  ((word)K_P30ZERO)    /* 2DEC 0 */
#define V06N11   ((word)KW_V06N11)    /* VN 0611 */
#define V06N13   ((word)KW_V06N13)    /* VN 0613 */
#define V06N75   ((word)KW_V06N75)    /* VN 0675 */
#define V06N55   ((word)KW_V06N55)    /* VN 0655 */
#define V06N37   ((word)KW_V06N37)    /* VN 0637 */
#endif

#if COMANCHE055
/* P32-P33_P72-P73.agc (CM): CSI (P32 and P72) and CDH (P33 and P73). Their
 * erasable words (CENTANG, NN, VERBNOUN) are named by ADDRESS_OF. */
#define P32      ((word)K_P32)
#define P72      ((word)K_P72)
#define P32STRT  ((word)K_P32STRT)
#define ALMXIT   ((word)K_ALMXIT)
#define P32_P72A ((word)K_P32_P72A) /* P32/P72A */
#define P32_P72F ((word)K_P32_P72F) /* P32/P72F */
#define P33      ((word)K_P33)
#define P73      ((word)K_P73)
#define P33_P73A ((word)K_P33_P73A) /* P33/P73A */
#define P33_P73B ((word)K_P33_P73B) /* P33/P73B */
#define P33_P73F ((word)K_P33_P73F) /* P33/P73F */
#define DISDVLVC ((word)K_DISDVLVC)
#define V06N11   ((word)KW_V06N11) /* VN 0611 */
#define V06N13   ((word)KW_V06N13) /* VN 0613 */
#define V06N75   ((word)KW_V06N75) /* VN 0675 */
#define V06N55   ((word)KW_V06N55) /* VN 0655 */
#define V06N37   ((word)KW_V06N37) /* VN 0637 */
#endif

#if LUMINARY099
/* BURN,_BABY,_BURN_--_MASTER_IGNITION_ROUTINE (LM): the ignition tables,
 * the countdown, the clock and KILLTASK. Its erasable words (WHICH, DISPDEX,
 * TIG, P40/RET, ITEMP1-4, LST2 ...) are named by ADDRESS_OF. */
#define ABRTABLE               ((word)K_ABRTABLE)
#define ABRTIGN                ((word)K_ABRTIGN)
#define ASTNRETN               ((word)K_ASTNRETN)
#define ATMAGADR               ((word)K_ATMAGADR) /* 2CADR ATMAG */
#define BURNBABY               ((word)K_BURNBABY)
#define B_STAR_RNB_STAR_B_STAR ((word)K_B_STAR_RNB_STAR_B_STAR) /* B*RNB*B* */
#define CALLT_35               ((word)K_CALLT_35) /* CALLT-35 */
#define CLOKTASK               ((word)K_CLOKTASK)
#define COMFAIL                ((word)K_COMFAIL)
#define COMFAIL2               ((word)K_COMFAIL2)
#define COMFAIL3               ((word)K_COMFAIL3)
#define COMFAIL4               ((word)K_COMFAIL4)
#define COMMON                 ((word)K_COMMON)
#define D29_9SEC               ((word)K_D29_9SEC) /* D29.9SEC: 2DEC 2990 B-28 */
#define DEAD                   ((word)K_DEAD)
#define DISPNOT                ((word)K_DISPNOT)
#define DSP2CADR               ((word)K_DSP2CADR) /* 2CADR P63DISPS -2 */
#define DVMONCON               ((word)K_DVMONCON)
#define FIVE_SECDP             ((word)K_5SECDP) /* 5SECDP */
#define GOBACK                 ((word)K_GOBACK)
#define GOCUTOFF               ((word)K_GOCUTOFF)
#define GOPOST                 ((word)K_GOPOST)
#define IGNITE                 ((word)K_IGNITE)
#define IMPLBURN               ((word)K_IMPLBURN)
#define INVFLAG                ((word)K_INVFLAG)
#define LUNLANAD               ((word)K_LUNLANAD)
#define NULLCLOK               ((word)K_NULLCLOK)
#define P12TABLE               ((word)K_P12TABLE)
#define P40ALM                 ((word)K_P40ALM)
#define P40AUTO                ((word)K_P40AUTO)
#define P40A_P                 ((word)K_P40A_P) /* P40A/P */
#define P40IGN                 ((word)K_P40IGN)
#define P40SJUNK               ((word)K_P40SJUNK)
#define P40SPOT                ((word)K_P40SPOT)
#define P40TABLE               ((word)K_P40TABLE)
#define P40ZOOMA               ((word)K_P40ZOOMA)
#define P41BLANK               ((word)K_P41BLANK)
#define P41SPOT                ((word)K_P41SPOT)
#define P41TABLE               ((word)K_P41TABLE)
#define P42TABLE               ((word)K_P42TABLE)
#define P63IGN                 ((word)K_P63IGN)
#define P63IGN1                ((word)K_P63IGN1)
#define P63TABLE               ((word)K_P63TABLE)
#define P63ZOOM                ((word)K_P63ZOOM)
#define REDO4_2                ((word)K_REDO4_2) /* REDO4.2 */
#define REP40ALM               ((word)K_REP40ALM)
#define STAR_PROCEED           ((word)K_STAR_PROCEED) /* *PROCEED */
#define STCLOK3                ((word)K_STCLOK3)
#define STOPCLOK               ((word)K_STOPCLOK)
#define TIGTASK                ((word)K_TIGTASK)
#define TIGTASK1               ((word)K_TIGTASK1)
#define TIG_0                  ((word)K_TIG_0) /* TIG-0 */
#define TIG_30                 ((word)K_TIG_30) /* TIG-30 */
#define TIG_30A                ((word)K_TIG_30A) /* TIG-30A */
#define TIG_30_1               ((word)K_TIG_30_1) /* TIG-30.1 */
#define TIG_35                 ((word)K_TIG_35) /* TIG-35 */
#define TIG_5                  ((word)K_TIG_5) /* TIG-5 */
#define TURNITON               ((word)K_TURNITON)
#define ULLAGOFF               ((word)K_ULLAGOFF)
#define ULLGTASK               ((word)K_ULLGTASK)
#define ASTNBIT                ((word)KW_ASTNBIT)
#define BLANKDEX               ((word)KW_BLANKDEX)
#define CNTDNDEX               ((word)KW_CNTDNDEX)
#define DRIFTBIT               ((word)KW_DRIFTBIT)
#define FIVE_SEC               ((word)KW_5SEC) /* 5SEC: DEC 500 */
#define FLUNDBIT               ((word)KW_FLUNDBIT)
#define FOUR_9SEC              ((word)KW_4_9SEC) /* 4.9SEC: DEC 490 B-14 */
#define IGNFLBIT               ((word)KW_IGNFLBIT)
#define IMPULBIT               ((word)KW_IMPULBIT)
#define KILLBB                 ((word)KW_KILLBB) /* BBCON KILLTSK2 */
#define LETABBIT               ((word)KW_LETABBIT)
#define LSTLIM                 ((word)KW_LSTLIM) /* EQUALS BIT5 */
#define NOTHRBIT               ((word)KW_NOTHRBIT)
#define P40A_PMD               ((word)KW_P40A_PMD) /* P40A/PMD: OCT 00203 */
#define PRIO13                 ((word)KW_PRIO13)
#define PRIO17                 ((word)KW_PRIO17)
#define S24_9SEC               ((word)KW_S24_9SEC) /* DEC 2490 B-14 */
#define SWANDBIT               ((word)KW_SWANDBIT)
#define TCTSKOVR               ((word)KW_TCTSKOVR)
#define ULLAGER                ((word)KW_ULLAGER)
#define V06N61                 ((word)KW_V06N61) /* VN 0661 */
#define V16N85B                ((word)KW_V16N85B)
#define VB97DEX                ((word)KW_VB97DEX)
#define VB99DEX                ((word)KW_VB99DEX)
#endif

#if LUMINARY099
/* P40-P47 (LM): P40, P41, P42 and P47, the burn programs; the engine off
 * routines and STEERING. Its erasable words (WHICH, DVTHRUSH, DVCNTR,
 * DISPDEX, AVEGEXIT, TRKMKCNT, TGO, PHASE2 ...) are named by ADDRESS_OF. */
#define ACADN83          ((word)K_ACADN83) /* 2CADR CALCN83 */
#define ACADN85          ((word)K_ACADN85) /* = P41TABLE +2 */
#define ALARMIT          ((word)K_ALARMIT)
#define ALLCOAST         ((word)K_ALLCOAST)
#define BLNKWAIT         ((word)K_BLNKWAIT)
#define CALCN83          ((word)K_CALCN83)
#define CALCN85          ((word)K_CALCN85)
#define COASTSET         ((word)K_COASTSET)
#define DYNMDISP         ((word)K_DYNMDISP)
#define ENDSTEER         ((word)K_ENDSTEER)
#define ENGINOF1         ((word)K_ENGINOF1)
#define ENGINOF2         ((word)K_ENGINOF2)
#define ENGINOF3         ((word)K_ENGINOF3)
#define ENGINOF4         ((word)K_ENGINOF4)
#define ENGINOFF         ((word)K_ENGINOFF)
#define ENGOFTSK         ((word)K_ENGOFTSK)
#define GETDT            ((word)K_GETDT)
#define GODSPRET         ((word)K_GODSPRET)
#define GOFLASHR         ((word)K_GOFLASHR)
#define NO_9             ((word)K_NO_9) /* NO.9 */
#define NOULLAGE         ((word)K_NOULLAGE)
#define NSTEER           ((word)K_NSTEER)
#define ONE_EIGHTY_SETUP ((word)K_180SETUP) /* 180SETUP */
#define P40LM            ((word)K_P40LM)
#define P40PHS1          ((word)K_P40PHS1)
#define P40SXT4          ((word)K_P40SXT4)
#define P41LM            ((word)K_P41LM)
#define P42LM            ((word)K_P42LM)
#define P42STAGE         ((word)K_P42STAGE)
#define P47BOD           ((word)K_P47BOD)
#define P47BODY          ((word)K_P47BODY)
#define P47LM            ((word)K_P47LM)
#define PFLITEDB         ((word)K_PFLITEDB)
#define POSTBURN         ((word)K_POSTBURN)
#define R02BOTH          ((word)K_R02BOTH)
#define REDO6_7          ((word)K_REDO6_7) /* REDO6.7 */
#define REFLASHR         ((word)K_REFLASHR)
#define RESTORDB         ((word)K_RESTORDB)
#define S40_1            ((word)K_S40_1)   /* S40.1 */
#define S40_2_3          ((word)K_S40_2_3) /* S40.2,3 */
#define S40_8            ((word)K_S40_8)   /* S40.8 */
#define S40_9            ((word)K_S40_9)   /* S40.9 */
#define S40_13           ((word)K_S40_13)  /* S40.13 */
#define S40_130V         ((word)K_S40_130V)   /* S40.130V */
#define S40_132_STAR     ((word)K_S40_132_STAR) /* S40.132* */
#define S40_134          ((word)K_S40_134) /* S40.134 */
#define ENDS40_9         ((word)K_ENDS40_9) /* ENDS40.9 */
#define STORETGO         ((word)K_STORETGO)
#define TRIMGIMB         ((word)K_TRIMGIMB)
#define RASTEER1         ((word)K_RASTEER1)
#define S41_1            ((word)K_S41_1) /* S41.1 */
#define ROLLOVER         ((word)K_ROLLOVER)
#define PITCHOFF         ((word)K_PITCHOFF)
#define SERVCADR         ((word)K_SERVCADR) /* = P63TABLE +7 */
#define SERVEXIT         ((word)K_SERVEXIT)
#define SETMINDB         ((word)K_SETMINDB)
#define STARTP47         ((word)K_STARTP47)
#define STEERING         ((word)K_STEERING)
#define TERM40           ((word)K_TERM40)
#define TIGNOW           ((word)K_TIGNOW)
#define UPDATEVG         ((word)K_UPDATEVG)
#define ZATTEROR         ((word)K_ZATTEROR)
#define GMBDRBIT         ((word)KW_GMBDRBIT) /* = BIT10 */
#define HIRTHROT         ((word)KW_HIRTHROT) /* = BIT13 */
#define IDLEFBIT         ((word)KW_IDLEFBIT) /* = BIT7 */
#define P40ADRES         ((word)KW_P40ADRES) /* ADRES P40TABLE */
#define P41ADRES         ((word)KW_P41ADRES) /* ADRES P41TABLE -5 */
#define P42ADRES         ((word)KW_P42ADRES) /* ADRES P42TABLE */
#define THRESH1          ((word)KW_THRESH1)  /* DEC 24 B-14 */
#define THRESH2          ((word)KW_THRESH2)  /* DEC 308 B-14 */
#define THRESH3          ((word)KW_THRESH3)  /* DEC 12 B-14 */
#define USEQRJTS         ((word)KW_USEQRJTS) /* = BIT14 */
#define V1683            ((word)KW_V1683)    /* VN 1683 */
#define V16N40           ((word)KW_V16N40)   /* VN 1640 */
#endif

#if LUMINARY099
/* THE_LUNAR_LANDING (LM): P63's set-up and ignition algorithm, and P68's
 * LANDJUNK. Its erasable words (WHICH, DVTHRUSH, DVCNTR, WCHPHASE, FLPASS0,
 * NIGNLOOP) are named by ADDRESS_OF. */
#define ASTNCLOK    ((word)K_ASTNCLOK)
#define ASTNDEX     ((word)KW_MD1) /* = MD1: OCT 25, CLOKTASK's index */
#define ASTNRET     ((word)K_ASTNRET)
#define CODE500     ((word)KW_CODE500) /* OCT 00500 */
#define DDUMCALC    ((word)K_DDUMCALC)
#define DDUMGOOD    ((word)K_DDUMGOOD)
#define DPSTHRSH    ((word)KW_DPSTHRSH) /* DEC 36 */
#define FLAGORGY    ((word)K_FLAGORGY)
#define IGNALG      ((word)K_IGNALG)
#define IGNALGRT    ((word)K_IGNALGRT)
#define IGNALOOP    ((word)K_IGNALOOP)
#define LANDJUNK    ((word)K_LANDJUNK)
#define P63ADRES    ((word)KW_P63ADRES) /* GENADR P63TABLE */
#define P63LM       ((word)K_P63LM)
#define P63SPOT2    ((word)K_P63SPOT2)
#define P63SPOT3    ((word)K_P63SPOT3)
#define P63SPOT4    ((word)K_P63SPOT4)
#define R60INIT     ((word)K_R60INIT)
#define V06N43_STAR ((word)KW_V06N43_STAR) /* V06N43*: VN 0643 */
#endif

#if LUMINARY099
/* THROTTLE_CONTROL_ROUTINES (LM), and LUNAR_LANDING_GUIDANCE_EQUATIONS'
 * FASTCHNG. Their erasable words (ABDELV, RTNHOLD, PIFPSET, FWEIGHT, PIF,
 * PSEUDO55, FC, TTHROT, FCOLD, /AFC/, FCODD, FP, LOWCRIT, L*WCR*T,
 * H*GHCR*T, THISTPIP, PIPTIME, MASS, THRUST, PHSNAME3) are named by
 * ADDRESS_OF. */
#define AFDUMP        ((word)K_AFDUMP)
#define DOPIF         ((word)K_DOPIF)
#define FASTCHNG      ((word)K_FASTCHNG)
#define FLATOUT       ((word)K_FLATOUT)
#define MASSMULT      ((word)K_MASSMULT)
#define THROTTLE      ((word)K_THROTTLE)
#define WHATOUT       ((word)K_WHATOUT)
#define DEC66         ((word)KW_DEC66)     /* DEC 66 */
#define FEXTRA        ((word)KW_FEXTRA)    /* = BIT13 (FEXT) */
#define FMAXODD       ((word)KW_FMAXODD)   /* DEC +3841 (FSAT) */
#define FMAXPOS       ((word)KW_FMAXPOS)   /* DEC +3467 (FMAX) */
#define SLASH_AF_CNST ((word)KW__AF_CNST)  /* /AF/CNST: DEC .13107 */
#define THREE_SECS    ((word)KW_3SECS)     /* 3SECS: DEC 300 */
#define THROTLAG      ((word)KW_THROTLAG)  /* DEC +20 (TAU (TH)) */
#endif

#if LUMINARY099
/* LUNAR_LANDING_GUIDANCE_EQUATIONS (LM). Its erasable words
 * (ELVIRA, ZERLINA, AZINCR1, ELINCR1, RODCOUNT, RETROOT, PWRPTR, DERPTR,
 * PWRCNT, ROOTPS, DXCRIT, RODSCAL1, VDGVERT, OLDPIPAX, PIPATMPX, TEMX,
 * DELVROD ...) are named by ADDRESS_OF. */
#define AFCCLEND ((word)K_AFCCLEND)
#define ALM1406  ((word)K_1406ALM) /* 1406ALM */
#define AFTRGUID ((word)K_AFTRGUID)
#define CALCRGVG ((word)K_CALCRGVG)
#define CGCALC   ((word)K_CGCALC)
#define CHAN31   031u
#define DERCLOOP ((word)K_DERCLOOP)
#define DISPCOMN ((word)K_DISPCOMN)
#define DISPEXIT ((word)K_DISPEXIT)
#define EXBRAK   ((word)K_EXBRAK)
#define EXGSUB   ((word)K_EXGSUB)
#define EXNORM   ((word)K_EXNORM)
#define EXOVFLOW ((word)K_EXOVFLOW)
#define EXSPOT1  ((word)K_EXSPOT1)
#define EXTLOGIC ((word)K_EXTLOGIC)
#define GDUMP1   ((word)K_GDUMP1)
#define LUNLAND  ((word)K_LUNLAND)
#define P63DISPS ((word)K_P63DISPS)
#define P64CEED  ((word)K_P64CEED)
#define P64DISPS ((word)K_P64DISPS)
#define P65VERT  ((word)K_P65VERT)
#define P66VERT  ((word)K_P66VERT)
#define P67VERT  ((word)K_P67VERT)
#define QUADGUID ((word)K_QUADGUID)
#define RATESTOP ((word)K_RATESTOP)
#define RED_OVER ((word)K_RED_OVER) /* RED-OVER */
#define REDES_OK ((word)K_REDES_OK) /* REDES-OK */
#define WHATDISP ((word)K_WHATDISP)
#define WHATEXIT ((word)K_WHATEXIT)
#define DMPNSUB  ((word)K_DMPNSUB)
#define INTPRETX ((word)K_INTPRETX)
#define ITRPNT1  ((word)K_ITRPNT1)
#define ITRPNT2  ((word)K_ITRPNT2)
#define P66VERTA ((word)K_P66VERTA)
#define POWRSERS ((word)K_POWRSERS)
#define PREMON2  ((word)K_PREMON2)
#define REDESMON ((word)K_REDESMON)
#define RESETRPT ((word)K_RESETRPT)
#define RODCOMP  ((word)K_RODCOMP)
#define RODTASK  ((word)K_RODTASK)
#define ROOTLOOP ((word)K_ROOTLOOP)
#define ROOTPSRS ((word)K_ROOTPSRS)
#define SHORTMP  ((word)K_SHORTMP)
#define TARGTDEX ((word)K_TARGTDEX)
#define VHORCOMP ((word)K_VHORCOMP)
#define BRSPOT1   ((word)K_BRSPOT1)
#define BRSPOT2   ((word)K_BRSPOT2)
#define BRSPOT3   ((word)K_BRSPOT3)
#define GUIDSUB   ((word)K_GUIDSUB)
#define GUILDEN   ((word)K_GUILDEN)
#define GUILDRET  ((word)K_GUILDRET)
#define NEWPHASE  ((word)K_NEWPHASE)
#define P00_1406  ((word)K_1406P00) /* 1406P00 */
#define P65START  ((word)K_P65START)
#define P66NOW_Q  ((word)K_P66NOW_Q) /* P66NOW? */
#define P67NOW_Q  ((word)K_P67NOW_Q) /* P67NOW? */
#define PREGUIDE  ((word)K_PREGUIDE)
#define Q_GUIDSUB ((word)K__QGUIDSUB) /* ?GUIDSUB */
#define REDESIG   ((word)K_REDESIG)
#define RESTART_Q ((word)K_RESTART_Q) /* RESTART? */
#define RGVGCALC  ((word)K_RGVGCALC)
#define SPARCSIN  ((word)K_SPARCSIN)
#define STABL_Q   ((word)K_STABL_Q) /* STABL? */
#define STARTP64  ((word)K_STARTP64)
#define STARTP66  ((word)K_STARTP66)
#define STARTP67  ((word)K_STARTP67)
#define STRTP66A  ((word)K_STRTP66A)
#define TDISPSET  ((word)K_TDISPSET)
#define TTF_8CL   ((word)K_TTF_8CL) /* TTF/8CL */
#define TTFINCR   ((word)K_TTFINCR)
#define VRTSTART  ((word)K_VRTSTART)
#define WHATALM   ((word)K_WHATALM)
#define WHATGUID  ((word)K_WHATGUID)
#define ALL4BITS ((word)KW_ALL4BITS) /* OCT 00063 */
#define AZEACH   ((word)KW_AZEACH)   /* DEC .03491: "2 DEGREES" */
#define DERTABLL ((word)KW_DERTABLL) /* ADRES DERCOFN -3 */
#define ELEACH   ((word)KW_ELEACH)   /* DEC .00873: "1/2 DEGREE" */
#define PROJMAX  ((word)KW_PROJMAX)  /* DEC .42262 B-3: "SIN(25')/8" */
#define PROJMIN  ((word)KW_PROJMIN)  /* DEC .25882 B-3: "SIN(15')/8" */
#define REDFLBIT ((word)KW_REDFLBIT) /* REDFLAG's bit (FLAGWRD6) */
#define STEERBIT ((word)KW_STEERBIT) /* STEERSW's bit (FLAGWRD2) */
#define V06N60   ((word)KW_V06N60)   /* VN 0660: "P65, P66, P67" */
#define V06N63   ((word)KW_V06N63)   /* VN 0663: "P63" */
#define V06N64   ((word)KW_V06N64)   /* VN 0664: "P64" */
#define HALF_DEG        ((word)KW_1_2DEG)   /* 1/2DEG: DEC +.00278 */
#define MINUS_DEC103    ((word)KW_MDEC103)  /* -DEC103: DEC -103 */
#define ONE_EIGHTY_DEGS ((word)KW_180DEGS)  /* 180DEGS: DEC +180 */
#define PLUS_DEC99      ((word)KW_PDEC99)   /* +DEC99: DEC +99 */
#define TABLTTFL        ((word)KW_TABLTTFL) /* ADRES TABLTTF +3 */
#define TREDESCL        ((word)KW_TREDESCL) /* DEC -.08 */
#define TSCALINV        ((word)KW_TSCALINV) /* = BIT4 */

/* P70-P71 (LM): the aborts, P70 on the descent engine and P71 on the
 * ascent engine, and R10,R11's abort monitor. Its erasable words by
 * ADDRESS_OF (PIPCTR, PIPCTR1, TGO1 and the like). */
#define ABORTALM  ((word)K_ABORTALM)
#define ABRTJASK  ((word)K_ABRTJASK)
#define BOTHPOLY  ((word)K_BOTHPOLY)
#define CONTABRT  ((word)K_CONTABRT)
#define FLASHH_Q  ((word)K_FLASHH_Q) /* FLASHH? */
#define FLASHV_Q  ((word)K_FLASHV_Q) /* FLASHV? */
#define GOABORT   ((word)K_GOABORT)
#define GRP4OFF   ((word)K_GRP4OFF)
#define INJTARG   ((word)K_INJTARG)
#define LANDISP   ((word)K_LANDISP)
#define LEGAL_Q   ((word)K_LEGAL_Q) /* LEGAL? */
#define LRHTASK   ((word)K_LRHTASK)
#define MODE70    ((word)K_MODE70) /* DEC 70, then OCTAL27 and MODE71 (DEC 71) */
#define OLDTIME   ((word)K_OLDTIME)
#define P70       ((word)K_P70)
#define P70A      ((word)K_P70A)
#define P70INIT   ((word)K_P70INIT)
#define P70NOW_Q  ((word)K_P70NOW_Q) /* P70NOW? */
#define P71       ((word)K_P71)
#define P71A      ((word)K_P71A)
#define P71NOW_Q  ((word)K_P71NOW_Q) /* P71NOW? */
#define P71RET    ((word)K_P71RET)
#define R10_R11   ((word)K_R10_R11)  /* R10,R11 */
#define R10_R11A  ((word)K_R10_R11A) /* R10,R11A */
#define TEN_11    ((word)K_10_11)    /* 10,11 */
#define TGOCOMP   ((word)K_TGOCOMP)
#define THROTUP   ((word)K_THROTUP)
#define UPRATE    ((word)K_UPRATE)
#define UPTHROT   ((word)K_UPTHROT)
#define UPTHROT1  ((word)K_UPTHROT1)
#define YOK       ((word)K_YOK)
#define ABRTJADR  ((word)KW_ABRTJADR) /* TCF ABRTJASK */
#define DAPBITS   ((word)KW_DAPBITS)  /* OCT 00640: bits 6, 8 and 9 */
#define HLITE     ((word)KW_HLITE)    /* = BIT5 */
#define OCT37774  ((word)KW_OCT37774)
#define OCTAL27   ((word)KW_OCTAL27)  /* OCT 27 */
#define ONE_DEC70 ((word)KW_1DEC70)   /* 1DEC70: DEC 70 */
#define ONE_DEC71 ((word)KW_1DEC71)   /* 1DEC71: DEC 71 */
#define R10FLBIT  ((word)KW_R10FLBIT) /* R10FLAG's bit (FLAGWRD0) */
#define VFLSHBIT  ((word)KW_VFLSHBIT) /* = BIT2 */
#define VLITE     ((word)KW_VLITE)    /* = BIT3 */
#define WHICHADR  ((word)KW_WHICHADR) /* REMADR ABRTABLE */
static const word ATMAGAD[2] = KW_ATMAGAD;   /* 2CADR ATMAG */
static const word CNTABTAD[2] = KW_CNTABTAD; /* 2CADR CONTABRT */
static const word SVEXITAD[2] = KW_SVEXITAD; /* 2CADR SERVEXIT */
/* P12.agc */
#define GUIDINIT ((word)K_GUIDINIT)
#define NEWLOAD  ((word)K_NEWLOAD)
#define P12INIT  ((word)K_P12INIT)
#define P12LM    ((word)K_P12LM)
#define P12ADRES ((word)KW_P12ADRES) /* REMADR P12TABLE */
#define V06N33A  ((word)KW_V06N33A)  /* VN 0633 */
#define V06N76   ((word)KW_V06N76)   /* VN 0676 */

/* ASCENT_GUIDANCE.agc */
#define ANG1CHEK    ((word)K_ANG1CHEK)
#define ASCENT      ((word)K_ASCENT)
#define ASCTERM     ((word)K_ASCTERM)
#define ASCTERM1    ((word)K_ASCTERM1)
#define ASCTERM2    ((word)K_ASCTERM2)
#define ASCTERM4    ((word)K_ASCTERM4)
#define ATMAG       ((word)K_ATMAG)
#define CHECKALT    ((word)K_CHECKALT)
#define CHECKYAW    ((word)K_CHECKYAW)
#define CUTOFF      ((word)K_CUTOFF)
#define CUTOFF1     ((word)K_CUTOFF1)
#define CUTOFF2     ((word)K_CUTOFF2)
#define ENGOFF      ((word)K_ENGOFF)
#define ENGOFF1     ((word)K_ENGOFF1)
#define KEEPVR      ((word)K_KEEPVR)
#define LOGSUB      ((word)K_LOGSUB)
#define TERMASC     ((word)K_TERMASC)
#define YCOMP       ((word)K_YCOMP)
#define FLRCSBIT    ((word)KW_FLRCSBIT)
#define V06N63_STAR ((word)KW_V06N63_STAR) /* V06N63*: VN 0663 */
#define V16N63      ((word)KW_V16N63)      /* VN 1663 */
#define V16N85C     ((word)KW_V16N85C)     /* VN 1685 */
static const word CLOG2_32[2] = KW_CLOG2_32; /* CLOG2/32: 2DEC .0216608494 */

/* SERVICER.agc */
#define ASCSPOT  ((word)K_ASCSPOT)
#define AVEGOUT  ((word)K_AVEGOUT)
#define AVERAGEG ((word)K_AVERAGEG)
#define AVERTRN  ((word)K_AVERTRN)
#define AVGEND   ((word)K_AVGEND)
#define BIBIBIAS ((word)K_BIBIBIAS)
#define CHKTEMX  ((word)K_CHKTEMX)
#define COPYCYC  ((word)K_COPYCYC)
#define COPYCYCL ((word)K_COPYCYCL)
#define DECCNTR  ((word)K_DECCNTR)
#define DODELVZ  ((word)K_DODELVZ)
#define EIGHTEEN ((word)KW_EIGHTEEN) /* DEC 18 */
#define DVCNTSET ((word)K_DVCNTSET)
#define DVMON    ((word)K_DVMON)
#define GETABVAL ((word)K_GETABVAL)
#define GNUFAZE5 ((word)K_GNUFAZE5)
#define GNUTFAZ5 ((word)K_GNUTFAZ5)
#define GOREADAX ((word)K_GOREADAX)
#define GOSERV   ((word)K_GOSERV)
#define GRP2OFF  ((word)K_GRP2OFF)
#define LOTHRUST ((word)K_LOTHRUST)
#define MAKEACCS ((word)K_MAKEACCS)
#define MASSMON  ((word)K_MASSMON)
#define MOONSPOT ((word)K_MOONSPOT)
#define NODVMON1 ((word)K_NODVMON1)
#define NODVMON2 ((word)K_NODVMON2)
#define NORMLIZ1 ((word)K_NORMLIZ1)
#define NORMLIZ2 ((word)K_NORMLIZ2)
#define NORMLIZE ((word)K_NORMLIZE)
#define PIPASR   ((word)K_PIPASR)
#define PIPSDONE ((word)K_PIPSDONE)
#define PRIO32   ((word)KW_PRIO32)
#define PREREAD  ((word)K_PREREAD)
#define QUIKFAZ5 ((word)K_QUIKFAZ5)
#define READACCS ((word)K_READACCS)
#define REDO5_5  ((word)K_REDO5_5) /* REDO5.5 */
#define REPIP1   ((word)K_REPIP1)
#define REPIP3   ((word)K_REPIP3)
#define REPIP4   ((word)K_REPIP4)
#define REREADAC ((word)K_REREADAC)
#define SERVICER ((word)K_SERVICER)
#define SERVIDLE ((word)K_SERVIDLE)
#define SERVOUT  ((word)K_SERVOUT)
#define TMPTOSPT ((word)K_TMPTOSPT)
#define USEGTS   ((word)K_USEGTS)
#define USEJETS  ((word)K_USEJETS)
#define AUXFLBIT ((word)KW_AUXFLBIT)
#define DONEADR  ((word)KW_DONEADR) /* GENADR PIPSDONE */
#define DPSVEX   ((word)KW_DPSVEX) /* APSVEX +1, the descent engine's */
#define ENDJBCAD ((word)KW_ENDJBCAD) /* CADR SERVEXIT +2 */
#define KPIP     ((word)KW_KPIP)
#define KPIP1    ((word)KW_KPIP1)
#define NOLRRBIT ((word)KW_NOLRRBIT)
#define OCT37771 ((word)KW_OCT37771)
#define READLBIT ((word)KW_READLBIT)
#define SUPER011 ((word)KW_SUPER011)
#define XNBPIPAD ((word)KW_XNBPIPAD) /* ECADR XNBPIP */
static const word APSVEX[2] = KW_APSVEX;     /* 2DEC: the exhaust velocities, APS and DPS */
static const word AVOUTCAD[2] = KW_AVOUTCAD; /* 2CADR AVGEND */
static const word SVEXTADR[2] = KW_SVEXTADR; /* 2CADR SERVEXIT */
#define ALTCHK           ((word)K_ALTCHK)
#define ALTCRIT          ((word)K_ALTCRIT) /* ALTCRIT = 25KFT */
#define BRANCH           ((word)K_BRANCH)
#define CALCGRAV         ((word)K_CALCGRAV)
#define CALCGRV1         ((word)K_CALCGRV1)
#define CALCRVG          ((word)K_CALCRVG)
#define CONTSERV         ((word)K_CONTSERV)
#define COPYCYC1         ((word)K_COPYCYC1)
#define COPYCYC2         ((word)K_COPYCYC2)
#define HIGATASK         ((word)K_HIGATASK)
#define HIGATCHK         ((word)K_HIGATCHK)
#define HITEST           ((word)K_HITEST)
#define LRPOSALM         ((word)K_LRPOSALM)
#define MUNGRAV          ((word)K_MUNGRAV)
#define MUNRETRN         ((word)K_MUNRETRN)
#define MUNRVG           ((word)K_MUNRVG)
#define NOR29NOW         ((word)K_NOR29NOW)
#define POS1CHK          ((word)K_POS1CHK)
#define POS2CHK          ((word)K_POS2CHK)
#define R29NODES         ((word)K_R29NODES)
#define R29_Q            ((word)K_R29_Q) /* R29? */
#define RVBOTH           ((word)K_RVBOTH)
#define THIRTY_FIVE_KCHK ((word)K_35KCHK) /* 35KCHK */
#define UPDATCHK         ((word)K_UPDATCHK)
#define XORCHK           ((word)K_XORCHK)
#define BITS4_7          ((word)KW_BITS4_7) /* BITS4-7, OCT 110 */
#define NR29_RDR         ((word)KW_NR29_RDR) /* NR29&RDR */
#define OCT10002         ((word)KW_OCT10002)
#define PSTHIBIT         ((word)KW_PSTHIBIT)
#define XORFLBIT         ((word)KW_XORFLBIT)
#define I_MINUS_MUDT     I_MMUDT    /* -MUDT */
#define I_MINUS_MUDTMUN  I_MMUDTMUN /* -MUDTMUN */
#define ALSIGNAG         ((word)K_ALSIGNAG)
#define ENDLRH           ((word)K_ENDLRH)
#define ENDLRV           ((word)K_ENDLRV)
#define GNURVST          ((word)K_GNURVST)
#define HBAD             ((word)K_HBAD)
#define HFAIL            ((word)K_HFAIL)
#define HIGATJOB         ((word)K_HIGATJOB)
#define HSTILBAD         ((word)K_HSTILBAD)
#define LRHJOB           ((word)K_LRHJOB)
#define LRVJOB           ((word)K_LRVJOB)
#define NOLITE           ((word)K_NOLITE)
#define NOREASON         ((word)K_NOREASON)
#define NORLITE          ((word)K_NORLITE)
#define POSALARM         ((word)K_POSALARM)
#define POSGOOD          ((word)K_POSGOOD)
#define POSUPDAT         ((word)K_POSUPDAT)
#define RDGIMS           ((word)K_RDGIMS)
#define READV            ((word)K_READV)
#define RUPDATED         ((word)K_RUPDATED)
#define SETPOS           ((word)K_SETPOS)
#define SETPOS1          ((word)K_SETPOS1)
#define SETPOS2          ((word)K_SETPOS2)
#define SHORTMP2         ((word)K_SHORTMP2)
#define USEVF            ((word)K_USEVF)
#define VALTCHK          ((word)K_VALTCHK)
#define VBAD             ((word)K_VBAD)
#define VELUPDAT         ((word)K_VELUPDAT)
#define VFAIL            ((word)K_VFAIL)
#define VMEASCHK         ((word)K_VMEASCHK)
#define VSTILBAD         ((word)K_VSTILBAD)
#define VUPDAT           ((word)K_VUPDAT)
#define VUPDATED         ((word)K_VUPDATED)
#define WSTOR            ((word)K_WSTOR)
#define ALTSCBIT         ((word)KW_ALTSCBIT)
#define K170MS           ((word)KW_170MS)    /* 170MS, EQUALS ND1 */
#define K2KFT_SEC        ((word)KW_2KFT_SEC) /* 2KFT/SEC */
#define NEG3             ((word)KW_NEG3)
#define LRINHBIT         ((word)KW_LRINHBIT)
#define OCT523           ((word)KW_OCT523)
#define READVBIT         ((word)KW_READVBIT)
#define RNGEDBIT         ((word)KW_RNGEDBIT)
#define VELDABIT         ((word)KW_VELDABIT)
#define VXINHBIT         ((word)KW_VXINHBIT)

/* LANDING_ANALOG_DISPLAYS.agc */
#define ABORTON  ((word)K_ABORTON)
#define ALTOUT   ((word)K_ALTOUT)
#define ALTROUT  ((word)K_ALTROUT)
#define ARCOMP   ((word)K_ARCOMP)
#define CHKLASTY ((word)K_CHKLASTY)
#define DATAOUT  ((word)K_DATAOUT)
#define DISINDAT ((word)K_DISINDAT)
#define DISPRSET ((word)K_DISPRSET)
#define GET22_32 ((word)K_GET22_32) /* GET22/32 */
#define INTLZE   ((word)K_INTLZE)
#define LASTNEGY ((word)K_LASTNEGY)
#define LASTOK   ((word)K_LASTOK)
#define LASTPOSY ((word)K_LASTPOSY)
#define LATFWDV  ((word)K_LATFWDV)
#define LATVNEG  ((word)K_LATVNEG)
#define LATVPOS  ((word)K_LATVPOS)
#define LVLIMITS ((word)K_LVLIMITS)
#define LVMINLM  ((word)K_LVMINLM)
#define NEGLMLV  ((word)K_NEGLMLV)
#define NEGVMAXY ((word)K_NEGVMAXY)
#define NEWDATA  ((word)K_NEWDATA)
#define OLDDATA  ((word)K_OLDDATA)
#define POSVMAXY ((word)K_POSVMAXY)
#define SPEEDRUN ((word)K_SPEEDRUN)
#define VMONITOR ((word)K_VMONITOR)
#define ZDATA2   ((word)K_ZDATA2)
#define ZERODATA ((word)K_ZERODATA)
#define ZEROLSTY ((word)K_ZEROLSTY)
#define ARCONV   ((word)KW_ARCONV)
#define ARTOA    ((word)KW_ARTOA)
#define ARTOA2   ((word)KW_ARTOA2)
#define BITS8_7  ((word)KW_BITS8_7) /* BITS8/7, OCT 00300 */
#define BITSET   PRIO6              /* BITSET = PRIO6 */
#define KPIP1_5  ((word)KW_KPIP1_5) /* KPIP1(5) */
#define MAXVBITS ((word)KW_MAXVBITS)
#define VELCONV  ((word)KW_VELCONV)

/* FINDCDUW--GUIDAP_INTERFACE.agc (and QUICTRIG, POWERED_FLIGHT_SUBROUTINES) */
#define AFTRFLTR    ((word)K_AFTRFLTR)
#define ALARMMGA    ((word)K_ALARMMGA)
#define ARCTRGSP    ((word)K_ARCTRGSP)
#define CDUWXFR     ((word)K_CDUWXFR)
#define DAXMAX      ((word)K_DAXMAX)   /* (DAZMAX = DAXMAX) */
#define DAY_2MAX    ((word)K_DAY_2MAX) /* DAY/2MAX */
#define DCMCL       ((word)K_DCMCL)
#define DELGMBLP    ((word)K_DELGMBLP)
#define DVBYCOSM    ((word)K_DVBYCOSM)
#define FETCHZNB    ((word)K_FETCHZNB)
#define FINDCDUW    ((word)K_FINDCDUW)
#define FLTRSUB     ((word)K_FLTRSUB)
#define GAINFLTR    ((word)K_GAINFLTR)
#define INITCDUW    ((word)K_INITCDUW)
#define LIMITSUB    ((word)K_LIMITSUB)
#define MGARET      ((word)K_MGARET)
#define NB2CDUSP    ((word)K_NB2CDUSP)
#define NOATTCNT    ((word)K_NOATTCNT)
#define ONESTO2S    ((word)K_ONESTO2S)
#define ONE_TO2_TCQ ((word)K_1TO2_TCQ) /* 1TO2&TCQ */
#define PAUTNO      ((word)K_PAUTNO)
#define QUICTRIG    ((word)K_QUICTRIG)
#define SINZERO     ((word)K_SINZERO)
#define TCQCDUW     ((word)K_TCQCDUW)
#define TSL_TCQ     ((word)K_TSL_TCQ) /* TSL&TCQ */
#define UNWCTEST    ((word)K_UNWCTEST)
#define USECOS      ((word)K_USECOS)
#define CDUZDLIM    ((word)KW_CDUZDLIM)
#define DELERLIM    ((word)KW_DAY_2MAX) /* DELERLIM = DAY/2MAX */
#define DPL1        ((word)KW_DPL1)
#define DPL3        ((word)KW_DPL3)
#define DPL5        ((word)KW_DPL5)
#define DPL7        ((word)KW_DPL7)
#define DPL9        ((word)KW_DPL9)
#define DT_DELT     ((word)KW_DT_DELT) /* DT/DELT */
#define DUNFVLIM    ((word)KW_DUNFVLIM)
#define ECDUWL      ((word)KW_ECDUWL)
#define OCT_16      ((word)KW_16OCT) /* 16OCT */
#define UNFVLIM     ((word)KW_UNFVLIM)
#define XOVINHIB    ((word)KW_XOVINHIB)
/* The erasable words, by address: -DELGMB +n, and the tables FINDCDUW and
 * QUICTRIG index (CPHI, DELCDUX, OMEGAPD, DELPEROR, 1JACC, CDUSPOT, SINCDU,
 * COSCDU). */
#define MINUS_DELGMB_AT(n) (K_MDELGMB + (n)) /* -DELGMB +n */
#define TEM2               ERASABLE(K_TEM2)
#define NDXCDUW            ERASABLE(K_NDXCDUW)

/* P51-P53.agc (LM) */
#define COARSRET    ((word)K_COARSRET)
#define COARSTYP    ((word)K_COARSTYP)
#define LSDISP      ((word)K_LSDISP)
#define N89DISP     ((word)K_N89DISP)
#define OPT4        ((word)K_OPT4)
#define P52A        ((word)K_P52A)
#define P52B        ((word)K_P52B)
#define P52D        ((word)K_P52D)
#define P52E        ((word)K_P52E)
#define P52F        ((word)K_P52F)
#define P52H        ((word)K_P52H)
#define P52LS       ((word)K_P52LS)
#define P52OUT      ((word)K_P52OUT)
#define P52T        ((word)K_P52T)
#define P52V        ((word)K_P52V)
#define P52W        ((word)K_P52W)
#define PROG52      ((word)K_PROG52)
#define REGCOARS    ((word)K_REGCOARS)
#define PFRATBIT    ((word)KW_PFRATBIT)
#define V06N34_STAR ((word)KW_V06N34_STAR) /* V06N34* */
#define V06N89_STAR ((word)KW_V06N89_STAR) /* V06N89* */
#define OCT13       ((word)KW_OCT13)
#define LOCSAM      ((word)K_LOCSAM)
#define PICAPAR     ((word)K_PICAPAR)
#define PICBXT      ((word)K_PICBXT)
#define PICEND      ((word)K_PICEND)
#define PICGXT      ((word)K_PICGXT)
#define PICNSWP     ((word)K_PICNSWP)
#define PIC2        ((word)K_PIC2)
#define PIC4        ((word)K_PIC4)
#define STRAT       ((word)K_STRAT)
#define R51         ((word)K_R51)
#define R51_1       ((word)K_R51_1) /* R51.1 */
#define R51_2       ((word)K_R51_2) /* R51.2 */
#define R51_3       ((word)K_R51_3) /* R51.3 */
#define R51_4       ((word)K_R51_4) /* R51.4 */
#define R51C        ((word)K_R51C)
#define R51E        ((word)K_R51E)
#define R51F        ((word)K_R51F)
#define R51I        ((word)K_R51I)
#define R51K        ((word)K_R51K)
#define R51P63      ((word)K_R51P63)
#define GYCOARS     ((word)K_GYCOARS)
#define GOPERF1     ((word)K_GOPERF1)
#define IMUPULSE    ((word)K_IMUPULSE)
#define OPTSTALL    ((word)K_OPTSTALL)
#define OCT14       ((word)KW_OCT14)
#define VB05N09     ((word)KW_VB05N09)
#define V16N20      ((word)KW_V16N20)
#define R55CDR      ((word)KW_R55CDR)
#define R55         ((word)K_R55)
#define R55_1       ((word)K_R55_1) /* R55.1 */
#define R55_2       ((word)K_R55_2) /* R55.2 */
#define R55RET      ((word)K_R55RET)
#define CHKSDATA    ((word)K_CHKSDATA)
#define CHKSDA      ((word)K_CHKSDA)
#define CAL53A      ((word)K_CAL53A)
#define COARFINE    ((word)K_COARFINE)
#define MATMOVE     ((word)K_MATMOVE)
#define RDCDUS      ((word)K_RDCDUS)
#define V06N93      ((word)KW_V06N93)
#define VB6N5       ((word)KW_VB6N5)
#define P51         ((word)K_P51)
#define P51B        ((word)K_P51B)
#define P51C        ((word)K_P51C)
#define P51D        ((word)K_P51D)
#define P51E        ((word)K_P51E)
#define P51G        ((word)K_P51G)
#define COARSE      ((word)K_COARSE)
#define NCOARSE     ((word)K_NCOARSE)
#define S52_2       ((word)K_S52_2) /* S52.2 */
#define S52_3       ((word)K_S52_3) /* S52.3 */
#define V41K        ((word)KW_V41K)
#define R52         ((word)K_R52)
#define R52A        ((word)K_R52A)
#define R52B        ((word)K_R52B)
#define GETAZEL     ((word)K_GETAZEL)
#define AZEL        ((word)K_AZEL)
#define ONE_6TH     ((word)KW_1_6TH) /* 1/6TH */
#define V01N70      ((word)KW_V01N70)
#define V06N87      ((word)KW_V06N87)
#define R59         ((word)K_R59)
#define R59A        ((word)K_R59A)
#define INCAZ       ((word)K_INCAZ)
#define R59D        ((word)K_R59D)
#define ZSPCR       ((word)K_ZSPCR)
#define NXAX        ((word)K_NXAX)
#define R59ALM      ((word)K_R59ALM)
#define SEVENTY_NINE_DISP ((word)K_79DISP) /* 79DISP */
#define R59E        ((word)K_R59E)
#define R59OUT      ((word)K_R59OUT)
#define V01N70_STAR ((word)KW_V01N70_STAR) /* V01N70* */
#define V06N79      ((word)KW_V06N79)
#define DEC227      ((word)KW_DEC227)
#define PLANET      ((word)K_PLANET)
#define NOTPLAN     ((word)K_NOTPLAN)
#define CALSAM1     ((word)K_CALSAM1)
#define CALSAM      ((word)K_CALSAM)
#define VNPLANV     ((word)KW_VNPLANV)
#define GVDETER     ((word)K_GVDETER)
#define PROGRAV     ((word)K_PROGRAV)
#define LUNG        ((word)K_LUNG)
#define GREED       ((word)K_GREED)
#define GRABGRAV    ((word)K_GRABGRAV)
#define ADDGRAV     ((word)K_ADDGRAV)
#define QMINEXIT    ((word)K_QMINEXIT)
#define T_2SEC      ((word)KW_T_2SEC) /* T/2SEC: DEC -20 */
#define DISGRVER    ((word)KW_DISGRVER)
#define FORTY_TWO_DEG   ((word)KW_42DEG) /* 42DEG */
#define THIRTY_FIVE_DEG ((word)KW_35DEG) /* 35DEG */
#define FREEFBIT    ((word)KW_FREEFBIT)
#define GYROTRIM    ((word)K_GYROTRIM)
#define JUSTTRIM    ((word)K_JUSTTRIM)
#define GYRCDR      ((word)KW_GYRCDR) /* ECADR OGC */
#define ONE_STAR    ((word)K_1STAR)   /* 1STAR */
#define R59RET      ((word)K_R59RET)
#define ASTAR       ((word)K_ASTAR)
#define SURFLINE    ((word)K_SURFLINE)
#define INITBY      ((word)K_INITBY)
#define INITABIT    ((word)KW_INITABIT)
#define DISPGYRO    ((word)KW_DISPGYRO)
#define FIVE_DEG_TEST ((word)K_5DEGTEST) /* 5DEGTEST */
#define SURFDISP    ((word)K_SURFDISP)
#define B2F8        ((word)K_B2F8)
#define P57POST     ((word)K_P57POST)
#define COATRIM     ((word)K_COATRIM)
#define CORSIT      ((word)K_CORSIT)
#define ATTCHK      ((word)K_ATTCHK)
#define P57JUMP     ((word)K_P57JUMP)
#define TWO_STARS   ((word)K_2STARS) /* 2STARS */
#define P57         ((word)K_P57)
#define P57OPT      ((word)K_P57OPT)
#define ALIGNOPT    ((word)K_ALIGNOPT)
#define TDISP       ((word)K_TDISP)
#define P57AA       ((word)K_P57AA)
#define PACKOPTN    ((word)K_PACKOPTN)
#define DSPOPTN     ((word)K_DSPOPTN)
#define GETLMATT    ((word)K_GETLMATT)
#define BYLMATT     ((word)K_BYLMATT)
#define P57OPT0     ((word)K_P57OPT0)
#define P57OPT1     ((word)K_P57OPT1)
#define P57OPT3     ((word)K_P57OPT3)
#define IMUCHK      ((word)K_IMUCHK)
#define MFREF       ((word)K_MFREF)
#define REFMF       ((word)K_REFMF)
#define LSORIENT    ((word)K_LSORIENT) /* in bank 4 */
#define ATTFLBIT    ((word)KW_ATTFLBIT)
#define VB05N06     ((word)KW_VB05N06)
#endif

#if COMANCHE055
/* P40-P47 (CM): P40 and P41, the SPS and RCS burns; P47, the thrust
 * monitor; the burn's countdown and the steering. Its erasable words
 * (NBRCYCLS, MRKRTMP, CNTR, P40TMP, THETADX, ADB, DAPDATR1, AVEGEXIT,
 * NVWORD1, TIG ...) are named by ADDRESS_OF. */
#define ACADN83    ((word)K_ACADN83) /* 2CADR CALCN83 */
#define ACADN85    ((word)K_ACADN85) /* 2CADR CALCN85 */
#define AGAIN      ((word)K_AGAIN)
#define ASTNV99P   ((word)K_ASTNV99P)
#define BESTTRIM   ((word)K_BESTTRIM)
#define BTRIMR     ((word)K_BTRIMR)
#define CALCN83    ((word)K_CALCN83)
#define CALCN85    ((word)K_CALCN85)
#define CALL40_8   ((word)K_CALL40_8) /* CALL40.8 */
#define CKNVWRD1   ((word)K_CKNVWRD1)
#define CLOCKJOB   ((word)K_CLOCKJOB)
#define CLOCKON    ((word)K_CLOCKON)
#define CLOKTASK   ((word)K_CLOKTASK)
#define DOSPSOFF   ((word)K_DOSPSOFF)
#define DOSTRULL   ((word)K_DOSTRULL)
#define DOTVCON    ((word)K_DOTVCON)
#define DOTVCRCS   ((word)K_DOTVCRCS)
#define DPAGREE    ((word)K_DPAGREE)
#define DYNDISP    ((word)K_DYNDISP)
#define E6SETTER   ((word)K_E6SETTER)
#define E7SETTER   ((word)K_E7SETTER)
#define COPY40_9   ((word)K_COPY40_9) /* COPY40.9 */
#define ENDIGN     ((word)K_ENDIGN)
#define ENDLAMB    ((word)K_ENDLAMB)
#define ENDS40_9   ((word)K_ENDS40_9) /* ENDS40.9 */
#define ENDV97E    ((word)K_ENDV97E)
#define ENDV97P    ((word)K_ENDV97P)
#define ENDV97T    ((word)K_ENDV97T)
#define ENDV99P    ((word)K_ENDV99P)
#define ENDV99PI   ((word)K_ENDV99PI)
#define ENGINOFF   ((word)K_ENGINOFF)
#define ENGREQST   ((word)K_ENGREQST)
#define FAILDSP    ((word)K_FAILDSP)
#define IGNITION   ((word)K_IGNITION)
#define IMPLBURN   ((word)K_IMPLBURN)
#define IMPLCONT   ((word)K_IMPLCONT)
#define IMPULCHK   ((word)K_IMPULCHK)
#define INCRSVG    ((word)K_INCRSVG)
#define MASSBACK   ((word)K_MASSBACK)
#define LOTHRUST   ((word)K_LOTHRUST)
#define MOONCASE   ((word)K_MOONCASE)
#define NOFLASH    ((word)K_NOFLASH)
#define NOTADDUL   ((word)K_NOTADDUL)
#define P40BLNKR   ((word)K_P40BLNKR)
#define P40CLK     ((word)K_P40CLK)
#define P40CSM     ((word)K_P40CSM)
#define P40GMB     ((word)K_P40GMB)
#define P40MINDB   ((word)K_P40MINDB)
#define P40PVA     ((word)K_P40PVA)
#define P40RCS     ((word)K_P40RCS)
#define P40SET     ((word)K_P40SET)
#define P40SNEWM   ((word)K_P40SNEWM)
#define P40SXTY    ((word)K_P40SXTY)
#define P40S_RS    ((word)K_P40S_RS) /* P40S/RS */
#define P40S_SV    ((word)K_P40S_SV) /* P40S/SV */
#define P40TSK     ((word)K_P40TSK)
#define P40TTOG    ((word)K_P40TTOG)
#define P41CSM     ((word)K_P41CSM)
#define P41REDSP   ((word)K_P41REDSP)
#define P41_DSP    ((word)K_P41_DSP) /* P41/DSP */
#define P41_P40    ((word)K_P41_P40) /* P41/P40 */
#define P47BOD     ((word)K_P47BOD)
#define P47BODY    ((word)K_P47BODY)
#define P47CSM     ((word)K_P47CSM)
#define P47_DSP    ((word)K_P47_DSP) /* P47/DSP */
#define POST41     ((word)K_POST41)
#define POSTBURN   ((word)K_POSTBURN)
#define PRECHECK   ((word)K_PRECHECK)
#define PREPTVC    ((word)K_PREPTVC)
#define QUICKIGN   ((word)K_QUICKIGN)
#define R40ENABL   ((word)K_R40ENABL)
#define REDO40_9   ((word)K_REDO40_9) /* REDO40.9 */
#define S40_1      ((word)K_S40_1)   /* S40.1 */
#define S40_2_3    ((word)K_S40_2_3) /* S40.2,3 */
#define S40_8      ((word)K_S40_8)   /* S40.8 */
#define S40_81     ((word)K_S40_81)  /* S40.81 */
#define S40_9      ((word)K_S40_9)   /* S40.9 */
#define S40_13     ((word)K_S40_13)  /* S40.13 */
#define S40_130    ((word)K_S40_130) /* S40.130 */
#define S40_132    ((word)K_S40_132) /* S40.132 */
#define S40_133    ((word)K_S40_133) /* S40.133 */
#define S40_134    ((word)K_S40_134) /* S40.134 */
#define S41_1      ((word)K_S41_1)   /* S41.1 */
#define SEC29_96   ((word)K_SEC29_96) /* SEC29.96: 2DEC 2996 */
#define SERVCADR   ((word)K_SERVCADR) /* 2CADR SERVEXIT */
#define SERVXT     ((word)K_SERVXT)
#define SETCLOCK   ((word)K_SETCLOCK)
#define SETMAXDB   ((word)K_SETMAXDB)
#define SETMINDB   ((word)K_SETMINDB)
#define SETTB6     ((word)K_SETTB6)
#define SETUPDYN   ((word)K_SETUPDYN)
#define SETUP_9    ((word)K_SETUP_9) /* SETUP.9 */
#define SPSOFF     ((word)K_SPSOFF)
#define SPSOFF97   ((word)K_SPSOFF97)
#define SPSON      ((word)K_SPSON)
#define STEERADS   ((word)K_STEERADS) /* 2CADR STEERING */
#define STEERING   ((word)K_STEERING)
#define STEERULL   ((word)K_STEERULL)
#define SURELY_9   ((word)K_SURELY_9) /* SURELY.9 */
#define T5IDL24    ((word)K_T5IDL24) /* 2CADR T5IDLOC */
#define TGOCALC    ((word)K_TGOCALC)
#define TIGAVEG    ((word)K_TIGAVEG)
#define TIGBLNK    ((word)K_TIGBLNK)
#define TIGNOW     ((word)K_TIGNOW)
#define TIGON      ((word)K_TIGON)
#define TIG_0      ((word)K_TIG_0) /* TIG-0 */
#define TIG_5      ((word)K_TIG_5) /* TIG-5 */
#define TRIMONLY   ((word)K_TRIMONLY)
#define TST_TRIM   ((word)K_TST_TRIM) /* TST,TRIM */
#define TTGPHS     ((word)K_TTGPHS)
#define TTG_0      ((word)K_TTG_0) /* TTG/0 */
#define TVCON2C    ((word)K_TVCON2C) /* 2CADR TVCDAPON */
#define TVCZAP     ((word)K_TVCZAP)
#define ULAGEOFF   ((word)K_ULAGEOFF)
#define UPDATEVG   ((word)K_UPDATEVG)
#define V97E       ((word)K_V97E)
#define V97E40_6   ((word)K_V97E40_6) /* V97E40.6 */
#define V97ETASK   ((word)K_V97ETASK)
#define V97P       ((word)K_V97P)
#define V97PTASK   ((word)K_V97PTASK)
#define V97T       ((word)K_V97T)
#define V97TRCS    ((word)K_V97TRCS)
#define V97TTASK   ((word)K_V97TTASK)
#define V99E       ((word)K_V99E)
#define V99EJOB    ((word)K_V99EJOB)
#define V99FLASH   ((word)K_V99FLASH)
#define V99P       ((word)K_V99P)
#define V99P_TIG   ((word)K_V99P_TIG) /* V99P/TIG */
#define V99T       ((word)K_V99T)
#define V99TJOB    ((word)K_V99TJOB)
#define FIVE_SECDP ((word)K_5SECDP) /* 5SECDP: 2DEC 500 */
#define EIGHTEEN_SEC ((word)KW_18SEC) /* 18SEC: DEC 1800 */
#define FIVE_SEC   ((word)KW_5SEC)    /* 5SEC: DEC 500 */
#define MAXDB      ((word)KW_MAXDB)   /* DEC 455 */
#define OCT02202   ((word)KW_OCT02202) /* BITS 2, 8, 11 FOR CHANNEL 12 */
#define OCT27_24   ((word)KW_OCT27_24) /* OCT27/24: OCT 27 */
#define ONE_SEC    ((word)KW_1SEC)     /* 1SEC: DEC 100 */
#define SEC24_96   ((word)KW_SEC24_96) /* SEC24.96: DEC 2496 */
#define THREE_MDOT ((word)KW_3MDOT)    /* 3MDOT: "3SEC MASS LOSS" */
#define PRIO27     ((word)KW_PRIO27)
#define SEC29_96_PLUS_1 ((word)KW_SEC29_96P1) /* SEC29.96 +1: its low word, 2996 */
#define MINDB      ((word)KW_MINDB)   /* DEC 46 */
#define P40CKLS2   ((word)KW_P40CKLS2) /* OCT 204 */
#define TWO_SECS   ((word)KW_2SECS)    /* 2SECS: DEC 200 */
#define V06N40     ((word)KW_V06N40)  /* VN 0640 */
#define V06N85B    ((word)KW_V06N85B) /* VN 0685 */
#define V16N40     ((word)KW_V16N40)  /* VN 1640 */
#define V1683      ((word)KW_V1683)   /* VN 1683 */
#define V16N85B    ((word)KW_V16N85B) /* VN 1685 */
/* Part 7 of P40-P47 (CM). */
#define CONTONE      ((word)KW_CONTONE)  /* DEC .662034, "2PI/M" */
#define CONTTWO      ((word)K_CONTTWO)   /* 2DEC .00118 */
#define DAPFIG       ((word)K_DAPFIG)
#define DEC409       ((word)KW_DEC409)
#define DEC46        ((word)KW_DEC46)
#define ENDFIG       ((word)K_ENDFIG)
#define GDTSETUP     ((word)K_GDTSETUP)
#define GIMDTEST     ((word)K_GIMDTEST)
#define GIMTRIM      ((word)K_GIMTRIM)
#define M4ACTDEG     ((word)KW_M4ACTDEG) /* -4ACTDEG: DEC -168 */
#define MBIT14       ((word)KW_MBIT14)   /* -BIT14: OCT 57777 */
#define NODAPUP      ((word)K_NODAPUP)
#define OCT02200     ((word)KW_OCT02200) /* "BITS 8,11 FOR CHANNEL 12 TVC/OPTICS" */
#define OUTPUT       ((word)K_OUTPUT)
#define P2ACTDEG     ((word)KW_P2ACTDEG) /* +2ACTDEG: DEC +84, "+2 DEG" */
#define POINT6_SECT5 ((word)KW_0_6SECT5) /* 0.6SECT5: OCT 37704 */
#define PRIO6        ((word)KW_PRIO6)
#define PRE40_6      ((word)K_PRE40_6)   /* PRE40.6 */
#define RCSADDR      ((word)K_RCSADDR)   /* 2CADR RCSATT */
#define RCSDAPON     ((word)K_RCSDAPON)
#define RCSDAPUP     ((word)K_RCSDAPUP)
#define RCSUP        ((word)K_RCSUP)
#define RSTRTST      ((word)K_RSTRTST)
#define S40_6        ((word)K_S40_6)     /* S40.6 */
#define S40_14       ((word)K_S40_14)    /* S40.14 */
#define S41_2        ((word)K_S41_2)     /* S41.2 */
#define T5IDLDAP     ((word)K_T5IDLDAP)  /* 2CADR T5IDLOC */
#endif

#if COMANCHE055
/* P51-P53.agc (CM). STATE +n is flag word n; OPTION2, DSPTEM1 and TIME1
 * are named by ADDRESS_OF. */
#define PROG52      ((word)K_PROG52)
#define P52A        ((word)K_P52A)
#define P52B        ((word)K_P52B)
#define P52T        ((word)K_P52T)
#define P52D        ((word)K_P52D)
#define P52J        ((word)K_P52J)
#define CAL53RET    ((word)K_CAL53RET)
#define P52C        ((word)K_P52C)
#define P52E        ((word)K_P52E)
#define P52F        ((word)K_P52F)
#define ENDP50S     ((word)K_ENDP50S)
#define P52I        ((word)K_P52I)
#define COARSTYP    ((word)K_COARSTYP)
#define GYCRS       ((word)K_GYCRS)
#define SET1_PDT    ((word)K_SET1_PDT) /* SET1/PDT */
#define V06N34      ((word)KW_V06N34)
#define VB06N22     ((word)KW_VB06N22)
#define ALRM15      ((word)KW_ALRM15) /* ALRM15 EQUALS OCT15 */
#define OCT13       ((word)KW_OCT13)
#define V16N20      ((word)KW_V16N20)
#define R55CDR      ((word)KW_R55CDR)
#define P52LS       ((word)K_P52LS)
#define LSDISP      ((word)K_LSDISP)
#define V06N89_STAR ((word)KW_V06N89_STAR) /* V06N89* */
#define R52         ((word)K_R52)
#define R52VRB      ((word)K_R52VRB)
#define R52A        ((word)K_R52A)
#define R52C        ((word)K_R52C)
#define R52D        ((word)K_R52D)
#define R52JA       ((word)K_R52JA)
#define R52E        ((word)K_R52E)
#define R52F        ((word)K_R52F)
#define R52Q        ((word)K_R52Q)
#define R52HA       ((word)K_R52HA)
#define R52I        ((word)K_R52I)
#define R52SYNC     ((word)K_R52SYNC)
#define R52J        ((word)K_R52J)
#define R52M        ((word)K_R52M)
#define R53JOB      ((word)K_R53JOB)
#define ENDPLAC     ((word)K_ENDPLAC)
#define R52L        ((word)K_R52L)
#define TERM52      ((word)K_TERM52)
#define ADVORB      ((word)K_ADVORB)
#define R61CSM      ((word)K_R61CSM)
#define V06N92      ((word)KW_V06N92)
#define OCT404      ((word)KW_OCT404)
#define ONE_8SEC    ((word)KW_1_8SEC) /* 1.8SEC */
#define PRIO24      ((word)KW_PRIO24)
#define LOCSAM      ((word)K_LOCSAM) /* LOCSAM = S50 */
#define PICAPAR     ((word)K_PICAPAR)
#define PICEND      ((word)K_PICEND)
#define PICGXT      ((word)K_PICGXT)
#define PICBXT      ((word)K_PICBXT)
#define PIC2        ((word)K_PIC2)
#define PIC4        ((word)K_PIC4)
#define STRAT       ((word)K_STRAT)
#define R51         ((word)K_R51)
#define R51_2       ((word)K_R51_2) /* R51.2 */
#define R51_3       ((word)K_R51_3) /* R51.3 */
#define R51_4       ((word)K_R51_4) /* R51.4 */
#define R51DSP      ((word)K_R51DSP)
#define R51ST       ((word)K_R51ST)
#define R51A        ((word)K_R51A)
#define R51K        ((word)K_R51K)
#define R51B        ((word)K_R51B)
#define CHKSCODE    ((word)K_CHKSCODE)
#define R55         ((word)K_R55)
#define R55_1       ((word)K_R55_1) /* R55.1 */
#define R55_2       ((word)K_R55_2) /* R55.2 */
#define R55RET      ((word)K_R55RET)
#define ONE_6TH     ((word)KW_1_6TH) /* 1/6TH */
#define V01N70      ((word)KW_V01N70)
#define V06N93      ((word)KW_V06N93)
#define OCT14       ((word)KW_OCT14)
#define OCT66       ((word)KW_OCT66)
#define NEG47       ((word)KW_NEG47)
#define CHKSDATA    ((word)K_CHKSDATA)
#define CHKSDA      ((word)K_CHKSDA)
#define CAL53A      ((word)K_CAL53A)
#define COARFINE    ((word)K_COARFINE)
#define RDCDUS      ((word)K_RDCDUS)
#define CALCSMSC    ((word)K_CALCSMSC)
#define VB6N5       ((word)KW_VB6N5)
#define P51         ((word)K_P51)
#define P51A        ((word)K_P51A)
#define P51AA       ((word)K_P51AA)
#define P51B        ((word)K_P51B)
#define P51C        ((word)K_P51C)
#define P51C_1      ((word)K_P51C_1) /* P51C.1 */
#define P51E        ((word)K_P51E)
#define P51F        ((word)K_P51F)
#define P51G        ((word)K_P51G)
#define V41K        ((word)KW_V41K)
#define SXTSM       ((word)K_SXTSM)
#define R53         ((word)K_R53)
#define R53A        ((word)K_R53A)
#define R53B        ((word)K_R53B)
#define R53C        ((word)K_R53C)
#define R53C1       ((word)K_R53C1)
#define R53D        ((word)K_R53D)
#define R53Z        ((word)K_R53Z)
#define S52_2       ((word)K_S52_2) /* S52.2 */
#define SR52_1      ((word)K_SR52_1) /* SR52.1 */
#define SR52E1      ((word)K_SR52E1)
#define SR52E22     ((word)K_SR52E22)
#define SR52E3      ((word)K_SR52E3)
#define TRUN38      ((word)K_TRUN38)
#define V01N71      ((word)KW_V01N71)
#define ADVTRACK    ((word)K_ADVTRACK)
#define S52_3       ((word)K_S52_3) /* S52.3 */
#define R56         ((word)K_R56)
#define R56A        ((word)K_R56A)
#define R56A1       ((word)K_R56A1)
#define R56B        ((word)K_R56B)
#define VB53        ((word)KW_VB53)
#define V06N94B     ((word)KW_V06N94B)
#define OCT16       ((word)KW_OCT16)
#define PLANET      ((word)K_PLANET)
#define NOSAM       ((word)K_NOSAM)
#define NOTPLAN     ((word)K_NOTPLAN)
#define CALSAM1     ((word)K_CALSAM1)
#define SIGHTSIX    ((word)KW_SIGHTSIX)
#define DEC227      ((word)KW_DEC227)
#define VNPLANV     ((word)KW_VNPLANV)
#endif

#if COMANCHE055
/* P61-P67.agc (CM), the entry programs. The erasable words (HEADSUP,
 * ROLLC, ALFAPAD, ALFACOM, P63FLAG, ENTRYVN, POSEXIT, S61DT, 60GENRET,
 * CM/FLAGS, CMDAPMOD, EXTVBACT, DNLSTCOD, AVEGEXIT) are named by
 * ADDRESS_OF. */
#define P61       ((word)K_P61)
#define P61_1     ((word)K_P61_1) /* P61.1 */
#define P61_2     ((word)K_P61_2) /* P61.2 */
#define P61_3     ((word)K_P61_3) /* P61.3 */
#define P61_4     ((word)K_P61_4) /* P61.4 */
#define P62       ((word)K_P62)
#define P62_1     ((word)K_P62_1) /* P62.1 */
#define P62_2     ((word)K_P62_2) /* P62.2 */
#define WAKEP62   ((word)K_WAKEP62)
#define P63       ((word)K_P63)
#define P63_1     ((word)K_P63_1) /* P63.1 */
#define P64       ((word)K_P64)
#define P65       ((word)K_P65)
#define P65_1     ((word)K_P65_1) /* P65.1 */
#define P66       ((word)K_P66)
#define P67       ((word)K_P67)
#define P66END    ((word)K_P66END)
#define KILLGRP4  ((word)K_KILLGRP4)
#define P67_1     ((word)K_P67_1) /* P67.1 */
#define P67_2     ((word)K_P67_2) /* P67.2 */
#define S61_1     ((word)K_S61_1)  /* S61.1 */
#define S61_1C    ((word)K_S61_1C) /* S61.1C */
#define RETRN1    ((word)K_RETRN1)
#define RETRN2    ((word)K_RETRN2)
#define S61_2     ((word)K_S61_2) /* S61.2 */
#define FISHCALC  ((word)K_FISHCALC)
#define TFF_TRIG  ((word)K_TFF_TRIG) /* TFF/TRIG */
#define S62_3     ((word)K_S62_3)    /* S62.3 */
#define RETRN3    ((word)K_RETRN3)
#define GOFLASHR  ((word)K_GOFLASHR)
#define GOPERF1R  ((word)K_GOPERF1R)
#define CLEANDSP  ((word)K_CLEANDSP)
#define R02BOTH   ((word)K_R02BOTH)
#define V06N60    ((word)KW_V06N60) /* VN 0660 */
#define V06N61    ((word)KW_V06N61) /* VN 0661 */
#define V06N63    ((word)KW_V06N63) /* VN 0663 */
#define V06N64    ((word)KW_V06N64) /* VN 0664 */
#define V06N66    ((word)KW_V06N66) /* VN 0666 */
#define V06N74    ((word)KW_V06N74) /* VN 0674 */
#define V16N67    ((word)KW_V16N67) /* VN 1667 */
#define V16N69    ((word)KW_V16N69) /* VN 1669 */
#define ENTCADR   ((word)KW_ENTCADR) /* CADR STARTENT */
#define EBENTRY   ((word)KW_EBENTRY) /* EBENTRY = EBANK7 */
#define TEN_SECS  ((word)KW_10SECS)  /* 10SECS: DEC 1000 */
#define OCT41     ((word)KW_OCT41)   /* OCT41 = 33DEC */
static const word POSECADR[2] = KW_POSECADR; /* 2CADR CM/POSE */
static const word SERVCAD1[2] = KW_SERVCAD1; /* 2CADR SERVEXIT (SERVCAD2 = SERVCAD1) */

/* SERVICER207.agc (the CM's) */
#define PREREAD       ((word)K_PREREAD)
#define READACCS      ((word)K_READACCS)
#define PIPSDONE      ((word)K_PIPSDONE)
#define REDO5_5       ((word)K_REDO5_5) /* REDO5.5 */
#define SERVICER      ((word)K_SERVICER)
#define AVERAGEG      ((word)K_AVERAGEG)
#define AVGEND        ((word)K_AVGEND)
#define SERVEXIT      ((word)K_SERVEXIT)
#define NORMLIZE      ((word)K_NORMLIZE)
#define REREADAC      ((word)K_REREADAC)
#define DONEADR       ((word)KW_DONEADR) /* GENADR PIPSDONE */
static const word AVOUTCAD[2] = KW_AVOUTCAD; /* 2CADR AVGEND */
#define REDO5_31      ((word)K_REDO5_31) /* REDO5.31 */
#define PREREAD1      ((word)K_PREREAD1)
#define NOSAVPIP      ((word)K_NOSAVPIP)
#define CHEKAVEG      ((word)K_CHEKAVEG)
#define MAKESERV      ((word)K_MAKESERV)
#define ONMNITOR      ((word)K_ONMNITOR)
#define QUIKREAD      ((word)K_QUIKREAD)
#define CHKCTR        ((word)K_CHKCTR)
#define PIPCHECK      ((word)K_PIPCHECK)
#define PIPLOOP       ((word)K_PIPLOOP)
#define DVTOTUP       ((word)K_DVTOTUP)
#define SETJTAG       ((word)K_SETJTAG)
#define CALCGRAV      ((word)K_CALCGRAV)
#define CALCRVG       ((word)K_CALCRVG)
#define JTAGTIME      ((word)KW_JTAGTIME) /* DEC 120 */
#define POINT5_SEC    ((word)KW_0_5SEC)   /* 0.5SEC: DEC 50 */
#define MINUS_MAXDELV ((word)KW_MMAXDELV) /* -MAXDELV: DEC -6398 */
#endif

#if COMANCHE055
/* REENTRY_CONTROL.agc */
#define STARTENT ((word)K_STARTENT)
#define REFAZE10 ((word)K_REFAZE10)
#define SCALEPOP ((word)K_SCALEPOP)
#define TARGETNG ((word)K_TARGETNG)
#define GETETA   ((word)K_GETETA)
#define INITROLL ((word)K_INITROLL)
#define HUNTEST  ((word)K_HUNTEST)
#define RANGER   ((word)K_RANGER)
#define SIDETRAK ((word)K_SIDETRAK)
#define PRE_HUNT ((word)K_PRE_HUNT)
#define GOTOUPSY ((word)K_GOTOUPSY)
#define UPCONTRL ((word)K_UPCONTRL)
#define KEP      ((word)K_KEP)
#define PREDICT3 ((word)K_PREDICT3)
#define DCONSTD  ((word)K_DCONSTD)
#define HAVDNRNG ((word)K_HAVDNRNG)
#define OVERNOUT ((word)K_OVERNOUT)
#define NODISKY  ((word)K_NODISKY)
#define SERVNOUT ((word)K_SERVNOUT)
#define STEEROFF ((word)K_STEEROFF)
#define P67_2CAD ((word)KW_P67_2CAD) /* P67.2CAD: the GOTO's address P67.2 */
#define ADENDEXT ((word)KW_ADENDEXT) /* CADR ENDEXIT */
#define ENTMASK  ((word)KW_ENTMASK) /* OCT 11774 */
#define ENTRYSW  ((word)KW_ENTRYSW) /* OCT 11010 */

/* CM_BODY_ATTITUDE.agc */
#define CM_POSE  ((word)K_CM_POSE)  /* CM/POSE */
#define CM_ATUP  ((word)K_CM_ATUP)  /* CM/ATUP */
#define CMTR1    ((word)K_CMTR1)
#define REDOPOSE ((word)K_REDOPOSE)
#define CM_POSE3 ((word)K_CM_POSE3) /* CM/POSE3 */
#define EBAOG    ((word)KW_EBAOG)   /* EBAOG EQUALS EBANK6 */
#define TCDU     ((word)KW_TCDU)    /* DEC .1 */

/* P37_P70.agc */
#define P37      ((word)K_P37)
#define RTE299   ((word)K_RTE299)
#define RTE317   ((word)K_RTE317)
#define RTE369   ((word)K_RTE369)
#define RTEVN    ((word)K_RTEVN)
#define P37N     ((word)K_P37N)
#define P37T     ((word)K_P37T)
#define P370GOFR ((word)K_P370GOFR)
#define P370GOF  ((word)K_P370GOF)
#define P370ALRM ((word)K_P370ALRM)
#define TMRAD100 ((word)K_TMRAD100)
#define INVC100  ((word)K_INVC100)
#define PREC100  ((word)K_PREC100)
#define PREC220  ((word)K_PREC220)
#define RTENCK1  ((word)K_RTENCK1)
#define V2T100   ((word)K_V2T100)
#define V2TERROR ((word)K_V2TERROR)
#define GAMDV10  ((word)K_GAMDV10)
#define DVCALC   ((word)K_DVCALC)
#define XT1LIM   ((word)K_XT1LIM)
#define V6N33RTE ((word)KW_V6N33RTE) /* VN 0633 */
#define V4N06RTE ((word)KW_V4N06RTE) /* VN 0406 */
#define V6N61RTE ((word)KW_V6N61RTE) /* VN 0661 */
#define V6N39RTE ((word)KW_V6N39RTE) /* VN 0639 */
#define V6N60RTE ((word)KW_V6N60RTE) /* VN 0660 */
#define V6N81RTE ((word)KW_V6N81RTE) /* VN 0681 */
#define V5N09RTE ((word)KW_V5N09RTE) /* VN 0509 */
/* TVCINITIALIZE.agc */
#define TVCDAPON        ((word)K_TVCDAPON)
#define MRCLEAN         ((word)K_MRCLEAN)
#define TVCINIT1        ((word)K_TVCINIT1)
#define TVCINIT2        ((word)K_TVCINIT2)
#define TVCINIT4        ((word)K_TVCINIT4)
#define NEEDLEIN        ((word)K_NEEDLEIN)
#define PRESWTCH        ((word)K_PRESWTCH)
#define SWICHOVR        ((word)K_SWICHOVR)
#define LOADCOEF        ((word)K_LOADCOEF)
#define S40_15          ((word)K_S40_15)       /* S40.15 */
#define NZERO           ((word)KW_NZERO)       /* DEC 51 */
#define NZEROJR         ((word)KW_NZEROJR)     /* DEC 23 */
#define ONETHOU         ((word)KW_ONETHOU)     /* DEC 1000 B-13 */
#define ATTLIM          ((word)KW_ATTLIM)      /* DEC 0.00833 */
#define ONE_OVER_ATTLIM ((word)KW_1_ATTLIM)    /* 1/ATTLIM: DEC 0.007325 */
#define FKPRIMDT        ((word)KW_FKPRIMDT)    /* DEC .0102 */
#define FREPFRAC        ((word)KW_FREPFRAC)    /* DEC .0375 B-2 */
#define TWO_PI_OVER_M   ((word)KW_2PI_M)       /* 2PI/M: DEC .00331017 B+8 */
#define LBCFADR         ((word)KW_LBCFADR)     /* GENADR LBN10 */
#define NINETEEN        ((word)KW_NINETEEN)    /* NINETEEN = VD1 */

/* TVCEXECUTIVE.agc */
#define TVCEXEC  ((word)K_TVCEXEC)
#define ROLLPREP ((word)K_ROLLPREP)
#define NEEDLEUP ((word)K_NEEDLEUP)
#define VARGAINS ((word)K_VARGAINS)
#define GAINCHNG ((word)K_GAINCHNG)
#define EXECCOPY ((word)K_EXECCOPY)
#define ONESHOTCHK ((word)K_1SHOTCHK) /* 1SHOTCHK */
#define TEMPSET  ((word)K_TEMPSET)
#define CORSETUP ((word)K_CORSETUP)
#define CORCOPY  ((word)K_CORCOPY)
#define CNTRCOPY ((word)K_CNTRCOPY)
#define FCORFRAC ((word)KW_FCORFRAC) /* OCT 10000 */

/* TVCMASSPROP.agc */
#define MASSPROP ((word)K_MASSPROP)
#define FIXCW    ((word)K_FIXCW)
#define NINE     ((word)KW_NINE)
#define NEGBPW   ((word)KW_NEGBPW)  /* DEC -15402.17 B-16 */
#define DXITFIX  ((word)KW_DXITFIX) /* DEC* -1.88275 E-5 B+12* */

/* TVCRESTARTS.agc */
#define REDOTVC  ((word)K_REDOTVC)
#define TVCDAPHS ((word)K_TVCDAPHS)
#define ENABL2   ((word)K_ENABL2)
#define CMDSOUT  ((word)K_CMDSOUT)
#define CHKSTRK  ((word)K_CHKSTRK)
#define FINCOPY  ((word)K_FINCOPY)
#define EXRSTRT  ((word)K_EXRSTRT)

/* TVCDAPS.agc */
#define PITCHDAP        ((word)K_PITCHDAP)
#define PSTROKER        ((word)K_PSTROKER)
#define PCDUDOTS        ((word)K_PCDUDOTS)
#define PFORWARD        ((word)K_PFORWARD)
#define PCOPY           ((word)K_PCOPY)
#define YAWDAP          ((word)K_YAWDAP)
#define YSTROKER        ((word)K_YSTROKER)
#define YINTEGRL        ((word)K_YINTEGRL)
#define YFORWARD        ((word)K_YFORWARD)
#define YCOPY           ((word)K_YCOPY)
#define DAPINIT         ((word)K_DAPINIT)
#define POFFSET         ((word)K_POFFSET)
#define POUT            ((word)K_POUT)
#define DELBARP         ((word)K_DELBARP)
#define PDAPEND         ((word)K_PDAPEND)
#define YOFFSET         ((word)K_YOFFSET)
#define YOUT            ((word)K_YOUT)
#define DELBARY         ((word)K_DELBARY)
#define YDAPEND         ((word)K_YDAPEND)
#define HACK            ((word)K_HACK) /* TVCSTROKETEST.agc */
#define ACTSAT          ((word)KW_ACTSAT)   /* DEC 253 B-14 */
#define ONE_OVER_ACTSAT ((word)KW_1_ACTSAT) /* 1/ACTSAT: DEC .0039525692 */
#define ERRLIM          BIT13               /* ERRLIM EQUALS BIT13 */
#define ONE_OVER_ERRLIM BIT3                /* 1/ERRLIM EQUALS BIT3 */
#define PITCHT5         ((word)KW_PITCHT5)  /* GENADR PITCHDAP */
#define YAWT5           ((word)KW_YAWT5)    /* GENADR YAWDAP */
#define ONE_OVER_RTLIM  ((word)KW_1_RTLIM)  /* 1/RTLIM: DEC 0.004715 */
/* The filter's constants, read where they are (MP). */
#define K_E_MINUS_AT           K_E_MAT   /* E(-AT): OCT 37535 */
#define K_ONE_MINUS_E_MINUS_AT K_1_E_MAT /* 1-E(-AT): OCT 00243 */
/* TVCSTROKETEST.agc */
#define STRKTSTI  ((word)K_STRKTSTI)
#define HACKWLST  ((word)K_HACKWLST)
#define TWENTY_MS BIT2 /* 20MS = BIT2 */
#define TCTSKOVR  ((word)KW_TCTSKOVR) /* TC TASKOVER */
/* TVCROLLDAP.agc */
#define ROLLDAP          ((word)K_ROLLDAP)
#define NOROLL1          ((word)K_NOROLL1)
#define DB               ((word)KW_DB)       /* DEC .01388889: "DEAD BAND (5 DEG)" */
#define MINUS_SLOPE      ((word)KW_MSLOPE)   /* -SLOPE: DEC 0.2 */
#define LMCRATE          ((word)KW_LMCRATE)  /* DEC .00027778 B+4 */
#define INTERCEP         ((word)KW_INTERCEP) /* DEC .0025 B+3 */
#define MINLIM           ((word)KW_MINLIM)   /* DEC .00277778 B+4 */
#define MAXLIM           ((word)KW_MAXLIM)   /* DEC .01388889 B+4 */
#define TMINFIRE         ((word)KW_TMINFIRE) /* DEC 1.5 B+4 */
#define TMAXFIRE         ((word)KW_TMAXFIRE) /* DEC 250 B+4 */
#define ONE_OVER_TMXFIR  BIT3                /* 1/TMXFIR = BIT3 */
#define T6SCALE          PRIO31              /* T6SCALE = PRIO31 */
#define PLUS_ROLL1       FIVE                /* +ROLL1 = FIVE */
#define PLUS_ROLL2       OCT120              /* +ROLL2 = OCT120 */
#define MINUS_ROLL1      TEN                 /* -ROLL1 = TEN */
#define MINUS_ROLL2      ((word)KW_MROLL2)   /* -ROLL2: OCT 240 */

/* MYSUBS.agc */
#define SMALLMP          ((word)K_SMALLMP)
#define DPADD            ((word)K_DPADD)

/* RCS-CSM_DIGITAL_AUTOPILOT.agc */
#define REDORCS          ((word)K_REDORCS)
#define RCSATT           ((word)K_RCSATT)
#define SETT5            ((word)K_SETT5)
#define FREECHK          ((word)K_FREECHK)
#define RATEFILT         ((word)K_RATEFILT)
#define DRHOLOOP         ((word)K_DRHOLOOP)
#define ADOTLOOP         ((word)K_ADOTLOOP)
#define KMATRIX          ((word)K_KMATRIX)
#define TENTHSEK         ((word)K_TENTHSEK)
#define DCDUINCR         ((word)K_DCDUINCR)
#define DELOOP           ((word)K_DELOOP)
#define DACNDLS          ((word)K_DACNDLS)
#define FDAIDSP1         ((word)K_FDAIDSP1)
#define FDAIDSP2         ((word)K_FDAIDSP2)
#define KRESUME1         ((word)K_KRESUME1)
#define NEEDLER          ((word)K_NEEDLER)
#define NEEDLER1         ((word)K_NEEDLER1)
#define NEEDLE11         ((word)K_NEEDLE11)
#define NEEDLER2         ((word)K_NEEDLER2)
#define NEEDLES3         ((word)K_NEEDLES3)
#define NEEDLES          ((word)K_NEEDLES)
#define DACLOOP          ((word)K_DACLOOP)
#define REINIT           ((word)K_REINIT)
#define FRESHDAP         ((word)K_FRESHDAP)
#define REDAP            ((word)K_REDAP)
#define ZEROJET          ((word)K_ZEROJET)
#define T5PHASE2         ((word)K_T5PHASE2)
#define AMBGUPDT         ((word)K_AMBGUPDT)
#define OCT37766         ((word)KW_OCT37766) /* 100 ms (TIME5) */
#define DELTATT2         ((word)KW_DELTATT2) /* OCT 37776: "20MS (TIME5)" */
#define ZEROT5           ((word)K_ZEROT5)
#define IMUAOK           ((word)K_IMUAOK)
#define RCSSWIT          ((word)K_RCSSWIT)
#define NOHIAUTO         ((word)K_NOHIAUTO)
#define NOCHANGE         ((word)K_NOCHANGE)
#define SETWBODY         ((word)K_SETWBODY)
#define ZEROER           ((word)K_ZEROER)
#define NORATE           ((word)K_NORATE)
#define MERUPDAT         ((word)K_MERUPDAT)
#define SPNDXCHK         ((word)K_SPNDXCHK)
#define AHFNOROT         ((word)K_AHFNOROT) /* AUTOMATIC_MANEUVERS.agc */
#define JETS             ((word)K_JETS)     /* AUTOMATIC_MANEUVERS.agc */
#define STICKCHK         ((word)K_STICKCHK)
#define KALUPDT          ((word)K_KALUPDT)
#define FREEFUNC         ((word)K_FREEFUNC)
#define T6PROGM          ((word)K_T6PROGM)
#define T6PROG           ((word)K_T6PROG) /* AUTOMATIC_MANEUVERS.agc */
#define DELAY200         ((word)KW_DELAY200) /* DEC 16364: "200MS" */
#define NO_T5VAR         ((word)KW_NO_T5VAR) /* NO.T5VAR, DEC 36 */
#define SLOPE_24         ((word)KW__EQ__24)  /* =.24, DEC .24 */
#define RCSINIT          ((word)KW_RCSINIT)  /* OCT 00004 */
#define RCSINITB         ((word)KW_RCSINITB) /* OCT 20004 */
#define T5WAIT60         ((word)KW_T5WAIT60) /* DEC 16378: "= 6 CS" */
#define MANROT           ((word)KW_MANROT)   /* OCT 77 */
#define OCT16000         ((word)KW_OCT16000)
#define OCT01760         ((word)KW_OCT01760)
#define PLUS_14MS        ((word)KW__EQ_P14MS) /* =+14MS, DEC 23 */
#define KRESUME2         ((word)K_KRESUME2)
#define FDAITOTL         ((word)K_FDAITOTL)
#define WRTN22           ((word)K_WRTN22)
#define GETAKS           ((word)K_GETAKS)
#define WRTN17           ((word)K_WRTN17)
#define DELTATT          ((word)KW_DELTATT) /* OCT 37770: "80MS (TIME5)" */
/* AUTOMATIC_MANEUVERS.agc. */
#define FREECONT         ((word)K_FREECONT)
#define ATTHOLD          ((word)K_ATTHOLD)
#define HOLDFUNC         ((word)K_HOLDFUNC)
#define GRABANG          ((word)K_GRABANG)
#define JLOOP            ((word)K_JLOOP)
#define INHOLD           ((word)K_INHOLD)
#define SIGNCK1          ((word)K_SIGNCK1)
#define POSVEL           ((word)K_POSVEL)
#define NEGVEL           ((word)K_NEGVEL)
#define J18              ((word)K_J18)
#define J20              ((word)K_J20)
#define J21              ((word)K_J21)
#define J22              ((word)K_J22)
#define J23              ((word)K_J23)
#define NJ22             ((word)K_NJ22)
#define WLH_SLOP         ((word)KW_WLH_SLOP) /* WLH/SLOP, DEC .00463: "= WL+H/SLOPE" */
#define WL_H_SLP         ((word)KW_WL_H_SLP) /* WL-H/SLP, DEC .00277: "= WL-H/SLOPE" */
#define NJ23             ((word)K_NJ23)
#define SIGNCK3          ((word)K_SIGNCK3)
#define SIGNCK4          ((word)K_SIGNCK4)
#define SIGNCK5          ((word)K_SIGNCK5)
#define JN               ((word)K_JN)
#define JP               ((word)K_JP)
#define J24              ((word)K_J24)
#define JTIME            ((word)K_JTIME)
#define DOJET            ((word)K_DOJET)
#define SLOPE2           ((word)KW_SLOPE2) /* DEC .32: "= 0.8 DEG/SEC/DEG" */
#define QUADANGL         ((word)KW_QUADANGL) /* DEC 660: "= 7.25 DEGREES" */
#define JETSLECT         ((word)K_JETSLECT)
#define TABPCOM          ((word)K_TABPCOM)
#define TABYCOM          ((word)K_TABYCOM)
#define TABRCOM          ((word)K_TABRCOM)
#define BDZCHECK         ((word)K_BDZCHECK)
#define NOBDZ            ((word)K_NOBDZ)
#define BDROLL           ((word)K_BDROLL)
#define ACYCHECK         ((word)K_ACYCHECK)
#define BDRACZ           ((word)K_BDRACZ)
#define TAUCHCK          ((word)K_TAUCHCK)
#define NOACY            ((word)K_NOACY)
#define ROLLTIME         ((word)K_ROLLTIME)
#define PITCHTIM         ((word)K_PITCHTIM)
#define PTAUPOS          ((word)K_PTAUPOS)
#define AD14MSP          ((word)K_AD14MSP)
#define PBLASTOK         ((word)K_PBLASTOK)
#define YAWTIME          ((word)K_YAWTIME)
#define YTAUNEG          ((word)K_YTAUNEG)
#define YTAUPOS          ((word)K_YTAUPOS)
#define AD14MSY          ((word)K_AD14MSY)
#define YBLASTOK         ((word)K_YBLASTOK)
#define RBLAST           ((word)K_RBLAST)
#define AD14MSR          ((word)K_AD14MSR)
#define RBLASTOK         ((word)K_RBLASTOK)
#define PBYPASS          ((word)K_PBYPASS)
#define YBYPASS          ((word)K_YBYPASS)
#define RBYPASS          ((word)K_RBYPASS)
#define ASMBLWP          ((word)K_ASMBLWP)
#define ASMBLWY          ((word)K_ASMBLWY)
#define ASMBLWR          ((word)K_ASMBLWR)
#define NO2Y             ((word)K_NO2Y)
#define NO2Z             ((word)K_NO2Z)
#define ACBD2Y           ((word)K_ACBD2Y)
#define ACBD2Z           ((word)K_ACBD2Z)
#define AC2Y             ((word)K_AC2Y)
#define BDF2Z            ((word)K_BDF2Z)
#define FPX2             ((word)K_FPX2)
#define FYX2             ((word)K_FYX2)
#define T6SETUP          ((word)K_T6SETUP)
#define CHECKT23         ((word)K_CHECKT23)
#define CALCDT6          ((word)K_CALCDT6)
#define DXCHT12          ((word)K_DXCHT12)
#define DXCHT23          ((word)K_DXCHT23)
#define RCS2CADR         ((word)K_RCS2CADR)
#define ENDJETS          ((word)K_ENDJETS)
#define T6START          ((word)K_T6START)
#define T6RUPTOR         ((word)K_T6RUPTOR)
#define T6L1             ((word)K_T6L1)
#define T6L2             ((word)K_T6L2)
#define REPLACE          ((word)K_REPLACE)
#define REPLACE1         ((word)K_REPLACE1)
#define REPLACE2         ((word)K_REPLACE2)
#define REPLACER         ((word)K_REPLACER)
#define REPLACEP         ((word)K_REPLACEP)
#define REPLACEY         ((word)K_REPLACEY)
#define ZBLAST           ((word)K_ZBLAST)
#define ZBLAST1          ((word)K_ZBLAST1)
#define ZBLAST2          ((word)K_ZBLAST2)
#define ENABT6           ((word)K_ENABT6)
#define DELTATT3         ((word)KW_DELTATT3) /* DEC 16378: "= 60 MS" (TIME5) */
#define DELATT20         ((word)KW_DELATT20) /* DEC 16382: "= 20 MS" (TIME5) */
#define EQ_14MS          ((word)KW__EQ_14MS) /* =14MS, DEC 23 (TIME6) */
#define EQ_M_1SEC        ((word)KW__EQ_M_1SEC) /* =-.1SEC, DEC -160 */
#define EQ_P_1SEC        ((word)KW__EQ_P_1SEC) /* =+.1SEC, DEC 160 */
#define XLNMASK          ((word)KW_XLNMASK)  /* OCT 7700 */
#define PJETS            ((word)KW_PJETS)    /* OCT 1417 */
#define YJETS            ((word)KW_YJETS)    /* OCT 6360 */
#define ACRJETS          ((word)KW_ACRJETS)  /* OCT 03760 */
#define BDRJETS          ((word)KW_BDRJETS)  /* OCT 34017 */
#define BDZJETS          ((word)KW_BDZJETS)  /* OCT 03417 */
#define ACYJETS          ((word)KW_ACYJETS)  /* OCT 34360 */
#endif

/* NO.CORES + 1 core sets. */
#define CORE_SETS (NO_CORES + 1u)

#endif
