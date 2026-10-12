/* SPDX-License-Identifier: MIT */
#include "agc_transfer.h"
uint16_t agc_fixed_physical(agc_word logical,agc_word fb,agc_word superbank){agc_word bank;
 logical&=07777u;if(logical<02000u)return AGC_TRANSFER_NONE;
 if(logical>=06000u)bank=3;else if(logical>=04000u)bank=2;
 else{bank=(agc_word)((fb&076000u)>>10);if(bank>=030u&&(superbank&0100u))bank=(agc_word)(bank+8u);}
 return (uint16_t)(bank*1024u+(logical&01777u));
}
bool agc_site_lookup(const agc_site_operand *t,uint16_t count,uint16_t physical,agc_word *word){uint16_t lo=0,hi=count,mid;
 if(!t||!word)return false;
 while(lo<hi){mid=(uint16_t)(lo+(hi-lo)/2u);if(t[mid].physical==physical){*word=t[mid].word;return true;}if(t[mid].physical<physical)lo=(uint16_t)(mid+1u);else hi=mid;}
 return false;
}
static agc_routine find(const agc_routine_table *tables,uint8_t count,uint16_t physical){uint8_t i;uint16_t lo,hi,mid;
 if(!tables||physical==AGC_TRANSFER_NONE)return 0;
 for(i=0;i<count;++i){const agc_routine_entry *e=tables[i].entries;lo=0;hi=tables[i].count;
  while(lo<hi){mid=(uint16_t)(lo+(hi-lo)/2u);if(e[mid].physical==physical)return e[mid].call;if(e[mid].physical<physical)lo=(uint16_t)(mid+1u);else hi=mid;}
 }
 return 0;
}
/* Never called: an entry at DANZIG's own address in the interpretive tables
 * marks the program as running pending right-hand orders (agc_transfer.h). */
static enum exec_status danzig_marker(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)s;(void)io;return EXEC_OUT_OF_SCOPE;}
static const agc_routine_entry lm_danzig[1]={{AGC_LM_DANZIG,danzig_marker}},cm_danzig[1]={{AGC_CM_DANZIG,danzig_marker}};
const agc_routine_table agc_lm_danzig_marker={lm_danzig,1},agc_cm_danzig_marker={cm_danzig,1};
static bool interpreting(const agc_native_program *p,const apollo_overlay_state *s,agc_word superbank){uint16_t z;agc_word edop;
 if(p->loc>=APOLLO_SHARED_WORDS||p->bankset>=APOLLO_SHARED_WORDS)return false;
 z=agc_fixed_physical(s->words[5],s->words[4],superbank);
 if(p->interpreter_fetch!=AGC_TRANSFER_NONE&&z==p->interpreter_fetch)return true;
 edop=(agc_word)(s->words[AGC_EDOP]&077777u);
 return (z==AGC_LM_DANZIG||z==AGC_CM_DANZIG)&&edop!=0u&&edop<040000u&&find(p->interpretive,p->interpretive_count,z)==danzig_marker;
}
uint16_t agc_erasable_point(agc_word logical,agc_word eb){
 logical&=07777u;if(logical>=02000u)return AGC_TRANSFER_NONE;
 if(logical>=01400u)logical=(agc_word)((eb&03400u)+(logical&0377u));
 return (uint16_t)(AGC_TRANSFER_ERASABLE_BASE+logical);
}
uint16_t agc_transfer_point(const agc_native_program *p,const apollo_overlay_state *s,agc_word superbank){
 if(!p||!s)return AGC_TRANSFER_NONE;
 if(interpreting(p,s,superbank))return agc_fixed_physical(s->words[p->loc],s->words[p->bankset],superbank);
 if((s->words[5]&07777u)<02000u)return agc_erasable_point(s->words[5],s->words[3]);
 return agc_fixed_physical(s->words[5],s->words[4],superbank);
}
agc_routine agc_transfer_resolve(const agc_native_program *p,const apollo_overlay_state *s,agc_word superbank){uint16_t point;
 if(!p||!s)return 0;
 point=agc_transfer_point(p,s,superbank);
 if(interpreting(p,s,superbank))return find(p->interpretive,p->interpretive_count,point);
 return find(p->basic,p->basic_count,point);
}
bool agc_transfer_operand(const agc_native_program *p,const apollo_overlay_state *s,agc_word superbank,agc_word q,agc_word *word){uint16_t physical;
 if(!p||!s||!word||q>07777u)return false;
 if(q<01400u){if(q<0100u)return false;*word=s->words[q];return true;}
 if(q<02000u){physical=(uint16_t)(s->words[3]+(q&0377u));if(physical<0100u||physical>=APOLLO_SHARED_WORDS)return false;*word=s->words[physical];return true;}
 return agc_site_lookup(p->operands,p->operand_count,agc_fixed_physical(q,s->words[4],superbank),word);
}
enum exec_status agc_transfer_run(const agc_native_program *p,apollo_overlay_state *s,void *io,const agc_word *superbank,uint16_t stop,uint16_t max_steps,uint16_t *steps){uint16_t n=0;enum exec_status status=EXEC_OK;agc_routine call;
 if(!p||!s||!superbank)return EXEC_OUT_OF_SCOPE;
 while(agc_transfer_point(p,s,*superbank)!=stop){
  if(n==max_steps){status=EXEC_OUT_OF_SCOPE;break;}
  call=agc_transfer_resolve(p,s,*superbank);
  if(!call){status=EXEC_UNMAPPED;break;}
  ++n;status=call(p,s,io);
  if(status!=EXEC_OK)break;
 }
 if(steps)*steps=n;
 return status;
}
