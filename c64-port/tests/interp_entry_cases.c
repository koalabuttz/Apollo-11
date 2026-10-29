/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "interp_entry_cases.h"
agc_word ie_entry(unsigned mission,uint16_t n){static const agc_word off[3]={0,2,011};
 return (agc_word)((mission?INTERP_ENTRY_CM_INTPRET:INTERP_ENTRY_LM_INTPRET)+off[n/IE_PER_ENTRY]);}
static uint32_t rnd(uint32_t *s){*s=*s*1664525u+1013904223u;return *s>>9;}
static agc_word bbcon(uint32_t *r){return (agc_word)((rnd(r)%32u)*02000u|(rnd(r)%8u));}
/* A fixed-memory address: the word after a TC, or an interpretive LOC. */
static agc_word fixed(uint32_t *r,unsigned bank_only){uint32_t x=rnd(r);return (agc_word)(02000u+x%(bank_only?02000u:06000u));}
void ie_input(unsigned mission,apollo_overlay_state *m,ie_io *io,uint16_t n){uint32_t r=0x6037u+mission*0x9e3779b9u+n*2654435761u;unsigned i,k=n%8u;agc_word bb;
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)(rnd(&r)&AGC_MASK);
 m->words[7]=0;
 /* Raw 16-bit A, L and Q; in most cases L and Q hold what a caller leaves:
  * a return address (TC), a LOC (interpretive branch) or a BBCON (INTRSM). */
 m->words[0]=(agc_word)(rnd(&r)&0177777u);m->words[1]=(agc_word)(rnd(&r)&0177777u);m->words[2]=(agc_word)(rnd(&r)&0177777u);
 if(k<6u){m->words[2]=fixed(&r,0);m->words[1]=n/IE_PER_ENTRY==2u?agc_extend(bbcon(&r)):fixed(&r,n/IE_PER_ENTRY==1u);}
 bb=bbcon(&r);m->words[6]=(agc_word)(bb&076007u);m->words[3]=(agc_word)((bb&7u)<<8);m->words[4]=(agc_word)(bb&076000u);
 m->words[5]=ie_entry(mission,n);
 /* Not the E/CALL form (Q at E/CALL's CADR in bank 04 or 13), which INTPRET runs on through its CALL. */
 if(n<IE_PER_ENTRY&&m->words[2]==(mission?02064u:02624u)&&m->words[4]==(mission?026000u:010000u))m->words[2]^=1u;
 memset(io,0,sizeof *io);
 if(mission){io->cm.superbank=(agc_word)((rnd(&r)%8u)*020u);io->cm.interrupt_enabled=(agc_word)(rnd(&r)&1u);}
 else{io->lm.superbank=(agc_word)((rnd(&r)%8u)*020u);io->lm.interrupt_enabled=(agc_word)(rnd(&r)&1u);}
}
unsigned ie_run(unsigned mission,apollo_overlay_state *s,ie_io *io,uint16_t n){
 agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};agc_routine call;agc_word sb;
 program.basic=mission?&cm_interpreter_routines:&lm_interpreter_routines;program.basic_count=1;
 ie_input(mission,s,io,n);sb=mission?io->cm.superbank:io->lm.superbank;
 call=agc_transfer_resolve(&program,s,sb);
 if(!call)return 3u;
 return call(&program,s,mission?(void *)&io->cm:(void *)&io->lm)==EXEC_OK?IE_RETURNED:IE_REFUSED;
}
static uint16_t sum_a,sum_b;
static void mix(uint16_t v){sum_a=(uint16_t)(sum_a+v);sum_b=(uint16_t)(sum_b+sum_a);}
uint32_t ie_digest(unsigned mission,apollo_overlay_state *s,uint16_t n){static ie_io io;unsigned i,kind=ie_run(mission,s,&io,n);
 sum_a=sum_b=0;mix((uint16_t)kind);
 if(kind!=IE_REFUSED){for(i=0;i<2048u;++i)mix((uint16_t)(s->words[i]&(i<3u?0177777u:077777u)));
  mix(mission?io.cm.superbank:io.lm.superbank);mix(mission?io.cm.interrupt_enabled:io.lm.interrupt_enabled);}
 return (uint32_t)sum_b<<16|sum_a;
}
