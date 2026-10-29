/* SPDX-License-Identifier: MIT */
#include "dsky_decimal_select.h"
#include "noun_reader.h"
static const agc_word scale_cadr[2][14]={
 {061416,062566,060615,060677,060710,060715,060635,060717,065230,065303,060715,060704,060732,060623},
 {061323,062566,060516,060603,060614,060621,060524,060623,065143,065216,060621,060610,060636,060653}
};
enum p66_bias_status dsky_decimal_select(apollo_overlay_state *s,p66_bias_io *io,enum dsky_nv_mission mission,agc_word *route)
{
    static const agc_word row[3]={016,011,004};
    static const agc_word displacement[2][3]={{03025,04322,04331},{03025,04336,04345}};
    uint16_t *w;agc_word k,factor,routine,cadr,bank;unsigned shift;
    enum apollo_fixed_mission m;apollo_fixed_table table;apollo_noun_reader_io reader;
    if(s==0||io==0||route==0||(unsigned)mission>1u)return P66_BIAS_OUT_OF_SCOPE;
    w=s->words;
    if(w[5]!=02537u||w[4]!=062000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(062000u|(w[3]>>8))||w[7]!=0||io->superbank>0160u||(io->superbank&017u)||!(io->superbank&0100u)||io->interrupt_enabled>1u||(w[0140]!=1u&&w[0140]!=2u)||w[0117]>2u)return P66_BIAS_OUT_OF_SCOPE;
    k=w[0117];shift=w[0140]==1u?0u:5u*k;
    factor=(agc_word)(((unsigned)w[0147]>>shift)&037u);
    routine=(agc_word)(((unsigned)w[w[0140]==1u?0147u:0153u]>>(w[0140]==1u?5u:shift))&037u);
    m=mission==DSKY_NV_LM?APOLLO_FIXED_LM:APOLLO_FIXED_CM;
    if(routine>13u||!apollo_noun_table_info(m,APOLLO_SFOUTAB,&table)||(unsigned)factor*2u+1u>=table.word_count)return P66_BIAS_OUT_OF_SCOPE;
    w[1]=agc_extend(agc_correct(w[1]));w[0155]=0;w[0156]=0;w[0777]=row[k];w[0154]=agc_negate(w[01003u+k]);
    w[0114]=02551;
    if(w[0140]==2u){w[1]=displacement[(unsigned)mission][k];if(k==1u)w[020]=agc_negate(factor);else if(k==2u)w[022]=agc_negate(factor);}
    w[0123]=(agc_word)(factor*2u);bank=w[6];w[0]=02555;w[1]=agc_extend(bank);w[3]=0400;w[4]=064000;w[6]=064001;
    w[5]=mission==DSKY_NV_LM?02141u:02120u;reader.superbank=io->superbank;reader.interrupt_enabled=io->interrupt_enabled;
    /* Prevalidated table pair and exact source exchange guarantee support. */
    if(!apollo_noun_reader(s,&reader,m,APOLLO_GTSFOUT))return P66_BIAS_OUT_OF_SCOPE;
    if(w[0140]==1u){w[0114]=02563;w[020]=agc_negate(routine);}
    else {w[0114]=02561;w[1]=displacement[(unsigned)mission][k];if(k==1u)w[020]=agc_negate(routine);else if(k==2u)w[022]=agc_negate(routine);}
    cadr=scale_cadr[(unsigned)mission][routine];w[0]=02600;w[2]=(agc_word)(cadr&01777u);w[4]=(agc_word)(cadr&076000u);w[6]=(agc_word)(w[4]|(w[3]>>8));w[5]=(agc_word)(02000u+(cadr&01777u));*route=routine;
    return P66_BIAS_OK;
}
