/* SPDX-License-Identifier: MIT */
#include "lm_t6rupt_routines.h"
#include "agc_image_ops.h"
#define W s->words
#define FF(z) (uint16_t)(2048u+((z)-04000u))
#define B17(z) (uint16_t)(017u*1024u+((z)-02000u))
#define POSMAX 037777u
#define PRIO37 037000u
#define BIT15 040000u
#define OCT00314 000314u

/* Channel instructions (yaAGC): the overflow-corrected A combines with the
 * channel; WAND and WOR write the result back, A takes it sign-extended.
 * The CPU cannot leave channel 13's trap-reset bits set. */
static agc_word a15(const apollo_overlay_state *s){return agc_correct(W[0]);}
static void wand13(apollo_overlay_state *s,agc_hardware *h){agc_word v=(agc_word)(a15(s)&h->channel13);h->channel13=(agc_word)(v&043777u);W[0]=agc_extend(v);}
static void wor13(apollo_overlay_state *s,agc_hardware *h){agc_word v=(agc_word)(a15(s)|h->channel13);h->channel13=(agc_word)(v&043777u);W[0]=agc_extend(v);}
static void rand5(apollo_overlay_state *s,const agc_hardware *h){W[0]=agc_extend((agc_word)(a15(s)&h->channel5));}
/* TC Q: Z goes to the return address in Q, and executing Q as TC sets Q 3. */
static int returnable(const apollo_overlay_state *s){return W[2]>=02000u&&W[2]<=07777u;}
static void tc_q(apollo_overlay_state *s){W[5]=W[2];W[2]=3;}
static int overflowed(agc_word a){return ((a>>15)^(a>>14))&1u;}
static int eb6(const apollo_overlay_state *s){return W[3]==03000u;}

/* WRITEU (05760), WRITEV (05771): TS L; CS (or CA) 00314OCT; EXTEND; RAND
 * CHAN5; AD L; EXTEND; WRITE CHAN5; TC Q. */
static void write5(apollo_overlay_state *s,agc_hardware *h,int v){
 (void)ai_ts(s,1u);if(v)ai_caf(s,OCT00314);else ai_csf(s,OCT00314);
 rand5(s,h);ai_ad(s,1u);h->channel5=a15(s);tc_q(s);
}
/* From WRITEP-1, WRITEU-1 or WRITEV-1 (offset 0, 4 or 13 from 05753). */
static void fire(apollo_overlay_state *s,agc_hardware *h,unsigned offset){
 if(offset==0u){ai_ca(s,LM_NEXTP);h->channel6=a15(s);tc_q(s);}   /* CA NEXTP; WRITEP: WRITE CHAN6; TC Q */
 else{ai_ca(s,offset==4u?LM_NEXTU:LM_NEXTV);write5(s,h,offset!=4u);}
}
/* T6JOBCHK, taken whole. The caller has checked t6_ok. */
static int t6_ok(const apollo_overlay_state *s){
 agc_word t=W[LM_TIME6];
 if(t!=077777u&&(t&040000u))return 0;              /* negative: TC T6JOBCHK +3 loops */
 if(t==077777u){agc_word x=s->words[03000u+(LM_NXT6ADR-01400u)];if(!eb6(s)||(x!=0u&&x!=4u&&x!=13u))return 0;}
 return returnable(s)||t==0u;
}
static void t6jobchk(apollo_overlay_state *s,agc_hardware *h){uint8_t c=ai_ccs(s,LM_TIME6);agc_word offset;
 if(c==0u){tc_q(s);return;}                        /* TC Q */
 if(c==1u){ai_tc(s,02032u);W[5]=LM_CCSHOLE;return;} /* TC CCSHOLE */
 /* T6JOB */
 ai_caf(s,POSMAX);wand13(s,h);ai_caf(s,POSMAX);ai_zl(s);
 ai_dxch(s,LM_T6FURTHA);ai_dxch(s,LM_T6NEXT);ai_lxch(s,LM_NXT6ADR);
 if(!ai_ts(s,LM_TIME6))ai_adf(s,PRIO37);           /* TS TIME6; AD PRIO37 */
 if(!ai_ts(s,0u)){                                 /* TS A; TCF ENABLET6 */
  ai_caf(s,BIT15);wor13(s,h);ai_ca(s,LM_T6NEXT);ai_adf(s,PRIO37);
  if(ai_ts(s,0u)){ai_caf(s,POSMAX);(void)ai_ts(s,LM_T6NEXT);}  /* else TCF GOCH56 */
 }else{ai_caf(s,POSMAX);(void)ai_ts(s,LM_TIME6);}  /* TCF GOCH56 */
 offset=agc_correct(W[1]);fire(s,h,offset);         /* GOCH56: INDEX L; TCF WRITEP -1 */
}
static agc_hardware *hw(const agc_native_program *p){return p?p->hardware:0;}
static enum exec_status t6jobchk_entry(const agc_native_program *p,apollo_overlay_state *s,void *v){agc_hardware *h=hw(p);
 if(!h||!s||!v||!t6_ok(s))return EXEC_OUT_OF_SCOPE;
 t6jobchk(s,h);return EXEC_OK;
}
/* DOT6RUPT, in the T6 interrupt with FB 17, EB 6. */
static enum exec_status dot6rupt(const agc_native_program *p,apollo_overlay_state *s,void *v){agc_hardware *h=hw(p);
 if(!h||!s||!v||h->in_interrupt!=1u||W[4]!=LM_T6_BANK||!eb6(s))return EXEC_OUT_OF_SCOPE;
 {agc_word q=W[2];W[2]=02071u;if(!t6_ok(s)){W[2]=q;return EXEC_OUT_OF_SCOPE;}W[2]=q;}
 ai_lxch(s,016u);ai_qxch(s,012u);ai_tc(s,02071u);t6jobchk(s,h);
 if(W[5]==02071u)W[5]=LM_RESUME;                   /* TCF RESUME */
 return EXEC_OK;
}
/* The T6RUPT lead-in at 04004: DXCH ARUPT; EXTEND; DCA T6ADR; DTCB. */
static enum exec_status vector(const agc_native_program *p,apollo_overlay_state *s,void *v){agc_hardware *h=hw(p);agc_word bb;
 if(!h||!s||!v||h->in_interrupt!=1u)return EXEC_OUT_OF_SCOPE;
 ai_dxch(s,010u);bb=W[6];
 W[0]=04010u;W[1]=agc_extend(bb);W[5]=LM_DOT6RUPT;
 W[6]=(agc_word)(036106u&076007u);W[3]=(agc_word)((W[6]&7u)<<8);W[4]=(agc_word)(W[6]&076000u);
 return EXEC_OK;
}
static enum exec_status writex(const agc_native_program *p,apollo_overlay_state *s,void *v){agc_hardware *h=hw(p);agc_word z;
 if(!h||!s||!v||!returnable(s))return EXEC_OUT_OF_SCOPE;
 z=W[5];
 if(z==LM_WRITEP-1u||z==LM_WRITEU-1u||z==LM_WRITEV-1u){if(!eb6(s))return EXEC_OUT_OF_SCOPE;fire(s,h,(unsigned)(z-(LM_WRITEP-1u)));return EXEC_OK;}
 if(z==LM_WRITEP){h->channel6=a15(s);tc_q(s);return EXEC_OK;}
 if(overflowed(W[0]))return EXEC_OUT_OF_SCOPE;    /* TS L would skip */
 write5(s,h,z==LM_WRITEV);return EXEC_OK;
}
/* RESUME 05270 (QXCH QRUPT), NOQRSM 05272 (CA BANKRUPT; XCH BBANK), NOQBRSM
 * 05274 (DXCH ARUPT; RELINT; RESUME), and NOQBRSM +1 05275 (RELINT; RESUME),
 * which the P-axis autopilot's TCF NOQBRSM +1 enters to resume at SUPERJOB. */
