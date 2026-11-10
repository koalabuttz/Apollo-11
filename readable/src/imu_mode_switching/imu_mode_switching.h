/* SPDX-License-Identifier: MIT */
/* IMU_MODE_SWITCHING_ROUTINES.agc, both missions (Luminary099 pages
 * 1309-1337, Comanche055 pages 1420-1448): zeroing the IMU's CDUs, coarse
 * and fine align, the program's use of the PIPAs, torquing the gyros, the
 * ends of a mode switch and the stalls that wait for them, R02 (IMU
 * status check) and P06/P05 (standby and back). The faithful C is
 * c64-port/src/imu_mode_switching.c.
 *
 * The missions run the same code but for a few words: the CM names the
 * flag bits RNDREFDR and R02BOTH clear and test by their BIT (and its
 * R02BOTH reads REFSMFLG in STATE +3), its GOODEND has no BZF for a -0
 * MODECADR (a -0 wakes the program like a CADR), and its MODABORT is TC
 * POODOO where the LM's is DXCH BUF2; TC BAILOUT1.
 *
 * Each function below is an entry: a label, or the word after a call out
 * of the file, where that routine returns (the name says which). A run
 * ends where control leaves the file. The subroutines ZEROICDU, SETCOARS,
 * RNDREFDR and NOATTOFF return by TC Q: the AGC runs the word Q holds
 * (the caller's TC address plus one) as TC to it, so Z goes back to the
 * caller and Q becomes 3. */
#ifndef READABLE_IMU_MODE_SWITCHING_H
#define READABLE_IMU_MODE_SWITCHING_H

#include "../agc/names.h"

/* Zeroing (fixed-fixed ZEROICDU; IMUZERO, bank 07). */
void zeroicdu(void);                /* ZEROICDU: TC Q */
void imuzero(void);                 /* IMUZERO */
void imuzero_after_alarm(void);     /* IMUZERO +7: TCF CAGETSTJ +4 */
void imuzeroa_after_waitlist(void); /* IMUZEROA +023 */
void modeexit(void);                /* MODEEXIT */
void imuzero2(void);                /* a task */
void imuzero3(void);                /* VARDELAY's task */
void imuzero3_after_setissw(void);  /* IMUZERO3 +011: TCF ENDIMU */

/* Coarse align. */
void imucoars(void);                /* IMUCOARS */
void imucoars_after_waitlist(void); /* IMUCOARS +7: TCF MODEEXIT */
void coars(void);                   /* a task */
void coars2(void);                  /* VARDELAY's task */
void nextcdu_after_fixdelay(void);  /* NEXTCDU +012 */
void coarserr_after_alarm(void);    /* COARSERR +5: TCF IMUBAD */
void ca_plus_ece(void);             /* CA+ECE, a task */
void setcoars(void);                /* SETCOARS: TC Q */
void rndrefdr(void);                /* RNDREFDR: TC Q */

/* Fine align. */
void imufine(void);                 /* IMUFINE */
void imufine_after_waitlist(void);  /* IMUFINE +015 */
void imufine_after_waitlist2(void); /* IMUFINE +021: TCF MODEEXIT */
void imufined(void);                /* a task */
void ifailok(void);                 /* a task */
void pfailok2_after_setissw(void);  /* PFAILOK2 +4: TCF TASKOVER */
void pfailok(void);                 /* a task */
void noattoff(void);                /* NOATTOFF: TC Q */

/* The PIPAs. */
void pipuse(void);                 /* PIPUSE */
void pipuse1(void);                /* PIPUSE1 */
void pipfree2_after_setissw(void); /* PIPFREE2 +2: TCF MODEEXIT */
void pipfree(void);                /* PIPFREE */
void pipfree_after_alarm(void);    /* PIPFREE +011 */

/* Gyro torquing. */
void imupulse(void);                  /* IMUPULSE */
void gwake2_after_waitlist(void);     /* GWAKE2 +3 */
void gyroagre_after_tpagree(void);    /* GYROAGRE +011 */
void gwake(void);                     /* JOBWAKE's job */
void strtgyro(void);                  /* a task */
void noreset(void);                   /* NORESET: JOBWAKE's return */
void gyroexit(void);                  /* GYROEXIT */
void aug3_after_waitlist(void);       /* AUG3 +6: TCF GYROEXIT */
void eighty_one_ninety_two_aug(void); /* 8192AUG, a task */

/* The ends of a mode switch, and the stalls. */
void endimu(void);                 /* ENDIMU */
void imubad(void);                 /* IMUBAD */
void badend(void);                 /* BADEND */
void goodend(void);                /* GOODEND */
void goodend_after_jobwake(void);  /* ENDMODE -4 */
void aotstall(void);               /* AOTSTALL */
void radstall(void);               /* RADSTALL */
void imustall(void);               /* IMUSTALL */
void modeslp_after_makecadr(void); /* MODESLP +1 */
void gomanur(void);                /* GOMANUR */

/* R02 (LM bank 04, CM bank 07). */
void r02both(void);                /* R02BOTH */
void r02both_after_varalarm(void); /* R02BOTH +013: TC GOTOPOOH */
void r02zero(void);                /* R02ZERO */
void r02zero_after_upflag(void);   /* R02ZERO +2: TCF SWRETURN */

/* P06 and P05 (LM bank 37, CM bank 26). */
void p06(void);                      /* P06 */
void prestand(void);                 /* PRESTAND */
void prestand_retry(void);           /* PRESTAND +5: SCALPREP's L+1 */
void prestand_compatible(void);      /* PRESTAND +6: SCALPREP's L+2 */
void prestand_after_rndrefdr(void);  /* PRESTAND +013 */
void prestand_after_downflag(void);  /* PRESTAND +015 */
void prestand_after_downflag2(void); /* PRESTAND +017 */
void prestand_after_phaschng(void);  /* PRESTAND +027 */
void prestand_after_goperf1(void);   /* PRESTAND +032 to +034 */
void scalprep(void);                 /* SCALPREP */
void scalprep_after_finetime(void);  /* SCALPREP +3 */
void postand(void);                  /* POSTAND, the restart */
void postand_retry(void);            /* POSTAND +010: SCALPREP's L+1 */
void postand_compatible(void);       /* POSTAND +011: SCALPREP's L+2 */
void postand_after_shortmp(void);    /* POSTAND +016 */
void postand_after_tpagree(void);    /* POSTAND +021 */
void postcom_after_tpagree(void);    /* POSTCOM +4 */
void postcom_after_downflag(void);   /* POSTCOM +010: TC GOTOPOOH */

#endif
