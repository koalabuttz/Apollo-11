/* SPDX-License-Identifier: MIT */
/* lm-q-r-axis: Luminary099 Q_R-AXIS_RCS_AUTOPILOT as native routines,
 * against digests frozen from the original program; every routine and
 * every reachable branch is taken; inputs no original producer supplies
 * are refused with the image unchanged. Built with LM_QR_PATH counting
 * branches. */
#include <stdio.h>
#include <string.h>
#include "lm_q_r_axis_cases.h"
#include "lm_q_r_axis_vectors.h"
#include "lm_interbank_routines.h"
#define P17(z) (uint16_t)(017u*1024u+((z)-02000u))
#define EB6(a) (03000u+((a)-01400u))
#define PATHS 169u
unsigned lm_qr_paths[PATHS];
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s %u\n",what,n);}}
static agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
static unsigned reached[LM_QR_ENTRIES];
static void refused(apollo_overlay_state *s,lm_flight_io *io,const char *what){apollo_overlay_state before=*s;lm_flight_io bio=*io;agc_routine call;
 call=agc_transfer_resolve(&program,s,0);
 check(call&&call(&program,s,io)==EXEC_OUT_OF_SCOPE&&!memcmp(s,&before,sizeof before)&&!memcmp(io,&bio,sizeof bio),what,0);
}
int main(void){apollo_overlay_state s,in;lm_flight_io io,iin;unsigned n,i,left=0,refusals=0;uint16_t steps;agc_routine call;
 /* Branches no admitted input reaches:
  *  4, 5, 16, 17, 27, 28, 32, 33, 36, 37, 40, 41, 44, 45, 48, 49, 66, 67,
  *   117, 118, 131, 132, 135, 136: the negative and -0 exits of a CCS of a
  *   MASK result (CCS A after MASK of a positive constant or of a channel
  *   bit, CCS POLYTEMP of TYPEPOLY MASK ALLJETS), which is never negative;
  *  9, 10: FAILOOP's CCS NUMBERT negative and -0 exits: NUMBERT counts down
  *   from THREE by CCS;
  *  147, 148: LOOPER's CCS AXISCTR negative and -0 exits: AXISCTR is 0 or
  *   1 (stored by the file's loop; refused otherwise at a return entry);
  *  152: LOOPER's TC RETJADR to another word: RETJADR is stored by the
  *   file only (MINRTN, BACKHAND, TJLAW +3; refused otherwise at a return
  *   entry);
  *  155, 157: DOSKIP's CCS TJU +0 and -0 exits: DOSKIP follows DOROTAT,
  *   whose CCS TJU sends +-0 to NOROTAT, and nothing between stores TJU. */
 static const unsigned unreachable[]={4u,5u,9u,10u,16u,17u,27u,28u,32u,33u,36u,37u,40u,41u,44u,45u,48u,49u,66u,67u,
  117u,118u,131u,132u,135u,136u,147u,148u,152u,155u,157u};
 program.basic=&lm_q_r_axis_routines;program.basic_count=1;
 memset(lm_qr_paths,0,sizeof lm_qr_paths);
 for(n=0;n<LM_QR_CASES;++n){unsigned kind;
  check(lm_qr_digest(&s,(uint16_t)n)==lm_qr_expected[n],"digest",n);
  kind=lm_qr_run(&s,&io,(uint16_t)n,&steps);
  if(kind==LM_QR_LEFT){++left;++reached[n%LM_QR_ENTRIES];}
  if(kind==LM_QR_REFUSED){++refusals;lm_qr_input(&in,&iin,(uint16_t)n);
   check(!memcmp(&s,&in,sizeof s)&&!memcmp(&io,&iin,sizeof io),"refused case leaves the image unchanged",n);}
  check(kind!=LM_QR_NONE,"every case leaves the file or is refused",n);}
 check(left+refusals==LM_QR_CASES,"all cases reach their stop",left);
 for(i=0;i<LM_QR_ENTRIES;++i)check(reached[i]>0u,"routine reached",i);
 for(i=0;i<PATHS;++i){unsigned j,skip=0;for(j=0;j<sizeof unreachable/sizeof unreachable[0];++j)skip|=unreachable[j]==i;
  if(!skip)check(lm_qr_paths[i]>0u,"branch taken",i);}
 /* The IBNKCALL CADRs are the compiled interbank table's, and the data
  * words answer for their addresses only. */
 {static const agc_site_operand cadrs[4]={{P17(02213u),040153u},{P17(02453u),040153u},{P17(02617u),043330u},{P17(02723u),043727u}};
  uint16_t ni,k,j;const agc_site_operand *ib=lm_interbank_site_table(&ni);
  for(j=0;j<4u;++j){int found=0;for(k=0;k<ni;++k)found|=ib[k].physical==cadrs[j].physical&&ib[k].word==cadrs[j].word;
   check(found,"IBNKCALL CADR in the compiled table",j);}}
 {agc_word w=0;check(lm_q_r_axis_data(P17(02572u),&w)&&w==012566u&&!lm_q_r_axis_data(P17(02573u),&w)&&!lm_q_r_axis_data(P17(02572u),0),"data words",0);}
 /* Inputs no producer supplies. */
 lm_qr_input(&s,&io,0);s.words[3]=01400u;s.words[6]=(agc_word)(LM_QR_FB17|1u);refused(&s,&io,"CALLQERR under EBANK1");
 lm_qr_input(&s,&io,16);s.words[3]=02400u;s.words[6]=(agc_word)(LM_QR_FB17|5u);refused(&s,&io,"CLOSEOUT under EBANK5");
 lm_qr_input(&s,&io,5);s.words[2]=01000u;refused(&s,&io,"QERRCALC return into erasable");
 lm_qr_input(&s,&io,13);s.words[2]=050000u;refused(&s,&io,"JTLST return not a TC");
 lm_qr_input(&s,&io,14);s.words[2]=0u;refused(&s,&io,"ROT-TOUV return to A");
 lm_qr_input(&s,&io,0);s.words[EB6(01535u)]=1u;refused(&s,&io,"CALLQERR with SKIPU 1");
 lm_qr_input(&s,&io,8);s.words[EB6(01536u)]=077777u;refused(&s,&io,"17,2775 with SKIPV -0");
 lm_qr_input(&s,&io,0);s.words[EB6(01502u)]=077777u;refused(&s,&io,"CALLQERR with ALLOWGTS -0");
 lm_qr_input(&s,&io,0);io.chan5=040000u;refused(&s,&io,"CALLQERR with channel 5 negative");
 lm_qr_input(&s,&io,9);s.words[EB6(01505u)]=2u;refused(&s,&io,"17,3004 with AXISCTR 2");
 lm_qr_input(&s,&io,6);s.words[EB6(01743u)]=5u;refused(&s,&io,"17,2714 with NUMBERT 5");
 lm_qr_input(&s,&io,6);s.words[EB6(01500u)]=6u;refused(&s,&io,"17,2714 with SENSETYP 6");
 lm_qr_input(&s,&io,9);s.words[EB6(01505u)]=1u;s.words[EB6(01477u)]=0u;refused(&s,&io,"17,3004 with AXISCTR 1 and RETJADR 0");
 lm_qr_input(&s,&io,10);s.words[EB6(01505u)]=1u;s.words[EB6(01477u)]=02274u;refused(&s,&io,"17,3042 with AXISCTR 1 and RETJADR MINRTN +1");
 lm_qr_input(&s,&io,0);call=agc_transfer_resolve(&program,&s,0);
 {apollo_overlay_state before=s;check(call&&call(&program,&s,0)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s),"CALLQERR without io",0);}
 printf("lm-q-r-axis: %u cases left the file, %u refusals; entries",left,refusals);
 for(i=0;i<LM_QR_ENTRIES;++i)printf(" %u",reached[i]);
 printf("; branches");for(i=0;i<PATHS;++i)printf(" %u",lm_qr_paths[i]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
