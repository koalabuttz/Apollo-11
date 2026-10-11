/* SPDX-License-Identifier: MIT
 * Native C64 DSKY device adapter. Relay wiring and digit codes follow the
 * public-domain Luminary099 PINBALL comments, lines392..449. */
#include "dsky_display.h"
void dsky_display_reset(dsky_display_state *s){uint8_t i;for(i=0;i<12;++i)s->relay[i]=0;s->valid=0;s->channel11=0;s->flash_phase=0;}
void dsky_display_advance(dsky_display_state *s,uint16_t dt){s->flash_phase=(uint8_t)((s->flash_phase+(dt&127u))&127u);}
uint8_t dsky_display_flash_visible(const dsky_display_state *s){return (uint8_t)(s->flash_phase>=32);}
void dsky_display_output(dsky_display_state *s,uint8_t channel,uint16_t word){
 uint8_t selector=(uint8_t)((word>>11)&15u);
 if(channel==011){s->channel11=(uint16_t)(word&077777u);return;}
 if(channel==010 && selector!=0 && selector<=12){s->relay[selector-1u]=(uint16_t)(word&03777u);s->valid=(uint16_t)(s->valid|(uint16_t)(1u<<(selector-1u)));}
}
static void label(volatile uint8_t *screen,uint16_t position,const char *text){while(*text){screen[position++]=(uint8_t)((uint8_t)*text++&077u);}}
static uint8_t digit(uint8_t code){
 switch(code){case 0:return 32;case 025:return 48;case 003:return 49;case 031:return 50;case 033:return 51;case 017:return 52;case 036:return 53;case 034:return 54;case 023:return 55;case 035:return 56;case 037:return 57;default:return 63;}
}
static void lamp(volatile uint8_t *screen,volatile uint8_t *color,uint8_t row,const char *name,uint8_t on){uint16_t p=(uint16_t)(row*40u+2u);label(screen,p,name);while(*name++){color[p++]=on!=0 ? 7 : 11;}}
void dsky_display_render(const dsky_display_state *s,volatile uint8_t *screen,volatile uint8_t *color,uint8_t visible){
 static const uint16_t places[22]={665,664,663,662,661,545,544,543,542,541,425,424,423,422,421,0,270,269,257,256,144,143};
 static const uint16_t signs[3]={418,538,658};
 static const uint8_t positive[3]={6,4,1},negative[3]={5,3,0};
 uint16_t p,status=s->relay[11];uint8_t i;
 for(p=0;p<1000;++p){screen[p]=32;color[p]=5;}
 label(screen,40+14,"APOLLO GUIDANCE COMPUTER");label(screen,102,"MODE");label(screen,215,"VERB");label(screen,228,"NOUN");label(screen,375,"R1");label(screen,495,"R2");label(screen,615,"R3");
 for(i=0;i<22;++i){uint8_t selector=(uint8_t)(i/2u),code;if(i==15 || (s->valid&(uint16_t)(1u<<selector))==0)continue;
  code=(uint8_t)((s->relay[selector]>>(i%2u*5u))&037u);screen[places[i]]=digit(code);
 }
 for(i=0;i<3;++i){uint8_t plus=(uint8_t)((s->relay[positive[i]]&02000u)!=0),minus=(uint8_t)((s->relay[negative[i]]&02000u)!=0);screen[signs[i]]=plus!=0 ? 43 : minus!=0 ? 45 : 32;}
 if((s->channel11&040u)!=0 && visible==0){screen[256]=screen[257]=screen[269]=screen[270]=32;}
 lamp(screen,color,3,"UPLINK ACTY",(uint8_t)((s->channel11&4u)!=0));lamp(screen,color,5,"NO ATT",(uint8_t)((status&010u)!=0));lamp(screen,color,7,"TEMP",(uint8_t)((s->channel11&010u)!=0));lamp(screen,color,9,"KEY REL",(uint8_t)((s->channel11&020u)!=0 && visible!=0));lamp(screen,color,11,"OPR ERR",(uint8_t)((s->channel11&0100u)!=0 && visible!=0));lamp(screen,color,13,"GIMBAL LOCK",(uint8_t)((status&040u)!=0));lamp(screen,color,15,"PROG",(uint8_t)((status&0400u)!=0));lamp(screen,color,17,"RESTART",(uint8_t)((s->channel11&0200u)!=0));lamp(screen,color,19,"TRACKER",(uint8_t)((status&0200u)!=0));lamp(screen,color,21,"ALT",(uint8_t)((status&020u)!=0));lamp(screen,color,22,"VEL",(uint8_t)((status&4u)!=0));lamp(screen,color,23,"STBY",(uint8_t)((s->channel11&0400u)!=0));
 lamp(screen,color,24,"COMP ACTY",(uint8_t)((s->channel11&2u)!=0));
}
