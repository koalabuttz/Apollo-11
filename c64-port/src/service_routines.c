/* SPDX-License-Identifier: MIT; compiled public-domain original controls. */
#include "service_routines.h"
#include "service_routines_source.h"
bool service_routines_literal(enum service_routines_mission mission,uint16_t address,agc_word *out){const service_routines_bindings *b;uint16_t alarm;
 if((unsigned)mission>1u||!out)return false;
 b=&service_routines_source[mission];alarm=(uint16_t)(b->delayjob+(mission==SERVICE_ROUTINES_LM?014u:013u)-02000u);
 if(address==alarm)*out=01104;else if(address==b->tcsleep-02000u)*out=b->tcsleep_word;else if(address==b->wakecad-02000u)*out=b->wakecad_word;else return false;return true;
}
static uint16_t physical(const apollo_overlay_state *s,uint16_t a){return a<01400u?a:(uint16_t)(s->words[3]+(a&0377u));}
static agc_word inline_read(const apollo_overlay_state *s,uint16_t a,agc_word fixed){return a<02000u?s->words[physical(s,a)]:fixed;}
static agc_register load(const apollo_overlay_state *s,uint16_t a){uint16_t p=physical(s,a);return p<3u?s->words[p]:agc_extend(s->words[p]);}
static bool ordinary_address(const apollo_overlay_state *s,uint16_t a){return a<02000u&&physical(s,a)>=030u;}
static void b5off(apollo_overlay_state *s,const service_routines_bindings *b,bool mask_only){if(!mask_only)s->words[0]=agc_extend(077757);s->words[0]=agc_extend((agc_word)(agc_correct(s->words[0])&s->words[01044]));s->words[01044]=agc_correct(s->words[0]);s->words[2]=(agc_word)(b->b5off+4u);s->words[5]=b->endofjob;}
bool service_routines_step(apollo_overlay_state *s,p66_bias_io *io,enum service_routines_mission mission,enum service_routines_entry entry,agc_word fixed0,agc_word fixed1,agc_word fixedword){
 const service_routines_bindings *b;agc_word *w,q,pc,n,descriptor,index,bits,old,source,dest,operand;agc_register a,l;enum agc_ccs_class cl;unsigned i;int ov;
 if(!s||!io||(unsigned)mission>1u||(unsigned)entry>12u||fixed0>AGC_MASK||fixed1>AGC_MASK||fixedword>AGC_MASK)return false;
 b=&service_routines_source[mission];w=s->words;q=w[2];
 if(w[7]!=0||q<0100u||q>07775u||w[3]>03400u||(w[3]&0377u)||(w[4]&~076000u)||w[6]!=(agc_word)(w[4]|(w[3]>>8))||(io->superbank&~0160u)||io->interrupt_enabled>1u)return false;
 switch(entry){case SERVICE_UPENT2:pc=b->upent2;break;case SERVICE_DOWNENT2:pc=b->downent2;break;case SERVICE_UPFLAG:pc=b->upflag;break;case SERVICE_DOWNFLAG:pc=b->downflag;break;case SERVICE_DELAYJOB:pc=b->delayjob;break;case SERVICE_TCGETCAD:pc=b->tcgetcad;break;case SERVICE_WAKER:pc=b->waker;break;case SERVICE_WAKER_FINISH:pc=(agc_word)(b->waker+4u);break;case SERVICE_GENTRAN:pc=b->gentran;break;case SERVICE_GENTRAN_LOOP:pc=(agc_word)(b->gentran+1u);break;case SERVICE_B5OFF:pc=b->b5off;break;case SERVICE_TRFAILOF:pc=b->trfailof;break;default:pc=b->trfailon;break;}
 if(!pc||w[5]!=pc)return false;
 if((entry==SERVICE_UPENT2||entry==SERVICE_DOWNENT2)&&mission==SERVICE_ROUTINES_LM&&w[4]!=020000u)return false;
 if((entry>=SERVICE_DELAYJOB&&entry<=SERVICE_WAKER_FINISH)&&w[4]!=0)return false;
 if((entry==SERVICE_UPFLAG||entry==SERVICE_DOWNFLAG)&&q<02000u&&!ordinary_address(s,q))return false;
 descriptor=0;source=0;dest=0;
 if(entry==SERVICE_UPFLAG||entry==SERVICE_DOWNFLAG){descriptor=inline_read(s,q,fixed0);if(q<02000u&&physical(s,q)==061u)descriptor=15;if(q<02000u&&physical(s,q)==063u)descriptor=(agc_word)(q+1u);if(descriptor>14459u||!ordinary_address(s,(uint16_t)(074u+descriptor/15u)))return false;}
 if(entry==SERVICE_TCGETCAD&&w[070]>b->delay_count)return false;
 if(entry==SERVICE_WAKER&&w[6]>7u)return false;
 if(entry==SERVICE_GENTRAN||entry==SERVICE_GENTRAN_LOOP){
  n=agc_correct(w[0]);if(agc_overflow(w[0])||n>01777u)return false;
  if(q<02000u&&(!ordinary_address(s,q)||!ordinary_address(s,(uint16_t)(q+1u))))return false;
  /* Source TS precedes both inline reads. Evaluate its one scratch alias
   * before admission; later reads still happen against the actual image. */
  source=inline_read(s,q,fixed0);if(q<02000u&&physical(s,q)==061u)source=n;
  dest=inline_read(s,(uint16_t)(q+1u),fixed1);if(q<02000u&&physical(s,(uint16_t)(q+1u))==061u)dest=n;
  a=agc_add_register(agc_extend(n),agc_extend(source));source=agc_correct(a);if(agc_overflow(a)||source>07777u||(source<02000u&&!ordinary_address(s,source)))return false;
  a=agc_add_register(agc_extend(n),agc_extend(dest));dest=agc_correct(a);if(agc_overflow(a)||!ordinary_address(s,dest))return false;
 }
 w[1]=agc_extend(agc_correct(w[1]));
 if(entry==SERVICE_UPENT2||entry==SERVICE_DOWNENT2){
  a=w[0];index=4;bits=(agc_word)(agc_correct(a)&077770u);
  if(mission==SERVICE_ROUTINES_CM){ov=agc_overflow(a);l=agc_extend(agc_correct(a));index=ov?(agc_word)(ov>0?1u:077776u):(agc_word)(agc_correct(a)&7u);w[1]=agc_extend(index);a=l;bits=(agc_word)(agc_correct(a)&077770u);if(entry==SERVICE_UPENT2)w[061]=bits;}
  else if(entry==SERVICE_UPENT2)w[1]=agc_extend(bits);
  index=agc_add_word(074,index);old=w[index];
  if(entry==SERVICE_UPENT2)a=agc_add_register(agc_extend((agc_word)((AGC_MASK^old)&bits)),agc_extend(old));else a=agc_extend((agc_word)(old&(AGC_MASK^bits)));
  w[index]=agc_correct(a);w[0]=a;io->interrupt_enabled=1;
  w[5]=(agc_word)(q+1u);w[2]=mission==SERVICE_ROUTINES_CM?3:(agc_word)(b->qplus1+2u);return true;
 }
 if(entry==SERVICE_UPFLAG||entry==SERVICE_DOWNFLAG){
  index=(agc_word)(descriptor/15u);bits=(agc_word)(040000u>>(descriptor%15u));w[061]=index;w[062]=(agc_word)(descriptor%15u);old=w[physical(s,(uint16_t)(074u+index))];
  w[0]=agc_extend(entry==SERVICE_UPFLAG?(agc_word)(old|bits):(agc_word)(old&(AGC_MASK^bits)));w[physical(s,(uint16_t)(074u+index))]=agc_correct(w[0]);w[063]=old;w[1]=(agc_word)(q+1u);w[2]=2;w[5]=(agc_word)(q+1u);io->interrupt_enabled=1;return true;
 }
 if(entry==SERVICE_DELAYJOB){
  io->interrupt_enabled=0;ov=agc_overflow(w[0]);w[2]=w[0];index=ov?(agc_word)(ov>0?1u:077776u):b->delay_count;
  if(ov<0){
   w[070]=077776;operand=w[b->delayloc-1u];w[0]=agc_extend(operand);
   if(operand!=0&&operand!=AGC_NEG_ZERO){w[0]=0;w[2]=(agc_word)(b->delayjob+014u);w[5]=mission==SERVICE_ROUTINES_LM?b->bailout1:01104;return true;}
   w[061]=b->tcsleep_word;w[0]=agc_extend(b->wakecad_word);w[1]=agc_extend(agc_correct(agc_add_register(agc_extend(w[4]),agc_extend(w[070]))));w[5]=(agc_word)(b->dly2-1u);return true;
  }
  for(i=index+1u;i>0u;--i){index=(agc_word)(i-1u);w[070]=index;operand=w[b->delayloc+index];w[0]=agc_extend(operand);if(operand==0||operand==AGC_NEG_ZERO)break;w[0]=agc_diminished_abs(index);}
  if(i==0u){
   if(mission==SERVICE_ROUTINES_LM){a=w[0];l=w[1];w[0]=agc_extend(w[0133]);w[1]=agc_extend(w[0134]);w[0133]=agc_correct(a);w[0134]=agc_correct(l);w[2]=(agc_word)(b->delayjob+014u);w[5]=b->bailout1;}
   else{w[2]=(agc_word)(b->delayjob+013u);w[5]=b->bailout;}return true;
  }
  w[061]=b->tcsleep_word;w[0]=agc_extend(b->wakecad_word);w[1]=agc_extend(agc_correct(agc_add_register(agc_extend(w[4]),agc_extend(w[070]))));w[5]=(agc_word)(b->dly2-1u);return true;
 }
 if(entry==SERVICE_TCGETCAD){a=agc_add_register(agc_extend((agc_word)(w[0133]&01777u)),agc_extend(w[0134]));w[b->delayloc+w[070]]=agc_correct(a);ov=agc_overflow(a);if(ov){w[0]=agc_extend(w[b->delayloc+w[6]]);w[b->delayloc+w[6]]=0;w[2]=(agc_word)(b->waker+4u);w[5]=b->jobwake;}else{w[0]=a;w[2]=(agc_word)(b->tcgetcad+4u);w[5]=b->jobsleep;}return true;}
 if(entry==SERVICE_WAKER){w[0]=agc_extend(w[b->delayloc+w[6]]);w[b->delayloc+w[6]]=0;w[2]=(agc_word)(b->waker+4u);w[5]=b->jobwake;return true;}
 if(entry==SERVICE_WAKER_FINISH){w[2]=(agc_word)(b->waker+5u);w[5]=b->taskover;return true;}
 if(entry==SERVICE_GENTRAN||entry==SERVICE_GENTRAN_LOOP){if(entry==SERVICE_GENTRAN)io->interrupt_enabled=0;w[061]=agc_correct(w[0]);w[0]=source<02000u?load(s,source):agc_extend(fixedword);w[1]=agc_extend(agc_correct(w[0]));
  w[0]=agc_add_register(agc_extend(w[061]),agc_extend(inline_read(s,(uint16_t)(q+1u),fixed1)));dest=agc_correct(w[0]);l=w[1];w[1]=agc_extend(w[physical(s,dest)]);w[physical(s,dest)]=agc_correct(l);
  operand=w[061];cl=agc_classify(operand);w[0]=agc_diminished_abs(operand);
  if(cl==AGC_POSITIVE){w[5]=(agc_word)(b->gentran+1u);return true;}
  if(cl==AGC_PLUS_ZERO){w[5]=(agc_word)(q+2u);w[2]=(agc_word)(b->qplus2+2u);return true;}
  b5off(s,b,cl==AGC_MINUS_ZERO);return true;
 }
 if(entry==SERVICE_B5OFF){b5off(s,b,false);return true;}
 if(entry==SERVICE_TRFAILOF){w[01036]=(agc_word)((w[01036]&037577u)+040000u);a=agc_add_register(agc_extend((agc_word)((AGC_MASK^w[01331])&0100u)),agc_extend(w[01331]));w[01331]=agc_correct(a);w[0]=a;}
 else{a=agc_add_register(agc_extend((agc_word)((AGC_MASK^w[01036])&040200u)),agc_extend(w[01036]));w[01036]=agc_correct(a);w[0]=a;}
 io->interrupt_enabled=1;w[2]=3;w[5]=q;return true;
}

