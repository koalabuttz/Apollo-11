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
#if LUMINARY099
#define OCT35    ((word)KW_OCT35)
#define BBSERVDL ((word)KW_BBSERVDL) /* BBCON SERVIDLE */
#define STATEBIT ((word)KW_STATEBIT)
#define REINTBIT ((word)KW_REINTBIT)
#define NODOBIT  ((word)KW_NODOBIT)
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

/* NO.CORES + 1 core sets. */
#define CORE_SETS (NO_CORES + 1u)

#endif
