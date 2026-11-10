/* SPDX-License-Identifier: MIT */
/* DISPLAY_INTERFACE_ROUTINES.agc, both missions (Luminary099 pages
 * 1341-1373, Comanche055 pages 1452-1484): the routines a program calls
 * to put a verb and noun on the DSKY and wait for the astronaut's answer.
 * "THE DISPLAY ROUTINES ARE INTENDED TO SERVE AS AN INTERFACE BETWEEN THE
 * USER AND PINBALL." The faithful C is c64-port/src/lanec_di_*.c.
 *
 * Three kinds of display share the DSKY, each with its own set of words,
 * indexed by COPINDEX: 0 the PRIO display, 1 the MARK (extended verb)
 * display, 2 the NORMAL display. NVWORD (the verb and noun), DSPFLG (the
 * flags and EBANK, also called EBANKSAV) and CADRFLSH (the caller's
 * return) each have a word per kind, and so do the constant tables
 * PRIOOCT (the FLAGWRD4 bits of each kind), WAKECADR and NVCADR.
 *
 * The missions run the same code. The LM's listing names some bits
 * (XDSPBIT, MRKNVBIT, MKOVBIT, PRIODBIT) where the CM's writes the BIT
 * itself, and the CM has no REFLASHR.
 *
 * Each function below is an entry: a label, or the word after a call out
 * of the file, where that routine returns (the name says which). A run
 * ends where control leaves the file, or reaches another part of it that
 * the faithful port keeps as a separate entry (it starts there next time:
 * the x86 clock counts the steps). The subroutines return by TC Q: the
 * AGC runs the word Q holds (the caller's TC address plus one) as TC to
 * it, so Z goes back to the caller and Q becomes 3. */
#ifndef READABLE_DISPLAY_INTERFACE_H
#define READABLE_DISPLAY_INTERFACE_H

#include "../agc/names.h"

/* TS address, then CAF constant, as most entries begin: an overflow in A
 * skips the CAF and leaves A +1 or -1. */
static inline void ts_caf(unsigned address, word constant)
{
    if (!ts(address)) set_A(constant);
}

/* COPINDEX, the kind of display (0 PRIO, 1 MARK, 2 NORMAL), where a step
 * indexes a three-word set with it. Another value is not a state the
 * program makes: the step is not modelled. */
static inline word display_kind(void)
{
    word kind = fetch(ADDRESS_OF(COPINDEX));
    if (kind > 2u) not_modelled();
    return kind;
}

/* The highest register set offset (LOCCTR) whose LOC is in unswitched
 * erasable: a larger one is not a state the program makes. */
#define LAST_JOB (01376u - ADDRESS_OF(LOC))

/* RELINTQ -- LM p. 1367, CM p. 1478: RELINT, and TC Q, "BACK TO USER". */
static inline void relintq(void)
{
    relint();
    tc_through(Q_ADDRESS);
}

/* The mark displays (marks.c): BLANKET, ENDMARK and CLEARMRK in
 * fixed-fixed, the rest in bank 10. */
void blanket(void);        /* BLANKET: TC Q */
void endmark(void);        /* ENDMARK */
void clearmrk(void);       /* CLEARMRK: TC Q */
void clearmrk_plus2(void); /* CLEARMRK +2: TC Q */
void markend(void);        /* MARKEND */
void gomark(void);         /* GOMARK (GOXDSP) */
void gomars(void);         /* GOMARS */
void kleenex(void);        /* KLEENEX */
void gomarkf(void);        /* GOMARKF (GOXDSPF) */
void gomark2(void);        /* GOMARK2 */
void markform(void);       /* MARKFORM */
void gomark3(void);        /* GOMARK3 */
void gomark4(void);        /* GOMARK4 */
void gomarkr(void);        /* GOMARKR (GOXDSPR) */
void gomarkfr(void);       /* GOMARKFR (GOXDSPFR) */
void gomark2r(void);       /* GOMARK2R */
void gomark3r(void);       /* GOMARK3R */
void makemark(void);       /* MAKEMARK */
void markplay(void);       /* MARKPLAY */
void gogomark(void);       /* GOGOMARK */
void markcop(void);        /* MARKCOP */
void copytogo(void);       /* COPYTOGO: TC Q */
void copypacs(void);       /* COPYPACS: TC Q */

