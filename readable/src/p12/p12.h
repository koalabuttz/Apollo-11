/* SPDX-License-Identifier: MIT */
/* P12.agc (Luminary099 pp. 838-842): P12, the powered ascent from the
 * moon's surface. P12LM checks the IMU (R02BOTH), sets the flags, flashes
 * the time of ignition (noun 33) and then the cross-range and apolune
 * (noun 76); the interpretive code works out the state at ignition, the
 * targets and the first guidance quantities, and BURNBABY starts the burn
 * (p12.c, bank 30). P12INIT and COMMINIT set the ascent engine's numbers
 * and the injection targets, for P71 too; GUIDINIT the moon's rate and
 * /LAND/ (guidinit.c, bank 23). The faithful C is c64-port/src/lm_p12.c.
 *
 * Labels of the listing no instruction names, so no C name either:
 * P12LMB and P12RET (interpretive, pp. 839-840).
 *
 * Each function runs from a word the program can resume at (an entry, a
 * subroutine's return, the word after an EXIT) until its code leaves the
 * file: at a TC or TCF out of it (Z the routine, Q the return), or at a TC
 * INTPRET, run on to NEWOPS (intpret()). The faithful port stops there
 * too: the binding, src/binding/p12_routines.c, calls each from its key. */
#ifndef READABLE_P12_H
#define READABLE_P12_H

#include "../agc/agc.h"

#if LUMINARY099
#include "../interpreter/notation.h"

/* TC at, to a routine out of the file: Q gets the return and the routine
 * runs next. */
static inline void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* P12LM, its flashing displays and BURNBABY -- p12.c */
void p12lm(void);                 /* P12LM */
void p12lm_after_phaschng(void);  /* PHASCHNG's return: TC BANKCALL */
void p12lm_after_r02both(void);   /* R02BOTH's return: MUNFLAG */
void p12lm_after_munflag(void);   /* UPFLAG's return: ACC4-2FL */
void p12lm_after_acc4_2fl(void);  /* UPFLAG's return: R10FLAG */
void p12lm_after_r10flag(void);   /* UPFLAG's return: TC CLRADMOD */
void p12lm_after_clradmod(void);  /* CLRADMOD's return: RNDVZFLG */
void p12lm_after_rndvzflg(void);  /* DOWNFLAG's return */
void p12lm_terminate(void);       /* GOFLASH's first return */
void p12lm_proceed(void);         /* GOFLASH's second */
void p12lm_enter(void);           /* GOFLASH's third */
void p12lm_intpret(void);         /* PHASCHNG's return: TC INTPRET */
void p12lmb_after_exit(void);     /* the word after the EXIT */
void newload(void);               /* NEWLOAD */
void newload_terminate(void);     /* GOFLASH's first return */
void newload_proceed(void);       /* GOFLASH's second */
void newload_enter(void);         /* GOFLASH's third */
void newload_intpret(void);       /* PHASCHNG's return: TC INTPRET */
void p12ret_after_exit(void);     /* the word after the EXIT */
void p12ret_after_phaschng(void); /* PHASCHNG's return: FLPI */
void p12ret_after_flpi(void);     /* DOWNFLAG's return */
void p12ret_after_pflitedb(void); /* IBNKCALL's return */
extern const struct interpretive_code P12LM_CODE, NEWLOAD_CODE, P12INIT_CODE;

/* GUIDINIT -- guidinit.c */
extern const struct interpretive_code GUIDINIT_CODE;
#endif

#endif
