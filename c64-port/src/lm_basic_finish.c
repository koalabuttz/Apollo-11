/* SPDX-License-Identifier: MIT */
#include "lm_basic_finish.h"
static void view(executive *e,uint16_t *w,uint8_t read){uint8_t i,j;for(i=0;i<EXEC_CORE_COUNT;++i)for(j=0;j<EXEC_CORE_WORDS;++j){uint16_t a=(uint16_t)(0154u+i*12u+j);if(read)e->core[i][j]=w[a];else w[a]=e->core[i][j];}for(i=0;i<EXEC_VAC_COUNT;++i)for(j=0;j<EXEC_VAC_WORDS;++j){uint16_t a=(uint16_t)(0400u+i*44u+j);if(read)e->vac[i][j]=w[a];else w[a]=e->vac[i][j];}if(read){e->newjob=w[067];e->newprio=w[063];e->newloc[0]=w[065];e->newloc[1]=w[066];e->locctr=w[064];e->fixloc=w[0120];e->ovfind=w[0121];e->alarm=0;}else{w[067]=e->newjob;w[063]=e->newprio;w[065]=e->newloc[0];w[066]=e->newloc[1];w[064]=e->locctr;w[0120]=e->fixloc;w[0121]=e->ovfind;}}
enum exec_status lm_basic_finish(apollo_overlay_state *s,p66_bias_io *io,agc_word *channel11,exec_transfer *next){uint16_t *w=s->words,base=(uint16_t)(w[0167]&0777u),oldEB=w[3],highest=0,buf0=w[0130],buf1=AGC_NEG_ZERO,buf2=w[0132],q=w[2];uint8_t i,winner=0,interpretive=0;executive e;exec_address result;enum exec_status status;static const agc_word calls[7]={03122,03126,03132,03136,03142,03146,03152};
 if(next==0||w[5]!=05155u||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||w[7]!=0||w[0167]==0||w[0167]>=040000u||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled>1u||*channel11>AGC_MASK||(base>EXEC_FAKEPRET&&(base<0401u||base>0661u||(base-0401u)%054u)))return EXEC_OUT_OF_SCOPE;
 for(i=1;i<8u;++i){uint16_t priority=w[0167u+12u*i];if(priority==0)return EXEC_OUT_OF_SCOPE;if(priority<040000u){buf2=(uint16_t)(priority-1u);if(buf2>highest){highest=buf2;buf1=agc_negate(buf2);buf0=calls[i-1u];winner=i;q=03206;}else q=03201;}}
 if(winner){agc_word loc=w[0164u+12u*winner],bank=w[0165u+12u*winner];interpretive=(uint8_t)((loc&AGC_SIGN)!=0);if((bank&(agc_word)~076167u)||(!interpretive&&(loc==0||loc>07777u))||(interpretive&&agc_negate(loc)>=07777u))return EXEC_OUT_OF_SCOPE;}
 view(&e,w,1);for(i=1;i<8u;++i)if(w[0167u+12u*i]==1u)e.core[i][EXEC_PRIORITY]=AGC_NEG_ZERO;
 /* Reuse native source context exchange. The canonical negative LOC is
  * transformed only in this transient typed view to its positive INTRSM next
  * address; the selected slot receives the same original outgoing LOC. */
 if(interpretive)e.core[winner][EXEC_LOC]=agc_add_word(agc_negate(e.core[winner][EXEC_LOC]),1);
 status=exec_complete_basic(&e,io->superbank,&result);if(status!=EXEC_OK&&status!=EXEC_IDLE)return status;
 for(i=1;i<8u;++i)if(w[0167u+12u*i]==1u)e.core[i][EXEC_PRIORITY]=1;
 view(&e,w,0);w[0130]=buf0;w[0131]=buf1;w[0132]=buf2;w[2]=q;io->interrupt_enabled=1;
 if(status==EXEC_IDLE){*channel11=(agc_word)(*channel11&077775u);w[0]=agc_extend(*channel11);w[1]=base;w[4]=02000;w[6]=(agc_word)(02000u|(oldEB>>8));w[5]=03214;}
 else{w[1]=(agc_word)(02000u|(oldEB>>8));w[6]=(agc_word)(result.bank&076007u);w[3]=(agc_word)((w[6]&7u)*0400u);w[4]=(agc_word)(w[6]&076000u);io->superbank=(agc_word)(result.bank&0160u);if(interpretive){w[0164]=result.location;w[0165]=w[6];w[0115]=(agc_word)(w[6]&040000u);w[023]=0;w[0]=agc_extend(w[0115]);w[5]=06070;}else{w[0]=02772;w[5]=result.location;}}
 next->address=result;next->interpretive=interpretive;return status;}
