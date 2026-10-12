/* SPDX-License-Identifier: MIT */
/* lm-p-axis: Luminary099 P-AXIS_RCS_AUTOPILOT as native routines, against
 * digests frozen from the original program; every routine and every
 * reachable branch is taken; inputs no original producer supplies are
 * refused with the image unchanged. Built with LM_PA_PATH counting
 * branches. */
#include <stdio.h>
#include <string.h>
#include "lm_p_axis_cases.h"
#include "lm_p_axis_vectors.h"
#include "lm_interbank_routines.h"
#define P16(z) (uint16_t)(016u*1024u+((z)-02000u))
#define EB6(a) (03000u+((a)-01400u))
#define PATHS 115u
unsigned lm_pa_paths[PATHS];
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s %u\n",what,n);}}
static agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
static unsigned reached[LM_PA_ENTRIES];
static void refused(apollo_overlay_state *s,lm_flight_io *io,const char *what){apollo_overlay_state before=*s;lm_flight_io bio=*io;agc_routine call;
 call=agc_transfer_resolve(&program,s,0);
 check(call&&call(&program,s,io)==EXEC_OUT_OF_SCOPE&&!memcmp(s,&before,sizeof before)&&!memcmp(io,&bio,sizeof bio),what,0);
}
int main(void){apollo_overlay_state s,in;lm_flight_io io,iin;unsigned n,i,left=0,refusals=0;uint16_t steps;agc_routine call;
 /* Branches no admitted input reaches:
  *  3: 1STOTWOS's CCS A -0 exit: A is an MSU difference, which is never -0;
  *  10, 12: SUBDIVDE's second CCS OMEGAU +0 and -0 exits: it is reached
  *   only with |OMEGAU| above 0631 (AD -OCT630 positive);
  *  14, 50, 53: the overflow skips of TS DAPBOOLS (ALTSYST) and
  *   TS RCSFLAGS (ALTERYZ, ABORTYZ +2): A is BIT1 or AORBSYST RXOR a
  *   sign-extended L (a TS L of a CA'd word, an LXCH of an erasable word),
  *   so its two sign bits stay equal;
  *  60: CCS OLDPMIN's -0 exit: refused (no producer stores -0). */
 static const unsigned unreachable[]={3u,10u,12u,14u,50u,53u,60u};
 program.basic=&lm_p_axis_routines;program.basic_count=1;
 memset(lm_pa_paths,0,sizeof lm_pa_paths);
 for(n=0;n<LM_PA_CASES;++n){unsigned kind;
  check(lm_pa_digest(&s,(uint16_t)n)==lm_pa_expected[n],"digest",n);
  kind=lm_pa_run(&s,&io,(uint16_t)n,&steps);
  if(kind==LM_PA_LEFT){++left;++reached[n%LM_PA_ENTRIES];}
  if(kind==LM_PA_REFUSED){++refusals;lm_pa_input(&in,&iin,(uint16_t)n);
   check(!memcmp(&s,&in,sizeof s)&&!memcmp(&io,&iin,sizeof io),"refused case leaves the image unchanged",n);}
  check(kind!=LM_PA_NONE,"every case leaves the file or is refused",n);}
 check(left+refusals==LM_PA_CASES,"all cases reach their stop",left);
 for(i=0;i<LM_PA_ENTRIES;++i)check(reached[i]>0u,"routine reached",i);
 for(i=0;i<PATHS;++i){unsigned j,skip=0;for(j=0;j<sizeof unreachable/sizeof unreachable[0];++j)skip|=unreachable[j]==i;
  if(!skip)check(lm_pa_paths[i]>0u,"branch taken",i);}
 /* The IBNKCALL CADRs are the compiled interbank table's, and the data
  * words answer for their addresses only. */
 {static const agc_site_operand cadrs[4]={{P16(03153u),040153u},{P16(03423u),037114u},{P16(03477u),043727u},{P16(03511u),037252u}};
  uint16_t ni,k,j;const agc_site_operand *ib=lm_interbank_site_table(&ni);
  for(j=0;j<4u;++j){int found=0;for(k=0;k<ni;++k)found|=ib[k].physical==cadrs[j].physical&&ib[k].word==cadrs[j].word;
   check(found,"IBNKCALL CADR in the compiled table",j);}}
 {agc_word w=0;check(lm_p_axis_data(P16(03615u),&w)&&w==074631u&&!lm_p_axis_data(P16(03616u),&w)&&!lm_p_axis_data(P16(03615u),0),"data words",0);}
 /* Inputs no producer supplies. */
 lm_pa_input(&s,&io,3);s.words[3]=01400u;s.words[6]=(agc_word)(LM_PA_FB16|1u);refused(&s,&io,"BACKP under EBANK1");
 lm_pa_input(&s,&io,0);s.words[3]=03400u;s.words[6]=(agc_word)(LM_PA_FB16|7u);refused(&s,&io,"PAXIS under EBANK7");
 lm_pa_input(&s,&io,11);s.words[2]=01000u;refused(&s,&io,"CALCPERR return into erasable");
 lm_pa_input(&s,&io,11);s.words[2]=050000u;refused(&s,&io,"CALCPERR return not a TC");
 lm_pa_input(&s,&io,8);s.words[EB6(01744u)]=2u;refused(&s,&io,"16,3403 with ROTINDEX 2");
 lm_pa_input(&s,&io,8);s.words[EB6(01744u)]=077777u;refused(&s,&io,"16,3403 with ROTINDEX -0");
 lm_pa_input(&s,&io,3);s.words[EB6(01460u)]=077777u;refused(&s,&io,"BACKP with OLDPMIN -0");
 lm_pa_input(&s,&io,6);s.words[EB6(01460u)]=077777u;refused(&s,&io,"16,2773 with OLDPMIN -0");
 lm_pa_input(&s,&io,3);call=agc_transfer_resolve(&program,&s,0);
 {apollo_overlay_state before=s;check(call&&call(&program,&s,0)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s),"BACKP without io",0);}
 printf("lm-p-axis: %u cases left the file, %u refusals; entries",left,refusals);
 for(i=0;i<LM_PA_ENTRIES;++i)printf(" %u",reached[i]);
 printf("; branches");for(i=0;i<PATHS;++i)printf(" %u",lm_pa_paths[i]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