enum service_routines_block_status service_routines_block(apollo_overlay_state *s,p66_bias_io *io,enum service_routines_mission mission,agc_word fixed0,agc_word fixed1,service_routines_fixed_reader reader,void *ctx,uint16_t limit,uint16_t *iterations){
 const service_routines_bindings *b;uint16_t count=0,logical,bank,physical_word;agc_word value;enum service_routines_entry entry;
 if(!s||!io||(unsigned)mission>1u||!limit||!iterations)return SERVICE_BLOCK_BAD;
 b=&service_routines_source[mission];
 if(s->words[7]!=0||s->words[2]<0100u||s->words[2]>07775u||s->words[3]>03400u||(s->words[3]&0377u)||(s->words[4]&~076000u)||s->words[6]!=(agc_word)(s->words[4]|(s->words[3]>>8))||(io->superbank&~0160u)||io->interrupt_enabled>1u)return SERVICE_BLOCK_BAD;
 while(count<limit){
  if(s->words[5]==b->gentran)entry=SERVICE_GENTRAN;else if(s->words[5]==b->gentran+1u)entry=SERVICE_GENTRAN_LOOP;else return SERVICE_BLOCK_BAD;
  value=inline_read(s,s->words[2],fixed0);if(s->words[2]<02000u&&physical(s,s->words[2])==061u)value=agc_correct(s->words[0]);logical=agc_correct(agc_add_register(s->words[0],agc_extend(value)));value=0;
  if(logical>=02000u){
   if(logical>=04000u)physical_word=logical;else{bank=(uint16_t)(s->words[4]>>10);if(bank>=030u&&(io->superbank&0100u))bank=(uint16_t)(bank+8u);physical_word=(uint16_t)(bank*1024u+(logical&01777u));}
   if(!reader||!reader(ctx,physical_word,&value))return SERVICE_BLOCK_BAD;
  }
  if(!service_routines_step(s,io,mission,entry,fixed0,fixed1,value))return SERVICE_BLOCK_BAD;
  ++count;*iterations=count;
  if(s->words[5]!=b->gentran+1u)return SERVICE_BLOCK_DONE;
 }
 return SERVICE_BLOCK_LOOP;
}
