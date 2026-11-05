/* SPDX-License-Identifier: MIT */
/* P70-P71.agc (Luminary099 pp. 829-836): the aborts. P70 aborts on the
 * descent engine (DPS), P71 on the ascent engine (APS, after staging);
 * both go into orbit with the ascent guidance. R10,R11's task starts
 * either when the crew presses ABORT or ABORT STAGE (abort.c, pp. 829-830
 * and 835-836, bank 21); CONTABRT and ABRTJASK leave the interrupt and
 * clear the landing's jobs and tasks (contabrt.c, pp. 830-832, bank 5);
 * GOABORT sets up the burn, P70INIT and P71RET the targets, and THROTUP
 * runs the engine up (goabort.c, pp. 832-836, bank 32). The faithful C is
 * c64-port/src/lm_p70_p71_file.c (and lm_p70_p71.c, an older module of the
 * same keys).
 *
 * Words of the listing no instruction names, so no C name either: the
 * constants 70DEC (GOABORT's DEC 70 after TC CHECKMM, p. 833) and MODE71
 * (DEC 71, which INDEX Q reads as MODE70 +2, p. 832), and the labels
 * P71NOW? and P70NOW? (p. 830), GRP4OFF (p. 835), INJTARG, BOTHPOLY,
 * UPRATE, YOK and OLDTIME (pp. 833-835), which are interpretive.
 *
 * Each function runs from a word the program can resume at (an entry, a
 * subroutine's return, the word after an EXIT) until its code leaves the
 * file: at a TC or TCF out of it (Z the routine, Q the return), or at a TC
 * INTPRET, run on to NEWOPS (intpret()). A TC Q that returns into the
 * file (LEGAL?'s, THROTUP's), a DTCB (P70A's to CONTABRT) and a RESUME
 * (CONTABRT's) leave Z at the word: the binding, src/binding/
 * p70_routines.c, runs on there, as the faithful port does. */
#ifndef READABLE_P70_P71_H
#define READABLE_P70_P71_H

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

/* TC Q: back to the return Q holds (the AGC runs register Q as the
 * instruction TC <return>), Q becoming 3. The faithful port takes a return
 * in 010-07777 only, as every caller's is. */
static inline void tc_q(void)
{
    word to = overflow_corrected(Q);

    if (to < 010u || to > 07777u) not_modelled();
    tc_at(2u);
    continue_at(to);
}

/* R10,R11, P70, P71 and LEGAL? -- abort.c */
void r10_r11(void);               /* R10,R11 */
void r10_r11a(void);              /* R10,R11A: TWIDDLE's return */
void flashv_q(void);              /* FLASHV?: the first FLIP's return */
void ten_11(void);                /* 10,11: the second FLIP's return */
void p70(void);                   /* P70 */
void p70a(void);                  /* P70A */
void p71(void);                   /* P71 */
void p71a(void);                  /* P71A */
void legal_q(void);               /* LEGAL? */
void abortalm_after_falton(void); /* ABORTALM +1: TC RELDSP */
void abortalm_after_reldsp(void); /* ABORTALM +2: TC POSTJUMP */

/* CONTABRT and ABRTJASK -- contabrt.c */
void contabrt(void);                /* CONTABRT */
void abrtjask(void);                /* ABRTJASK */
void abrtjask_after_clradmod(void); /* CLRADMOD's return */

/* GOABORT, P70INIT, P71RET, TGOCOMP and THROTUP -- goabort.c */
void goabort(void);                /* GOABORT */
void goabort_after_exit(void);     /* the word after the EXIT */
void goabort_after_flrcs(void);    /* the first DOWNFLAG's return */
void goabort_after_flundisp(void); /* the second's */
void goabort_after_idleflag(void); /* the third's */
void goabort_after_acc4_2fl(void); /* UPFLAG's return: TC CHECKMM */
void goabort_after_checkmm(void);  /* CHECKMM's return in P71: TCF P71RET */
void p70init(void);                /* P70INIT */
void upthrot_after_exit(void);     /* UPTHROT's EXIT +1 */
void upthrot_after_rotflag(void);  /* UPFLAG's return: TC THROTUP */
void upthrot_after_throtup(void);  /* THROTUP's return: TC PHASCHNG */
void upthrot1_minus3(void);        /* UPTHROT1 -3: TC BANKCALL */
void upthrot1_after_p40auto(void); /* P40AUTO's return: TC THROTUP */
void upthrot1(void);               /* UPTHROT1 */
void grp4off_after_phaschng(void); /* GRP4OFF +2: TCF ENDOFJOB */
void p71ret(void);                 /* P71RET */
void p71ret_after_letabort(void);  /* DOWNFLAG's return */
void p71ret_after_exit(void);      /* the word after the EXIT */
void p71ret_after_phaschng(void);  /* PHASCHNG's return */
void throtup(void);                /* THROTUP */
extern const struct interpretive_code GOABORT_CODE, P70INIT_CODE, P71RET_CODE, TGOCOMP_CODE;
#endif

#endif