/* The display starts (starts.c). */
void chkprio(void);  /* CHKPRIO */
void markperf(void); /* MARKPERF */
void godsp(void);    /* GODSP */
void godsp2(void);   /* GODSP2 */
void godspret(void); /* GODSPRET */
void godspr(void);   /* GODSPR */
void godspr1(void);  /* GODSPR1 */
void godspr2(void);  /* GODSPR2 */
void cleandsp(void); /* CLEANDSP */
void reflash(void);  /* REFLASH */
#if LUMINARY099
void reflashr(void); /* REFLASHR */
#endif
void regodsp(void);  /* REGODSP */
void regodspr(void); /* REGODSPR */
void clocplay(void); /* CLOCPLAY */
void goflash(void);  /* GOFLASH */
void goflash2(void); /* GOFLASH2 */
void priodspr(void); /* PRIODSPR */
void priodsp(void);  /* PRIODSP */
void setprio(void);  /* SETPRIO */
/* SAVELOCS (r_displays.c), which GOFLASH2 calls: on to MAKECADR. */
void savelocs(void);

/* MAKEPLAY: which kind, and whether it may start (makeplay.c). */
void makeprio(void);                /* MAKEPRIO */
void makeprio_after_linuschr(void); /* MAKEPRIO +4: not a LINUS */
void hiprio(void);                  /* HIPRIO */
void setmark(void);                 /* SETMARK */
void askifnrm(void);                /* ASKIFNRM */
void setnorm(void);                 /* SETNORM */
void oktocopy(void);                /* OKTOCOPY */
void oktocopy_after_jobwake(void);  /* OKTOCOPY +3 */
void redoprio(void);                /* REDOPRIO */
void keepprio(void);                /* KEEPPRIO */
void makeplay(void);                /* MAKEPLAY */
void makeplay_after_priochng(void); /* MAKEPLAY +5 */
void iflegal(void);                 /* IFLEGAL */
void iflegal_after_linuschr(void);  /* IFLEGAL +4: not a LINUS */
void priobort(void);                /* PRIOBORT */
void oktoplay(void);                /* OKTOPLAY */
void oktoplay_after_jobwake(void);  /* OKTOPLAY +015 */
void playjum1(void);                /* PLAYJUM1 */
void prioplay(void);                /* PRIOPLAY */

/* The R displays, with an immediate return to the user (r_displays.c). */
void exdspret(void);                /* EXDSPRET */
void goperf1(void);                 /* GOPERF1 */
void goperfs(void);                 /* GOPERFS */
void goperf2(void);                 /* GOPERF2 */
void goperf4(void);                 /* GOPERF4 */
void goflashr(void);                /* GOFLASHR */
void godsprs(void);                 /* GODSPRS */
void godsprs1(void);                /* GODSPRS1 */
void godsprs1_after_novac(void);    /* GODSPRS1 +015: TCF BOTHJOBS */
void bothjobs(void);                /* BOTHJOBS (SPVAC's return) */
void goperf1r(void);                /* GOPERF1R */
void goperfrs(void);                /* GOPERFRS */
void goperf2r(void);                /* GOPERF2R */
void goperf4r(void);                /* GOPERF4R */
void savelocs_after_makecadr(void); /* SAVELOCS +7: TC L */
void copynorm(void);                /* COPYNORM: TC Q */
void copies(void);                  /* COPIES: TC Q */
void copies2(void);                 /* COPIES2: TC Q */

/* Display jobs put to sleep and woken (sleep.c). */
void gosleeps(void);                /* GOSLEEPS */
void gosleeps_after_upent2(void);   /* GOSLEEPS +5 */
void xchsleep(void);                /* XCHSLEEP */
void xchsleep_after_jobwake(void);  /* XCHSLEEP +4 */
void xchsleep_after_xchtoend(void); /* XCHSLEEP +5 */
void jobxchs(void);                 /* JOBXCHS */
void jobxchs_after_jobwake(void);   /* JOBXCHS +3 */
void jobxchs_after_xchnyloc(void);  /* JOBXCHS +010 */
void idlemask_after_downent2(void); /* IDLEMASK +1 */
void xchqadd_after_upent2(void);    /* XCHQADD +1 */
void markwake(void);                /* MARKWAKE */
void wakeplay(void);                /* WAKEPLAY */
void markfmsk_after_downent2(void); /* MARKFMSK +1 */
void markfmsk_after_jobwake(void);  /* MARKFMSK +5 */
void initdsp(void);                 /* INITDSP */
void initdsp_after_supersw(void);   /* INITDSP +4 */
void initdsp_after_priochng(void);  /* INITDSP +6 */

