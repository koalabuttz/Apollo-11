/* SPDX-License-Identifier: MIT */
/* cm-alarm-sites on the C64: the native ALARM/ALARM2/abort-word entries take
 * their inline codes from the compiled site table. No rope reader or overlay is
 * linked. Each case's resulting image digest must equal the host native run's,
 * which the differential proves equal to the original assembly.
 * Mailbox (tools/vice_run.py): $c001-2 failures, $c003-4 checks, $a5 at $c000. */
#include <stdint.h>
#include "cm_alarm_sites_cases.h"
#include "cm-alarm-sites-vectors.h"
static apollo_overlay_state image;
static cm_native_io io;
static uint16_t sum_a,sum_b;
static void word(uint16_t w){sum_a=(uint16_t)(sum_a+w);sum_b=(uint16_t)(sum_b+sum_a);}
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc000;uint16_t i,failures=0,checks=0;

 for(i=0;i<CM_ALARM_SITES_TARGET_CASES;++i){uint32_t n=cm_alarm_sites_expected[i].n;uint16_t k;enum exec_status status;
  enum cm_alarm_sites_kind kind=cm_alarm_sites_input(&image,&io,n);
  status=cm_alarm_sites_run(&image,&io,kind);
  sum_a=0;sum_b=0;word((uint16_t)status);for(k=0;k<2048u;++k)word(image.words[k]);
  word(io.channel11);word(io.channel12);word(io.channel13);word(io.channel14);word(io.main_keys);word(io.navigation_keys);
  word(io.superbank);word(io.interrupt_enabled);word(io.trap31a);word(io.trap31b);word(io.trap32);word(io.executive_alarm);word(io.waitlist_alarm);
  ++checks;if(status!=EXEC_OK||(((uint32_t)sum_b<<16)|sum_a)!=cm_alarm_sites_expected[i].digest)++failures;
 }
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checks;mail[4]=(uint8_t)(checks>>8);mail[0]=0xa5;
 for(;;){}
}
