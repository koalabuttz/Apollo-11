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
#define HALF_SEC ((word)KW__5SEC) /* .5SEC */
#define OCT37    ((word)KW_OCT37)
#define OCT77603 ((word)KW_OCT77603)
#define OCT74777 ((word)KW_OCT74777)
#define PRIO25   ((word)KW_PRIO25)
#define SKIPVBIT ((word)KW_SKIPVBIT)
/* T5IDLER: four 2CADRs, for the T5RUPT owner FLAGWRD6 names. */
static const word T5IDLER[4][2] = {KW_T5IDLER, KW_T5IDLERP2, KW_T5IDLERP4, KW_T5IDLERP6};
#endif

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

/* The listing's "nD" in basic code: word n (decimal) of the job's work
 * area, an offset from FIXLOC (INDEX FIXLOC; TS 10D). */
#define D(n) ((unsigned)(n))

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
#define BLANKET  ((word)K_BLANKET)
#else
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

/* TCDANZIG = ENDT(X) (TIME_OF_FREE_FALL.agc), the basic TC DANZIG a BOVB
 * goes to only to reset OVFIND. */
#define TCDANZIG       ((word)K_TCDANZIG)
#endif

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
/* (ISITAUTO, R60_62's, is only BANKCALL's CADR.) */
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

/* 1STO2S, V1STO2S, 2V1STO2S and their 1TO2SUB, and LOADTIME, RTB routines
 * (RTB_OP_CODES.agc). */
#define LOADTIME    ((word)K_LOADTIME)
#define ONE_STO2S   ((word)K_1STO2S)
#define TWO_V1STO2S ((word)K_2V1STO2S)
#define ONE_TO2SUB  ((word)K_1TO2SUB)
#define V1STO2S     ((word)K_V1STO2S)
#define TPMODE      ((word)K_TPMODE)

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
#define TWIDDLE     ((word)K_TWIDDLE)
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

/* NO.CORES + 1 core sets. */
#define CORE_SETS (NO_CORES + 1u)

#endif
