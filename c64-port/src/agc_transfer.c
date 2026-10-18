/* SPDX-License-Identifier: MIT */
#include "agc_transfer.h"
/* Built a byte at a time: the high byte is the bank times four (FB's bank
 * bits already sit there) plus Z's top two bits. */
uint16_t agc_fixed_physical(agc_word logical, agc_word fb, agc_word superbank)
{
    uint8_t hi;
    logical &= 07777u;
    if (logical < 02000u) return AGC_TRANSFER_NONE;
    if (logical >= 04000u)
        hi = (uint8_t)(logical >= 06000u ? 014u : 010u);
    else {
        hi = (uint8_t)((fb >> 8) & 0174u);
        if (hi >= 0140u && (superbank & 0100u)) hi = (uint8_t)(hi + 040u);
    }
    return (uint16_t)((unsigned)(hi | ((uint8_t)(logical >> 8) & 3u)) << 8 | (logical & 0377u));
}
bool agc_resume_exact(const agc_native_program *p,const apollo_overlay_state *s,agc_word fb,agc_word eb,agc_word superbank){
 agc_word z=(agc_word)((s->words[015]-1u)&07777u),w;
 if(z>=02000u){if(!p->fixed_data||!p->fixed_data(p,agc_fixed_physical(z,fb,superbank),&w))return true;}
 else w=s->words[z<01400u?z:((eb>>8)&7u)*0400u+(z-01400u)];
 return ((w^s->words[017])&077777u)==0u;
}
bool agc_site_lookup(const agc_site_operand *t,uint16_t count,uint16_t physical,agc_word *word){uint16_t lo=0,hi=count,mid;
 if(!t||!word)return false;
 while(lo<hi){mid=(uint16_t)(lo+(hi-lo)/2u);if(t[mid].physical==physical){*word=t[mid].word;return true;}if(t[mid].physical<physical)lo=(uint16_t)(mid+1u);else hi=mid;}
 return false;
}
/* Entries are sorted by physical, so a table whose first..last range misses
 * the point is skipped without a search (most tables, for most points). */
#ifdef AGC_TRANSFER_FIND_ASM
/* The C64 resident's find (platform/c64/agc_find.S): the same result as the
 * C below, which stays the reference (tools/agc_find_check.c, make
 * c64-agc-find, compares the two in mos-sim). */
agc_routine agc_find(const agc_routine_table *tables, uint8_t count, uint16_t physical);
#define find agc_find
#else
static agc_routine find(const agc_routine_table *tables,uint8_t count,uint16_t physical){uint16_t lo,hi,mid;
 if(!tables||physical==AGC_TRANSFER_NONE)return 0;
 for(;count;--count,++tables){const agc_routine_entry *e=tables->entries;lo=0;hi=tables->count;
  if(!hi||physical<e[0].physical||physical>e[hi-1u].physical)continue;
  while(lo<hi){mid=(uint16_t)(lo+(hi-lo)/2u);if(e[mid].physical==physical)return e[mid].call;if(e[mid].physical<physical)lo=(uint16_t)(mid+1u);else hi=mid;}
 }
 return 0;
}
#endif
#ifdef AGC_TRANSFER_DANZIG_MARKER
/* Never called: an entry at DANZIG's own address in the interpretive tables
 * marks the program as running pending right-hand orders (agc_transfer.h). */
