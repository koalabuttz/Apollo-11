/* SPDX-License-Identifier: MIT */
#include "self_check_cases.h"
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s;}
static agc_word word(uint32_t *seed){return (agc_word)((rnd(seed)>>12)&AGC_MASK);}
static unsigned upto(uint32_t *seed,unsigned n){return (unsigned)((rnd(seed)>>16)%n);}
#define W s->words
/* A Comanche055 address of the file in the case's mission (Luminary099 is two lower). */
#define K(z) ((agc_word)((z)-(mission?0u:2u)))

const agc_routine_table *self_check_table(unsigned mission){return mission?&cm_self_check_routines:&lm_self_check_routines;}
const uint16_t *self_check_entry_list(unsigned mission){return mission?cm_self_check_entries:lm_self_check_entries;}

int self_check_leaves(unsigned mission,agc_word z,agc_word fb,agc_word superbank){
 if(z<02000u)return 0;                                     /* TC Q, TC SFAIL/SKEEP1 run the register's word */
 if(z>=04000u||(fb&076000u)!=(033u<<10)||!(superbank&0100u))return 1;
 if(z==K(03561u)||z==K(03562u))return 1;                   /* FXADRS */
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

void self_check_input(unsigned mission,apollo_overlay_state *s,p66_bias_io *io,uint16_t n){
 uint32_t seed=UINT32_C(0x4343)+(uint32_t)n*UINT32_C(7937)+mission*UINT32_C(104729);
 unsigned i,k=n%SELF_CHECK_ROUTINES;agc_word z=self_check_entry_list(mission)[k],eb;
 for(i=0;i<2048u;++i)W[i]=word(&seed);
 W[7]=0;W[0]=agc_extend(W[0]);W[1]=agc_extend(W[1]);
 io->interrupt_enabled=(uint16_t)upto(&seed,2);io->superbank=upto(&seed,4)?0100u:0140u;
 eb=(agc_word)upto(&seed,8);W[3]=(agc_word)(eb<<8);W[4]=033u<<10;W[6]=(agc_word)(W[4]|eb);W[5]=z;
 W[01362]=smode(&seed);
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

enum exec_status self_check_run(unsigned mission,apollo_overlay_state *s,p66_bias_io *io,uint16_t n,uint16_t *steps){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};enum exec_status st;
 program.basic=self_check_table(mission);program.basic_count=1;
 self_check_input(mission,s,io,n);
 st=agc_transfer_run(&program,s,io,&io->superbank,AGC_TRANSFER_NONE,64,steps);
 return st==EXEC_UNMAPPED?EXEC_OK:st==EXEC_OK?EXEC_OUT_OF_SCOPE:st;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t self_check_digest(unsigned mission,apollo_overlay_state *s,uint16_t n,enum exec_status *status){p66_bias_io io;uint16_t steps=0;unsigned i;
 *status=self_check_run(mission,s,&io,n,&steps);sum_a=sum_b=0;
 mix(io.interrupt_enabled);mix(io.superbank);
 for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<2u?0177777u:077777u)));
 return (uint32_t)sum_b<<16|sum_a;
}
