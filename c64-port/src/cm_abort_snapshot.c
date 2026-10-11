/* SPDX-License-Identifier: MIT */
#include "cm_abort_snapshot.h"
enum exec_status cm_abort_snapshot_stage(apollo_overlay_state *s,cm_native_io *io,uint8_t phase,agc_word literal){agc_word *w,q,entry;
 if(!s||!io||phase>4u||literal>AGC_MASK)return EXEC_OUT_OF_SCOPE;
 if(phase==1u)return cm_vac5_store(s,io);
 if(phase==2u){if(s->words[0133]!=07762u&&s->words[0133]!=05726u&&s->words[0133]!=05754u)return EXEC_OUT_OF_SCOPE;return cm_interbank_extended(s,io,CM_IB_SWRETURN,0,0);}
 if(phase==4u)return cm_alarm_record(s,io,CM_RECORD_BORTENT,0);
 w=s->words;q=w[2];entry=w[5];
 if(w[7]||w[3]>03400u||(w[3]&0377u)||(w[4]&~076000u)||w[6]!=(agc_word)(w[4]|(w[3]>>8))||(io->superbank&~0160u)||io->interrupt_enabled>1u)return EXEC_OUT_OF_SCOPE;
 if(phase==0u){if(q<0100u||q>07775u||(entry!=07755u&&entry!=05721u&&entry!=05747u))return EXEC_OUT_OF_SCOPE;
  io->interrupt_enabled=0;w[01363]=q;w[0]=agc_extend(q);w[2]=entry==07755u?07761u:entry==05721u?05725u:05753u;w[5]=04662;return cm_interbank_extended(s,io,CM_IB_BANKCALL,040000,0);
 }
 if(entry!=07762u&&entry!=05726u&&entry!=05754u)return EXEC_OUT_OF_SCOPE;
 w[0]=agc_extend(entry==05754u?01103u:literal);w[1]=agc_extend(agc_correct(w[1]));w[2]=entry==07762u?07765u:05731u;w[5]=05655;return EXEC_OK;
}
