/* SPDX-License-Identifier: MIT */
/* lm-dap-interface: Luminary099 DAP_INTERFACE_SUBROUTINES as native
 * routines, against digests frozen from the original program; every routine
 * is reached and composes with the native NOVAC; inputs no original producer
 * supplies are refused with the image unchanged. */
#include <stdio.h>
#include <string.h>
#include "lm_dap_interface_cases.h"
#include "lm_dap_interface_vectors.h"
#define P(z) (uint16_t)(020u*1024u+((z)-02000u))
#define FF(z) (uint16_t)(2048u+((z)-04000u))
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s case %u\n",what,n);}}
static agc_routine_table tables[2];
static agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
#define POINTS 13u
static const uint16_t points[POINTS]={P(02123u),P(02124u),P(02127u),P(02137u),P(02140u),P(02142u),P(02145u),P(02153u),P(02165u),P(02204u),P(02207u),P(02225u),FF(LM_EXEC_NOVAC)};
static unsigned reached[POINTS];
static void cover(uint16_t n){apollo_overlay_state s;p66_bias_io io;lm_dap_case c;uint16_t point,stop;unsigned i,k;agc_routine call;
 lm_dap_input(&s,&io,&c,n);stop=agc_fixed_physical(c.stop_z,c.stop_fb==0177777u?0u:c.stop_fb,c.superbank);
 for(k=0;k<24u;++k){point=agc_transfer_point(&program,&s,c.superbank);if(point==stop)return;
  call=agc_transfer_resolve(&program,&s,c.superbank);if(!call)return;
  for(i=0;i<POINTS;++i)if(points[i]==point)++reached[i];
  if(call(&program,&s,&io)!=EXEC_OK)return;}
}
static enum exec_status run_from(apollo_overlay_state *s){p66_bias_io io={0,0};agc_word sb=0;uint16_t steps;
 return agc_transfer_run(&program,s,&io,&sb,0xfffeu,24,&steps);
}
/* A refusal leaves the image as it was. */
static void refused(apollo_overlay_state *s,const char *what){apollo_overlay_state before=*s,at=*s;agc_routine call;
 at.words[4]=LM_DAP_FB;call=agc_transfer_resolve(&program,&at,0);  /* the routine at Z in bank 20 */
 check(call&&call(&program,s,0)==EXEC_OUT_OF_SCOPE&&!memcmp(s,&before,sizeof before),what,0);
}
int main(void){apollo_overlay_state s;p66_bias_io io;lm_dap_case c;unsigned n,i,kind[2]={0,0};uint16_t steps;
 tables[0]=lm_dap_interface_routines;tables[1]=lm_exec_routines;program.basic=tables;program.basic_count=2;
 program.operands=lm_exec_site_table(&program.operand_count);
 for(n=0;n<LM_DAP_CASES;++n){unsigned k=lm_dap_run(&s,(uint16_t)n,&steps);
  check(lm_dap_digest(&s,(uint16_t)n)==lm_dap_expected[n],"digest",n);
  check(k<2u,"outcome",n);if(k<2u)++kind[k];
  cover((uint16_t)n);}
 for(i=0;i<POINTS;++i)check(reached[i]>0u,"routine reached",i);
 /* Inputs no producer supplies. */
 lm_dap_input(&s,&io,&c,5);s.words[2]=0100u;refused(&s,"STOPRATE Q into erasable");
 lm_dap_input(&s,&io,&c,4);s.words[2]=030000u;refused(&s,"ZATTEROR Q not a TC");
 lm_dap_input(&s,&io,&c,0);s.words[4]=s.words[6]=042000u;refused(&s,"RESTORDB outside bank 20");
 lm_dap_input(&s,&io,&c,0);s.words[5]=02137u;s.words[LM_DAP_RUPTREG1]=01777u;refused(&s,"RUPTREG1 into erasable");
 lm_dap_input(&s,&io,&c,6);s.words[5]=02225u;s.words[LM_DAP_RUPTREG2]=050000u;refused(&s,"RUPTREG2 not a TC");
 lm_dap_input(&s,&io,&c,6);s.words[5]=02207u;s.words[0]=0100000u;refused(&s,"ALLCOAST TS with A overflowed");
 /* ALLCOAST reaches its caller through STOPRATE, RESTORDB+1 and NOVAC. */
 lm_dap_input(&s,&io,&c,6);check(run_from(&s)==EXEC_UNMAPPED&&s.words[5]==c.stop_z,"ALLCOAST unmapped at caller",6);
 printf("lm-dap-interface: %u cases (%u returned, %u alarm); routines",LM_DAP_CASES,kind[0],kind[1]);
 for(i=0;i<POINTS;++i)printf(" %u",reached[i]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
