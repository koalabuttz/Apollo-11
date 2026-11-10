/* SPDX-License-Identifier: MIT */
/* AGC_BLOCK_TWO_SELF-CHECK.agc, both missions (Luminary099 pages
 * 1284-1293, Comanche055 pages 1394-1403): self-check, which the
 * computer runs as its back-up idle loop, and the bank sums of verb 91
 * (SHOW-BANKSUM). The faithful C is c64-port/src/self_check_body.inc.
 *
 * Every pass leaves the file, and each place it comes back to is a
 * routine here: through SELFRET after CHECKNJ (the Executive's ADVAN
 * finds no job), after ALARM2, SUPDACAL, CHANG1, LEFT5 or SDISPLAY, and
 * where TC SFAIL or TC SKEEP1 sends it. */
#ifndef READABLE_SELF_CHECK_H
#define READABLE_SELF_CHECK_H

#include "../agc/agc.h"

/* The idle loop: SELFCHK, SMODECHK, CHECKNJ. */
void selfchk(void);
void smodechk(void);
void smodechk_plus2(void); /* TC CHECKNJ */
void smodechk_plus3(void); /* CCS SMODE, back from CHECKNJ */
void smodechk_plus5(void); /* TC SMODECHK +2 */
void checknj(void);
void checknj_plus2(void); /* TC POSTJUMP */

/* The options SMODE picks. */
void smodechk_plus4(void); /* TC SOPTIONS */
void smodechk_plus6(void); /* TC SOPTIONS */
void smodechk_plus7(void); /* INCR SCOUNT */
void smodechk_plus8(void); /* TC SKEEP1 */
void tcalarm2_plus2(void); /* back from ALARM2 */

/* ERASCHK, CNTRCHK and CYCLSHFT. */
void eraschk(void);
void erasloop_plus18(void); /* after the first -1CHK */
void erasloop_plus31(void); /* after the second */
void eloopfin_plus2(void);  /* back from CHECKNJ */
void cyclshft_plus11(void); /* after the first -1CHK */
void cyclshft_plus17(void); /* after the second */

/* ROPECHK and STSHOSUM. */
void ropechk(void);
void stshosum(void);
void comadrs_plus5(void); /* back from SUPDACAL */
void comadrs_plus6(void); /* back from ADSUM */
void fxadrs_plus3(void);  /* back from ADSUM */
void adsum(void);
void continu_plus5(void); /* back from CHANG1 */
void adrs_plus_1(void);
void nxtbnk(void);
void soption_plus3(void); /* back from LEFT5 */
void sopt_plus4(void);    /* back from SDISPLAY */
void bnkchk_plus4(void);  /* after BNKCHK's -1CHK */

#endif