static enum exec_status resume(const agc_native_program *p,apollo_overlay_state *s,void *v){agc_hardware *h=hw(p);p66_bias_io *io=v;agc_word z,bank;
 if(!h||!io||!s||h->in_interrupt!=1u||W[015]==0u||W[015]>010000u)return EXEC_OUT_OF_SCOPE;
 z=W[5];bank=W[016];
 if(z!=LM_NOQBRSM&&z!=LM_NOQBRSM+1u&&(bank&(agc_word)~076167u))return EXEC_OUT_OF_SCOPE;
 /* BRUPT must be the word at ZRUPT-1 under the banks restored (#144). */
 {bool noq=z==LM_NOQBRSM||z==LM_NOQBRSM+1u;
  if(!agc_resume_exact(p,s,noq?W[4]:(agc_word)(bank&076000u),noq?W[3]:(agc_word)((bank&7u)<<8),io->superbank))
   return EXEC_OUT_OF_SCOPE;}
 if(z==LM_RESUME)ai_qxch(s,012u);
 if(z!=LM_NOQBRSM&&z!=LM_NOQBRSM+1u){ai_ca(s,016u);ai_xch(s,6u);}
 if(z!=LM_NOQBRSM+1u)ai_dxch(s,010u);
 io->interrupt_enabled=1;
 W[5]=(agc_word)(W[015]-1u);W[7]=0;h->in_interrupt=0;return EXEC_OK;
}
static const agc_routine_entry entries[13]={
 {FF(LM_T6RUPT_VECTOR),vector},{FF(LM_RESUME),resume},{FF(LM_NOQRSM),resume},{FF(LM_NOQBRSM),resume},{FF(LM_NOQBRSM+1u),resume},
 {FF(LM_WRITEP-1u),writex},{FF(LM_WRITEP),writex},{FF(LM_WRITEU-1u),writex},{FF(LM_WRITEU),writex},
 {FF(LM_WRITEV-1u),writex},{FF(LM_WRITEV),writex},
 {B17(LM_T6JOBCHK),t6jobchk_entry},{B17(LM_DOT6RUPT),dot6rupt}};
const agc_routine_table lm_t6rupt_routines={entries,13};
