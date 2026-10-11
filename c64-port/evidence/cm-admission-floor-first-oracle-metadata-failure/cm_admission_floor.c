/* SPDX-License-Identifier: MIT; source+0 scheduling priority.
 * Original CM NOVAC/FINDVAC/CORFOUND/SETLOC bodies, additive qualification;
 * existing positive-priority APIs and their published proofs unchanged. */
#include "cm_admission_floor.h"
static enum exec_status floor_admit(cm_executive *s, agc_word priority,
                           agc_word location, agc_word bank, int needs_vac) {
    uint8_t i, candidate;
    agc_word scratch_l=s->newloc[1], scratch_a;
    agc_register difference;
    if (s->alarm != 0) return EXEC_BAILOUT_PENDING;
    if (priority != 0 || (priority & UINT16_C(0140777)) != 0 ||
        location > AGC_MASK || bank > AGC_MASK ||
        (s->newjob != AGC_NEG_ZERO &&
         (s->newjob > 72u || s->newjob % CM_EXEC_CORE_WORDS != 0)))
        return EXEC_OUT_OF_SCOPE;
    s->newprio=priority; s->newloc[0]=location; s->newloc[1]=bank;
    if (needs_vac) {
        /* FINDVAC2: scan actual use words in order. +address is free. */
        for (i=0; i<CM_EXEC_VAC_COUNT; ++i)
            if (agc_classify(s->vac[i][0]) == AGC_POSITIVE) break;
        if (i == CM_EXEC_VAC_COUNT) { s->alarm=01201; return EXEC_NO_VAC; }
        scratch_l=s->vac[i][0];
        /* CCS gives use-address-1; AD TWO gives VAC base. LXCH reserves. */
        s->newprio=agc_correct(agc_add_register(agc_extend(s->newprio),
                         agc_extend((agc_word)(s->vac[i][0]+1u))));
        s->vac[i][0]=0;
    } else {
        /* NOVAC uses MPAC-36, not a generic "no work area" bit. */
        s->newprio=agc_correct(agc_add_register(agc_extend(priority),
                                              agc_extend(CM_EXEC_FAKEPRET)));
    }
    /* NOVAC2 / NOVAC3 / NEXTCORE: -0 only means available. */
    s->locctr=0;
    for (i=0; i<CM_EXEC_ADMISSION_COUNT; ++i) {
        if (s->core[i][CM_EXEC_PRIORITY] == AGC_NEG_ZERO) break;
        s->locctr=(agc_word)(s->locctr + CM_EXEC_CORE_WORDS);
    }
    if (i == CM_EXEC_ADMISSION_COUNT) {
        /* The assembly does NOT release a VAC acquired above before bailout. */
        s->alarm=01202; return EXEC_NO_CORE;
    }
    /* CORFOUND */
    s->core[i][CM_EXEC_PRIORITY]=s->newprio;
    s->core[i][CM_EXEC_PUSH]=(agc_word)(s->newprio & CM_EXEC_LOW9);
    if (i == 0) {
        s->ovfind=0; s->fixloc=s->core[0][CM_EXEC_PUSH];
        /* SPECTEST: idle -0 -> +0; a queued job keeps NEWJOB unchanged. */
        if (s->newjob == AGC_NEG_ZERO) {
            s->newjob=0;
            s->core[0][CM_EXEC_LOC]=location; s->core[0][CM_EXEC_BANK]=bank;
            s->newloc[0]=0; s->newloc[1]=scratch_l;
            return EXEC_OK;
        }
    }
    /* SETLOC: first DXCH puts A/L debris into NEWLOC; second leaves the
     * previous slot's 2CADR in A/L, not in NEWLOC. */
    scratch_a=i==0 ? agc_diminished_abs(s->newjob) : (agc_word)(s->locctr-1u);
    s->core[i][CM_EXEC_LOC]=location; s->core[i][CM_EXEC_BANK]=bank;
    s->newloc[0]=scratch_a; s->newloc[1]=scratch_l;
    candidate=s->newjob == AGC_NEG_ZERO ? 0 : (uint8_t)(s->newjob / CM_EXEC_CORE_WORDS);
    difference=agc_add_register(agc_extend(agc_negate(s->core[candidate][CM_EXEC_PRIORITY])),
                               agc_extend(s->newprio));
    /* BZMF examines accumulator sign and either zero, before overflow correction. */
    if (difference != 0 && difference != UINT16_C(0177777) &&
        (difference & UINT16_C(0100000)) == 0) s->newjob=s->locctr;
    return EXEC_OK;
}
enum exec_status cm_admission_floor(apollo_overlay_state *s,cm_native_io *io,enum cm_admission_entry entry,exec_address job){cm_executive e;agc_word priority,q,oldfb,oldbank,scratch_l;uint8_t i,v,needs_vac;enum exec_status status;
 static const agc_word pc[4]={05134,05147,05160,05154};
 if(!s||!io||(unsigned)entry>=4u||s->words[5]!=pc[entry]||s->words[7]||s->words[2]<0100u||s->words[2]>07775u||s->words[3]>03400u||(s->words[3]&0377u)||(s->words[4]&~076000u)||s->words[6]!=(agc_word)(s->words[4]|(s->words[3]>>8))||(io->superbank&~0160u)||io->interrupt_enabled>1u||io->executive_alarm)return EXEC_OUT_OF_SCOPE;
 if(entry>=CM_ADMIT_SPVAC){if(io->interrupt_enabled)return EXEC_OUT_OF_SCOPE;priority=s->words[0063];job.location=agc_correct(s->words[0]);job.bank=agc_correct(s->words[1]);}else{if(agc_overflow(s->words[0]))return EXEC_OUT_OF_SCOPE;priority=agc_correct(s->words[0]);}
 if(priority!=0||(priority&0140777u)||job.location>AGC_MASK||job.bank>AGC_MASK)return EXEC_OUT_OF_SCOPE;
 cm_native_gather_executive(s,io,&e);
 if(e.newjob!=AGC_NEG_ZERO&&(e.newjob>72u||e.newjob%12u))return EXEC_OUT_OF_SCOPE;
 if(e.core[0][CM_EXEC_PRIORITY]==AGC_NEG_ZERO&&e.newjob!=AGC_NEG_ZERO)return EXEC_OUT_OF_SCOPE;
 for(v=0;v<5u;++v)if(e.vac[v][0]&&e.vac[v][0]!=(agc_word)(0400u+v*44u))return EXEC_OUT_OF_SCOPE;
 for(i=0;i<7u&&e.core[i][CM_EXEC_PRIORITY]!=AGC_NEG_ZERO;++i){}
 oldfb=s->words[4];oldbank=i<7u?e.core[i][CM_EXEC_BANK]:0;scratch_l=e.newloc[1];q=s->words[2];needs_vac=entry!=CM_ADMIT_NOVAC;
 if(entry==CM_ADMIT_SPVAC){if(q<2u)return EXEC_OUT_OF_SCOPE;q=(agc_word)(q-2u);}
 if(needs_vac)for(v=0;v<5u&&e.vac[v][0]==0;++v){}
 status=floor_admit(&e,priority,job.location,job.bank,needs_vac);
 if(status!=EXEC_OK&&status!=EXEC_NO_VAC&&status!=EXEC_NO_CORE)return status;
 cm_native_commit_executive(s,io,&e);s->words[061]=oldfb;io->interrupt_enabled=0;
 if(status==EXEC_OK){s->words[062]=(agc_word)(6u-i);s->words[0]=agc_extend(oldfb);s->words[1]=agc_extend(oldbank);s->words[5]=(agc_word)(q+2u);s->words[2]=06712;return status;}
 s->words[4]=02000;s->words[6]=(agc_word)(02000u|(s->words[3]>>8));s->words[0]=0;s->words[1]=agc_extend(needs_vac&&v<5u?(agc_word)(0400u+v*44u):scratch_l);s->words[5]=07755;s->words[2]=status==EXEC_NO_VAC?02642:02724;
 if(status==EXEC_NO_CORE)s->words[062]=0;
 return status;
}
