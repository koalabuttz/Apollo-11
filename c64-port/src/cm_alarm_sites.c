/* SPDX-License-Identifier: MIT */
#include "cm_alarm_sites.h"
#include "cm_alarm_sites.inc"
const agc_site_operand *cm_alarm_site_table(uint16_t *count){if(count)*count=CM_ALARM_SITES_COUNT;return cm_alarm_sites;}
bool cm_alarm_site_code(const apollo_overlay_state *s,const cm_native_io *io,agc_word q,agc_word *code){
 if(!s||!io||q<02000u||q>07777u)return false;
 return agc_site_lookup(cm_alarm_sites,CM_ALARM_SITES_COUNT,agc_fixed_physical(q,s->words[4],io->superbank),code);
}
/* q<02000 leaves the erasable read to the callee; a fixed q must be a site. */
static enum exec_status operand(const apollo_overlay_state *s,const cm_native_io *io,agc_word q,agc_word *code){
 *code=0;if(q<02000u)return EXEC_OK;
 return cm_alarm_site_code(s,io,q,code)?EXEC_OK:EXEC_OUT_OF_SCOPE;
}
enum exec_status cm_alarm_site_call(apollo_overlay_state *s,cm_native_io *io,enum cm_alarm_record_entry entry){agc_word code=0;
 if(!s||!io||(unsigned)entry>CM_RECORD_BORTENT)return EXEC_OUT_OF_SCOPE;
 if(entry==CM_RECORD_ALARM&&s->words[5]==05650u&&operand(s,io,s->words[2],&code)!=EXEC_OK)return EXEC_OUT_OF_SCOPE;
 return cm_alarm_complete(s,io,entry,code);
}
enum exec_status cm_alarm2_site_call(apollo_overlay_state *s,cm_native_io *io){agc_word code=0;
 if(!s||!io)return EXEC_OUT_OF_SCOPE;
 if(s->words[5]==05652u&&operand(s,io,s->words[2],&code)!=EXEC_OK)return EXEC_OUT_OF_SCOPE;
 return cm_alarm2_image(s,io,code);
}
enum exec_status cm_abort_site_call(apollo_overlay_state *s,cm_native_io *io){agc_word code=0;
 if(!s||!io)return EXEC_OUT_OF_SCOPE;
 if((s->words[5]==07762u||s->words[5]==05726u)&&operand(s,io,s->words[01363],&code)!=EXEC_OK)return EXEC_OUT_OF_SCOPE;
 return cm_abort_word(s,io,code);
}