/* Starting the display (nvdsp.c). */
void pinbrnch(void);                /* PINBRNCH (GOAGIN) */
void normbnch(void);                /* NORMBNCH */
void normbnch_after_upflag(void);   /* NORMBNCH +2 */
void nvdsp(void);                   /* NVDSP (GOPLAY) */
void nvdsp_after_copypacs(void);    /* NVDSP +1 */
void nvdsp_mark(void);              /* NVDSP +022: CS MARKNV */
void nvdsp_mark_plus1(void);        /* NVDSP +023: TS MARKNV */
void nvdsp1(void);                  /* NVDSP1 */
void nv50dsp(void);                 /* NV50DSP */
void nv50dsp_busy(void);            /* NV50DSP +1: NVMONOPT busy */
void nv50dsp_after_nvmonopt(void);  /* NV50DSP +2 */
void nv50dsp_after_flashoff(void);  /* NV50DSP +3 */
void nv50dsp_after_copytogo(void);  /* NV50DSP +4 */
void nv50dsp_after_downflag(void);  /* NV50DSP +6 */
void nv50dsp_after_downflag2(void); /* NV50DSP +010 */
void blankchk(void);                /* BLANKCHK */
void blankchk_after_blanksub(void); /* BLANKCHK +2 */
void perfchek(void);                /* PERFCHEK */
void goanidle(void);                /* GOANIDLE */
void goanidle_plus4(void);          /* GOANIDLE +4 */
void goanidle_plus8(void);          /* GOANIDLE +010 */
void isitn00(void);                 /* ISITN00 */
void first_or_second(void);         /* 1STOR2ND */
void first_or_second_plus4(void);   /* 1STOR2ND +4 */
void v50paste(void);                /* V50PASTE */
void witchone(void);                /* WITCHONE: TC Q */
void xchtoend(void);                /* XCHTOEND: TC Q */
void xchnyloc(void);                /* XCHNYLOC: TC Q */

/* ENDIDLE and the returns to the user (endidle.c). */
void cleanend(void);                /* CLEANEND */
void cleanend_after_findvac(void);  /* CLEANEND +4 */
void isitprio(void);                /* ISITPRIO */
void rest(void);                    /* REST */
void restslep(void);                /* RESTSLEP */
void restslep_after_upent2(void);   /* RESTSLEP +4 */
void flashsub(void);                /* FLASHSUB */
void flashsub_plus1(void);          /* FLASHSUB +1 */
void itismask_after_upent2(void);   /* ITISMASK +1 */
void goidle(void);                  /* ITISMASK +012: TC ENDIDLE */
void idleret1(void);                /* IDLERET1: terminate */
void idleret1_plus1(void);          /* IDLERET1 +1: proceed */
void idleret1_plus2(void);          /* IDLERET1 +2: enter */
void oktoent(void);                 /* OKTOENT */
void endout(void);                  /* ENDOUT */
void timechek(void);                /* TIMECHEK */
void normwake(void);                /* NORMWAKE */
void endret(void);                  /* ENDRET */
void endret_plus4(void);            /* ENDRET +4 */
void pinidmsk_after_downent2(void); /* PINIDMSK +1 */
void pinidmsk_after_nvsub(void);    /* PINIDMSK +3 */
void endit(void);                   /* ENDIT */
void endit_after_priochng(void);    /* ENDIT +3 */
void unsetr1(void);                 /* UNSETR1 */
void unsetr1_after_supersw(void);   /* UNSETR1 +6 (IMMEDRET -1) */
void immedret(void);                /* IMMEDRET */
void termate(void);                 /* TERMATE */
void linuschr(void);                /* LINUSCHR: TC Q, or Q+1 */
void proceed(void);                 /* PROCEED */
void markret(void);                 /* MARKRET */
void markover(void);                /* MARKOVER */
void normret(void);                 /* NORMRET */
void normret_plus4(void);           /* NORMRET +4 */
void normret_plus8(void);           /* NORMRET +010 */
void normret_plus12(void);          /* NORMRET +014 */
void normret_after_novac(void);     /* NORMRET +024 */
void marsleep(void);                /* MARSLEEP */
void loaditis(void);                /* LOADITIS */
void eraser(void);                  /* ERASER */
void eraser_after_nvsub(void);      /* ERASER +2 and +3 */

#endif
