/* SPDX-License-Identifier: MIT */
#include "cm_smcdure.h"
#include "guidance_product.h"
#include "guidance_dp_divide.h"
#include "guidance_sum.h"
#include "guidance_shift.h"
static const agc_word loc[17]={03743,03744,03746,03747,03750,03752,03753,03755,03755,03757,03761,03763,03764,03766,03766,03767,03770};
static const agc_word edop[17]={0172,0,0172,0,0,0,0,025,0,0,0,052,0,0162,0,0,0};
static const agc_word cyr[17]={020006,020036,020012,020036,020033,020021,020021,020036,024000,020033,020006,020036,020012,020036,020034,024000,024000};
static const agc_word addr[17]={01611,01744,01615,01736,0,01746,01603,01740,0,01613,01611,01736,01615,01744,0,0,01607};
static const uint8_t bank[17]={6,1,6,1,8,1,6,1,8,6,6,1,6,1,8,8,6};
static agc_word expected_push(agc_word base,unsigned phase){return (agc_word)(base+((phase==3u||phase==4u||phase==13u||phase==14u)?2u:0u));}
static uint8_t valid(const apollo_overlay_state *m,unsigned phase){agc_word base=m->words[0120],fb=m->words[4];if(phase>17u||m->words[7]!=0||m->words[0163]>077777u||m->words[3]>03400u||(m->words[3]&0377u)!=0||m->words[6]!=(fb|(m->words[3]>>8))||(m->words[0165]&077770u)!=046000u||m->words[0115]!=040000u||base<0401u||base>0661u||(base-0401u)%054u!=0||m->words[0166]!=expected_push(base,phase))return 0;
if(!phase)return (uint8_t)(m->words[5]==06037u&&m->words[0164]==03742u&&m->words[023]==0&&fb==046000u&&m->words[6]==m->words[0165]);
return (uint8_t)(m->words[5]==06030u&&m->words[0164]==loc[phase-1u]&&m->words[023]==edop[phase-1u]&&m->words[0163]==0&&fb==((phase==6u||phase==9u||phase==16u)?0u:046000u));}
static void dp_load(apollo_overlay_state *m,unsigned address,agc_register low){m->words[0154]=m->words[address];m->words[0155]=m->words[address+1u];m->words[0156]=0;m->words[0163]=0;m->words[0]=0;m->words[1]=low;}
static void push_load(apollo_overlay_state *m){agc_word base=m->words[0166],oldlow=m->words[base+1u];m->words[base]=m->words[0154];m->words[base+1u]=m->words[0155];dp_load(m,03215,agc_extend(oldlow));m->words[0166]=(agc_word)(base+2u);}
static void store(apollo_overlay_state *m,unsigned address){agc_word oldlow=m->words[address+1u];m->words[address]=m->words[0154];m->words[address+1u]=m->words[0155];m->words[0]=0;m->words[1]=agc_extend(oldlow);m->words[2]=3;}
enum cm_smcdure_status cm_smcdure(apollo_overlay_state *m,enum cm_smcdure_phase selected){p66_preparation s={0};p66_scale_scratch sc;p66_elapsed_state el={0};p66_divide_scratch d;agc_dp operand;uint8_t i;unsigned phase=(unsigned)selected;agc_word base=m->words[0120],eb;enum p66_divide_result division;
if(!valid(m,phase))return CM_SMCDURE_OUT_OF_SCOPE;
if(phase==17u){agc_word ret=m->words[base+052u],fb;if(ret>077777u)return CM_SMCDURE_OUT_OF_SCOPE;fb=(agc_word)(ret&076000u);m->words[3]=(agc_word)((m->words[0165]&7u)<<8);m->words[4]=fb;m->words[6]=(agc_word)(fb|(m->words[3]>>8));m->words[0165]=m->words[6];m->words[0117]=ret;m->words[0164]=(agc_word)(02000u+(ret&01777u));m->words[020]=7;m->words[023]=0;m->words[0115]=(agc_word)(ret&040000u);m->words[0]=agc_extend(m->words[0115]);m->words[5]=06037;return CM_SMCDURE_OK;}
if(phase==0u)dp_load(m,03211,agc_extend(m->words[0155]));
else if(phase==2u||phase==12u)push_load(m);
else if(phase==6u)store(m,03203);
else if(phase==10u){store(m,03205);dp_load(m,03211,agc_extend(m->words[0155]));}
else if(phase==16u)store(m,03207);
else{
for(i=0;i<8u;++i){s.mpac[i]=m->words[0154u+i];}s.rate.registers.a=m->words[0];s.rate.registers.l=m->words[1];sc.mptemp=m->words[0135];sc.ovfind=m->words[0121];el.buf=m->words[0130];d.buf_low=m->words[0131];d.dvsign=m->words[0136];d.dvnormct=m->words[0137];d.maxdvsw=m->words[0140];
if(phase==1u||phase==3u||phase==7u||phase==11u||phase==13u){unsigned address=phase==1u||phase==13u?0744u:phase==7u?0740u:0736u;operand.high=m->words[address];operand.low=m->words[address+1u];(void)p66_scalar_product(&s,&sc,&operand);m->words[2]=3;}
else if(phase==4u||phase==9u||phase==14u){unsigned address=phase==9u?03213u:base;operand.high=m->words[address];operand.low=m->words[address+1u];if(phase==14u)(void)p66_scalar_add(&s,&sc,&el,&operand);else(void)p66_scalar_reverse_subtract(&s,&sc,&el,&operand);if(phase!=9u)m->words[0166]=base;}
else if(phase==5u){operand.high=m->words[0746];operand.low=m->words[0747];division=p66_dp_divide(&s,&sc,&el,&d,operand,0);{int high=agc_to_int(operand.high),low=agc_to_int(operand.low);uint8_t buffer_zero=(uint8_t)(!high||(high==1&&low<0)||(high==-1&&low>0));m->words[2]=division==P66_DIVIDE_OVERFLOW?02413u:division==P66_DIVIDE_ZERO&&buffer_zero?2u:3u;}}
else{(void)p66_scalar_shift_left(&s,&sc,1,1);m->words[021]=0;}
for(i=0;i<8u;++i){m->words[0154u+i]=s.mpac[i];}m->words[0]=s.rate.registers.a;m->words[1]=s.rate.registers.l;m->words[0135]=sc.mptemp;m->words[0121]=sc.ovfind;m->words[0130]=el.buf;m->words[0131]=d.buf_low;m->words[0136]=d.dvsign;m->words[0137]=d.dvnormct;m->words[0140]=d.maxdvsw;
}
eb=bank[phase]==8u?(agc_word)((m->words[0165]&7u)<<8):(agc_word)((unsigned)bank[phase]<<8);m->words[3]=eb;m->words[4]=(phase==5u||phase==8u||phase==15u)?0u:046000u;m->words[6]=(agc_word)(m->words[4]|(eb>>8));m->words[5]=06030;m->words[0164]=loc[phase];m->words[023]=edop[phase];m->words[020]=cyr[phase];if(addr[phase])m->words[0116]=addr[phase];else if(phase==4u||phase==14u)m->words[0116]=base;return CM_SMCDURE_OK;}
