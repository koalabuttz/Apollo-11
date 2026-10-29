/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_irq_sequence.h"
static uint8_t enter(void *user) { (void)user; return 0; }
static void leave(void *user,uint8_t token) { (void)user; (void)token; }
static const exec_exclusion x={enter,leave,0};
static void row(void *user,uint8_t step,const cm_executive *s,enum exec_status status,exec_address next) {
 uint8_t i,j; uint8_t n=*(uint8_t *)user; printf("%u %u %d",n,step,status);
 for(i=0;i<7;++i) for(j=0;j<12;++j) printf(" %u",s->core[i][j]);
 for(i=0;i<5;++i) for(j=0;j<44;++j) printf(" %u",s->vac[i][j]);
 printf(" %u %u %u %u %u %u %u %u %u %u\n",s->newjob,s->newprio,s->newloc[0],s->newloc[1],s->locctr,s->fixloc,s->ovfind,s->alarm,next.location,next.bank);
}
int main(void) { uint8_t n; cm_executive s; for(n=42;n<50;++n) if(cm_irq_sequence_run(&s,n,&x,0,0,row,&n,1,0)!=0) return 2; return 0; }
