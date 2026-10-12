/* SPDX-License-Identifier: MIT */
/* lm-lunar-landing: Luminary099 THE_LUNAR_LANDING as native routines,
 * against digests frozen from the original program; every routine, every
 * reachable instruction and order pair is taken; inputs no original
 * producer supplies are refused with the image unchanged. Built with
 * LLD_PATH counting. */
#include <stdio.h>
#include <string.h>
#include "lm_lunar_landing_cases.h"
#include "lm_lunar_landing_vectors.h"
#define PATHS 456u
unsigned lld_paths[PATHS];
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s %u\n",what,n);}}
static apollo_overlay_state undo;
static agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
static unsigned reached[LLD_ROUTINES];
/* Case n's image with `edit` applied must be refused, image unchanged. */
static void refused(uint16_t n,void (*edit)(apollo_overlay_state *,lm_lld_io *),const char *what){
 apollo_overlay_state s,before;lld_case c;lm_lld_io io;agc_routine call;
 lld_input(&s,&c,n);io.control.superbank=c.superbank;io.control.interrupt_enabled=c.interrupt_enabled;
 io.chan12=c.chan12;io.chan33=c.chan33;io.undo=&undo;
 call=agc_transfer_resolve(&program,&s,c.superbank);
 edit(&s,&io);before=s;
 check(call&&call(&program,&s,&io)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s),what,n);
}
static void fb10(apollo_overlay_state *s,lm_lld_io *io){(void)io;s->words[4]=020000u;s->words[6]=(agc_word)(020000u|(s->words[3]>>8));}
static void bankset10(apollo_overlay_state *s,lm_lld_io *io){(void)io;s->words[0165]=020000u;}
static void vac_bad(apollo_overlay_state *s,lm_lld_io *io){(void)io;s->words[0120]=0402u;}
static void pushloc_bad(apollo_overlay_state *s,lm_lld_io *io){(void)io;s->words[0166]=(agc_word)(s->words[0120]+043u);}
static void edop(apollo_overlay_state *s,lm_lld_io *io){(void)io;s->words[023]=1u;}
static void newjob_neg(apollo_overlay_state *s,lm_lld_io *io){(void)io;s->words[067]=040000u;}
static void a_overflow(apollo_overlay_state *s,lm_lld_io *io){(void)io;s->words[0]=040000u;}
static void no_undo(apollo_overlay_state *s,lm_lld_io *io){(void)s;io->undo=0;}
/* The routine index of a key (bank 31 or 32, address a). */
static uint16_t slot(unsigned bank,unsigned a){unsigned i;
 for(i=0;i<LLD_ROUTINES;++i)if(lld_routine_keys[i]==bank*1024u+a-02000u)return (uint16_t)i;
 return 0;}
int main(void){apollo_overlay_state s;unsigned n,i,stopped=0,refusals=0;
 /* Paths no admitted input reaches: 0702 and 0703, UNIT's and ABVAL's
  * alarm 1302 (SQRTSUB's abort on a negative argument); both take the
  * square root of VSQ's sum of squares, which the sampled vectors, zero
  * vectors included, never make negative. */
 static const unsigned unreachable[]={0702u,0703u};
 /* Every instruction address (a - 02144 in bank 31, 0100 + a - 02776 in
  * bank 32), order pair (0400 + LOC - 02154, 0440 + LOC - 03020) and
  * decision outcome (0700 + n) of the file. */
 static const unsigned paths[]={00u,02u,03u,05u,07u,034u,035u,037u,040u,041u,042u,050u,0100u,0102u,0104u,0105u,0106u,0107u,0110u,0111u,0112u,0113u,0114u,0115u,0116u,0117u,0120u,0121u,0204u,0205u,0304u,0306u,0307u,0311u,0312u,0325u,0326u,0327u,0330u,0332u,0333u,0335u,0337u,0340u,0341u,0342u,0343u,0344u,0345u,0347u,0350u,0351u,0353u,0400u,0403u,0406u,0410u,0413u,0416u,0420u,0422u,0423u,0433u,0435u,0437u,0440u,0443u,0446u,0451u,0454u,0456u,0460u,0462u,0464u,0467u,0471u,0475u,0477u,0501u,0503u,0505u,0507u,0510u,0512u,0514u,0516u,0520u,0524u,0527u,0531u,0534u,0536u,0540u,0542u,0545u,0547u,0552u,0553u,0554u,0556u,0560u,0561u,0563u,0566u,0571u,0573u,0575u,0577u,0601u,0603u,0606u,0610u,0612u,0614u,0616u,0620u,0621u,0631u,0635u,0637u,0641u,0642u,0700u,0701u,0702u,0703u,0704u,0705u,0706u,0707u};
 {static agc_routine_table basic[1],interp[1];basic[0]=lm_lunar_landing_routines;interp[0]=lm_lunar_landing_interpretive;
  program.basic=basic;program.basic_count=1;program.interpretive=interp;program.interpretive_count=1;
  program.interpreter_fetch=(uint16_t)(2048u+06070u-04000u);program.loc=0164;program.bankset=0165;}
 memset(lld_paths,0,sizeof lld_paths);
 for(n=0;n<LLD_CASES;++n){unsigned kind;
  check(lld_digest(&s,(uint16_t)n)==lld_expected[n],"digest",n);
  kind=lld_run(&s,(uint16_t)n,0);
  if(kind==LLD_STOPPED){++stopped;++reached[n%LLD_ROUTINES];}
  if(kind==LLD_REFUSED)++refusals;
  check(kind!=3u,"every case leaves the file or is refused",n);}
 for(i=0;i<LLD_ROUTINES;++i)check(reached[i]>0u,"routine reached",i);
 for(i=0;i<sizeof paths/sizeof paths[0];++i){unsigned j,skip=0;
  for(j=0;j<sizeof unreachable/sizeof unreachable[0];++j)skip|=unreachable[j]==paths[i];
  if(!skip)check(lld_paths[paths[i]]>0u,"path taken",paths[i]);}
 /* Inputs no producer supplies. */
 refused(slot(032u,02776u),fb10,"P63LM under FB 10");
 refused(slot(031u,02144u),fb10,"LANDJUNK under FB 10");
 refused(slot(032u,03040u),bankset10,"32,3040 with BANKSET bank 10");
 refused(slot(032u,03044u),vac_bad,"32,3044 with a work area off the VAC areas");
 refused(slot(032u,03051u),pushloc_bad,"32,3051 with PUSHLOC outside the work area");
 refused(slot(031u,02167u),edop,"31,2167 with EDOP set");
 refused(slot(032u,03044u),newjob_neg,"32,3044 with NEWJOB negative");
 refused(slot(032u,03102u),a_overflow,"DDUMCALC with overflow in A");
 refused(slot(032u,03002u),no_undo,"32,3002 without an undo image");
 printf("lm-lunar-landing: %u cases left the file, %u refusals; routines",stopped,refusals);
 for(i=0;i<LLD_ROUTINES;++i)printf(" %u",reached[i]);
 printf("; paths");for(i=0;i<PATHS;++i)printf(" %u",lld_paths[i]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
