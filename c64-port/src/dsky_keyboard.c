/* SPDX-License-Identifier: MIT */
#include "dsky_keyboard.h"
#include <stddef.h>
typedef char basic_binding_must_be_first[(offsetof(dsky_keyboard_binding,basic)==0)?1:-1];
uint8_t dsky_keyboard_decode(uint8_t character,uint16_t *channel15){uint16_t code;
 if(channel15==0)return 0;
 if(character>='1' && character<='9')code=(uint16_t)(character-'0');
 else switch(character){
 case '0':code=020;break;
 case 'V':case 'v':case 0xd6:code=021;break;
 case 'R':case 'r':case 0xd2:code=022;break;
 case 'K':case 'k':case 0xcb:code=031;break;
 case '+':code=032;break;
 case '-':code=033;break;
 case 13:code=034;break;
 case 'C':case 'c':case 0xc3:case 20:code=036;break;
 case 'N':case 'n':case 0xce:code=037;break;
 default:return 0;
 }
 *channel15=code;return 1;
}
static enum exec_action charin(executive *s,void *user,exec_address *resume){dsky_keyboard_binding *k=user;p66_basic_binding *b=&k->basic;dsky_charin_io io;uint8_t token;uint16_t bank,*w;
 if(b->arithmetic.image==0 || b->arithmetic.runtime==0 || b->arithmetic.producers==0 || b->arithmetic.producers->enter==0 || b->arithmetic.producers->leave==0 || b->display==0){if(b->arithmetic.runtime!=0)b->arithmetic.runtime->fault=EXEC_OUT_OF_SCOPE;return EXEC_YIELD;}
 token=b->arithmetic.producers->enter(b->arithmetic.producers->user);
 p66_native_export_executive(b->arithmetic.image,s);w=b->arithmetic.image->words;bank=(uint16_t)(b->arithmetic.runtime->current.bank&076007u);
 w[3]=(uint16_t)((bank&7u)<<8);w[4]=(uint16_t)(bank&076000u);w[6]=bank;w[5]=02077;b->control.superbank=(uint16_t)(b->arithmetic.runtime->current.bank&0160u);
 io.control=b->control;io.channel11=b->display->channel11;io.wrote_channel11=0;
 b->status=(uint16_t)dsky_charin_nv(b->arithmetic.image,&io);b->control=io.control;
 if(b->status==P66_BIAS_OK){if(io.wrote_channel11!=0)dsky_display_output(b->display,011,io.channel11);if(b->arithmetic.observe!=0)b->arithmetic.observe(b->arithmetic.observer,b->arithmetic.image,w[5]);}
 else b->arithmetic.runtime->fault=EXEC_OUT_OF_SCOPE;
 p66_native_import_executive(s,b->arithmetic.image);resume->location=w[5];resume->bank=(uint16_t)(w[6]|b->control.superbank);
 b->arithmetic.producers->leave(b->arithmetic.producers->user,token);return b->status==P66_BIAS_OK ? EXEC_FINISH : EXEC_YIELD;
}
const exec_context_mapping dsky_keyboard_mappings[DSKY_KEYBOARD_MAPPING_COUNT]={{{02077,060101},EXEC_CONTEXT_BASIC,charin}};
enum exec_status dsky_keyboard_start(dsky_keyboard_binding *k){p66_basic_binding *b=&k->basic;exec_native *r=b->arithmetic.runtime;uint16_t *w,old_l;enum exec_status status;uint8_t token;
 if(r==0 || b->arithmetic.image==0 || b->arithmetic.producers==0 || b->arithmetic.producers->enter==0 || b->arithmetic.producers->leave==0 || b->display==0 || r->running!=0)return EXEC_OUT_OF_SCOPE;
 w=b->arithmetic.image->words;
 if(w[067]!=0 || w[0164]!=02077 || w[0165]!=060101 || w[0]!=2 || b->control.interrupt_enabled!=1)return EXEC_OUT_OF_SCOPE;
 old_l=w[1];token=b->arithmetic.producers->enter(b->arithmetic.producers->user);p66_native_import_executive(r->state,b->arithmetic.image);status=exec_native_start(r);
 /* WOR returns the actual activity channel word, which DXCH LOC stores.
  * TC L executes the literal one-word TC in L, leaving source Q=2. */
 if(status==EXEC_OK){uint16_t bank=(uint16_t)(r->current.bank&076007u);r->state->core[0][EXEC_LOC]=(uint16_t)(b->display->channel11|2u);r->state->core[0][EXEC_BANK]=agc_correct(old_l);p66_native_export_executive(b->arithmetic.image,r->state);w[0]=agc_extend(r->current.bank);w[1]=r->current.location;w[2]=2;w[3]=(uint16_t)((bank&7u)<<8);w[4]=(uint16_t)(bank&076000u);w[6]=bank;w[5]=r->current.location;b->control.superbank=(uint16_t)(r->current.bank&0160u);dsky_display_output(b->display,011,(uint16_t)(b->display->channel11|2u));}
 b->arithmetic.producers->leave(b->arithmetic.producers->user,token);return status;
}
enum exec_status dsky_keyboard_interrupt(dsky_keyboard_binding *k,uint16_t channel15,uint16_t source_instruction){p66_basic_binding *b=&k->basic;dsky_key_irq_io io;enum exec_status status;uint8_t token;uint16_t *w;
 if(channel15>077777 || source_instruction>077777 || b->arithmetic.image==0 || b->arithmetic.runtime==0 || b->arithmetic.producers==0 || b->arithmetic.producers->enter==0 || b->arithmetic.producers->leave==0 || b->control.interrupt_enabled!=1 || b->control.superbank>0160 || (b->control.superbank&017u)!=0 || b->arithmetic.runtime->state==0)return EXEC_OUT_OF_SCOPE;
 w=b->arithmetic.image->words;if(w[5]>07777 || w[3]>03400 || (w[3]&0377u)!=0 || w[4]>076000 || (w[4]&01777u)!=0 || w[6]!=(w[4]|(w[3]>>8)) || (w[067]!=077777 && (w[067]>84 || w[067]%12u!=0)))return EXEC_OUT_OF_SCOPE;
 token=b->arithmetic.producers->enter(b->arithmetic.producers->user);w[015]=(uint16_t)(w[5]+1u);w[017]=source_instruction;w[5]=04024;io.control=b->control;io.in_interrupt=1;status=dsky_key_irq(b->arithmetic.image,&io,channel15);b->control=io.control;p66_native_import_executive(b->arithmetic.runtime->state,b->arithmetic.image);
 if(status!=EXEC_OK){b->arithmetic.runtime->state->alarm=(uint16_t)status;b->arithmetic.runtime->fault=status;}
 b->arithmetic.producers->leave(b->arithmetic.producers->user,token);return status;
}
enum exec_status dsky_keyboard_idle(dsky_keyboard_binding *k){p66_basic_binding *b=&k->basic;uint16_t *w=b->arithmetic.image!=0 ? b->arithmetic.image->words : 0;
 if(w==0 || w[5]!=03214 || w[4]!=02000 || w[067]!=0 || w[0164]!=02077 || w[0165]!=060101 || b->control.interrupt_enabled!=1 || b->arithmetic.runtime==0 || b->arithmetic.runtime->running!=0)return EXEC_OUT_OF_SCOPE;
 /* Literal original CCS NEWJOB +0 → CAF TWO → TCF NUDIRECT. */
 w[0]=2;w[5]=03225;return dsky_keyboard_start(k);
}
