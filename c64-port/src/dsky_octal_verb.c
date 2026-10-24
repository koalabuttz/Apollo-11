/* SPDX-License-Identifier: MIT */
#include "dsky_octal_verb.h"
static uint8_t controls(uint16_t *w,p66_bias_io *io,enum dsky_nv_mission mission){return(uint8_t)((unsigned)mission<2u&&w[4]==062000u&&w[3]<=03400u&&(w[3]&0377u)==0&&w[6]==(062000u|(w[3]>>8))&&w[7]==0&&io->superbank<=0160u&&(io->superbank&017u)==0&&(io->superbank&0100u)!=0&&io->interrupt_enabled<=1u);}
static agc_word component(uint16_t *w){return(agc_word)((w[w[0140]==1u?0147u:0146u]&076000u)>>10);}
static agc_word resolve(uint16_t *w,agc_word n){return n<01400u?n:(agc_word)(w[3]+(n&0377u));}
/* A noun at the top of erasable (the LM's TCDH at 01776) reads its last
 * words from fixed memory: FB 31 with superbank 4 is bank 41, whose first
 * words are TC NVSUB1 and TC LOADLV1 in both missions. */
static agc_word read_source(uint16_t *w,agc_word n){if(n>=02000u)return n==02000u?03544u:02775u;return w[resolve(w,n)];}
/* Every word the verb reads: from 010 up (0-07 are the central registers
 * Pinball itself is using), never the editing registers 020-023 (a read
 * rewrites them) or BANKRUPT and BRUPT (016-017, which every interrupt
 * rewrites), and past 01777 only the two fixed words above, with the
 * superbank Pinball runs under. */
static uint8_t readable(uint16_t *w,p66_bias_io *io,agc_word first,agc_word count)
{agc_word k,at;for(k=0;k<count;++k){agc_word n=(agc_word)(first+k);if(n>=02000u){if(n>02001u||io->superbank!=0100u)return 0;continue;}at=resolve(w,n);if(at<010u||(at>=016u&&at<=023u))return 0;}return 1;}
static void row(uint16_t *w,agc_word k){static const agc_word count[3]={016,011,004};w[0122]=k;w[0777]=count[k];w[0]=agc_extend(agc_negate(w[0130u+k]));w[2]=02422;w[5]=03363;}
static uint8_t comp_test(uint16_t *w,agc_word index,agc_word caller){agc_word code=component(w),count=(agc_word)(code&3u);w[0123]=agc_negate(index);w[1]=caller;w[022]=agc_negate(code);w[0]=agc_diminished_abs(agc_correct(agc_add_register(count,agc_extend(w[0123]))));if(count<index){w[2]=02436;w[5]=02351;return 0;}return 1;}
enum p66_bias_status dsky_octal_verb_begin(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,enum dsky_octal_verb_route *route)
{
 static const agc_word pc[5]={02365,02373,02400,02360,02353};uint16_t *w;agc_word verb,code,routine,address;uint8_t channel;
 if(!s||!io||!route)return P66_BIAS_OUT_OF_SCOPE;
 w=s->words;verb=w[01001];channel=(uint8_t)(verb==1u&&w[0146]==077776u);
 if(!controls(w,io,mission)||verb<1u||verb>5u||w[5]!=pc[verb-1u]||(w[0140]!=1u&&w[0140]!=2u))return P66_BIAS_OUT_OF_SCOPE;
 routine=(agc_word)((w[0147]>>5)&037u);
 if(!channel){uint8_t step=(uint8_t)(verb==1u&&w[0140]==1u&&(routine==4u||routine==5u||routine==7u||routine==10u||(mission==DSKY_NV_CM&&routine==13u)));
  agc_word first=(agc_word)(w[0145]+(verb==2u||verb==3u?verb-1u:step)),count=verb==5u?3u:verb==4u?2u:1u;
  if(w[0145]>01777u||!readable(w,io,first,count))return P66_BIAS_OUT_OF_SCOPE;}
 if((verb==1u||verb>=4u)&&w[0140]==1u&&!channel&&routine>13u)return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));
 if(verb==5u){if(!comp_test(w,2,02355)){*route=DSKY_VERB_ALARM;return P66_BIAS_OK;}w[0132]=agc_negate(read_source(w,(agc_word)(w[0145]+2u)));}
 if(verb>=4u){if(!comp_test(w,1,02362)){*route=DSKY_VERB_ALARM;return P66_BIAS_OK;}w[0131]=agc_negate(read_source(w,(agc_word)(w[0145]+1u)));}
 if(verb==2u||verb==3u){w[0123]=agc_negate((agc_word)(verb-1u));w[1]=verb==2u?02375u:02402u;w[0156]=02442;}
 else w[0156]=02366;
 code=component(w);
 if(code&010u){w[0]=017777;w[2]=02451;w[5]=02351;*route=DSKY_VERB_ALARM;return P66_BIAS_OK;}
 if(verb==2u||verb==3u){if(!comp_test(w,(agc_word)(verb-1u),verb==2u?02375u:02402u)){*route=DSKY_VERB_ALARM;return P66_BIAS_OK;}address=(agc_word)(w[0145]+verb-1u);}
 else{
  agc_word old_l=w[1];w[1]=02367;
  if(channel){w[0]=0177777;w[2]=old_l;w[5]=02504;*route=DSKY_VERB_CHANNEL;return P66_BIAS_OK;}
  if(w[0140]==1u){w[0114]=02500;w[020]=agc_negate(routine);if(routine==4u||routine==5u||routine==7u||routine==10u||(mission==DSKY_NV_CM&&routine==13u))w[0145]=(agc_word)(w[0145]+1u);}
  address=w[0145];
 }
 w[0130]=agc_negate(read_source(w,address));row(w,verb>=4u?(agc_word)(verb-3u):0u);*route=DSKY_VERB_FORMAT;return P66_BIAS_OK;
}
enum p66_bias_status dsky_octal_verb_return(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,enum dsky_octal_verb_route *route)
{
 uint16_t *w;agc_word k;
 if(!s||!io||!route)return P66_BIAS_OUT_OF_SCOPE;
 w=s->words;if(!controls(w,io,mission)||w[5]!=02422u||w[0122]>2u)return P66_BIAS_OUT_OF_SCOPE;
 k=w[0122];w[0122]=agc_correct(w[0]);w[0]=k;w[1]=agc_extend(agc_correct(w[1]));
 if(k){row(w,(agc_word)(k-1u));*route=DSKY_VERB_FORMAT;}
 else{w[0]=0;w[2]=02412;w[5]=0136;*route=DSKY_VERB_ENTRET;}
 return P66_BIAS_OK;
}

enum p66_bias_status dsky_octal_verb_channel(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,agc_word value,enum dsky_octal_verb_route *route)
{
 uint16_t *w;agc_word channel,verb;
 if(!s||!io||!route)return P66_BIAS_OUT_OF_SCOPE;
 w=s->words;verb=w[01001];
 if(!controls(w,io,mission)||w[5]!=02504u||value>AGC_MASK||verb<1u||verb>5u)return P66_BIAS_OUT_OF_SCOPE;
 channel=(agc_word)(w[01017]&0777u);
 if(channel==1u||channel==2u)value=agc_correct(w[channel]);else if(channel==7u)value=io->superbank;
 w[1]=agc_extend(agc_correct(w[1]));w[0130]=agc_negate(value);
 row(w,verb>=4u?(agc_word)(verb-3u):0u);*route=DSKY_VERB_FORMAT;return P66_BIAS_OK;
}
