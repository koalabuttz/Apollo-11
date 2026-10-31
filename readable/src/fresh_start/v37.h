/* SPDX-License-Identifier: MIT */
/* Verb 37 and P00 (FRESH_START_AND_RESTART.agc, its part in bank 04, and
 * the CM's VAC5STOR in bank 20):
 *   - GOP00FIX, where GOTOPOOH (and a restart with no program) comes:
 *     flash V37 N99, "please choose a program";
 *   - V37, the astronaut's "change major mode": check the program is
 *     allowed and listed, stop SERVICER, wait for integration, kill the
 *     engine and the old program's restart groups, and start the new one
 *     (POOH for P00, V37XEQ for the others);
 *   - the CM's INITSUB (stop the DAP's rates, restore its deadband, clear
 *     the program flags) and VAC5STOR (save the jobs for debugging).
 * Each routine runs until it leaves the file through Z. The places those
 * calls return to are routines too, named for what they follow. */
#ifndef READABLE_V37_H
#define READABLE_V37_H

#include "../agc/agc.h"

void gop00fix(void);
void gop00fix_after_clearmrk(void);
void gop00fix_after_goflash(void);
void v37(void);
void v37bad(void);
void v37bad_after_reldsp(void);
void canv37(void);
void pooh_after_reldsp(void);
void nouveau_after_downflag(void);
void v37nono_after_falton(void);
void v37xeq(void);
void v37xeqc(void);
void v37xeqc_after_newmodea(void);
void v37xeqc_after_reldsp(void);
#if LUMINARY099
void gop00fix_after_downflag_xovinflg(void);
void gop00fix_after_downflag_ullagflg(void);
void isservon_after_downflag(void);
void v37ret(void);
void r00(void);
void r00_after_intpret(void);
void r00_after_downflag_3axisflg(void);
void r00_after_downflag_r04flag(void);
void r00_after_downflag_munflag(void);
void r00_after_downflag_xovinflg(void);
void pooh_after_clradmod(void);
void seudop00_after_enginof1(void);
void seudop00_after_allcoast(void);
void seudop00_after_v37klean(void);
void gomod_after_p00klean(void);
void kill2(void);
#else
void r00top00_after_spsoff(void);
void r00top00_after_massprop(void);
void r00top00_after_rcsdapon(void);
void r00top00_after_tvczap(void);
void r00top00_after_delayjob(void);
void roc(void);
void roc_after_intpret(void);
void roc_after_clearmrk(void);
void roc_after_downflag_stikflag(void);
void roc_after_upactoff(void);
void roc_after_downflag_vhfrflag(void);
void roc_after_downflag_r21mark(void);
void groupkil_after_v37klean(void);
void gomod(void);
void initsub_after_stoprate(void);
void initsub_after_setmindb(void);
void rake(void);
void rake_after_upflag(void);
void vac5stor(void);
void vac5stor_after_gentran(void);
#endif

/* The tables V37 reads, for the tests: PREMM1 (each program's priority,
 * E bank and major mode), FCADRMM1 (where it starts) and DNLADMM1 (its
 * downlist), and their length. */
extern const word PREMM1[], FCADRMM1[], DNLADMM1[];
extern const unsigned V37_PROGRAMS;
#if COMANCHE055
extern const word FLAGTABL[12];
#endif

#endif
