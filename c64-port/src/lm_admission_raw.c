/* SPDX-License-Identifier: MIT; original Luminary099 Executive admission. */
#include "lm_admission_raw.h"
static enum exec_status admit_raw(executive *s, agc_word priority,
                           agc_word location, agc_word bank, int needs_vac) {
    uint8_t i, candidate;
    agc_word scratch_l=s->newloc[1], scratch_a;
    agc_register difference;
    if (s->alarm != 0) return EXEC_BAILOUT_PENDING;
    if (priority > 037000u ||
        location > AGC_MASK || bank > AGC_MASK ||
        (s->newjob != AGC_NEG_ZERO &&
         (s->newjob > 84u || s->newjob % EXEC_CORE_WORDS != 0)))
        return EXEC_OUT_OF_SCOPE;
    s->newprio=priority; s->newloc[0]=location; s->newloc[1]=bank;
    if (needs_vac) {
        /* FINDVAC2: scan actual use words in order. +address is free. */
        for (i=0; i<EXEC_VAC_COUNT; ++i)
            if (agc_classify(s->vac[i][0]) == AGC_POSITIVE) break;
        if (i == EXEC_VAC_COUNT) { s->alarm=01201; return EXEC_NO_VAC; }
        scratch_l=s->vac[i][0];
        /* CCS gives use-address-1; AD TWO gives VAC base. LXCH reserves. */
        s->newprio=agc_correct(agc_add_register(agc_extend(s->newprio),
                         agc_extend((agc_word)(s->vac[i][0]+1u))));
        s->vac[i][0]=0;
    } else {
        /* NOVAC uses MPAC-36, not a generic "no work area" bit. */
        s->newprio=agc_correct(agc_add_register(agc_extend(priority),
                                              agc_extend(EXEC_FAKEPRET)));
    }
    /* NOVAC2 / NOVAC3 / NEXTCORE: -0 only means available. */
    s->locctr=0;
    for (i=0; i<EXEC_ADMISSION_COUNT; ++i) {
        if (s->core[i][EXEC_PRIORITY] == AGC_NEG_ZERO) break;
        s->locctr=(agc_word)(s->locctr + EXEC_CORE_WORDS);
    }
    if (i == EXEC_ADMISSION_COUNT) {
        /* The assembly does NOT release a VAC acquired above before bailout. */
        s->alarm=01202; return EXEC_NO_CORE;
    }
    /* CORFOUND */
    s->core[i][EXEC_PRIORITY]=s->newprio;
    s->core[i][EXEC_PUSH]=(agc_word)(s->newprio & EXEC_LOW9);
    if (i == 0) {
        s->ovfind=0; s->fixloc=s->core[0][EXEC_PUSH];
        /* SPECTEST: idle -0 -> +0; a queued job keeps NEWJOB unchanged. */
        if (s->newjob == AGC_NEG_ZERO) {
            s->newjob=0;
            s->core[0][EXEC_LOC]=location; s->core[0][EXEC_BANK]=bank;
            s->newloc[0]=0; s->newloc[1]=scratch_l;
            return EXEC_OK;
        }
    }
    /* SETLOC: first DXCH puts A/L debris into NEWLOC; second leaves the
     * previous slot's 2CADR in A/L, not in NEWLOC. */
    scratch_a=i==0 ? agc_diminished_abs(s->newjob) : (agc_word)(s->locctr-1u);
    s->core[i][EXEC_LOC]=location; s->core[i][EXEC_BANK]=bank;
    s->newloc[0]=scratch_a; s->newloc[1]=scratch_l;
    candidate=s->newjob == AGC_NEG_ZERO ? 0 : (uint8_t)(s->newjob / EXEC_CORE_WORDS);
    difference=agc_add_register(agc_extend(agc_negate(s->core[candidate][EXEC_PRIORITY])),
                               agc_extend(s->newprio));
    /* BZMF examines accumulator sign and either zero, before overflow correction. */
    if (difference != 0 && difference != UINT16_C(0177777) &&
        (difference & UINT16_C(0100000)) == 0) s->newjob=s->locctr;
    return EXEC_OK;
}

