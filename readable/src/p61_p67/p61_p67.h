/* SPDX-License-Identifier: MIT */
/* P61-P67.agc, the CM's (Comanche055 pp. 789-818): the entry programs.
 * P61 and P62 ready the CM for entry (the entry displays, the CM/SM
 * separation, the entry attitude); P63 starts the entry guidance; P64 to
 * P67 are the phases of the guidance itself, each started by it. S61.1,
 * S61.2, the TFF routines and S62.3 serve them. The faithful C is
 * c64-port/src/cm_p61_p67.c.
 *
 * Written part by part, in the listing's order (README.md):
 *   part 1, p61_p62.c: P61 and P62 (pp. 789-794, bank 26); p63_p67.c: P63
 *     to P67 (pp. 795-802, bank 26); s61_1.c: S61.1, the state vector and
 *     IMU check (pp. 803-805, bank 26). All the file's basic code is in
 *     this part: its programs run into each other's.
 *   part 2, s61_2.c: S61.2, the entry prediction for N60 and N63 (pp.
 *     806-811, bank 26), all interpretive.
 *   part 3, s61_2_subroutines.c: FISHCALC, VGAMCALC, TFF/TRIG, DISPTARG
 *     and VRCALC (pp. 812-816, banks 26 and 27); s62_3.c: S62.3, the
 *     entry attitude's gimbal angles (pp. 817-818, bank 10). All
 *     interpretive; with them the file is whole.
 * The binding (src/binding/cm_p61_routines.c) runs the keys these parts
 * write: all of the faithful port's.
 *
 * Basic code round stretches of interpretive code, which are in the
 * listing's notation (src/interpreter/notation.h). The entries below are
 * the faithful port's keys, by the listing's labels. */
#ifndef READABLE_P61_P67_H
#define READABLE_P61_P67_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if COMANCHE055

/* TC at, to a routine out of the file (or to a keyed label): Q gets the
 * return, and the routine runs next. */
static inline void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* EXTEND; DCA of a 2CADR constant: A gets its first word, L its second. */
static inline void dca_2cadr(const word pair[2])
{
    set_L(pair[1]);
    set_A(pair[0]);
}

/* Part 1, P61 and P62 -- p61_p62.c */
void p61(void);                    /* P61 */
void p61_display(void);            /* P61 +5 (S61.1's return) */
void p61_terminate(void);          /* P61 +010 (GOFLASHR's return) */
void p61_proceed(void);            /* P61 +011 */
void p61_enter(void);              /* P61 +012 */
void p61_3(void);                  /* P61.3 (the display's immediate return) */
void p61_3_after_phaschng(void);   /* P61.3 +2 */
void p61_1(void);                  /* P61.1 (S61.2's return) */
void p61_1_display(void);          /* P61.1 +1 (CLEARMRK's return) */
void p61_1_terminate(void);        /* P61.1 +4 (GOFLASH's return) */
void p61_1_proceed(void);          /* P61.1 +5 */
void p61_1_enter(void);            /* P61.1 +6 */
void p61_2_display(void);          /* P61.2 +010 (after EXIT) */
void p61_2_terminate(void);        /* P61.2 +013 (GOFLASH's return) */
void p61_2_proceed(void);          /* P61.2 +014 */
void p61_2_redo(void);             /* P61.2 +015 */
void p62_from_p61(void);           /* P62 -2 (NEWMODEX's return) */
void p62(void);                    /* P62 */
void p62_after_s61_1(void);        /* P62 +1 (S61.1's return) */
void p62_2(void);                  /* P62.2 (CM/DAPIC's return) */
void p62_2_terminate(void);        /* P62.2 +6 (GOPERF1R's return) */
void p62_2_proceed(void);          /* P62.2 +7 */
void p62_2_enter(void);            /* P62.2 +010 */
void p62_2_immediate_return(void); /* P62.2 +011 */
void p62_1(void);                  /* P62.1 */
void p62_1_terminate(void);        /* P62.1 +3 (GOFLASH's return) */
void p62_1_proceed(void);          /* P62.1 +4 */
void p62_1_enter(void);            /* P62.1 +5 */
void p62_1_after_phaschng(void);   /* P62.1 +010 */
void p62_1_after_upflag(void);     /* P62.1 +025 */
void wakep62(void);                /* WAKEP62 (a task) */
void wakep62_after_novac(void);    /* WAKEP62 +4 */
extern const struct interpretive_code NEWRNVN_CODE, P61_2_CODE, P62_CODE, P62_3_CODE;

