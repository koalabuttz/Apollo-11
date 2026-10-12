/* SPDX-License-Identifier: MIT */
#ifndef LANEC_DI_SLEEP_H
#define LANEC_DI_SLEEP_H
#include "lanec.h"
/* DISPLAY_INTERFACE_ROUTINES display sleep and wake (both missions, bank 10):
 *   GOSLEEPS -> UPENT2; WAITMASK +1 and XCHSLEEP -> JOBWAKE;
 *   XCHSLEEP +4 (JOBWAKE returns) and +5 -> JOBSLEEP;
 *   JOBXCHS -> JOBWAKE; JOBXCHS +3 (JOBWAKE returns) and +8 -> DOWNENT2;
 *   IDLEMASK +1 -> UPENT2; XCHQADD +1 -> MARKPLAY or OKTOCOPY;
 *   MARKWAKE, WAKEPLAY -> DOWNENT2; MARKFMSK +1 -> JOBWAKE; MARKFMSK +5 -> ENDRET;
 *   INITDSP -> SUPERSW; INITDSP +4 -> PRIOCHNG; INITDSP +6 -> BANKJUMP.
 * Out-of-scope input returns LANEC_OUT_OF_SCOPE before any change. */
enum lanec_status lanec_di_sleep_dispatch(apollo_overlay_state *, lanec_io *, enum lanec_mission);
#endif