enum exec_status lm_admission_raw(apollo_overlay_state *image,p66_bias_io *io,enum lm_admission_entry entry,exec_address job){executive e;uint16_t *w;agc_word priority,q,oldfb,oldbank,scratch_l;uint8_t core,vac,needs_vac;enum exec_status status;static const uint16_t pc[4]={05072,05105,05116,05112};
 if(!image||!io||(unsigned)entry>=4u)return EXEC_OUT_OF_SCOPE;
 w=image->words;if(w[5]!=pc[entry]||w[7]||w[2]<0100u||w[2]>07775u||w[3]>03400u||(w[3]&0377u)||(w[4]&~076000u)||w[6]!=(agc_word)(w[4]|(w[3]>>8))||(io->superbank&~0160u)||io->interrupt_enabled>1u)return EXEC_OUT_OF_SCOPE;
 if(entry>=LM_ADMIT_SPVAC){if(io->interrupt_enabled)return EXEC_OUT_OF_SCOPE;priority=w[063];job.location=agc_correct(w[0]);job.bank=agc_correct(w[1]);}else{if(agc_overflow(w[0]))return EXEC_OUT_OF_SCOPE;priority=agc_correct(w[0]);if(w[2]<02000u){uint16_t a=w[2],b=(uint16_t)(a+1u);a=a<01400u?a:(uint16_t)((w[3]>>8)*0400u+(a&0377u));b=b<01400u?b:(uint16_t)((w[3]>>8)*0400u+(b&0377u));job.location=w[a];job.bank=w[b];}}
 if(priority>037000u||job.location>AGC_MASK||job.bank>AGC_MASK)return EXEC_OUT_OF_SCOPE;
 p66_native_import_executive(&e,image);e.alarm=0;
 if(e.newjob!=AGC_NEG_ZERO&&(e.newjob>84u||e.newjob%12u))return EXEC_OUT_OF_SCOPE;
 for(core=0;core<EXEC_CORE_COUNT;++core)if(e.core[core][EXEC_PRIORITY]==0)return EXEC_OUT_OF_SCOPE;
 if(e.core[0][EXEC_PRIORITY]==AGC_NEG_ZERO&&e.newjob==0)return EXEC_OUT_OF_SCOPE;
 for(vac=0;vac<EXEC_VAC_COUNT;++vac)if(e.vac[vac][0]&&e.vac[vac][0]!=(agc_word)(0400u+vac*44u))return EXEC_OUT_OF_SCOPE;
 for(core=0;core<EXEC_CORE_COUNT&&e.core[core][EXEC_PRIORITY]!=AGC_NEG_ZERO;++core){}
 oldfb=w[4];oldbank=core<EXEC_CORE_COUNT?e.core[core][EXEC_BANK]:0;scratch_l=e.newloc[1];q=w[2];needs_vac=(uint8_t)(entry!=LM_ADMIT_NOVAC);
 if(entry==LM_ADMIT_SPVAC){if(q<2u)return EXEC_OUT_OF_SCOPE;q=(agc_word)(q-2u);}
 if(needs_vac)for(vac=0;vac<EXEC_VAC_COUNT&&e.vac[vac][0]==0;++vac){}
 status=admit_raw(&e,priority,job.location,job.bank,needs_vac);
 if(status!=EXEC_OK&&status!=EXEC_NO_VAC&&status!=EXEC_NO_CORE)return status;
 p66_native_export_executive(image,&e);w[061]=oldfb;io->interrupt_enabled=0;
 if(status==EXEC_OK){w[062]=(agc_word)(7u-core);w[0]=agc_extend(oldfb);w[1]=agc_extend(oldbank);w[5]=(agc_word)(q+2u);w[2]=06743;return status;}
 if(needs_vac&&vac<EXEC_VAC_COUNT)scratch_l=(agc_word)(0400u+vac*44u);
 w[061]=scratch_l;w[0]=q;w[1]=agc_extend(oldfb);w[4]=02000;w[6]=(agc_word)(02000u|(w[3]>>8));w[5]=05716;w[2]=status==EXEC_NO_VAC?02617:02703;
 if(status==EXEC_NO_CORE)w[062]=0;
 return status;
}
