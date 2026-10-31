/* SPDX-License-Identifier: MIT */
/* SXTMARK.agc, the CM's only: the sextant's sighting marks (Comanche055
 * pages 222-235). The faithful C is c64-port/src/cm_sxtmark.c.
 *
 * P51-P53 and R21-R22, the programs that sight stars or landmarks,
 * call SXTMARK (through BANKCALL) with the marks they want. It takes a
 * VAC area for them, and the job MKVB51 asks the crew to mark (V51); each
 * press of the MARK button raises MARKRUPT, which keeps the CDU angles
 * and the time, and MARKIT checks that the ship was not turning too fast
 * (MARKDIF, DIFCHK) before MARKCONT stores the mark in the VAC area. MARK
 * REJECT takes the last one back (MKREJECT). Once all are in, V50N25 asks
 * the crew to proceed (MKVB50), and ENTANSWR leaves the count for the
 * caller.
 *
 * The entries are below: the routines' starts, each word a call out of
 * the file returns to (BANKCALL's, CHECKMM's two, ALARM's, NOVAC's job,
 * WAITLIST's task, GENTRAN's), and MARKRUPT, the interrupt. All of it is
 * basic code. */
#ifndef READABLE_SXTMARK_H
#define READABLE_SXTMARK_H

#include "../agc/agc.h"

#if COMANCHE055
void sxtmark(void);
void sxtmark_plus2(void); /* SXTMARK +2: CAF SIX (P51-P53's CADR SXTMARK +2) */
void markok(void);        /* where BAILOUT returns */
void mkvacfnd(void);
void mkvacfnd_after_checkmm53_no(void);  /* MKVACFND +011: not in P53 */
void mkvacfnd_after_checkmm53_yes(void); /* MKVACFND +012: in P53 */
void mkvacfnd_after_checkmm54_no(void);  /* MKVACFND +015: not in P54 */
void mkvacfnd_after_checkmm54_yes(void); /* MKVACFND +016: in P54 */
void mkvacfnd_after_novac(void);         /* MKVACFND +030, MKVB51's job started */
void mkreleas(void);
void mkrlees(void);
void markrupt(void);
void keycall_after_alarm(void);           /* KEYCALL +011 */
void markcont(void);                      /* IBNKCALL's, from MKACPT */
void markit1_after_gentran_mrkbuf2(void); /* MARKIT1 +4 */
void markit1_after_gentran_mrkbuf1(void); /* MARKIT1 +010 */
void market_after_novac(void);            /* MARKET +016, MARKDISP's job started */
void market_after_gentran_markdown(void); /* MARKET +022 */
void market_after_gentran_mrkbuf1(void);  /* MARKET +026 */
void market3_after_alarm(void);           /* MARKET3 +2 */
void hundred14_alm_after_alarm(void);     /* 114ALM +2 */
void vacstor_after_novac(void);           /* VACSTOR +034, MKVB50's job started */
void mrkrejct_after_alarm(void);          /* MRKREJCT +4 */
void reject2_after_alarm(void);           /* REJECT2 +010 */
void reject3_after_novac(void);           /* REJECT3 +020, MKVB51's job started */
void mkvb51(void);                        /* NOVAC's job */
void mkvb51_after_kleenex(void);          /* MKVB51 +2 */
void mkvb51_after_terminate(void);        /* MKVB51 +5 to +7, GOMARK4's */
void mkvb51_after_proceed(void);          /* three returns (V34, V33, ENTER) */
void mkvb51_after_enter(void);
void termsxt_after_clearmrk(void);      /* TERMSXT +1 */
void termsxt_after_checkmm03_no(void);  /* TERMSXT +3: not in P03 */
void termsxt_after_checkmm03_yes(void); /* TERMSXT +4: in P03 */
void entanswr(void);                    /* also where TERMP03's UPFLAG returns */
void jamit_after_waitlist(void);        /* JAMIT +7 */
void endmarks(void);                    /* WAITLIST's task, from JAMIT */
void mkvb5x(void);
void mkvb50(void);                /* NOVAC's job */
void markit_after_waitlist(void); /* MARKIT +010 */
void markdif(void);               /* WAITLIST's task, from MARKIT */
void difchk(void);                /* also where MKACPT's IBNKCALL returns */
void difchk_after_alarm(void);    /* DIFCHK +016 */
#endif

#endif
