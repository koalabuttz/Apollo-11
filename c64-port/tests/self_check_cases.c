/* SPDX-License-Identifier: MIT */
#include "self_check_cases.h"
#include "agc_image_ops.h"
#include "lm_interbank_routines.h"
#include "cm_interbank_routines.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word word(uint32_t *seed){return (agc_word)((rnd(seed)>>12)&AGC_MASK);}
static unsigned upto(uint32_t *seed,unsigned n){return (unsigned)((rnd(seed)>>16)%n);}
#define W s->words
/* A C64 image links one mission (-DSELF_CHECK_LM_ONLY or -DSELF_CHECK_CM_ONLY). */
#if defined(SELF_CHECK_LM_ONLY)
#define PICK(m,lm,cm) ((void)(m),(lm))
#elif defined(SELF_CHECK_CM_ONLY)
#define PICK(m,lm,cm) ((void)(m),(cm))
#else
#define PICK(m,lm,cm) ((m)?(cm):(lm))
#endif
/* A Comanche055 address of the file in the case's mission (Luminary099 is two lower). */
#define K(z) ((agc_word)((z)-(mission?0u:2u)))

const agc_routine_table *self_check_table(unsigned mission){static agc_routine_table t[SELF_CHECK_TABLES];
 t[0]=*PICK(mission,&lm_self_check_idle_routines,&cm_self_check_idle_routines);t[1]=*PICK(mission,&lm_self_check_routines,&cm_self_check_routines);
 return t;}
const agc_site_operand *self_check_site_table(unsigned mission,uint16_t *count){return PICK(mission,lm_self_check_site_table(count),cm_self_check_site_table(count));}
const uint16_t *self_check_entry_list(unsigned mission){return PICK(mission,lm_self_check_entries,cm_self_check_entries);}

agc_word *self_check_superbank(unsigned mission,self_check_io *io){return mission?&io->cm.superbank:&io->lm.superbank;}
agc_word *self_check_interrupts(unsigned mission,self_check_io *io){return mission?&io->cm.interrupt_enabled:&io->lm.interrupt_enabled;}
int self_check_leaves(unsigned mission,agc_word z,agc_word fb,agc_word superbank){
 if(z<02000u)return 0;                                     /* TC Q, TC SFAIL/SKEEP1 run the register's word */
 if(z>=04000u||(fb&076000u)!=(033u<<10)||!(superbank&0100u))return 1;
 return z<K(03234u)||z>K(03725u);
}

/* SKEEP1/SFAIL: a TC to where the file continues, or out of it. */
static agc_word tc_word(unsigned mission,uint32_t *seed,int fail){
 static const agc_word fails[5]={03413u,03430u,03512u,03520u,03724u};
 unsigned m=upto(seed,8);
 if(m==0u)return (agc_word)(04000u+upto(seed,04000u));    /* fixed-fixed */
 if(m==1u)return (agc_word)(02000u+upto(seed,01200u));    /* bank 43 before the file */
 if(fail)return K(fails[upto(seed,5)]);
 return upto(seed,2)?K(03341u):K(03522u);
}
static agc_word smode(uint32_t *seed){unsigned m=upto(seed,8);
 if(m==0u)return 0u;
 if(m==1u)return AGC_MASK;
 if(m<5u)return (agc_word)(1u+upto(seed,8));
 if(m<7u)return (agc_word)(AGC_MASK-1u-upto(seed,8));
 return word(seed);
}
/* ERASCHK's address: an erasable word whose next two words stay at or
 * above 063 after the window of EB bank eb (ERASLOOP takes the pair at it,
 * the next entry the pair after it). */
static agc_word address(unsigned eb,uint32_t *seed){agc_word x;
 if(upto(seed,2))return (agc_word)(063u+upto(seed,01375u-063u));
 x=(agc_word)(01400u+upto(seed,0375u));
 if(eb==0u&&x<01463u)x=(agc_word)(x+063u);
 return x;
}
/* ROPECHK's bank word SKEEP4: the last bank, one before a numbering step
 * (bank 17, 27, a superbank's last) or any. */