/* P63 to P67 -- p63_p67.c */
void p63(void);                     /* P63 */
void p63_after_newmodex(void);      /* P63 +2 */
void p63_after_phaschng(void);      /* P63 +012 */
void p63_1(void);                   /* P63.1 (BANKCALL's return) */
void p63_1_after_phaschng(void);    /* P63.1 +2 */
void p64(void);                     /* P64 */
void p64_after_newmodex(void);      /* P64 +2 */
void p65(void);                     /* P65 */
void p65_after_newmodex(void);      /* P65 +2 */
void p65_after_novac(void);         /* P65 +6 */
void p65_after_2phschng(void);      /* P65 +011 */
void p65_1(void);                   /* P65.1 (a job) */
void p65_1_display(void);           /* P65.1 +2 (DOWNFLAG's return) */
void p65_1_terminate(void);         /* P65.1 +5 (GOFLASHR's return) */
void p65_1_proceed(void);           /* P65.1 +6 */
void p65_1_enter(void);             /* P65.1 +7 */
void p65_1_immediate_return(void);  /* P65.1 +010 */
void p65_1_after_upflag(void);      /* P65.1 +013 */
void p66(void);                     /* P66 */
void p66_after_newmodex(void);      /* P66 +2 */
void p67(void);                     /* P67 */
void p67_after_newmodex(void);      /* P67 +2 */
void killgrp4(void);                /* KILLGRP4 (UPFLAG's return, or by RTB) */
void killgrp4_after_phaschng(void); /* KILLGRP4 +2 */
void p67_1(void);                   /* P67.1 */
void p67_1_terminate(void);         /* P67.1 +3 (GOFLASH's return) */
void p67_1_proceed(void);           /* P67.1 +4 */
void p67_1_enter(void);             /* P67.1 +5 */
extern const struct interpretive_code P65_CODE, P67_2_CODE;

/* S61.1 -- s61_1.c */
void s61_1_after_r02both(void);   /* S61.1 +4 (BANKCALL's return) */
void s61_1_future_state(void);    /* S61.1 +011 (CALRB's return) */
void s61_1_after_waitlist(void);  /* S61.1 +016 */
void s61_1_after_phaschng(void);  /* S61.1 +020 */
void s61_1c(void);                /* S61.1C (a task) */
void s61_1c_after_findvac(void);  /* S61.1C +4 */
void s61_1c_after_2phschng(void); /* S61.1C +012 */
void s61_1a_job(void);            /* S61.1A -1 (the job FINDVAC starts) */
void retrn3(void);                /* RETRN3 (by RTB) */
void retrn3_after_alarm(void);    /* RETRN3 +2 */
void retrn2(void);                /* RETRN2 (by BOVB) */
void retrn2_after_alarm(void);    /* RETRN2 +2 */
void retrn2_after_godspr(void);   /* RETRN2 +5 (BANKCALL's return) */
void retrn1(void);                /* RETRN1 (BANKCALL's return, or by BOVB) */
void s61_1_return(void);          /* E6,1773: 60GENRET, run as TC */
extern const struct interpretive_code S61_1_CODE, S61_1A_CODE;

/* Part 2, S61.2 -- s61_2.c */
extern const struct interpretive_code S61_2_CODE;

/* Part 3, S61.2's subroutines -- s61_2_subroutines.c; S62.3 -- s62_3.c */
extern const struct interpretive_code FISHCALC_CODE, TFF_TRIG_CODE, S62_3_CODE;

/* S61.1, as P61 and P62 call it (TC S61.1, from at). */
void s61_1(word at);

#endif

#endif
