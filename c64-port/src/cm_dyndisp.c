/* SPDX-License-Identifier: MIT */
#include "cm_dyndisp.h"
#include "guidance_product.h"
#include "guidance_shift.h"
static const uint8_t positions[6]={0,1,3,4,5,6};
static void bank(uint16_t *w,agc_word fb,agc_word eb){w[3]=eb;w[4]=fb;w[6]=(agc_word)(fb|(eb>>8));}
static void normal_l(uint16_t *w){w[1]=agc_extend(agc_correct(w[1]));}
static void call(uint16_t *w,agc_word target,agc_word ret){w[w[0120]+052u]=ret;w[0117]=target;w[0164]=(agc_word)(02000u+(target&01777u));bank(w,(agc_word)(target&076000u),(agc_word)((w[0165]&7u)<<8));w[0165]=w[6];w[0115]=(agc_word)(w[6]&040000u);w[023]=0;w[020]=050006;w[0]=agc_extend(w[0115]);w[5]=06037;normal_l(w);}
static void next(uint16_t *w,agc_word loc){bank(w,(agc_word)(w[0165]&076000u),(agc_word)((w[0165]&7u)<<8));w[0]=0;w[0164]=loc;w[5]=06037;}
static void load(uint16_t *w,agc_word at){agc_word old=w[0162];uint8_t i;for(i=0;i<6u;++i)w[0154u+positions[i]]=w[at+i];w[1]=agc_extend(old);w[0]=agc_extend(077776);w[0163]=077776;}
static void arithmetic(uint16_t *w,uint8_t shift){p66_preparation p={0};p66_scale_scratch sc;p66_elapsed_state el={0};agc_dp factor={024000,0};uint8_t i;for(i=0;i<8u;++i)p.mpac[i]=w[0154u+i];p.rate.registers.a=w[0];p.rate.registers.l=w[1];sc.mptemp=w[0135];sc.ovfind=w[0121];el.buf=w[0130];if(shift)(void)p66_vector_shift_left(&p,&sc,&el,5,1);else(void)p66_vector_multiply(&p,&sc,&factor);for(i=0;i<8u;++i)w[0154u+i]=p.mpac[i];w[0]=p.rate.registers.a;w[1]=p.rate.registers.l;w[0135]=sc.mptemp;w[0121]=sc.ovfind;w[0130]=el.buf;}
uint8_t cm_dyndisp_stage(apollo_overlay_state *s,cm_native_io *io,uint8_t phase){static const agc_word pc[17]={03325,06037,06037,06030,06037,06030,06030,06037,06037,06030,06030,06037,06030,06030,06030,06037,06037};static const agc_word loc[17]={0,03326,02370,02371,02373,02374,02374,03431,03434,03434,03436,03440,03441,03442,03443,02377,03330};static const agc_word fbs[17]={050000,050000,050000,02000,050000,050000,0,044000,044000,044000,044000,044000,044000,044000,0,050000,050000};static const agc_word ebs[17]={03000,03000,03000,03400,03000,03400,03000,03000,03000,03000,01400,03000,03000,03000,03000,03000,03000};static const agc_word edops[17]={0,0,0,0176,0,0171,0,0,0,0,0153,0,016,0,0127,0,0};uint16_t *w;agc_word base,old;uint8_t i;if(s==0||io==0||phase>16u)return 0;w=s->words;base=w[0120];if(w[5]!=pc[phase]||(phase&&w[0164]!=loc[phase])||w[7]!=0||w[067]!=0||base<0401u||base>0661u||(base-0401u)%054u||io->interrupt_enabled>1u||io->superbank>060u||(io->superbank&017u)||w[0163]!=077776u||w[4]!=fbs[phase]||w[3]!=ebs[phase]||w[6]!=(agc_word)(fbs[phase]|(ebs[phase]>>8))||(phase&&w[023]!=edops[phase])||(phase&&w[0165]!=(phase<7u?050006u:phase<15u?044006u:050006u)))return 0;
 switch(phase){case 0:if(w[4]!=050000u||w[3]!=03000u||w[6]!=050006u)return 0;w[2]=agc_extend(w[0164]);w[0164]=03326;w[0165]=w[6];w[0115]=040000;w[023]=0;w[0]=agc_extend(040000);w[5]=06037;normal_l(w);io->interrupt_enabled=1;break;
 case 1:call(w,050370,051330);break;
 case 2:old=w[base+052u];w[03730]=old;w[0130]=base;w[0117]=01730;w[0164]=02371;w[023]=0176;w[020]=034003;w[2]=3;w[0]=agc_extend(old);bank(w,02000,03400);w[5]=06030;normal_l(w);break;
 case 3:w[0116]=base;w[0166]=base;w[023]=0;w[020]=020037;next(w,02373);break;
 case 4:load(w,03720);w[0116]=01720;w[0164]=02374;w[023]=0171;w[020]=020000;bank(w,050000,03400);w[5]=06030;break;
 case 5:if(w[0166]!=base)return 0;old=w[0166];w[0]=agc_extend(w[old+4u]);w[1]=agc_extend(w[old+5u]);for(i=0;i<6u;++i)w[old+i]=w[0154u+positions[i]];w[0166]=(agc_word)(old+6u);w[023]=0;w[020]=040007;bank(w,0,03000);w[5]=06030;break;
 case 6:call(w,045431,050377);break;
 case 7:old=w[base+052u];w[03657]=old;w[0130]=base;call(w,047477,045434);w[2]=3;break;
 case 8:if(w[0166]!=base+6u)return 0;load(w,base);w[0166]=base;w[0116]=base;w[020]=020000;w[023]=0;w[5]=06030;break;
 case 9:next(w,03435);w[0164]=03436;w[0116]=01735;w[020]=020013;w[023]=0153;bank(w,044000,01400);w[0]=01735;w[5]=07303;break;
 case 10:call(w,047644,045440);break;
 case 11:w[0164]=03441;w[0116]=03002;w[020]=020013;w[023]=016;w[0]=agc_extend(045002);w[5]=07303;break;
 case 12:arithmetic(w,0);w[2]=3;w[0116]=03445;w[0164]=03442;w[020]=020003;w[023]=0;w[5]=06030;break;
 case 13:arithmetic(w,1);bank(w,0,03000);w[0164]=03443;w[020]=010002;w[023]=0127;w[2]=3;w[5]=06030;break;
 case 14:if(w[03657]!=050377u)return 0;old=w[03657];w[0117]=old;w[020]=030005;w[023]=0;w[0164]=(agc_word)(02000u+(old&01777u));bank(w,(agc_word)(old&076000u),03000);w[0165]=w[6];w[0]=agc_extend(040000);w[5]=06037;break;
 case 15:if(w[03730]!=051330u)return 0;old=w[03664+5u];for(i=0;i<6u;++i)w[03664u+i]=w[0154u+positions[i]];w[1]=agc_extend(old);w[0116]=01664;call(w,w[03730],050401);w[020]=040006;break;
 case 16:bank(w,050000,03000);w[0]=agc_extend(050006);w[020]=0;w[023]=0;w[0164]=03330;w[2]=06716;w[5]=03342;break;
 default:return 0;}return 1;}