static agc_word bankword(uint32_t *seed){unsigned m=upto(seed,6);
 switch(m){
 case 0:return 066100u;                                    /* LSTBNKCH */
 case 1:return 036000u;                                    /* bank 17 */
 case 2:return (agc_word)(056000u|(upto(seed,4)<<4));      /* bank 27 */
 case 3:return (agc_word)(076000u|(upto(seed,4)<<4));      /* bank 37 / a superbank's last */
 default:return (agc_word)((upto(seed,040u)<<10)|(upto(seed,2)?(upto(seed,8)<<4):0u));}
}
static agc_word pm(uint32_t *seed){static const agc_word v[4]={1u,0u,AGC_MASK-1u,AGC_MASK};return v[upto(seed,4)];}

void self_check_input(unsigned mission,apollo_overlay_state *s,self_check_io *io,uint16_t n){
 uint32_t seed=UINT32_C(0x4343)+(uint32_t)n*UINT32_C(7937)+mission*UINT32_C(104729);
 unsigned i,k=n%SELF_CHECK_ROUTINES;agc_word z=self_check_entry_list(mission)[k],eb;
 for(i=0;i<sizeof *io;++i)((uint8_t *)io)[i]=0;
 for(i=0;i<2048u;++i)W[i]=word(&seed);
 W[7]=0;W[0]=agc_extend(W[0]);W[1]=agc_extend(W[1]);
 *self_check_interrupts(mission,io)=(agc_word)upto(&seed,2);*self_check_superbank(mission,io)=upto(&seed,4)?0100u:0140u;
 eb=(agc_word)upto(&seed,8);W[3]=(agc_word)(eb<<8);W[4]=033u<<10;W[6]=(agc_word)(W[4]|eb);W[5]=z;
 W[01362]=smode(&seed);
 if((n/SELF_CHECK_ROUTINES)%16u==1u)W[01362]=0u;            /* SMODE +0 in every slot, whatever the seed gives */
 W[01371]=tc_word(mission,&seed,0);W[01357]=tc_word(mission,&seed,1);
 W[067]=upto(&seed,4)?0u:(agc_word)(1u+upto(&seed,0377u));
 /* ERASCHK state. */
 W[01374]=(agc_word)(upto(&seed,8)<<8);
 W[01377]=address(z==K(03443u)?(unsigned)W[01374]>>8:eb,&seed);
 W[01373]=upto(&seed,2)?(agc_word)(W[01377]+1u):(agc_word)(W[01377]+1u+upto(&seed,4));
 W[01372]=pm(&seed);
 W[01360]=upto(&seed,2)?0u:W[01377];
 /* ROPECHK state at its entries. */
 if(z==K(03542u)||z==K(03627u)||z==K(03625u)||z==K(03635u)||z==K(03675u)||z==K(03713u)){
  W[01373]=upto(&seed,4)?(agc_word)(upto(&seed,01777u)|(upto(&seed,4)?0u:01777u)):01777u;
  if(z==K(03627u)||z==K(03625u))W[01373]=(agc_word)(W[01373]&01776u);
  W[01375]=pm(&seed);W[01376]=pm(&seed);W[01377]=pm(&seed);
  W[01374]=bankword(&seed);
  if(z==K(03713u))switch(upto(&seed,6)){                  /* BNKCHK passes or fails */
   case 0:W[01371]=(agc_word)(W[1]&AGC_MASK);break;
   case 1:W[01371]=(agc_word)((W[1]+1u)&AGC_MASK);break;
   case 2:W[01371]=(agc_word)(~(W[1]+1u)&AGC_MASK);break;
   case 3:W[01371]=upto(&seed,2)?0u:AGC_MASK;break;
   default:break;}
  if(W[01360]&&!upto(&seed,4))W[01377]=address(eb,&seed);
  else W[01360]=0;
 }
 if(z==K(03675u))W[0]=(agc_word)upto(&seed,040u);         /* LEFT5's bank number */
 if(z==K(03542u)&&!upto(&seed,4))W[0]=(agc_word)((W[01373]+02000u)&AGC_MASK);  /* a TC to itself */
}

enum exec_status self_check_run(unsigned mission,apollo_overlay_state *s,self_check_io *io,uint16_t n,uint16_t *steps){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};enum exec_status st;
 program.basic=self_check_table(mission);program.basic_count=SELF_CHECK_TABLES;
 self_check_input(mission,s,io,n);
 st=agc_transfer_run(&program,s,io,self_check_superbank(mission,io),AGC_TRANSFER_NONE,64,steps);
 return st==EXEC_UNMAPPED?EXEC_OK:st==EXEC_OK?EXEC_OUT_OF_SCOPE:st;
}

