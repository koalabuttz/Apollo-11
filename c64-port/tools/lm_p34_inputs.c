/* SPDX-License-Identifier: MIT */
/* Host helper for tools/lm_p34_differential.py: for each case n given on
 * the command line (an interpretive-entry case), prints n, whether its run
 * reaches VN1645's CALL GET+MGA (NEWOPS 35,3630: 1, else 0), and the words
 * its lm-vn1645 or lm-midgim generator sets over that generator's debris
 * (address word pairs, octal). Fails if the device record differs. */
#include <stdio.h>
#include <stdlib.h>
#include "lm_p34_cases.h"
#include "lm_vn1645_cases.h"
#include "lm_midgim_cases.h"
/* C64 group 2's program: everything but VN1645's GET+MGA pair. */
static const agc_routine_entry get45[1]={{LM_P34_B35(03635),lm_p34_get45_comptgo}};
static const agc_routine_entry some[4]={{LM_P34_B10(02000),lm_p34_selectmu},{LM_P34_B35(03606),lm_p34_vn1645},
 {LM_P34_B35(03634),lm_p34_get45},{LM_P34_B35(03671),lm_p34_clupdate}};
static const agc_routine_table basic[1]={{get45,1}},interp[1]={{some,4}};
int main(int argc,char **argv){int a;agc_native_program program={basic,1,interp,1,0,0,LM_P34_FETCH,0164,0165,0,0,0};
 for(a=1;a<argc;++a){uint32_t n=(uint32_t)strtoul(argv[a],0,0),k=n-LM_P34_BASIC_CASES;apollo_overlay_state g,d,r;p66_bias_io gi,di,ri;unsigned i,mga;
  if(n<LM_P34_BASIC_CASES||n>=LM_P34_CASES)return 2;
  if(k<LM_P34_CLUPDATE_CASES){lm_vn1645_input(&g,&gi,k);lm_vn1645_debris(&d,&di,k);}
  else if((k-=LM_P34_CLUPDATE_CASES)<LM_P34_VN1645_CASES){lm_vn1645_input(&g,&gi,k);lm_vn1645_debris(&d,&di,k);}
  else{k-=LM_P34_VN1645_CASES;lm_midgim_input(&g,&gi,k);lm_midgim_debris(&d,&di,k);}
  if(gi.superbank!=di.superbank||gi.interrupt_enabled!=di.interrupt_enabled)return 3;
  (void)lm_p34_run_program(&program,&r,&ri,n);
  mga=r.words[5]==06070u&&r.words[4]==072000u&&r.words[0164]==03630u;
  printf("%lu %u",(unsigned long)n,mga);
  for(i=0;i<2048u;++i)if(g.words[i]!=d.words[i])printf(" %o %o",i,(unsigned)g.words[i]);
  printf("\n");}
 return 0;}
