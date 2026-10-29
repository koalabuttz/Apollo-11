/* SPDX-License-Identifier: MIT */
#include "dsky_keyboard_number.h"
static enum exec_action number_charin(executive *s,void *user,exec_address *resume){dsky_keyboard_binding *k=user;p66_basic_binding *b=&k->basic;dsky_charin_io io;uint8_t token;uint16_t bank,*w;
 if(b->arithmetic.image==0 || b->arithmetic.runtime==0 || b->arithmetic.producers==0 || b->arithmetic.producers->enter==0 || b->arithmetic.producers->leave==0 || b->display==0){if(b->arithmetic.runtime!=0)b->arithmetic.runtime->fault=EXEC_OUT_OF_SCOPE;return EXEC_YIELD;}
 token=b->arithmetic.producers->enter(b->arithmetic.producers->user);
 p66_native_export_executive(b->arithmetic.image,s);w=b->arithmetic.image->words;bank=(uint16_t)(b->arithmetic.runtime->current.bank&076007u);
 w[3]=(uint16_t)((bank&7u)<<8);w[4]=(uint16_t)(bank&076000u);w[6]=bank;w[5]=02077;b->control.superbank=(uint16_t)(b->arithmetic.runtime->current.bank&0160u);
 io.control=b->control;io.channel11=b->display->channel11;io.wrote_channel11=0;
 b->status=(uint16_t)dsky_charin_number(b->arithmetic.image,&io);b->control=io.control;
 if(b->status==P66_BIAS_OK){if(io.wrote_channel11!=0)dsky_display_output(b->display,011,io.channel11);if(b->arithmetic.observe!=0)b->arithmetic.observe(b->arithmetic.observer,b->arithmetic.image,w[5]);}
 else b->arithmetic.runtime->fault=EXEC_OUT_OF_SCOPE;
 p66_native_import_executive(s,b->arithmetic.image);resume->location=w[5];resume->bank=(uint16_t)(w[6]|b->control.superbank);
 b->arithmetic.producers->leave(b->arithmetic.producers->user,token);return b->status==P66_BIAS_OK ? EXEC_FINISH : EXEC_YIELD;
}
const exec_context_mapping dsky_keyboard_number_mappings[DSKY_KEYBOARD_MAPPING_COUNT]={{{02077,060101},EXEC_CONTEXT_BASIC,number_charin}};
