/* SPDX-License-Identifier: MIT */
/* lm-ags-init: Luminary099 AGS_INITIALIZATION as native routines, against
 * digests frozen from the original program; every routine, every
 * reachable instruction and order pair is taken; inputs no original
 * producer supplies are refused with the image unchanged. Built with
 * AGS_PATH counting. */
#include <stdio.h>
#include <string.h>
#include "lm_ags_init_cases.h"
#include "lm_ags_init_vectors.h"
#include "lm_interp_orders.h"
#include "lm_interp_steps.h"
#define PATHS 210u
unsigned ags_paths[PATHS];
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s %u\n",what,n);}}
static apollo_overlay_state undo;
static agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
static unsigned reached[AGS_ROUTINES];
/* Case n's image with `edit` applied must be refused, image unchanged. */
static void refused(uint16_t n,void (*edit)(apollo_overlay_state *,lm_ags_io *),const char *what){
 apollo_overlay_state s,before;ags_case c;lm_ags_io io;agc_routine call;
 ags_input(&s,&c,n);io.control.superbank=c.superbank;io.control.interrupt_enabled=c.interrupt_enabled;io.undo=&undo;
 call=agc_transfer_resolve(&program,&s,c.superbank);
 edit(&s,&io);before=s;
 check(call&&call(&program,&s,&io)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s),what,n);
}
static void fb10(apollo_overlay_state *s,lm_ags_io *io){(void)io;s->words[4]=020000u;s->words[6]=(agc_word)(020000u|(s->words[3]>>8));}
static void bankset10(apollo_overlay_state *s,lm_ags_io *io){(void)io;s->words[0165]=020000u;}
static void vac_bad(apollo_overlay_state *s,lm_ags_io *io){(void)io;s->words[0120]=0402u;}
static void pushloc_bad(apollo_overlay_state *s,lm_ags_io *io){(void)io;s->words[0166]=(agc_word)(s->words[0120]+043u);}
static void edop(apollo_overlay_state *s,lm_ags_io *io){(void)io;s->words[023]=1u;}
static void newjob_neg(apollo_overlay_state *s,lm_ags_io *io){(void)io;s->words[067]=040000u;}
static void qpret_erasable(apollo_overlay_state *s,lm_ags_io *io){(void)io;s->words[s->words[0120]+052u]=01234u;}
static void no_undo(apollo_overlay_state *s,lm_ags_io *io){(void)s;io->undo=0;}
/* The QPRET the CALL SCALEVEC at loc stores, through lmi_call. */
static agc_word call_qpret(agc_word loc){static apollo_overlay_state t;
 memset(&t,0,sizeof t);
 t.words[4]=LM_AGS_FB32;t.words[3]=02000u;t.words[6]=064004u;t.words[0165]=064004u;t.words[0115]=040000u;
 t.words[0120]=0401u;t.words[0166]=0405u;t.words[0164]=loc;t.words[5]=06070u;
 lmi_newops(&t,077624u);lmi_misc(&t,064132u,02507u);lmi_call(&t);
 return t.words[0401u+052u];
}
int main(void){apollo_overlay_state s;unsigned n,i,stopped=0,refusals=0;uint16_t ie;
 /* Paths no admitted input reaches:
  *  0302, 0303, 0312, 0313: the negative and -0 exits of AGSINIT's and
  *   the IMUSE test's CCS A after MASK of a positive constant (REFSMBIT,
  *   IMUSEBIT), which is never negative (the -0 exit would run TC ENDEXT,
  *   the negative one OCT 220 as TC 00220);
  *  0306: TS DNLSTCOD's overflow skip after CA AGSWORD, which loads a
  *   sign-extended word and never overflows. */
 static const unsigned unreachable[]={0302u,0303u,0306u,0312u,0313u};
 /* Every instruction address (a - 02005), order pair (0200 + LOC - 02037,
  * 0240 + LOC - 02132) and decision outcome (0300 + n) of the file. */
 static const unsigned paths[]={00u,01u,02u,03u,04u,06u,07u,010u,011u,012u,013u,014u,015u,016u,020u,021u,022u,023u,024u,025u,026u,027u,030u,031u,036u,037u,041u,065u,066u,067u,070u,072u,073u,074u,075u,076u,077u,0100u,0101u,0102u,0103u,0104u,0105u,0107u,0110u,0112u,0114u,0115u,0117u,0120u,0122u,0123u,0124u,0200u,0202u,0210u,0212u,0214u,0216u,0220u,0222u,0224u,0226u,0231u,0232u,0240u,0243u,0245u,0250u,0252u,0254u,0257u,0261u,0264u,0266u,0271u,0274u,0300u,0301u,0302u,0303u,0304u,0305u,0306u,0307u,0310u,0311u,0312u,0313u,0314u,0315u,0316u,0317u,0320u,0321u};
 {static agc_routine_table basic[1],interp[1];basic[0]=lm_ags_init_routines;interp[0]=lm_ags_init_interpretive;
  program.basic=basic;program.basic_count=1;program.interpretive=interp;program.interpretive_count=1;
  program.interpreter_fetch=(uint16_t)(2048u+06070u-04000u);program.loc=0164;program.bankset=0165;}
 memset(ags_paths,0,sizeof ags_paths);
 for(n=0;n<AGS_CASES;++n){unsigned kind;
  check(ags_digest(&s,(uint16_t)n)==ags_expected[n],"digest",n);
  kind=ags_run(&s,(uint16_t)n,&ie);
  if(kind==AGS_STOPPED){++stopped;++reached[n%AGS_ROUTINES];}
  if(kind==AGS_REFUSED)++refusals;
  check(kind!=3u,"every case leaves the file or is refused",n);}
 for(i=0;i<AGS_ROUTINES;++i)check(reached[i]>0u,"routine reached",i);
 for(i=0;i<sizeof paths/sizeof paths[0];++i){unsigned j,skip=0;
  for(j=0;j<sizeof unreachable/sizeof unreachable[0];++j)skip|=unreachable[j]==paths[i];
  if(!skip)check(ags_paths[paths[i]]>0u,"path taken",paths[i]);}
 /* CALL SCALEVEC stores the QPRET the cases use. */
 check(call_qpret(02053u)==AGS_QPRET_2053&&call_qpret(02061u)==AGS_QPRET_2061,"QPRET of CALL SCALEVEC",0);
 /* Inputs no producer supplies. */
 refused(0,fb10,"AGSINIT under FB 10");
 refused(AGS_BASIC_ROUTINES,bankset10,"32,2051 with BANKSET bank 10");
 refused(AGS_BASIC_ROUTINES+1u,vac_bad,"32,2053 with a work area off the VAC areas");
 refused(AGS_BASIC_ROUTINES+2u,pushloc_bad,"32,2055 with PUSHLOC outside the work area");
 refused(AGS_BASIC_ROUTINES+3u,edop,"32,2061 with EDOP set");
 refused(AGS_BASIC_ROUTINES+4u,newjob_neg,"32,2063 with NEWJOB negative");
 refused(AGS_BASIC_ROUTINES+6u,qpret_erasable,"32,2156 with QPRET in erasable");
 refused(5,no_undo,"32,2046 without an undo image");
 printf("lm-ags-init: %u cases left the file, %u refusals; routines",stopped,refusals);
 for(i=0;i<AGS_ROUTINES;++i)printf(" %u",reached[i]);
 printf("; paths");for(i=0;i<PATHS;++i)printf(" %u",ags_paths[i]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
