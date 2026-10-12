/* SPDX-License-Identifier: MIT */
#include "imu_compensation_cases.h"
/* The C64 diagnostics build subsets: -DICP_ENTRY_ONLY (entry cases, this
 * file's routines only) and -DICP_NO_EXEC (round trips without Luminary
 * NOVAC) with -DICP_TRIP_LM or -DICP_TRIP_CM (one mission's INTER-BANK
 * routines). Host builds have everything. */
#ifndef ICP_ENTRY_ONLY
#define AGC_SITES_CALLEES 1
#ifndef ICP_TRIP_CM
#define ICP_LM_TRIPS 1
#include "lm_interbank_routines.h"
#include "../src/lm_interbank_sites.inc"
#endif
#ifndef ICP_TRIP_LM
#define ICP_CM_TRIPS 1
#include "cm_interbank_routines.h"
#include "../src/cm_interbank_sites.inc"
#endif
#endif
#if !defined ICP_ENTRY_ONLY && !defined ICP_NO_EXEC
#include "lm_exec_routines.h"
#include "p66_native.h"
#include "../src/lm_exec_sites.inc"
#define ICP_EXEC 1
#endif
#define ANY_FB 0177777u
static const agc_word lm_points[ICP_LM_ROUTINES]={LM_ICP_1PIPA,LM_ICP_NOVAC_RETURN,LM_ICP_1GYRO,LM_ICP_IMUPULSE_RETURN,
 LM_ICP_IMUSTALL_RETURN,LM_ICP_GCOMP1,LM_ICP_NBDONLY,LM_ICP_PIPASR_RETURN,LM_ICP_LASTBIAS,LM_ICP_PIPUSE1_RETURN,LM_ICP_GCOMPZER};
static const agc_word cm_points[ICP_ROUTINES-ICP_LM_ROUTINES]={CM_ICP_1PIPA,CM_ICP_NOVAC_RETURN,CM_ICP_1GYRO,CM_ICP_IMUPULSE_RETURN,
 CM_ICP_IMUSTALL_RETURN,CM_ICP_GCOMP1,CM_ICP_NBDONLY,CM_ICP_LASTBIAS,CM_ICP_PIPUSE_RETURN};
static const agc_word lm_ext[6]={LM_ICP_SWRETURN,LM_ICP_BANKCALL,LM_ICP_IBNKCALL,LM_ICP_NOVAC,LM_ICP_ENDOFJOB,0};
static const agc_word cm_ext[5]={CM_ICP_SWRETURN,CM_ICP_BANKCALL,CM_ICP_NOVAC,CM_ICP_ENDOFJOB,0};
/* Round trips stop at the caller's continuation, or where native code ends:
 * Luminary's NOVAC alarms (BAILOUT1 05716, POODOO1 05726) through native
 * NOVAC, or the NOVAC entry itself (Comanche, and the Luminary cohort run
 * without the Executive). */
static const agc_word lm_trip_ext[3]={05716u,05726u,0};
static const agc_word lm_plain_ext[2]={LM_ICP_NOVAC,0};
static const agc_word cm_trip_ext[2]={CM_ICP_NOVAC,0};
const agc_word *icp_externals(const icp_case *c){
 if(c->stop_z)return c->cm?cm_trip_ext:c->exec?lm_trip_ext:lm_plain_ext;
 return c->cm?cm_ext:lm_ext;
}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
#ifndef ICP_ENTRY_ONLY
/* Round-trip site k (LM 0..3, CM 4..5): its index in the interbank table. */
static uint16_t site_index(uint16_t k,uint8_t *cm){uint16_t i,seen=0;
#ifdef ICP_LM_TRIPS
 for(i=0;i<LM_INTERBANK_SITES_COUNT;++i)if(lm_interbank_sites_callee[i]==LM_ICP_BANKCALL&&(lm_interbank_sites[i].word==015263u||lm_interbank_sites[i].word==015701u)){if(seen==k){*cm=0;return i;}++seen;}
#else
 seen=ICP_LM_SITES;
#endif
#ifdef ICP_CM_TRIPS
 for(i=0;i<CM_INTERBANK_SITES_COUNT;++i)if(cm_interbank_sites_callee[i]==CM_ICP_BANKCALL&&cm_interbank_sites[i].word==015262u){if(seen==k){*cm=1;return i;}++seen;}
#endif
 *cm=2;(void)k;return 0;
}
static uint16_t site_physical(uint8_t cm,uint16_t k){
#ifdef ICP_LM_TRIPS
 if(!cm)return lm_interbank_sites[k].physical;
#endif
#ifdef ICP_CM_TRIPS
 if(cm==1u)return cm_interbank_sites[k].physical;
#endif
 (void)cm;(void)k;return 0;
}
/* Site and state of round trip n. */
static uint16_t trip(uint16_t n,unsigned *v){
 if(n<ICP_PLAIN_FIRST){*v=(n-ICP_TRIP_FIRST)/ICP_SITES;return (uint16_t)((n-ICP_TRIP_FIRST)%ICP_SITES);}
 *v=(n-ICP_PLAIN_FIRST)/ICP_LM_SITES;return (uint16_t)((n-ICP_PLAIN_FIRST)%ICP_LM_SITES);
}
#endif
#ifdef ICP_EXEC
/* Canonical Executive (as the lm-exec-services cases): cores busy core
 * sets, the first vacs VAC areas in use. */
