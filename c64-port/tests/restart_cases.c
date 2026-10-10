/* SPDX-License-Identifier: MIT; manually constructed input fixtures only. */
#include "restart_cases.h"
void rs_prepare(restart_case *c,uint16_t n) {
    uint8_t i,j; agc_word pattern=(agc_word)((n*173u+05735u) & AGC_MASK);
    for(i=0;i<8;++i) for(j=0;j<12;++j) c->exec.core[i][j]=(agc_word)((pattern+i*71u+j*37u)&AGC_MASK);
    for(i=0;i<5;++i) for(j=0;j<44;++j) c->exec.vac[i][j]=(agc_word)((pattern+i*113u+j*29u)&AGC_MASK);
    for(i=0;i<8;++i) c->exec.core[i][EXEC_PRIORITY]=(agc_word)(02000+EXEC_FAKEPRET);
    for(i=0;i<5;++i) c->exec.vac[i][0]=0;
    c->exec.newjob=0; c->exec.newprio=05000; c->exec.newloc[0]=03210; c->exec.newloc[1]=07065;
    c->exec.locctr=0144; c->exec.fixloc=0110; c->exec.ovfind=077777; c->exec.alarm=0;
    c->wait.time3=021234; c->wait.ruptagn=077777; c->wait.alarm=0;
    for(i=0;i<8;++i) c->wait.interval[i]=(agc_word)(057777+i);
    for(i=0;i<9;++i) { c->wait.task[i].location=(agc_word)(03000+i); c->wait.task[i].bank=06065; }
    for(i=0;i<3;++i) c->record.failreg[i]=0;
    c->record.almcadr[0]=01372; c->record.almcadr[1]=06063;
    for(i=0;i<12;++i) c->record.dsptab[i]=pattern;
    for(i=0;i<RS_WORD_COUNT;++i) c->record.word[i]=(agc_word)((pattern+i*313u)&AGC_MASK);
    c->record.channel11=pattern; c->record.channel12=(agc_word)(pattern ^ 077777);
    c->record.channel13=(n & 1u)!=0 ? 077777 : 040160; c->record.channel14=pattern;
    c->record.trap31a=(agc_word)(n & 1u); c->record.trap31b=(agc_word)((n >> 1)&1u); c->record.trap32=(agc_word)((n >> 2)&1u);
    for(i=0;i<6;++i) {
        c->phases.phase[i]=0; c->phases.complement[i]=077777;
        c->phases.address[i].location=(agc_word)(03000+i);
        c->phases.address[i].bank=(agc_word)(06000+i);
        c->phases.priority[i]=02000;
    }
    c->caller.location=03721; c->caller.bank=06105; c->code=(agc_word)(01201+n%4u);
    if(n<96) {
        uint8_t variant=(uint8_t)(n%32u);
        static const agc_word codes[8]={0,1,01201,01202,01203,01204,02000,037777};
        c->code=codes[variant%8u];
        if(variant>=8) c->record.failreg[0]=01201;
        if(variant>=16) c->record.failreg[1]=01202;
        if(variant>=24) c->record.failreg[2]=(variant & 1u)!=0 ? 041203 : 01203;
        if(variant==22) c->record.failreg[2]=040000;
        if(variant==23) c->record.failreg[2]=077777;
    } else if(n<112) {
        c->record.failreg[0]=01201; c->record.failreg[1]=01202; c->record.failreg[2]=041203;
        c->record.word[RS_FLAGWRD3]=(n & 1u)!=0 ? 077777 : 0;
        c->record.word[RS_RADMODES]=077777;
        c->record.word[RS_IMODES33]=(n & 2u)!=0 ? 040 : 077777;
        c->record.channel11=(n & 4u)!=0 ? 077777 : 0;
        c->record.channel12=(n & 8u)!=0 ? 077777 : 0;
        c->exec.alarm=01202; c->wait.alarm=01203;
    } else if(n<138) {
        static const agc_word phases[8]={0,077777,1,00402,01777,037777,040000,052525};
        for(i=0;i<6;++i) {
            c->phases.phase[i]=phases[(n+i)%8u];
            c->phases.complement[i]=(agc_word)(c->phases.phase[i] ^ AGC_MASK);
        }
        if(n>=120) c->phases.complement[(n-120u)%6u]^=(agc_word)(1u << ((n-120u)/6u));
    } else if(n<186) {
        uint8_t group=(uint8_t)((n-138u)/8u),variant=(uint8_t)((n-138u)%8u);
        c->phases.phase[group]=(agc_word)(00402+(variant%4u)*0100u);
        c->phases.complement[group]=(agc_word)(c->phases.phase[group]^AGC_MASK);
        c->phases.priority[group]=(variant & 1u)!=0 ? (agc_word)(03000 ^ AGC_MASK) : 03000;
        if(variant<6) {
            if(variant<2) c->exec.newjob=AGC_NEG_ZERO;
            c->exec.core[(variant/2u)*3u][EXEC_PRIORITY]=AGC_NEG_ZERO;
            c->exec.vac[(variant/2u)*2u][0]=(agc_word)(0400+(variant/2u)*88u);
        }
        if(variant==6) c->exec.vac[0][0]=0400;
    } else {
        for(i=0;i<6;++i) { c->phases.phase[i]=00402; c->phases.complement[i]=077375;
            c->phases.priority[i]=i==0 ? 03000 : (agc_word)(02000 ^ AGC_MASK); }
        c->exec.vac[0][0]=0400; /* The failed FINDVAC reserves this VAC before 1202. */
    }
}
uint8_t rs_steps(uint16_t n) { return n>=186 ? 10 : 1; }
#ifndef RESTART_INPUT_ONLY
int rs_apply(restart_case *c,uint16_t n,uint8_t step) {
    if(n<96) return (int)rs_record(&c->record,(enum rs_alarm_entry)(n/32u),c->code,c->caller);
    if(n<112) { rs_cleanup(&c->record,&c->exec,&c->wait); return RS_OK; }
    if(n<138) return (int)rs_validate(&c->exec,&c->phases);
    if(n<186) return (int)rs_job(&c->exec,&c->phases,(uint8_t)((n-138u)/8u));
    if(step==0) return (int)exec_admit(&c->exec,03000,03200,06005,1);
    if(step==1) return (int)rs_record(&c->record,RS_BAILOUT1,c->exec.alarm,c->caller);
    if(step==2) { rs_cleanup(&c->record,&c->exec,&c->wait); return RS_OK; }
    if(step==3) return (int)rs_validate(&c->exec,&c->phases);
    return (int)rs_job(&c->exec,&c->phases,(uint8_t)(9u-step));
}
#endif
void rs_words(const restart_case *c,rs_word_sink emit,void *user) {
    uint8_t i,j;
    for(i=0;i<8;++i) for(j=0;j<12;++j) emit(user,c->exec.core[i][j]);
    for(i=0;i<5;++i) for(j=0;j<44;++j) emit(user,c->exec.vac[i][j]);
    emit(user,c->exec.newjob); emit(user,c->exec.newprio); emit(user,c->exec.newloc[0]); emit(user,c->exec.newloc[1]);
    emit(user,c->exec.locctr); emit(user,c->exec.fixloc); emit(user,c->exec.ovfind); emit(user,c->exec.alarm);
    emit(user,c->wait.time3); for(i=0;i<8;++i) emit(user,c->wait.interval[i]);
    for(i=0;i<9;++i) { emit(user,c->wait.task[i].location); emit(user,c->wait.task[i].bank); }
    emit(user,c->wait.ruptagn); emit(user,c->wait.alarm);
    for(i=0;i<3;++i) emit(user,c->record.failreg[i]);
    for(i=0;i<2;++i) emit(user,c->record.almcadr[i]);
    for(i=0;i<12;++i) emit(user,c->record.dsptab[i]);
    for(i=0;i<RS_WORD_COUNT;++i) emit(user,c->record.word[i]);
    emit(user,c->record.channel11); emit(user,c->record.channel12); emit(user,c->record.channel13); emit(user,c->record.channel14);
    emit(user,c->record.trap31a); emit(user,c->record.trap31b); emit(user,c->record.trap32);
    for(i=0;i<6;++i) { emit(user,c->phases.complement[i]); emit(user,c->phases.phase[i]);
        emit(user,c->phases.address[i].location); emit(user,c->phases.address[i].bank); emit(user,c->phases.priority[i]); }
}
#ifndef RESTART_INPUT_ONLY
static void digest_word(void *user,agc_word word) {
    uint32_t *hash=user;
    *hash=(*hash ^ (word & 255u))*UINT32_C(16777619);
    *hash=(*hash ^ (word >> 8))*UINT32_C(16777619);
}
uint32_t rs_digest(const restart_case *c,int status) {
    uint32_t hash=UINT32_C(2166136261);
    digest_word(&hash,(agc_word)status); rs_words(c,digest_word,&hash); return hash;
}
#endif
