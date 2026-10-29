/* SPDX-License-Identifier: MIT */
/* keyrupt-uprupt: KEYRUPT_UPRUPT.agc of Luminary099 and Comanche055 as
 * native routines, against digests frozen from the original programs; every
 * entry and every path is reached; inputs no original producer supplies are
 * refused with the image unchanged. */
#include <stdio.h>
#include <string.h>
#include "keyrupt_uprupt_cases.h"
#include "keyrupt_uprupt_vectors.h"
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
/* Uplink paths: first or second redundancy test failed, ERROR RESET
 * (CLUPLOCK), locked out (RESUME), accepted (NOVAC); and the TS QRUPT skip. */
enum {P_FAIL1,P_FAIL2,P_RESET,P_LOCKED,P_ACCEPTED,P_QSKIP,PATHS};
static unsigned path[PATHS];
static const unsigned resume[2]={05270u,05333u},novac[2]={05072u,05134u},lodsampt[2]={04400u,04414u};
static void refused(unsigned cm,uint16_t n,unsigned word,agc_word value,const char *what){
 static apollo_overlay_state s,before;ku_io io,io0;agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;
 ku_input(cm,&s,&io,n);if(word<2048u)s.words[word]=value;else io.hw.in_interrupt=0;before=s;io0=io;
 program.basic=cm?&cm_keyrupt_routines:&lm_keyrupt_routines;program.basic_count=1;program.hardware=&io.hw;
 call=agc_transfer_resolve(&program,&s,cm?io.cm.superbank:io.lm.superbank);
 check(call&&call(&program,&s,cm?(void *)&io.cm:(void *)&io.lm)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&io0,sizeof io),what,n);
}
int main(void){static apollo_overlay_state s,in;unsigned cm,n,e,kind[3]={0,0,0};
 for(cm=0;cm<2u;++cm){const uint32_t *expected=cm?ku_cm_expected:ku_lm_expected;unsigned seen[KU_ENTRIES];
  memset(seen,0,sizeof seen);
  for(n=0;n<KU_CASES;++n){static ku_io io;unsigned k,entry=n/KU_PER_ENTRY;agc_word z;
   ku_input(cm,&in,&io,(uint16_t)n);
   check(ku_digest(cm,&s,(uint16_t)n)==expected[n],"digest",n);
   k=ku_run(cm,&s,&io,(uint16_t)n);check(k==KU_RETURNED,"outcome",n);if(k<3u)++kind[k];
   ++seen[entry];z=s.words[5];
   check(z==resume[cm]||z==novac[cm]||z==lodsampt[cm],"continuation",n);
   if((entry==0u||entry==4u)&&z!=lodsampt[cm])++path[P_QSKIP];
   if(entry==5u||(entry==4u&&z!=lodsampt[cm])){agc_word w=in.words[045],c=(agc_word)(w&037u);
    if((((w>>5)&037u)^c)!=037u)++path[P_FAIL1];else if(((w>>10)&037u)!=c)++path[P_FAIL2];
    else if(c==022u)++path[P_RESET];else if(in.words[0103]&010u)++path[P_LOCKED];else ++path[P_ACCEPTED];}}
  for(e=0;e<KU_ENTRIES;++e)check(seen[e]==KU_PER_ENTRY,"entry reached",e);}
 for(e=0;e<PATHS;++e)check(path[e]>0u,"path reached",e);
 /* Inputs no producer supplies. */
 refused(0,0,0u,0100000u,"LM KEYRUPT1 with overflow in A");
 refused(1,2u*KU_PER_ENTRY,0u,040000u,"CM KEYCOM with overflow in A");
 refused(0,3u*KU_PER_ENTRY,064u,13u,"LM NOVAC return with LOCCTR not a core set");
 refused(1,3u*KU_PER_ENTRY,064u,84u,"CM NOVAC return with LOCCTR past the core sets");
 refused(1,4u*KU_PER_ENTRY,2048u,0,"CM UPRUPT outside the interrupt");
 refused(0,5u*KU_PER_ENTRY,2048u,0,"LM UPRUPT +4 outside the interrupt");
 printf("keyrupt-uprupt: %u LM and %u CM cases (%u returned, %u refused); redundancy failed first %u, second %u; ERROR RESET %u, locked %u, accepted %u; QRUPT skip %u; %u checks, %u failures\n",
  KU_CASES,KU_CASES,kind[0],kind[2],path[0],path[1],path[2],path[3],path[4],path[5],checks,failures);
 return failures!=0u;
}