/* ROPECHK over the whole rope. Stand-ins, in the original's instructions,
 * for the two routines outside the file a whole run passes through and no
 * lane has registered yet: Executive ADVAN (no new job: NEWJOB -0, back to
 * SELFRET through SUPDXCHZ +1; a waiting job: NUCHANG2 or NUDIRECT, where
 * the run ends) and Pinball LEFT5. SUPDACAL and POSTJUMP are the
 * INTER-BANK_COMMUNICATION routines, with the rope image as the program's
 * fixed_data hook. */
#define RK(z) ((agc_word)((z)-(mission?0u:2u)))
static const agc_word advan[2]={03214u,03231u},nuchang2[2]={03011u,03032u},nudirect[2]={03225u,03242u},left5[2]={04331u,04345u};
static enum exec_status advan_body(unsigned mission,apollo_overlay_state *s,agc_word *superbank){agc_word w;
 if((W[4]&076000u)!=(01u<<10))return EXEC_OUT_OF_SCOPE;
 switch(ai_ccs(s,067u)){                                   /* ADVAN CCS NEWJOB */
 case 0:W[5]=nuchang2[mission];return EXEC_OK;             /* TCF NUCHANG2 */
 case 1:ai_caf(s,2u);W[5]=nudirect[mission];return EXEC_OK; /* CAF TWO; TCF NUDIRECT */
 case 2:W[5]=nudirect[mission];return EXEC_OK;             /* TCF NUDIRECT */
 default:break;}
 ai_ca(s,01361u);if(ai_ts(s,1u))return EXEC_OUT_OF_SCOPE;  /* CA SELFRET; TS L */
 ai_caf(s,066102u);                                        /* CAF SELFBANK */
 *superbank=(agc_word)(W[0]&0160u);                        /* SUPDXCHZ +1: EXTEND; WRITE SUPERBNK */
 if(ai_ts(s,6u))return EXEC_OUT_OF_SCOPE;                  /* TS BBANK */
 w=(agc_word)(W[1]&077777u);if(w<02000u||w>=010000u)return EXEC_OUT_OF_SCOPE;
 W[2]=2u;W[5]=w;return EXEC_OK;                            /* TC L: the word in L runs as TC, Q = 2 */
}
static enum exec_status left5_body(apollo_overlay_state *s){agc_word w;
 if(ai_ts(s,022u))return EXEC_OUT_OF_SCOPE;                /* LEFT5 TS CYL */
 ai_cs(s,022u);ai_cs(s,022u);ai_cs(s,022u);ai_cs(s,022u);ai_xch(s,022u);
 w=(agc_word)(W[2]&077777u);if(w<02000u||w>=010000u)return EXEC_OUT_OF_SCOPE;
 W[2]=3u;W[5]=w;return EXEC_OK;                            /* TC Q: the word in Q runs as TC, Q = 3 */
}
static enum exec_status lm_advan(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;return io?advan_body(0u,s,&((p66_bias_io *)io)->superbank):EXEC_OUT_OF_SCOPE;}
static enum exec_status cm_advan(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;return io?advan_body(1u,s,&((cm_native_io *)io)->superbank):EXEC_OUT_OF_SCOPE;}
static enum exec_status any_left5(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;return left5_body(s);}
static const agc_routine_entry lm_stand[2]={{01u*1024u+03214u-02000u,lm_advan},{2048u+04331u-04000u,any_left5}};
static const agc_routine_entry cm_stand[2]={{01u*1024u+03231u-02000u,cm_advan},{2048u+04345u-04000u,any_left5}};
static const agc_routine_table stand_tables[2]={{lm_stand,2},{cm_stand,2}};
const agc_word *self_check_rope_ends(unsigned mission,unsigned *count){static agc_word ends[2][6];unsigned m=mission!=0u;
 ends[m][0]=mission?03011u:03131u;ends[m][1]=mission?02773u:03113u;ends[m][2]=nuchang2[m];ends[m][3]=nudirect[m];
 ends[m][4]=mission?05164u:05122u;ends[m][5]=mission?05652u:05571u;
 if(count){*count=6u;}
 return ends[m];
}
void self_check_rope_input(unsigned mission,apollo_overlay_state *s,self_check_io *io,uint16_t n){
 static const agc_word skeep6[4]={077777u,0u,1u,077776u};
 unsigned e=0;
 while(self_check_entry_list(mission)[e]!=K(03340u))++e;  /* SELFCHK's slot */
 self_check_input(mission,s,io,(uint16_t)(n*SELF_CHECK_ROUTINES+e)); /* a SELFCHK case's image */
 W[067]=077777u;W[01362]=5u;                               /* NEWJOB -0; SMODE option 5: ROPECHK */
 *self_check_superbank(mission,io)=0100u;
 switch(n){
 case 0:break;                                             /* SELFCHK, SMODECHK, SOPTIONS, ROPECHK */
 case 1:W[067]=1u;break;                                   /* a change job waiting: NUCHANG2 */
 case 2:W[067]=0u;break;                                   /* a job in position: NUDIRECT */
 default:W[5]=RK(03524u);W[01376]=n<7u?skeep6[n-3u]:1u;break; /* STSHOSUM with SKEEP6 -0, +0, +1, -1; SHOWSUM's +1 */
 }
 (void)advan;(void)left5;
}
unsigned self_check_rope_proceeds(uint16_t n){return n<7u?0u:(unsigned)n-6u;}
/* SDISPLAY's own words (LXCH SKEEP2: the bank number; LXCH SKEEP3: the sum,
 * the words verb 91 displays), then TC NXTBNK as after PROCEED. */
