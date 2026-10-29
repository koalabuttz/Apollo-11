/* SPDX-License-Identifier: MIT */
#include "cm_rectify.h"
#include "guidance_shift.h"
#include "guidance_vector.h"
static uint8_t legal_base(uint16_t base){return (uint8_t)(base==0401||base==0455||base==0531||base==0605||base==0661);}
static void load_vector(apollo_overlay_state *m,uint16_t address){static const uint8_t at[6]={0,1,3,4,5,6};unsigned i;uint16_t old=m->words[0162];for(i=0;i<6u;++i)m->words[0154+at[i]]=address?m->words[address+i]:0;m->words[0163]=077776;m->words[0]=0177776;m->words[1]=agc_extend(old);}
static void store(apollo_overlay_state *m,uint16_t address){static const uint8_t at[6]={0,1,3,4,5,6};unsigned i;uint8_t count=m->words[0163]==077776?6u:2u;uint16_t hi=m->words[address+count-2u],lo=m->words[address+count-1u];for(i=0;i<count;++i)m->words[address+i]=m->words[0154+at[i]];m->words[0]=count==2u?0:agc_extend(hi);m->words[1]=agc_extend(lo);}
static void shifted_add(apollo_overlay_state *m,uint8_t count,uint16_t address){p66_preparation p={0};p66_scale_scratch sc;p66_elapsed_state e;agc_word operand[6];unsigned i;for(i=0;i<8u;++i)p.mpac[i]=m->words[0154+i];p.rate.registers.a=m->words[0];p.rate.registers.l=m->words[1];sc.mptemp=m->words[0135];sc.ovfind=m->words[0121];e.piptime.high=e.piptime.low=0;e.buf=(agc_word)(m->words[0120]+1u);for(i=0;i<6u;++i)operand[i]=m->words[address+i];(void)p66_vector_shift_right(&p,&sc,count,0);(void)p66_vector_add(&p,&sc,&e,operand);for(i=0;i<8u;++i)m->words[0154+i]=p.mpac[i];m->words[0]=p.rate.registers.a;m->words[1]=p.rate.registers.l;m->words[0135]=sc.mptemp;m->words[0121]=sc.ovfind;m->words[0130]=e.buf;}
enum cm_rectify_status cm_rectify_phase(apollo_overlay_state *m,uint8_t phase){static const uint16_t loc[11]={03423,03426,03427,03431,03434,03435,03437,03440,03442,03443,03444};uint16_t base=m->words[0120],body=m->words[02150],addr=0,fb=022000,return_word;uint8_t expected_mode=(uint8_t)(phase>=9u?0u:1u);
 if(phase>=12u||(m->words[0165]&076000u)!=022000u||!legal_base(base)||m->words[023]!=0)return CM_RECTIFY_OUT_OF_SCOPE;
 if(phase==1u||phase==4u)body=m->words[base+047];
 if((phase==0u||phase==1u||phase==4u)&&body!=0&&body!=2)return CM_RECTIFY_OUT_OF_SCOPE;
 if(phase&&phase!=11u&&((expected_mode&&m->words[0163]!=077776)||(!expected_mode&&m->words[0163]!=0)))return CM_RECTIFY_OUT_OF_SCOPE;
 return_word=m->words[base+052];if(phase==11u&&(return_word<04000u||return_word>077777u))return CM_RECTIFY_OUT_OF_SCOPE;
 switch(phase){
 case 0:m->words[base+047]=body;m->words[0130]=(uint16_t)(base+1u);m->words[0117]=01550;load_vector(m,01520);addr=01520;m->words[020]=020000;break;
 case 1:shifted_add(m,(uint8_t)(7u+body),01534);addr=031534;m->words[020]=020024;break;
 case 2:store(m,01502);addr=01502;break;
 case 3:store(m,01534);load_vector(m,01526);addr=01526;m->words[020]=020000;break;
 case 4:shifted_add(m,(uint8_t)(4u+body),01542);addr=031542;m->words[020]=020024;break;
 case 5:store(m,01510);addr=01510;break;
 case 6:store(m,01542);load_vector(m,0);addr=03565;fb=010000;m->words[020]=020000;break;
 case 7:store(m,01520);addr=01520;break;
 case 8:store(m,01526);m->words[0]=0;m->words[1]=agc_extend(m->words[0155]);m->words[0154]=m->words[0155]=m->words[0156]=m->words[0163]=0;addr=03565;fb=010000;m->words[020]=020006;break;
 case 9:store(m,01550);addr=01550;break;
 case 10:store(m,01552);addr=01552;break;
 default:m->words[0117]=return_word;m->words[020]=7;fb=(uint16_t)(return_word&076000u);m->words[0165]=(uint16_t)(fb|(m->words[0165]&7u));m->words[0115]=(uint16_t)(fb&040000u);m->words[0]=agc_extend(m->words[0115]);m->words[0164]=(uint16_t)(02000u+(return_word&01777u));break;
 }
 m->words[3]=phase==11u?(uint16_t)((m->words[0165]&7u)<<8):01400;m->words[4]=fb;m->words[6]=(uint16_t)(fb|(m->words[3]>>8));m->words[2]=3;m->words[5]=phase==11u?06037:06030;m->words[023]=0;if(phase<11u){m->words[0164]=loc[phase];m->words[0116]=addr;}
 return CM_RECTIFY_OK;
}