static enum exec_status danzig_marker(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)s;(void)io;return EXEC_OUT_OF_SCOPE;}
static const agc_routine_entry lm_danzig[1]={{AGC_LM_DANZIG,danzig_marker}},cm_danzig[1]={{AGC_CM_DANZIG,danzig_marker}};
const agc_routine_table agc_lm_danzig_marker={lm_danzig,1},agc_cm_danzig_marker={cm_danzig,1};
#endif
/* z: the fixed point at Z (agc_fixed_physical of Z, FB). */
static bool interpreting(const agc_native_program *p,const apollo_overlay_state *s,uint16_t z){
 if(p->loc>=APOLLO_SHARED_WORDS||p->bankset>=APOLLO_SHARED_WORDS)return false;
 if(p->interpreter_fetch!=AGC_TRANSFER_NONE&&z==p->interpreter_fetch)return true;
#ifdef AGC_TRANSFER_DANZIG_MARKER
 /* The marker is the first interpretive table (by identity, so an empty {0,0}
  * table is never read), keyed at the mission's DANZIG; EDOP holds an order
  * (nonzero, bit 15 clear). */
 {agc_word edop;const agc_routine_entry *m;
 if(!p->interpretive_count)return false;
 m=p->interpretive[0].entries;
 if(m==lm_danzig?z!=AGC_LM_DANZIG:m!=cm_danzig||z!=AGC_CM_DANZIG)return false;
 edop=s->words[AGC_EDOP];
 return !(edop&040000u)&&(edop&037777u);}
#else
 (void)s;return false;
#endif
}
uint16_t agc_erasable_point(agc_word logical,agc_word eb){
 logical&=07777u;if(logical>=02000u)return AGC_TRANSFER_NONE;
 if(logical>=01400u)logical=(agc_word)((eb&03400u)+(logical&0377u));
 return (uint16_t)(AGC_TRANSFER_ERASABLE_BASE+logical);
}
/* Z in erasable is never the interpreter's fetch or DANZIG (both fixed), so
 * the fixed point at Z is computed once and is the answer unless it is. */
static uint16_t transfer_point(const agc_native_program *p,const apollo_overlay_state *s,agc_word superbank,bool *in){uint16_t z;
 *in=false;
 if((s->words[5]&07777u)<02000u)return agc_erasable_point(s->words[5],s->words[3]);
 z=agc_fixed_physical(s->words[5],s->words[4],superbank);
 if((*in=interpreting(p,s,z)))return agc_fixed_physical(s->words[p->loc],s->words[p->bankset],superbank);
 return z;
}
static agc_routine resolve_at(const agc_native_program *p,uint16_t point,bool in){
 return find(in?p->interpretive:p->basic,in?p->interpretive_count:p->basic_count,point);
}
uint16_t agc_transfer_point(const agc_native_program *p,const apollo_overlay_state *s,agc_word superbank){bool in;
 if(!p||!s)return AGC_TRANSFER_NONE;
 return transfer_point(p,s,superbank,&in);
}
uint16_t agc_transfer_next(const agc_native_program *p, const apollo_overlay_state *s,
                           agc_word superbank, uint16_t stop, agc_routine *call)
{
    uint16_t point;
    bool in;
    *call = 0;
    if (!p || !s) return AGC_TRANSFER_NONE;
    point = transfer_point(p, s, superbank, &in);
    if (point != stop) *call = resolve_at(p, point, in);
    return point;
}
agc_routine agc_transfer_resolve(const agc_native_program *p, const apollo_overlay_state *s,
                                 agc_word superbank)
{
    agc_routine call;
    (void)agc_transfer_next(p, s, superbank, AGC_TRANSFER_NONE, &call);
    return call;
}
bool agc_transfer_operand(const agc_native_program *p,const apollo_overlay_state *s,agc_word superbank,agc_word q,agc_word *word){uint16_t physical;
 if(!p||!s||!word||q>07777u)return false;
 if(q<01400u){if(q<0100u)return false;*word=s->words[q];return true;}
 if(q<02000u){physical=(uint16_t)(s->words[3]+(q&0377u));if(physical<0100u||physical>=APOLLO_SHARED_WORDS)return false;*word=s->words[physical];return true;}
 return agc_site_lookup(p->operands,p->operand_count,agc_fixed_physical(q,s->words[4],superbank),word);
}
/* One transfer point per step: the stop check and the routine lookup share it. */
enum exec_status agc_transfer_run(const agc_native_program *p,apollo_overlay_state *s,void *io,const agc_word *superbank,uint16_t stop,uint16_t max_steps,uint16_t *steps){uint16_t n=0,point;enum exec_status status=EXEC_OK;agc_routine call;bool in;
 if(!p||!s||!superbank)return EXEC_OUT_OF_SCOPE;
 while((point=transfer_point(p,s,*superbank,&in))!=stop){
  if(n==max_steps){status=EXEC_OUT_OF_SCOPE;break;}
  call=resolve_at(p,point,in);
  if(!call){status=EXEC_UNMAPPED;break;}
  ++n;status=call(p,s,io);
  if(status!=EXEC_OK)break;
 }
 if(steps)*steps=n;
 return status;
}
