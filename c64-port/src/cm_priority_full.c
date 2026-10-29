/* SPDX-License-Identifier: MIT; native original Comanche PRIOCHNG. */
#include "cm_priority_full.h"
static void exchange(cm_executive *s, uint8_t slot, exec_address outgoing,
                           exec_address *next) {
    uint8_t j;
    agc_word debris_bank=s->core[0][CM_EXEC_BANK];
    /* CHANJOB+4: outgoing LOC/BBCON arrive in A/L; current LOC/BANKSET
     * are scratch, and need not equal that pair. */
    next->location=s->core[slot][CM_EXEC_LOC]; next->bank=s->core[slot][CM_EXEC_BANK];
    s->core[slot][CM_EXEC_LOC]=outgoing.location;
    s->core[slot][CM_EXEC_BANK]=outgoing.bank;
    for (j=0; j<8; ++j) {
        agc_word old=s->core[0][j];
        s->core[0][j]=s->core[slot][j]; s->core[slot][j]=old;
    }
    if (s->ovfind != 0 && s->ovfind != AGC_NEG_ZERO)
        s->core[0][CM_EXEC_PUSH]=agc_negate(s->core[0][CM_EXEC_PUSH]);
    for (j=CM_EXEC_PUSH; j<=CM_EXEC_PRIORITY; ++j) {
        agc_word old=s->core[0][j];
        s->core[0][j]=s->core[slot][j]; s->core[slot][j]=old;
    }
    s->fixloc=(agc_word)(s->core[0][CM_EXEC_PRIORITY] & CM_EXEC_LOW9);
    s->ovfind=0;
    if ((s->core[0][CM_EXEC_PUSH] & AGC_SIGN) != 0) {
        s->core[0][CM_EXEC_PUSH]=agc_negate(s->core[0][CM_EXEC_PUSH]); s->ovfind=1;
    }
    s->newjob=0;
    /* ENDPRCHG DXCH LOC takes the selected pair into A/L for DTCB.
     * A=+0 and L=the old current BANKSET at this boundary. */
    s->core[0][CM_EXEC_LOC]=0; s->core[0][CM_EXEC_BANK]=debris_bank;
}
enum exec_status cm_priority_full(apollo_overlay_state *s,cm_native_io *io,exec_transfer *next){
 agc_word *w,bank,caller,oldl,highest,packed,base,buf0=0,buf1,buf2,q;uint8_t i,hole=0,winner=0,interp=0;cm_executive e;exec_address result,resume;
 static const agc_word calls[6]={03143,03147,03153,03157,03163,03167};
 if(!s||!io||!next)return EXEC_OUT_OF_SCOPE;
 w=s->words;bank=w[6];caller=w[2];oldl=w[1];base=(agc_word)(w[0167]&0777u);buf2=w[0132];q=caller;
 if(w[5]!=05210u||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||bank!=(agc_word)(w[4]|(w[3]>>8))||w[7]||(io->superbank&~0160u)||io->interrupt_enabled>1u||caller<0100u||caller>07777u||io->executive_alarm||(w[0]&0140777u)||!w[0167]||w[0167]>=040000u)return EXEC_OUT_OF_SCOPE;
 if(base!=0110u&&(base<0401u||base>0661u||(base-0401u)%054u))return EXEC_OUT_OF_SCOPE;
 packed=(agc_word)(w[0]+base);highest=packed;buf1=agc_negate(packed);
 for(i=1;i<7u;++i)if(i!=3u&&!w[0167u+12u*i]){hole=i;break;}
 for(i=1;i<(hole?hole:7u);++i){agc_word priority=w[0167u+12u*i];if(priority&&priority<040000u){buf2=(agc_word)(priority-1u);if(buf2>highest){highest=buf2;buf1=agc_negate(buf2);buf0=calls[i-1u];winner=i;q=03223;}else q=03216;}}
 if(hole){io->interrupt_enabled=0;w[0063]=w[0];w[6]=02006;w[3]=03000;w[4]=02000;w[0165]=bank;w[0164]=caller;w[0167]=packed;w[1]=agc_extend(agc_correct(oldl));w[0130]=buf0;w[0131]=buf1;w[0132]=buf2;w[0]=0;w[2]=(agc_word)(calls[hole-1u]+1u);w[5]=05747;next->interpretive=0;next->address=(exec_address){05747,(agc_word)(02006u|io->superbank)};return EXEC_OK;}
 cm_native_gather_executive(s,io,&e);resume=(exec_address){caller,(agc_word)(bank|io->superbank)};
 if(winner){agc_word loc=e.core[winner][CM_EXEC_LOC],saved=e.core[winner][CM_EXEC_BANK];interp=(uint8_t)((loc&AGC_SIGN)!=0);if((saved&~076167u)||(!interp&&(!loc||loc>07777u))||(interp&&agc_negate(loc)>=07777u))return EXEC_OUT_OF_SCOPE;if(interp)e.core[winner][CM_EXEC_LOC]=agc_add_word(agc_negate(loc),1);}
 e.newprio=w[0];e.core[0][CM_EXEC_PRIORITY]=packed;e.core[0][CM_EXEC_LOC]=caller;e.core[0][CM_EXEC_BANK]=(agc_word)(bank&076007u);e.newjob=(agc_word)(winner*12u);
 if(winner)exchange(&e,winner,resume,&result);else{result=resume;e.core[0][CM_EXEC_LOC]=0;e.core[0][CM_EXEC_BANK]=0;}
 if(interp){e.core[0][CM_EXEC_LOC]=result.location;e.core[0][CM_EXEC_BANK]=(agc_word)(result.bank&076007u);}
 cm_native_commit_executive(s,io,&e);if(!winner)w[0165]=agc_correct(oldl);w[0130]=buf0;w[0131]=buf1;w[0132]=buf2;w[0]=03013;w[1]=02006;w[2]=q;w[5]=result.location;w[6]=(agc_word)(result.bank&076007u);w[3]=(agc_word)((w[6]&7u)*0400u);w[4]=(agc_word)(w[6]&076000u);if(winner)io->superbank=(agc_word)(result.bank&0160u);io->interrupt_enabled=1;
 if(interp){w[0115]=(agc_word)(w[6]&040000u);w[023]=0;w[0]=agc_extend(w[0115]);w[5]=06037;}
 next->interpretive=interp;next->address=result;return EXEC_OK;
}
