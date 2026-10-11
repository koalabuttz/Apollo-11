/* SPDX-License-Identifier: MIT; source Luminary099 WAIT2 and delay callers. */
#include "lm_delay_image.h"
static agc_register add(agc_register a,agc_word b){return agc_add_register(a,agc_extend(b));}
static agc_register inv(agc_register a){return (agc_register)(a^0177777u);}
static int kind(agc_register a){int o=agc_overflow(a);return o>0?(int)AGC_POSITIVE:o<0?(int)AGC_NEGATIVE:(int)agc_classify(agc_correct(a));}
static agc_register dabs(agc_register a){a=(a&0100000u)?inv(a):a;return a?(agc_register)(a-1u):0;}
enum wl_status lm_waitlist_internal(apollo_overlay_state *image,p66_bias_io *io){
 uint16_t *w;waitlist s;agc_word caller,bank,delay,oldtemp;agc_register a;
 exec_address discarded,task;enum wl_status status;uint8_t slot=0,i,inserted=1;int k;
 if(!image||!io)return WL_OUT_OF_SCOPE;
 w=image->words;task=(exec_address){w[063],agc_correct(w[1])};
 if(w[5]!=03231u||w[7]||w[3]!=01400u||w[4]!=02000u||w[6]!=02003u||
  (io->superbank&~0160u)||io->interrupt_enabled||agc_overflow(w[0])||
  agc_overflow(w[2])||task.location<0100u||task.location>07777u||
  (task.bank&~076167u)||(agc_correct(w[0])&~076007u)||w[026]>037777u)return WL_OUT_OF_SCOPE;
 delay=agc_correct(w[2]);if(agc_classify(delay)==AGC_POSITIVE&&delay>WL_MAX_DELAY)return WL_OUT_OF_SCOPE;
 caller=w[061];bank=agc_correct(w[0]);oldtemp=w[064];
 s.time3=w[026];s.ruptagn=w[0734];s.alarm=0;for(i=0;i<8u;++i)s.interval[i]=w[01400u+i];for(i=0;i<9u;++i){s.task[i].location=w[01410u+2u*i];s.task[i].bank=w[01411u+2u*i];}
 discarded=s.task[8];a=agc_extend(delay);
 if(kind(a)==AGC_POSITIVE){
  a=add(agc_extend(agc_negate(s.time3)),0200);k=kind(a);a=dabs(a);
  if(k==AGC_POSITIVE)a=inv(add(a,040001));else if(k==AGC_PLUS_ZERO)a=inv(a);
  if(k!=AGC_MINUS_ZERO)a=add(a,040201);
  a=add(a,delay);k=kind(a);a=dabs(a);
  if(k==AGC_POSITIVE){a=add(a,s.interval[0]);for(slot=1;slot<9u;++slot){k=kind(a);a=dabs(a);if(k==AGC_NEGATIVE||k==AGC_MINUS_ZERO||slot==8u)break;if(k==AGC_POSITIVE)a=add(a,s.interval[slot]);}
   if(slot==8u&&k==AGC_POSITIVE)inserted=0;
   if(inserted){if(k==AGC_NEGATIVE||k==AGC_PLUS_ZERO)a=add(a,1);oldtemp=agc_correct(a);}
  }
  if(inserted)a=add(agc_extend(discarded.location),agc_negate(WL_END_LOCATION));
  status=wl_schedule(&s,delay,task);
 }else status=WL_BAD_DELAY;
 if(status!=WL_OK&&status!=WL_FULL&&status!=WL_BAD_DELAY)return status;
 io->interrupt_enabled=0;w[063]=task.location;w[064]=oldtemp;
 w[026]=s.time3;for(i=0;i<8u;++i)w[01400u+i]=s.interval[i];for(i=0;i<9u;++i){w[01410u+2u*i]=s.task[i].location;w[01411u+2u*i]=s.task[i].bank;}
 w[061]=agc_correct(a);w[062]=(inserted&&status!=WL_BAD_DELAY)?discarded.bank:task.bank;
 if(status==WL_OK){w[0]=05220;w[1]=02003;w[2]=slot;w[5]=(agc_word)((caller+2u)&07777u);w[6]=bank;w[3]=(agc_word)((bank&7u)<<8);w[4]=(agc_word)(bank&076000u);}
 else{w[0]=agc_extend(caller);w[1]=agc_extend(bank);w[4]=02000;w[3]=01400;w[6]=02003;w[2]=status==WL_FULL?03373:03525;w[5]=status==WL_FULL?05716:05726;} return status;
}
static uint16_t erasable(agc_word q,agc_word eb){return q<01400u?q:(uint16_t)((eb>>8)*0400u+(q&0377u));}
enum wl_status lm_delay_image(apollo_overlay_state *image,p66_bias_io *io,
 enum lm_delay_entry entry,agc_word fixed_delay){
 uint16_t *w;agc_word delay,task,bank;
 if(!image||!io||(unsigned)entry>1u)return WL_OUT_OF_SCOPE;
 w=image->words;
 if(w[5]!=(entry==LM_FIXDELAY_ENTRY?05221u:05224u)||w[7]||
  w[3]>03400u||(w[3]&0377u)||(w[4]&~076000u)||
  w[6]!=(agc_word)(w[4]|(w[3]>>8))||(io->superbank&~0160u)||
  io->interrupt_enabled||w[2]<0100u||w[2]>07775u||w[026]>037777u||
  (entry==LM_VARDELAY_ENTRY&&agc_overflow(w[0])))return WL_OUT_OF_SCOPE;
 delay=entry==LM_VARDELAY_ENTRY?agc_correct(w[0]):fixed_delay;
 if(entry==LM_FIXDELAY_ENTRY&&w[2]<02000u)delay=w[erasable(w[2],w[3])];
 if(delay>AGC_MASK||(agc_classify(delay)==AGC_POSITIVE&&delay>WL_MAX_DELAY))return WL_OUT_OF_SCOPE;
 task=(agc_word)(w[2]+(entry==LM_FIXDELAY_ENTRY?1u:0u));bank=w[6];
 w[2]=agc_extend(delay);w[063]=task;w[1]=agc_extend((agc_word)(bank|io->superbank));
 w[061]=015257;w[0]=agc_extend(bank);w[3]=01400;w[4]=02000;w[6]=02003;w[5]=03231;
 return lm_waitlist_internal(image,io);
}
