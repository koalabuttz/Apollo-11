/* SPDX-License-Identifier: GPL-2.0-or-later
 * Original unchanged GENSHIFT and short shifts; input constructors only. */
#define main admission_reference_main
#include "reference_runner.c"
#undef main
#include "shift_cases.h"
static agc_t baseline;
static const unsigned addresses[41]={
    03746,03756,03644,03645,RegA,RegL,
    037,040,041,024,025,03763,03764,03765,070,071,072,03761,03762,
    01160,01161,01162,01254,01255,01256,
    0154,0155,0156,0157,0160,0161,0162,0163,
    03766,03767,03770,03771,03772,03773,0135,0121
};
int main(int argc,char **argv) {
    uint16_t n;
    if(argc!=2) return 2;
    memset(&machine,0,sizeof machine);
    if(agc_load_binfile(&machine,argv[1])!=0) return 2;
    baseline=machine; InhibitAlarms=1;
    for(n=0;n<SH_CASE_COUNT;++n) {
        shift_case c; uint16_t w[SH_WORD_COUNT]; unsigned i,cycles,pc=0;
        static const unsigned guards[7]={03643,03646,03760,03774,0153,01233,01236};
        unsigned base,push;
        sh_prepare(&c,n); sh_words(&c,w); machine=baseline;
        base=c.previous.previous.push.base; push=c.previous.previous.push.pushloc;
        for(i=0;i<41;++i) *memory(addresses[i])=(int16_t)w[i];
        set(0120,base); set(0166,push);
        for(i=0;i<P66_VAC_WORDS;++i) set(base+i,w[43+i]);
        set(01234,w[86]); set(01235,w[87]); set(0130,w[88]);
        for(i=0;i<7;++i) set(guards[i],012345u+i);
        set(base-1u,054321);set(base+P66_VAC_WORDS,023456);
        set(RegEB,03400); set(RegFB,0); set(RegBB,7); set(RegQ,01030);
        set(7,0); machine.AllowInterrupt=0;
        if(c.short_form) {
            unsigned opcode=c.operation<2u ? 024u+040u*(c.count-1u) : 004u+020u*(c.count-1u);
            if(c.operation&1u) opcode+=010u;
            set(RegA,opcode); set(RegZ,06077);
        } else {
            set(0116,020200u|((c.operation&1u)?0400u:0u)|c.count);
            set(RegA,0177); set(RegZ,02214);
        }
        for(cycles=0;cycles<5000;++cycles) {
            pc=get(RegZ);
            if(!machine.PendFlag&&!machine.ExtraDelay&&pc==06061) break;
            machine.ScalerCounter=0;
            if(agc_engine(&machine)!=0) return 2;
        }
        if(cycles==5000||get(RegEB)!=03400||get(0163)!=(c.operation<2u?0u:077776u)||get(0166)!=push||get(0120)!=base||get(7)!=0) {
            fprintf(stderr,"case %u boundary failure %05o cycles%u\n",n,pc,cycles); return 2;
        }
        for(i=0;i<7;++i) if(get(guards[i])!=012345u+i) return 2;
        if(get(base-1u)!=054321||get(base+P66_VAC_WORDS)!=023456||machine.AllowInterrupt) return 2;
        for(i=0;i<41;++i) w[i]=(uint16_t)((unsigned)*memory(addresses[i])&(i==4||i==5?0177777u:077777u));
        w[41]=(uint16_t)get(0120);w[42]=(uint16_t)get(0166);
        for(i=0;i<P66_VAC_WORDS;++i) w[43+i]=(uint16_t)get(base+i);
        w[86]=(uint16_t)get(01234);w[87]=(uint16_t)get(01235);w[88]=(uint16_t)get(0130);
        printf("%u",n);for(i=0;i<SH_WORD_COUNT;++i) printf(" %u",w[i]);puts("");
        if(n%SH_GROUP_SIZE==0) fprintf(stderr,"group%u op%u count%u short%u A%06o L%06o MPTEMP%05o\n",n/SH_GROUP_SIZE,c.operation,c.count,c.short_form,w[4],w[5],w[39]);
    }
    return 0;
}
