/* SPDX-License-Identifier: MIT */
/* cm-alarm on the C64: every sixteenth Comanche055 ALARM_AND_ABORT case
 * and the cover rows (tests/cm_alarm_c64_rows.h, picked by host branch arc)
 * through the trampoline, against digests frozen from the original program
 * (tools/cm_alarm_freeze.py; the rows' digests equal it by the host test).
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "cm_alarm_cases.h"
#include "cm_alarm_c64_vectors.h"
#include "cm_alarm_c64_rows.h"
static apollo_overlay_state image;
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t failures=0,checks=0,n;enum exec_status st;
 for(n=0;n<CM_AL_C64_COUNT;++n){++checks;if(cm_al_digest(&image,(uint16_t)(n*CM_AL_C64_STRIDE),&st)!=cm_al_c64[n]||st!=EXEC_OK)++failures;}
#if CM_AL_C64_ROWS
 for(n=0;n<CM_AL_C64_ROWS;++n){++checks;if(cm_al_digest(&image,cm_al_c64_row[n],&st)!=cm_al_c64_row_digest[n]||st!=EXEC_OK)++failures;}
#endif
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
