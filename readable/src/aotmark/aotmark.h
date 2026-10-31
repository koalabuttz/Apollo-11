/* SPDX-License-Identifier: MIT */
/* AOTMARK.agc, the LM's only: the AOT and COAS sighting marks (Luminary099
 * pages 244-261). The faithful C is c64-port/src/lm_aotmark.c.
 *
 * P51-P53 and R59 call AOTMARK (through BANKCALL) to take star sightings:
 * the crew pick the AOT detent and star (V01N71), then mark with the X
 * and Y mark buttons (MARKRUPT, the interrupt the buttons raise) until
 * they proceed; each pair of marks becomes a star direction, averaged
 * over the pairs (AVESTAR, AVEIT). On the lunar surface the marks come
 * with cursor and spiral readings (V06N79) instead, and SURFSTAR turns
 * them into the star direction.
 *
 * The basic code's entries are below: the routines' starts, and each word
 * a call out of the file returns to (BANKCALL's three returns, ALARM's,
 * NOVAC's job, WAITLIST's task), or an interrupt resumes at. Its
 * interpretive code is the tables OPTAXIS_CODE, YZCHK_CODE, AVESTAR_CODE,
 * OANB_CODE and SURFSTAR_CODE, in the listing's notation, which the
 * readable interpreter runs. */
#ifndef READABLE_AOTMARK_H
#define READABLE_AOTMARK_H

#include "../agc/agc.h"
#include "../interpreter/notation.h"

#if LUMINARY099
void aotmark(void);
void aotmark_plus3(void); /* AOTMARK +3: TC EXTVBCHK */
void aotmark_plus4(void); /* AOTMARK +4: TC POODOO */
void extvbchk(void);
void mkvacfnd(void);
void mkvacfnd_after_findvac(void); /* MKVACFND +011, GETDAT's job started */
void mkreleas(void);               /* WAITLIST's task, from ENDMARKS */
void killaot(void);
void getdat(void);
void getdat_after_terminate(void); /* GETDAT +6, +7 and ENTERDAT: GOMARKF's */
void getdat_after_proceed(void);   /* three returns (V34, V33, ENTER) */
void enterdat(void);
void code7_after_terminate(void); /* CODE7 +3 to +5, GOMARKF's returns */
void code7_after_proceed(void);
void code7_after_enter(void);
void optaxis_after_exit(void);     /* OPTAXIS +026, after OPTAXIS_CODE's EXIT */
void pastit_after_terminate(void); /* PASTIT +2 to +4, GOMARK4's returns */
void pastit_after_proceed(void);
void pastit_after_enter(void);
void aveit_after_exit(void);        /* AVEIT +017, after AVESTAR_CODE's EXIT */
void endmarks_after_waitlist(void); /* ENDMARKS +5 */
void mkalarm(void);
void mkalarm_after_alarm(void); /* MKALARM +2 */
void markrupt(void);
void markrupt_after_resume(void); /* MARKRUPT +026 */
void markrupt_after_alarm(void);  /* MARKRUPT +032 */
void findkey(void);
void findkey_plus10(void);         /* FINDKEY +010: CCS A, the Y mark's bit */
void findkey_plus15(void);         /* FINDKEY +015: CCS A, the X mark's bit */
void somekey_plus3(void);          /* SOMEKEY +3: EXTEND; BZF +3 */
void somekey_after_postjump(void); /* SOMEKEY +7 */
void somekey_after_alarm(void);    /* SOMEKEY +011 */
void xmkrupt(void);
void ymkrupt_after_marktype(void);      /* YMKRUPT +5: a surface mark */
void verifymk_after_alarm(void);        /* VERIFYMK +7 */
void five_mkalarm(void);                /* 5MKALARM */
void five_mkalarm_after_alarm(void);    /* 5MKALARM +2 */
void five_mkalarm_after_marktype(void); /* 5MKALARM +3: on the surface */
void mkrej(void);
void mkrej_after_marktype(void); /* MKREJ +1: on the surface */
void rejalm_after_alarm(void);   /* REJALM +2 */
void reject(void);
void marktype(void);
void vacstor_after_marktype(void);   /* VACSTOR +027: a surface mark */
void surfjob_after_novac(void);      /* SURFJOB +4 */
void changevb(void);                 /* NOVAC's job, from SURFJOB */
void changevb_after_marktype(void);  /* CHANGEVB +1: on the surface */
void dspv6n79_after_terminate(void); /* DSPV6N79 +3 to +5, GOMARKF's returns */
void dspv6n79_after_proceed(void);
void dspv6n79_after_enter(void);

/* The interpretive code: OPTAXIS, YZCHK and AVESTAR's run with AVEIT in
 * bank 07, OANB in bank 04 and SURFSTAR in bank 15. */
extern const struct interpretive_code OPTAXIS_CODE, YZCHK_CODE, AVESTAR_CODE, OANB_CODE,
    SURFSTAR_CODE;
#endif

#endif