void self_check_showsum_proceed(unsigned mission,apollo_overlay_state *s){agc_word t;
 t=W[1];W[1]=W[01372];W[01372]=(agc_word)(t&077777u);
 t=W[1];W[1]=W[01373];W[01373]=(agc_word)(t&077777u);
 W[2]=(agc_word)(self_check_rope_ends(mission,0)[1]+9u);W[5]=RK(03635u);
}
enum exec_status self_check_rope_run(unsigned mission,apollo_overlay_state *s,self_check_io *io,uint16_t n,uint32_t *steps){
 agc_routine_table tables[SELF_CHECK_TABLES+2u];agc_native_program p={0,SELF_CHECK_TABLES+2u,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;enum exec_status st;uint32_t k;unsigned proceeds=self_check_rope_proceeds(n);
 tables[0]=self_check_table(mission)[0];tables[1]=self_check_table(mission)[1];
 tables[2]=PICK(mission,lm_interbank_routines,cm_interbank_routines);tables[3]=stand_tables[mission!=0u];
 p.basic=tables;p.operands=self_check_site_table(mission,&p.operand_count);p.fixed_data=PICK(mission,lm_self_check_rope_data,cm_self_check_rope_data);
 self_check_rope_input(mission,s,io,n);
 for(k=0;;++k){
  if(k==SELF_CHECK_ROPE_STEPS){st=EXEC_OUT_OF_SCOPE;break;}
  call=agc_transfer_resolve(&p,s,*self_check_superbank(mission,io));
  if(!call&&proceeds&&s->words[5]==self_check_rope_ends(mission,0)[1]){--proceeds;self_check_showsum_proceed(mission,s);continue;}
  if(!call){st=EXEC_OK;break;}
  st=call(&p,s,io);if(st!=EXEC_OK)break;}
 if(steps)*steps=k;
 return st;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t self_check_rope_digest(unsigned mission,apollo_overlay_state *s,uint16_t n,enum exec_status *status){self_check_io io;unsigned i;
 *status=self_check_rope_run(mission,s,&io,n,0);sum_a=sum_b=0;
 mix(*self_check_interrupts(mission,&io));mix(*self_check_superbank(mission,&io));
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
uint32_t self_check_digest(unsigned mission,apollo_overlay_state *s,uint16_t n,enum exec_status *status){self_check_io io;uint16_t steps=0;unsigned i;
 *status=self_check_run(mission,s,&io,n,&steps);sum_a=sum_b=0;
 mix(*self_check_interrupts(mission,&io));mix(*self_check_superbank(mission,&io));
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