static void canon(apollo_overlay_state *m,uint8_t cores,uint8_t vacs){executive e;unsigned i;
 p66_native_import_executive(&e,m);e.alarm=0;e.newjob=0;
 for(i=0;i<EXEC_CORE_COUNT;++i)e.core[i][EXEC_PRIORITY]=i<cores?(agc_word)(010110u+i*01000u):AGC_NEG_ZERO;
 for(i=0;i<EXEC_VAC_COUNT;++i)e.vac[i][0]=i<vacs?0:(agc_word)(0400u+i*44u);
 p66_native_export_executive(m,&e);
}
#endif
static void set_banks(apollo_overlay_state *m,agc_word fb,agc_word eb){m->words[4]=fb;m->words[3]=eb;m->words[6]=(agc_word)(fb|(eb>>8));}
/* A word of one of four CCS classes. */
static agc_word ccs_class(uint32_t *seed,unsigned k){agc_word v=(agc_word)((rnd(seed)>>17)%037776u+1u);
 return k==0u?v:k==1u?0u:k==2u?(agc_word)(~v&077777u):077777u;
}
void icp_input(apollo_overlay_state *m,icp_case *c,uint16_t n){uint32_t seed=0x1a7aul+(uint32_t)n*7u;unsigned i,v,eb;agc_word z,fb=014000u;
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)((rnd(&seed)>>12)&AGC_MASK);
 m->words[7]=0;for(i=010;i<=061u;++i)m->words[i]=0;
 m->words[025]=(agc_word)((rnd(&seed)>>17)&037777u);m->words[024]=(agc_word)((rnd(&seed)>>17)&037777u);
 m->words[0]=agc_extend(m->words[0]);m->words[1]=agc_extend(m->words[1]);
 c->stop_z=c->stop_fb=0;c->superbank=0;c->interrupt_enabled=(uint16_t)(n&1u);c->exec=0;
 if(n<ICP_ENTRY_CASES){uint16_t r=(uint16_t)(n%ICP_ROUTINES);v=n/ICP_ROUTINES;
  c->cm=r>=ICP_LM_ROUTINES;z=c->cm?cm_points[r-ICP_LM_ROUTINES]:lm_points[r];
 }else{
#ifdef ICP_ENTRY_ONLY
  c->cm=0;v=0;z=0;
#else
  uint8_t cm;uint16_t k=site_index(trip(n,&v),&cm),ph;
  /* A mission this build leaves out: cm 2, run refuses the case. */
  c->cm=cm;ph=site_physical(cm,k);if(cm>1u){c->stop_z=1;return;}
  fb=(agc_word)((ph/1024u)<<10);z=cm?CM_ICP_BANKCALL:LM_ICP_BANKCALL;
  m->words[2]=(agc_word)(02000u+ph%1024u);
  c->stop_z=(agc_word)(m->words[2]+1u);c->stop_fb=fb;c->interrupt_enabled=1;
  c->exec=(uint8_t)(!cm&&n<ICP_PLAIN_FIRST);
#ifdef ICP_EXEC
  if(c->exec)canon(m,(uint8_t)(v%9u==0u?8u:1u+v%7u),(uint8_t)(v%11u==0u?5u:v%5u));
#endif
#endif
 }
 /* EB: GCOMP's bank 3 mostly; 1/PIPA and GCOMPZER set it themselves. */
 eb=(v%4u==3u)?(unsigned)((rnd(&seed)>>20)%8u):3u;
 set_banks(m,fb,(agc_word)(eb<<8));
 m->words[5]=z;
 /* The compensation words in the bank the code addresses them in. */
 {unsigned b=(c->stop_z||z==LM_ICP_1PIPA||z==CM_ICP_1PIPA||z==LM_ICP_GCOMPZER)?3u:eb,base=b*0400u;
  /* Small coefficients and accumulations in a quarter of the states, so
   * the commands stay at two pulses or less. */
  if(v%4u==1u){
   for(i=052;i<=076u;++i){agc_word w=(agc_word)((rnd(&seed)>>17)%8u);m->words[base+i]=(rnd(&seed)>>16)&1u?(agc_word)(~w&077777u):w;}
   for(i=0;i<6u;++i)m->words[(c->cm?01162u:0324u)+i]=(agc_word)((rnd(&seed)>>17)%4u);
  }
  m->words[base+077u]=ccs_class(&seed,v%4u);                 /* GCOMPSW */
 }
 m->words[076]=ccs_class(&seed,(v/4u)%4u);                    /* FLAGWRD2 */
 if(!c->cm&&(z==LM_ICP_PIPASR_RETURN))m->words[0141]=(v&8u)?0200u:0u; /* TEM1 as NBDONLY stores it */
}
#ifdef ICP_EXEC
/* Luminary round trips read BANKCALL CADRs and the NOVAC 2CADR from one
 * operand table: the two generated site tables merged by address. */
