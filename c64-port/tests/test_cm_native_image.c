/* SPDX-License-Identifier: MIT */
#include "cm_native_image_check.h"
#include "cm_restart_addresses.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static apollo_overlay_state image;
static cm_native_io io;
static cm_rn_run run;
int main(void) {
 unsigned i,j;cm_executive e;cm_waitlist w;cm_restart_state r;cm_phase_state p;
 assert(cm_image_check(&image,&io,&run)==0);
 for(i=0;i<2048;++i) image.words[i]=(agc_word)((i*73u+05123u)&AGC_MASK);
 image.words[0]=0177777;image.words[1]=0100000;image.words[6]=076007;
 memset(&io,0,sizeof io);io.executive_alarm=01202;io.waitlist_alarm=01203;
 io.superbank=0140;io.interrupt_enabled=1;io.channel11=05001;io.channel13=043777;
 cm_native_gather_executive(&image,&io,&e);cm_native_gather_waitlist(&image,&io,&w);
 cm_native_gather_restart(&image,&io,&r);cm_native_gather_phase(&image,&io,&p);
 for(i=0;i<7;++i) for(j=0;j<12;++j) assert(e.core[i][j]==image.words[0154+i*12+j]);
 for(i=0;i<5;++i) for(j=0;j<44;++j) assert(e.vac[i][j]==image.words[0400+i*44+j]);
 assert(e.alarm==01202 && w.alarm==01203);
 for(i=0;i<CM_RS_WORD_COUNT;++i) assert(r.word[i]==image.words[cm_rs_addresses[i]]);
 assert(p.a==0177777 && p.l==0100000 && p.longbase[0]==image.words[01135]);
 p.scratch[CM_PH_TEMPNM]=01234;cm_native_commit_phase(&image,&io,&p);
 cm_native_gather_executive(&image,&io,&e);assert(e.newprio==01234);
 p.scratch[CM_PH_TEMPBB]=02345;cm_native_commit_phase(&image,&io,&p);
 cm_native_gather_executive(&image,&io,&e);assert(e.locctr==02345);
 assert(image.words[3]==03400 && image.words[4]==076000);
 puts("PASS:sole2048wordCMimage;64restart sequences/320source states;poison/regather each boundary;rawA/L, physicalbanks and scratch aliases");return 0;
}
