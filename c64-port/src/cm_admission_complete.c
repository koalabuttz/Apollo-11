/* SPDX-License-Identifier: MIT; original Comanche055 admission groups. */
#include "cm_admission_complete.h"
enum exec_status cm_admission_complete(apollo_overlay_state *s,cm_native_io *io,enum cm_admission_entry entry,exec_address job){cm_executive e;agc_word priority,q,oldfb,oldbank,scratch_l;uint8_t i,v,needs_vac;enum exec_status status;
 static const agc_word pc[4]={05134,05147,05160,05154};
 if(!s||!io||(unsigned)entry>=4u||s->words[5]!=pc[entry]||s->words[7]||s->words[2]>07775u||s->words[3]>03400u||(s->words[3]&0377u)||(s->words[4]&~076000u)||s->words[6]!=(agc_word)(s->words[4]|(s->words[3]>>8))||(io->superbank&~0160u)||io->interrupt_enabled>1u||io->executive_alarm)return EXEC_OUT_OF_SCOPE;
 if(entry>=CM_ADMIT_SPVAC){if(io->interrupt_enabled)return EXEC_OUT_OF_SCOPE;priority=s->words[0063];job.location=agc_correct(s->words[0]);job.bank=agc_correct(s->words[1]);}else{if(agc_overflow(s->words[0]))return EXEC_OUT_OF_SCOPE;priority=agc_correct(s->words[0]);}
 if(!priority||(priority&0140777u)||job.location>AGC_MASK||job.bank>AGC_MASK)return EXEC_OUT_OF_SCOPE;
 cm_native_gather_executive(s,io,&e);
 if(e.newjob!=AGC_NEG_ZERO&&(e.newjob>72u||e.newjob%12u))return EXEC_OUT_OF_SCOPE;
 for(i=0;i<7u;++i)if(e.core[i][CM_EXEC_PRIORITY]==0)return EXEC_OUT_OF_SCOPE;
 if(e.core[0][CM_EXEC_PRIORITY]==AGC_NEG_ZERO&&e.newjob!=AGC_NEG_ZERO)return EXEC_OUT_OF_SCOPE;
 for(v=0;v<5u;++v)if(e.vac[v][0]&&e.vac[v][0]!=(agc_word)(0400u+v*44u))return EXEC_OUT_OF_SCOPE;
 for(i=0;i<7u&&e.core[i][CM_EXEC_PRIORITY]!=AGC_NEG_ZERO;++i){}
 oldfb=s->words[4];oldbank=i<7u?e.core[i][CM_EXEC_BANK]:0;scratch_l=e.newloc[1];q=s->words[2];needs_vac=entry!=CM_ADMIT_NOVAC;
 if(entry==CM_ADMIT_SPVAC){if(q<2u)return EXEC_OUT_OF_SCOPE;q=(agc_word)(q-2u);}
 if(needs_vac)for(v=0;v<5u&&e.vac[v][0]==0;++v){}
 status=cm_exec_admit(&e,priority,job.location,job.bank,needs_vac);
 if(status!=EXEC_OK&&status!=EXEC_NO_VAC&&status!=EXEC_NO_CORE)return status;
 cm_native_commit_executive(s,io,&e);s->words[061]=oldfb;io->interrupt_enabled=0;
 if(status==EXEC_OK){s->words[062]=(agc_word)(6u-i);s->words[0]=agc_extend(oldfb);s->words[1]=agc_extend(oldbank);s->words[5]=(agc_word)(q+2u);s->words[2]=06712;return status;}
 s->words[4]=02000;s->words[6]=(agc_word)(02000u|(s->words[3]>>8));s->words[0]=0;s->words[1]=agc_extend(needs_vac&&v<5u?(agc_word)(0400u+v*44u):scratch_l);s->words[5]=07755;s->words[2]=status==EXEC_NO_VAC?02642:02724;
 if(status==EXEC_NO_CORE)s->words[062]=0;
 return status;
}
