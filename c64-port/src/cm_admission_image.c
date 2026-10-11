/* SPDX-License-Identifier: MIT */
#include "cm_admission_image.h"
enum exec_status cm_admission_image(apollo_overlay_state *s,cm_native_io *io,exec_address job,uint8_t needs_vac){cm_executive e;agc_word oldfb,oldbank;uint8_t i;enum exec_status status;
 if(s==0||io==0||needs_vac>1u||s->words[0]>037000u||s->words[2]>07775u||s->words[6]!=(agc_word)(s->words[4]|(s->words[3]>>8))||io->interrupt_enabled>1u||(io->superbank&077617u)!=0)return EXEC_OUT_OF_SCOPE;
 cm_native_gather_executive(s,io,&e);for(i=0;i<7u;++i)if(e.core[i][CM_EXEC_PRIORITY]==077777u)break;
 if(i==7u)return EXEC_NO_CORE;
 oldfb=s->words[4];oldbank=e.core[i][CM_EXEC_BANK];status=cm_exec_admit(&e,s->words[0],job.location,job.bank,needs_vac);
 if(status!=EXEC_OK)return status;
 cm_native_commit_executive(s,io,&e);s->words[061]=oldfb;s->words[062]=(agc_word)(6u-i);
 s->words[0]=agc_extend(oldfb);s->words[1]=agc_extend(oldbank);s->words[5]=agc_correct(agc_add_register(s->words[2],2));s->words[2]=06712;io->interrupt_enabled=0;
 return EXEC_OK;
}
