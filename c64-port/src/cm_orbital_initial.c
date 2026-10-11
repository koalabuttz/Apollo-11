/* SPDX-License-Identifier: MIT */
#include "cm_orbital_initial.h"
static uint8_t legal_base(uint16_t b){return (uint8_t)(b==0401||b==0455||b==0531||b==0605||b==0661);}
enum cm_orbital_initial_status cm_orbital_initial_phase(apollo_overlay_state *m,agc_word *irq,uint8_t phase){
 static const uint8_t at[6]={0,1,3,4,5,6};unsigned i;uint16_t fb=022000,eb=02000,old;
 if(phase>=4u||!legal_base(m->words[0120])||(m->words[0165]&076000u)!=022000u||m->words[023]!=0||*irq>1u)return CM_ORBITAL_INITIAL_OUT_OF_SCOPE;
 if(phase&&m->words[0163]!=(phase==1u?077776u:0u))return CM_ORBITAL_INITIAL_OUT_OF_SCOPE;
 if(phase==0u){old=m->words[0162];for(i=0;i<6u;++i)m->words[0154+at[i]]=m->words[01111+i];m->words[0163]=077776;m->words[01500]=0;m->words[0]=0;m->words[1]=agc_extend(old);m->words[0116]=01500;m->words[0164]=03310;m->words[020]=020011;eb=01400;}
 else if(phase==1u){for(i=0;i<6u;++i)m->words[02151+i]=m->words[0154+at[i]];old=m->words[0155];m->words[0154]=m->words[0155]=m->words[0156]=m->words[0163]=0;m->words[0]=0;m->words[1]=agc_extend(old);m->words[0116]=03565;m->words[0164]=03312;m->words[020]=020006;m->words[2]=3;fb=010000;}
 else if(phase==2u){old=m->words[02212];m->words[02211]=m->words[0154];m->words[02212]=m->words[0155];m->words[0]=0;m->words[1]=agc_extend(old);m->words[0116]=01611;m->words[0164]=03313;m->words[2]=3;}
 else{uint8_t on=(uint8_t)((m->words[074]&020000u)!=0);m->words[0130]=0;m->words[0131]=020000;m->words[2]=m->words[074];m->words[0]=0;m->words[1]=010000;m->words[020]=on?020016:030005;m->words[0117]=on?023665:022512;m->words[0164]=on?03665:02512;m->words[0115]=0;eb=(uint16_t)((m->words[0165]&7u)<<8);*irq=1;}
 m->words[3]=eb;m->words[4]=fb;m->words[6]=(uint16_t)(fb|(eb>>8));m->words[5]=phase==3u?06037:06030;m->words[023]=0;return CM_ORBITAL_INITIAL_OK;
}
