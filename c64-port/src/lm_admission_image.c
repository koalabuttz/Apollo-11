/* SPDX-License-Identifier: MIT; original Luminary099 Executive admission. */
#include "lm_admission_image.h"
enum exec_status lm_admission_image(apollo_overlay_state *image,p66_bias_io *io,enum lm_admission_entry entry,exec_address job){executive e;uint16_t *w;agc_word priority,q,oldfb,oldbank,scratch_l;uint8_t core,vac,needs_vac;enum exec_status status;static const uint16_t pc[4]={05072,05105,05116,05112};
 if(!image||!io||(unsigned)entry>=4u)return EXEC_OUT_OF_SCOPE;
 w=image->words;if(w[5]!=pc[entry]||w[7]||w[2]>07775u||w[3]>03400u||(w[3]&0377u)||(w[4]&~076000u)||w[6]!=(agc_word)(w[4]|(w[3]>>8))||(io->superbank&~0160u)||io->interrupt_enabled>1u)return EXEC_OUT_OF_SCOPE;
 if(entry>=LM_ADMIT_SPVAC){if(io->interrupt_enabled)return EXEC_OUT_OF_SCOPE;priority=w[063];job.location=agc_correct(w[0]);job.bank=agc_correct(w[1]);}else{if(agc_overflow(w[0]))return EXEC_OUT_OF_SCOPE;priority=agc_correct(w[0]);}
 if(!priority||(priority&0140777u)||job.location>AGC_MASK||job.bank>AGC_MASK)return EXEC_OUT_OF_SCOPE;
 p66_native_import_executive(&e,image);e.alarm=0;
 if(e.newjob!=AGC_NEG_ZERO&&(e.newjob>84u||e.newjob%12u))return EXEC_OUT_OF_SCOPE;
 for(core=0;core<EXEC_CORE_COUNT;++core)if(e.core[core][EXEC_PRIORITY]==0)return EXEC_OUT_OF_SCOPE;
 if(e.core[0][EXEC_PRIORITY]==AGC_NEG_ZERO&&e.newjob!=AGC_NEG_ZERO)return EXEC_OUT_OF_SCOPE;
 for(vac=0;vac<EXEC_VAC_COUNT;++vac)if(e.vac[vac][0]&&e.vac[vac][0]!=(agc_word)(0400u+vac*44u))return EXEC_OUT_OF_SCOPE;
 for(core=0;core<EXEC_CORE_COUNT&&e.core[core][EXEC_PRIORITY]!=AGC_NEG_ZERO;++core){}
 oldfb=w[4];oldbank=core<EXEC_CORE_COUNT?e.core[core][EXEC_BANK]:0;scratch_l=e.newloc[1];q=w[2];needs_vac=(uint8_t)(entry!=LM_ADMIT_NOVAC);
 if(entry==LM_ADMIT_SPVAC){if(q<2u)return EXEC_OUT_OF_SCOPE;q=(agc_word)(q-2u);}
 if(needs_vac)for(vac=0;vac<EXEC_VAC_COUNT&&e.vac[vac][0]==0;++vac){}
 status=exec_admit(&e,priority,job.location,job.bank,needs_vac);
 if(status!=EXEC_OK&&status!=EXEC_NO_VAC&&status!=EXEC_NO_CORE)return status;
 p66_native_export_executive(image,&e);w[061]=oldfb;io->interrupt_enabled=0;
 if(status==EXEC_OK){w[062]=(agc_word)(7u-core);w[0]=agc_extend(oldfb);w[1]=agc_extend(oldbank);w[5]=(agc_word)(q+2u);w[2]=06743;return status;}
 if(needs_vac&&vac<EXEC_VAC_COUNT)scratch_l=(agc_word)(0400u+vac*44u);
 w[061]=scratch_l;w[0]=q;w[1]=agc_extend(oldfb);w[4]=02000;w[6]=(agc_word)(02000u|(w[3]>>8));w[5]=05716;w[2]=status==EXEC_NO_VAC?02617:02703;
 if(status==EXEC_NO_CORE)w[062]=0;
 return status;
}