static agc_site_operand lm_operands[LM_INTERBANK_SITES_COUNT+LM_EXEC_SITES_COUNT];
static const agc_site_operand *lm_trip_operands(uint16_t *count){static uint16_t n;
 if(!n){uint16_t i=0,j=0;
  while(i<LM_INTERBANK_SITES_COUNT||j<LM_EXEC_SITES_COUNT)
   lm_operands[n++]=(j>=LM_EXEC_SITES_COUNT||(i<LM_INTERBANK_SITES_COUNT&&lm_interbank_sites[i].physical<lm_exec_sites[j].physical))?lm_interbank_sites[i++]:lm_exec_sites[j++];}
 *count=n;return lm_operands;
}
#endif
static const agc_routine_table *mine(uint8_t cm){return cm?&cm_imu_compensation_routines:&lm_imu_compensation_routines;}
static int external(const icp_case *c,agc_word z){const agc_word *e=icp_externals(c);for(;*e;++e)if(*e==z)return 1;return 0;}
unsigned icp_run(apollo_overlay_state *s,uint16_t n,uint16_t *ie){icp_case c;agc_routine_table tables[3];
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0};p66_bias_io lio;cm_native_io cio;void *io;enum exec_status st;uint16_t steps,stop;
 icp_input(s,&c,n);
 lio.superbank=0;lio.interrupt_enabled=c.interrupt_enabled;
 {unsigned i;uint16_t *p=(uint16_t *)&cio;for(i=0;i<sizeof cio/sizeof *p;++i)p[i]=0;}
 cio.interrupt_enabled=c.interrupt_enabled;io=c.cm?(void *)&cio:(void *)&lio;
 tables[0]=*mine(c.cm);program.basic=tables;program.basic_count=1;
 if(c.stop_z){
#ifdef ICP_ENTRY_ONLY
  return 3u;
#else
  if(c.cm>1u)return 3u;
#ifdef ICP_CM_TRIPS
  if(c.cm){tables[1]=cm_interbank_routines;program.basic_count=2;program.operands=cm_interbank_site_table(&program.operand_count);}
#else
  if(c.cm)return 3u;
#endif
#ifdef ICP_LM_TRIPS
  else if(!c.exec){tables[1]=lm_interbank_routines;program.basic_count=2;program.operands=lm_interbank_site_table(&program.operand_count);}
#endif
  else{
#ifdef ICP_EXEC
   tables[1]=lm_interbank_routines;tables[2]=lm_exec_routines;program.basic_count=3;program.operands=lm_trip_operands(&program.operand_count);
#else
   return 3u;
#endif
  }
#endif
 }
 stop=c.stop_z?agc_fixed_physical(c.stop_z,c.stop_fb,c.superbank):(uint16_t)0xfffeu;
 st=agc_transfer_run(&program,s,io,&c.superbank,stop,32,&steps);
 if(ie)*ie=c.cm?cio.interrupt_enabled:lio.interrupt_enabled;
 if(st==EXEC_OK)return ICP_STOPPED;
 if(st==EXEC_UNMAPPED&&external(&c,s->words[5]))return ICP_STOPPED;
 if(st==EXEC_OUT_OF_SCOPE)return ICP_REFUSED;
 return 3u;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t icp_digest(apollo_overlay_state *s,uint16_t n){uint16_t ie;unsigned i,kind=icp_run(s,n,&ie);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=ICP_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));mix(ie);}
 return (uint32_t)sum_b<<16|sum_a;
}
