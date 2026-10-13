/* SPDX-License-Identifier: MIT
 * Luminary099 LANDING_ANALOG_DISPLAYS.agc, all of it (bank 21, 02154-03076),
 * as routines of the control-transfer ABI. Each source instruction is one
 * agc_image_ops call (yaAGC-qualified register effects) or, for the channel
 * orders, a helper here with yaAGC's effect, named with its erasable address,
 * channel or the source's fixed constant. TS overflow skips and CCS exits
 * land where the original's do, including the exits that enter a word past
 * its EXTEND or the next routine. */
#include "lm_landing_displays.h"
#include "agc_image_ops.h"
#include "agc_image_w.h"
/* Branch coverage hook for the host test (empty in flight builds). */
#ifdef LM_LAD_COUNT_PATHS
extern unsigned lm_lad_paths[];
#define LM_LAD_PATH(n) ((void)++lm_lad_paths[n])
#else
#define LM_LAD_PATH(n) ((void)0)
#endif
#define P21(z) (uint16_t)(021u*1024u+((z)-02000u))
/* Erasable: central registers, counters, unswitched, EBANK7 (EBANK=
 * UNIT/R/) and, for GET22/32, EBANK6 (EBANK= M22). */
#define LREG 1u
#define QREG 2u
#define EBANK 3u
#define ZEROREG 7u
#define TIME1 025u
#define PIPAX 037u
#define PIPAY 040u
#define PIPAZ 041u
#define CDUTCMD 053u
#define ALTM 060u
#define ITEMP1 061u
#define ITEMP2 062u
#define ITEMP3 063u
#define ITEMP4 064u
#define ITEMP5 065u
#define ITEMP6 066u
#define RUPTREG1 070u
#define RUPTREG3 072u
#define FLAGWRD0 074u
#define FLAGWRD1 075u
#define FLAGWRD7 0103u
#define TBASE2 01055u
#define PIPATMPX 01160u
#define PIPATMPY 01161u
#define PIPATMPZ 01162u
#define PIPTIME1 01235u    /* PIPTIME +1 */
#define GDT2 01236u        /* GDT/2 */
#define IMODES33 01303u
#define M22 01417u          /* EBANK6 */
#define M32 01420u          /* EBANK6 */
#define V 01526u
#define LATVMETR 01676u
#define LATVEL 01700u
#define TRAKLATV 01702u
#define TRAKFWDV 01703u
#define FORVMETR 01677u
#define VHY 01704u
#define VHZ 01705u
#define VVECT 01706u
#define ALTRATE 01711u
#define ALTSAVE 01712u
#define LADQSAVE 01714u     /* = PIPCTR1 */
#define PIPCTR1 01714u
#define DT 01715u
#define DALTRATE 01716u
#define UHYP 01717u
#define UHZP 01725u
#define DELVS 01733u
#define ALTBITS 01741u
#define RUNIT 01743u
/* The other words of the vectors and double words above. */
#define ALTSAVE1 01713u
#define DELVS2 01735u
#define DELVS4 01737u
#define LATVEL1 01701u
#define RUNIT1 01744u
#define RUNIT2 01745u
#define UHYP2 01721u
#define UHYP4 01723u
#define UHZP2 01727u
#define UHZP4 01731u
#define VVECT1 01707u
#define VVECT2 01710u
#define GDT2_2 01240u
#define GDT2_4 01242u
#define V2 01530u
#define V4 01532u
/* Fixed constants: bank 21 outside this file, and fixed-fixed. */
#define ARCONV 024402u
#define ARTOA 01551u
#define ARTOA2 021357u
#define VELCONV 022316u
#define KPIP15 01507u       /* KPIP1(5) */
#define MAXVBITS 0547u
#define POSMAX 037777u
#define BIT15 040000u
#define HALF 020000u
#define DIDFLBIT 020000u
#define SWANDBIT 02000u
#define BIT10 01000u
#define BIT8 0200u
#define BIT7 0100u
#define BIT6 040u
#define BIT4 010u
#define BIT3 04u
#define BIT2 02u
#define R10FLBIT 02u
#define ONE 01u
#define NEG0 077777u
#define ZERO 0u
#define ONESEC 0144u        /* 1SEC */
#define EBANK6 03000u
#define EBANK7 03400u
#define PRIO6 06000u        /* BITSET */
#define BITS87 0300u        /* BITS8/7, 21,3076 */

static int entry(const agc_image *s,const void *io){return s&&io&&(W[4]&076000u)==LM_LAD_FB21&&W[3]==LM_LAD_EB;}
static agc_word oc(agc_register a){return (agc_word)((a&037777u)|((a>>1)&040000u));}
/* Channel orders, as yaAGC's: RAND, WOR, WAND. */
static void rand(agc_image *s,agc_word chan){W[0]=agc_extend((agc_word)(oc(W[0])&chan));}
static void wor(agc_image *s,agc_word *chan){agc_word v=(agc_word)((oc(W[0])|*chan)&077777u);*chan=v;W[0]=agc_extend(v);}
static void wand(agc_image *s,agc_word *chan){agc_word v=(agc_word)(oc(W[0])&*chan);*chan=v;W[0]=agc_extend(v);}
/* A TC at ret - 1 to z, which is outside this file; TCF keeps Q. */
static enum exec_status leave(agc_image *s,agc_word ret,agc_word z){ai_tc(s,ret);W[5]=z;return EXEC_OK;}
static enum exec_status jump(agc_image *s,agc_word z){W[5]=z;return EXEC_OK;}
/* TC LADQSAVE at ret - 1: the word in LADQSAVE runs. DISINDAT stores the
 * return addresses ALTROUT +1 and ALTOUT +1 there (TC words, Q = LADQSAVE
 * +1); any other word runs at LADQSAVE, outside the file. */
static int tc_ladqsave(agc_image *s,agc_word ret){agc_word w=ai_word(s,LADQSAVE);
 ai_tc(s,ret);
 if(w==LM_LAD_ALTROUT+1u||w==LM_LAD_ALTOUT+1u){ai_tc(s,LADQSAVE+1u);return w==LM_LAD_ALTROUT+1u?1:2;}
 W[5]=LADQSAVE;return 0;
}

enum {AT_LANDISP,AT_INTLZE,AT_TWIDRET,AT_VARRET};
static enum exec_status flow(agc_image *s,lm_flight_io *io,unsigned at){uint8_t c;unsigned i;
 switch(at){
 case AT_INTLZE:goto intlze;
 case AT_TWIDRET:goto twidret;
 case AT_VARRET:goto varret;
 default:break;
 }
 /* LANDISP: LXCH PIPCTR1; CS TIME1; DXCH TBASE2; CS FLAGWRD7; MASK
  * SWANDBIT; CCS A; TCF DISPRSET; CA IMODES33; MASK BIT7; CCS A; TCF ALTOUT. */
 ai_lxch(s,PIPCTR1);ai_cs(s,TIME1);ai_dxch(s,TBASE2);ai_cs(s,FLAGWRD7);ai_maskf(s,SWANDBIT);
 c=ai_ccs(s,0u);
 if(c==0u){LM_LAD_PATH(0);goto disprset;}
 if(c==2u){LM_LAD_PATH(1);goto w2164;}           /* - : MASK BIT7 */
 if(c==3u){LM_LAD_PATH(2);goto w2165;}           /* -0: CCS A */
 LM_LAD_PATH(3);
 ai_ca(s,IMODES33);
w2164:
 ai_maskf(s,BIT7);
w2165:
 c=ai_ccs(s,0u);
 if(c==0u){LM_LAD_PATH(4);goto altout;}
 if(c==2u){LM_LAD_PATH(5);goto altrout1;}        /* - : ALTROUT +1 */
 if(c==3u){LM_LAD_PATH(6);goto altrout2;}        /* -0: ALTROUT +2 */
 LM_LAD_PATH(7);
 /* ALTROUT: TC DISINDAT. */
 ai_tc(s,LM_LAD_ALTROUT+1u);goto disindat;
altrout1: /* CS IMODES33; MASK BIT7; ADS IMODES33; CAF BIT2; EXTEND; WOR CHAN14. */
 ai_cs(s,IMODES33);
altrout2:
 ai_maskf(s,BIT7);ai_ads(s,IMODES33);ai_caf(s,BIT2);wor(s,&io->chan14);
 /* ARCOMP: ALTRATE = RUNIT.VVECT, in bit units, plus DALTRATE x DT. */
 ai_ca(s,RUNIT);ai_mp(s,VVECT);ai_xch(s,RUPTREG1);ai_ca(s,RUNIT1);ai_mp(s,VVECT1);ai_ads(s,RUPTREG1);
 ai_ca(s,RUNIT2);ai_mp(s,VVECT2);ai_ads(s,RUPTREG1);
 ai_caf(s,ARCONV);ai_mp(s,RUPTREG1);ai_ddoubl(s);ai_ddoubl(s);ai_xch(s,RUPTREG1);
 ai_ca(s,DALTRATE);ai_mp(s,DT);ai_ad(s,RUPTREG1);
 if(ai_ts(s,ALTRATE)){LM_LAD_PATH(8);}           /* TS ALTRATE skips CS ALTRATE */
 else{LM_LAD_PATH(9);ai_cs(s,ALTRATE);}
 if(!ai_bzmf(s)){LM_LAD_PATH(10);goto dataout;}  /* EXTEND; BZMF +2; TCF DATAOUT */
 LM_LAD_PATH(11);
 ai_ca(s,ALTRATE);ai_adf(s,BIT15);
dataout: /* TS ALTM; CAF BIT3; EXTEND; WOR CHAN14; TCF TASKOVER. */
 if(ai_ts(s,ALTM))LM_LAD_PATH(12);                /* skips CAF BIT3 */
 else{LM_LAD_PATH(13);ai_caf(s,BIT3);}
 wor(s,&io->chan14);
 return jump(s,LM_LAD_TASKOVER);

altout: /* TC DISINDAT. */
 ai_tc(s,LM_LAD_ALTOUT+1u);goto disindat;
altout1: /* CS BIT7; MASK IMODES33; TS IMODES33; CS BIT2; EXTEND; WAND CHAN14. */
 ai_csf(s,BIT7);ai_mask(s,IMODES33);
 if(ai_ts(s,IMODES33))LM_LAD_PATH(14);            /* skips CS BIT2 */
 else{LM_LAD_PATH(15);ai_csf(s,BIT2);}
 wand(s,&io->chan14);
 /* CCS ALTBITS; TCF +4; TCF +3; TCF OLDDATA; TS ALTBITS. */
 c=ai_ccs(s,ALTBITS);
 if(c==2u){LM_LAD_PATH(16);goto olddata;}
 if(c==3u){LM_LAD_PATH(17);                    /* -0: TS ALTBITS of +0 */
  if(ai_ts(s,ALTBITS)){LM_LAD_PATH(18);goto w2255;}}  /* (skips CS ONE) */
 else LM_LAD_PATH(19u+c);                        /* 19: +, 20: +0 */
 /* CS ONE; DXCH ALTBITS; DXCH ALTSAVE; CA BIT10; XCH Q; LXCH 7; CA DT;
  * EXTEND; DV Q; EXTEND; MP ARTOA2; TCF OLDDATA +1. */
 ai_csf(s,ONE);
w2255:
 ai_dxch(s,ALTBITS);ai_dxch(s,ALTSAVE);ai_caf(s,BIT10);ai_xch(s,QREG);ai_lxch(s,ZEROREG);
 ai_ca(s,DT);ai_dv(s,QREG);ai_mpf(s,ARTOA2);
 goto olddata1;
zdata2: /* DXCH ALTSAVE; TCF NEWDATA. */
 ai_dxch(s,ALTSAVE);goto newdata;
olddata: /* CA ARTOA; EXTEND; MP ALTRATE; DDOUBL; AD ALTSAVE +1; TS ALTSAVE +1;
  * CAF ZERO; ADS ALTSAVE; CAF POSMAX; AD ONE; AD ALTSAVE +1; TS ALTSAVE +1;
  * CAF ZERO; AD POSMAX; AD ALTSAVE; TS ALTSAVE; TCF ZERODATA. */
 ai_caf(s,ARTOA);
olddata1:
 ai_mp(s,ALTRATE);ai_ddoubl(s);ai_ad(s,ALTSAVE1);
 if(ai_ts(s,ALTSAVE1))LM_LAD_PATH(21);          /* carry into ALTSAVE */
 else{LM_LAD_PATH(22);ai_caf(s,ZERO);}
 ai_ads(s,ALTSAVE);ai_caf(s,POSMAX);ai_adf(s,ONE);ai_ad(s,ALTSAVE1);
 if(ai_ts(s,ALTSAVE1))LM_LAD_PATH(23);
 else{LM_LAD_PATH(24);ai_caf(s,ZERO);}
 ai_adf(s,POSMAX);ai_ad(s,ALTSAVE);
 if(ai_ts(s,ALTSAVE)){LM_LAD_PATH(25);goto newdata;}
 LM_LAD_PATH(26);goto zerodata;
newdata: /* CCS ALTSAVE +1; TCF +4; TCF +3; CAF ZERO; TS ALTSAVE +1;
  * CCS ALTSAVE; CAF BIT15; AD ALTSAVE +1; TCF DATAOUT. */
 c=ai_ccs(s,ALTSAVE1);
 if(c>=2u){LM_LAD_PATH(27u+c-2u);                 /* 27: -, 28: -0 */
  if(c==2u)ai_caf(s,ZERO);
  if(ai_ts(s,ALTSAVE1))LM_LAD_PATH(29);}
 else LM_LAD_PATH(30u+c);                        /* 30: +, 31: +0 */
 c=ai_ccs(s,ALTSAVE);
 if(c==2u){LM_LAD_PATH(34);goto dataout;}        /* - : TCF DATAOUT */
 if(c==3u){LM_LAD_PATH(35);goto disindat;}       /* -0: DISINDAT's EXTEND */
 if(c==0u){LM_LAD_PATH(32);ai_caf(s,BIT15);}
 else LM_LAD_PATH(33);
 ai_ad(s,ALTSAVE1);goto dataout;

disindat: /* EXTEND; QXCH LADQSAVE; CAF BIT6; EXTEND; RAND CHAN30; CCS A;
  * TCF DISPRSET; CS FLAGWRD1; MASK DIDFLBIT; EXTEND; BZF SPEEDRUN. */
 ai_qxch(s,LADQSAVE);ai_caf(s,BIT6);rand(s,io->chan30);
 c=ai_ccs(s,0u);
 if(c==0u){LM_LAD_PATH(36);goto disprset;}
 if(c==2u){LM_LAD_PATH(37);goto w2334;}          /* - : MASK DIDFLBIT */
 if(c==3u){LM_LAD_PATH(38);goto w2335;}          /* -0: EXTEND; BZF */
 LM_LAD_PATH(39);
 ai_cs(s,FLAGWRD1);
w2334:
 ai_maskf(s,DIDFLBIT);
w2335:
 if(ai_bzf(s)){LM_LAD_PATH(40);goto speedrun;}
 LM_LAD_PATH(41);
 /* CS FLAGWRD1; MASK DIDFLBIT; ADS FLAGWRD1; CS BIT7; MASK IMODES33;
  * TS IMODES33; CS FLAGWRD0; MASK R10FLBIT; EXTEND; BZF TASKOVER. */
 ai_cs(s,FLAGWRD1);ai_maskf(s,DIDFLBIT);ai_ads(s,FLAGWRD1);ai_csf(s,BIT7);ai_mask(s,IMODES33);
 if(ai_ts(s,IMODES33))LM_LAD_PATH(42);            /* skips CS FLAGWRD0 */
 else{LM_LAD_PATH(43);ai_cs(s,FLAGWRD0);}
 ai_maskf(s,R10FLBIT);
 if(ai_bzf(s)){LM_LAD_PATH(44);return jump(s,LM_LAD_TASKOVER);}
 LM_LAD_PATH(45);
 /* CAF BIT8; EXTEND; WOR CHAN12; CAF ZERO; TS TRAKLATV; TS TRAKFWDV;
  * TS LATVMETR; TS FORVMETR; CAF BIT4; TC TWIDDLE; ADRES INTLZE. A is +0,
  * so no TS skips. */
 ai_caf(s,BIT8);wor(s,&io->chan12);ai_caf(s,ZERO);
 (void)ai_ts(s,TRAKLATV);(void)ai_ts(s,TRAKFWDV);(void)ai_ts(s,LATVMETR);(void)ai_ts(s,FORVMETR);
 ai_caf(s,BIT4);
 return leave(s,LM_LAD_TWIDRET-1u,LM_LAD_TWIDDLE);
twidret: /* TCF TASKOVER. */
 return jump(s,LM_LAD_TASKOVER);
intlze: /* CAF BIT2; EXTEND; WOR CHAN12; CS IMODES33; MASK BIT8; ADS IMODES33;
  * TCF TASKOVER. */
 ai_caf(s,BIT2);wor(s,&io->chan12);ai_cs(s,IMODES33);ai_maskf(s,BIT8);ai_ads(s,IMODES33);
 return jump(s,LM_LAD_TASKOVER);

speedrun: /* DT = TIME1 - PIPTIME +1 (mod 2^14); ITEMP5 = 1SEC; for each axis
  * VVECT = G (T - TN) / 1SEC + V + (PIPA + PIPATMP) x KPIP1(5). TS ITEMP5 of
  * 1SEC does not skip. */
 ai_cs(s,PIPTIME1);ai_ad(s,TIME1);ai_adf(s,HALF);ai_adf(s,HALF);ai_xch(s,DT);ai_caf(s,ONESEC);(void)ai_ts(s,ITEMP5);
 for(i=0;i<3u;++i){static const unsigned pipa[3]={PIPAX,PIPAY,PIPAZ},tmp[3]={PIPATMPX,PIPATMPY,PIPATMPZ},
   gdt[3]={GDT2,GDT2_2,GDT2_4},vel[3]={V,V2,V4},vv[3]={VVECT,VVECT1,VVECT2};
  ai_dca(s,gdt[i]);ai_ddoubl(s);ai_ddoubl(s);ai_mp(s,DT);ai_dv(s,ITEMP5);ai_xch(s,vv[i]);
  ai_dca(s,vel[i]);ai_ddoubl(s);ai_ddoubl(s);ai_ads(s,vv[i]);
  ai_ca(s,pipa[i]);ai_ad(s,tmp[i]);ai_mpf(s,KPIP15);ai_ads(s,vv[i]);}
 /* CAF BIT3; TC VARDELAY. */
 ai_caf(s,BIT3);
 return leave(s,LM_LAD_VARRET,LM_LAD_VARDELAY);
varret: /* CS FLAGWRD0; MASK R10FLBIT; CCS A; TCF +2; TC LADQSAVE. */
 ai_cs(s,FLAGWRD0);ai_maskf(s,R10FLBIT);
 c=ai_ccs(s,0u);
 if(c==1u){LM_LAD_PATH(46);
  switch(tc_ladqsave(s,LM_LAD_VARRET+5u)){case 1:goto altrout1;case 2:goto altout1;default:return EXEC_OK;}}
 if(c==3u){LM_LAD_PATH(47);goto w2504;}          /* -0: AD VVECT */
 LM_LAD_PATH(48u+c/2u);                          /* 48: +, 49: - */
 /* CA DELVS; AD VVECT; TS ITEMP1; ... (VMP = VVECT + DELVS). */
 ai_ca(s,DELVS);
w2504:
 ai_ad(s,VVECT);
 if(ai_ts(s,ITEMP1))LM_LAD_PATH(50);else{LM_LAD_PATH(51);ai_ca(s,DELVS2);}
 ai_ad(s,VVECT1);
 if(ai_ts(s,ITEMP2))LM_LAD_PATH(52);else{LM_LAD_PATH(53);ai_ca(s,DELVS4);}
 ai_ad(s,VVECT2);
 if(ai_ts(s,ITEMP3))LM_LAD_PATH(54);else{LM_LAD_PATH(55);ai_ca(s,ITEMP1);}
 /* VHY = 2 VMP.UHYP, VHZ = 2 VMP.UHZP. */
 ai_mp(s,UHYP);ai_xch(s,RUPTREG1);ai_ca(s,ITEMP2);ai_mp(s,UHYP2);ai_ads(s,RUPTREG1);
 ai_ca(s,ITEMP3);ai_mp(s,UHYP4);ai_ads(s,RUPTREG1);ai_ca(s,RUPTREG1);ai_double(s);ai_xch(s,VHY);
 ai_ca(s,ITEMP1);ai_mp(s,UHZP);ai_xch(s,RUPTREG1);ai_ca(s,ITEMP2);ai_mp(s,UHZP2);ai_ads(s,RUPTREG1);
 ai_ca(s,ITEMP3);ai_mp(s,UHZP4);ai_ads(s,RUPTREG1);ai_ca(s,RUPTREG1);ai_double(s);ai_xch(s,VHZ);
 /* GET22/32: CAF EBANK6; TS EBANK; CA M22; TS ITEMP3; CA M32; TS ITEMP4;
  * CAF EBANK7; TS EBANK. The TS EBANK and TS ITEMP of CA'd words do not
  * skip. */
 ai_caf(s,EBANK6);(void)ai_ts(s,EBANK);ai_ca(s,M22);(void)ai_ts(s,ITEMP3);ai_ca(s,M32);(void)ai_ts(s,ITEMP4);
 ai_caf(s,EBANK7);(void)ai_ts(s,EBANK);
 /* LATFWDV: LATVEL and FORVEL in bit units. */
 ai_ca(s,ITEMP4);ai_mp(s,VHY);ai_xch(s,RUPTREG1);ai_ca(s,ITEMP3);ai_mp(s,VHZ);ai_ads(s,RUPTREG1);
 ai_caf(s,VELCONV);ai_mp(s,RUPTREG1);ai_ddoubl(s);ai_xch(s,LATVEL);
 ai_ca(s,ITEMP4);ai_mp(s,VHZ);ai_xch(s,RUPTREG1);ai_ca(s,ITEMP3);ai_mp(s,VHY);ai_cs(s,0u);ai_ads(s,RUPTREG1);
 ai_caf(s,VELCONV);ai_mp(s,RUPTREG1);ai_ddoubl(s);ai_xch(s,LATVEL1);
 /* CS MAXVBITS; TS ITEMP6; CAF ONE. */
 ai_csf(s,MAXVBITS);(void)ai_ts(s,ITEMP6);ai_caf(s,ONE);
vmonitor: /* TS ITEMP5 (1, then 0: no skip); INDEX ITEMP5; CCS LATVEL; TCF +4;
  * TCF LVLIMITS; TCF +8D; TCF LVLIMITS. */
 (void)ai_ts(s,ITEMP5);i=(unsigned)ai_word(s,ITEMP5);
 c=ai_ccs(s,LATVEL+i);
 if(c==1u||c==3u){LM_LAD_PATH(56u+c/2u);goto lvlimits;}  /* 56: +0, 57: -0 */
 if(c==0u){LM_LAD_PATH(58);
  /* INDEX ITEMP5; CS LATVEL; AD MAXVBITS; EXTEND; BZMF CHKLASTY; TCF LVLIMITS. */
  ai_cs(s,LATVEL+i);ai_adf(s,MAXVBITS);
  if(ai_bzmf(s)){LM_LAD_PATH(59);goto chklasty;}
  LM_LAD_PATH(60);goto lvlimits;}
 LM_LAD_PATH(61);
 /* INDEX ITEMP5; CA LATVEL; AD MAXVBITS; EXTEND; BZMF +2; TCF LVLIMITS. */
 ai_ca(s,LATVEL+i);ai_adf(s,MAXVBITS);
 if(!ai_bzmf(s)){LM_LAD_PATH(62);goto lvlimits;}
 LM_LAD_PATH(63);
chklasty: /* INDEX ITEMP5; CCS LATVMETR; TCF +4; TCF LASTOK; TCF +7; TCF LASTOK. */
 c=ai_ccs(s,LATVMETR+i);
 if(c==1u||c==3u){LM_LAD_PATH(64u+c/2u);goto lastok;}    /* 64: +0, 65: -0 */
 if(c==0u){LM_LAD_PATH(66);
  /* INDEX ITEMP5; CA LATVEL; EXTEND; BZMF LASTPOSY +5; TCF +5. */
  ai_ca(s,LATVEL+i);
  if(ai_bzmf(s)){LM_LAD_PATH(67);goto lastposy5;}
  LM_LAD_PATH(68);goto lastok;}
 LM_LAD_PATH(69);
 /* INDEX ITEMP5; CS LATVEL; EXTEND; BZMF LASTNEGY +4. */
 ai_cs(s,LATVEL+i);
 if(ai_bzmf(s)){LM_LAD_PATH(70);goto lastnegy4;}
 LM_LAD_PATH(71);
lastok: /* INDEX ITEMP5; CCS TRAKLATV; TCF LASTPOSY; TCF +2; TCF LASTNEGY. */
 c=ai_ccs(s,TRAKLATV+i);
 if(c==0u){LM_LAD_PATH(72);goto lastposy;}
 if(c==2u){LM_LAD_PATH(73);goto lastnegy;}
 if(c==3u){LM_LAD_PATH(74);goto w2670;}           /* -0: CA LATVEL */
 LM_LAD_PATH(75);
 /* INDEX ITEMP5; CA LATVEL; EXTEND; BZMF NEGVMAXY; TCF POSVMAXY. */
w2670:
 ai_ca(s,LATVEL+i);
 if(ai_bzmf(s)){LM_LAD_PATH(76);goto negvmaxy;}
 LM_LAD_PATH(77);goto posvmaxy;
lastposy: /* INDEX ITEMP5; CA LATVEL; EXTEND; BZMF +2; TCF POSVMAXY; CS
  * MAXVBITS; TCF ZEROLSTY. */
 ai_ca(s,LATVEL+i);
 if(!ai_bzmf(s)){LM_LAD_PATH(78);goto posvmaxy;}
 LM_LAD_PATH(79);
lastposy5:
 ai_csf(s,MAXVBITS);goto zerolsty;
posvmaxy: /* INDEX ITEMP5; CS LATVMETR; AD MAXVBITS; INDEX ITEMP5; XCH
  * RUPTREG3; CAF ONE; TCF ZEROLSTY +3. */
 ai_cs(s,LATVMETR+i);ai_adf(s,MAXVBITS);ai_xch(s,RUPTREG3+i);ai_caf(s,ONE);goto zerolsty3;
lastnegy: /* INDEX ITEMP5; CA LATVEL; EXTEND; BZMF NEGVMAXY; CA MAXVBITS;
  * TCF ZEROLSTY. */
 ai_ca(s,LATVEL+i);
 if(ai_bzmf(s)){LM_LAD_PATH(80);goto negvmaxy;}
 LM_LAD_PATH(81);
lastnegy4:
 ai_caf(s,MAXVBITS);goto zerolsty;
negvmaxy: /* INDEX ITEMP5; CA LATVMETR; AD MAXVBITS; COM; INDEX ITEMP5; XCH
  * RUPTREG3; CS ONE; TCF ZEROLSTY +3. */
 ai_ca(s,LATVMETR+i);ai_adf(s,MAXVBITS);ai_com(s);ai_xch(s,RUPTREG3+i);ai_csf(s,ONE);goto zerolsty3;
lvlimits: /* INDEX ITEMP5; CCS TRAKLATV; TCF LATVPOS; TCF +2; TCF LATVNEG. */
 c=ai_ccs(s,TRAKLATV+i);
 if(c==0u){LM_LAD_PATH(82);goto latvpos;}
 if(c==2u){LM_LAD_PATH(83);goto latvneg;}
 if(c==3u){LM_LAD_PATH(84);goto w2736;}           /* -0: CS LATVMETR */
 LM_LAD_PATH(85);
 /* INDEX ITEMP5; CS LATVMETR; EXTEND; BZMF +2; TCF NEGLMLV. */
w2736:
 ai_cs(s,LATVMETR+i);
 if(!ai_bzmf(s)){LM_LAD_PATH(86);goto neglmlv;}
 LM_LAD_PATH(87);
 /* INDEX ITEMP5; CS LATVEL; EXTEND; BZMF LVMINLM; AD ITEMP6; INDEX ITEMP5;
  * AD LATVMETR; EXTEND; BZMF LVMINLM; INDEX ITEMP5; AD LATVEL; EXTEND;
  * INDEX ITEMP5; SU LATVMETR; TCF ZEROLSTY. */
 ai_cs(s,LATVEL+i);
 if(ai_bzmf(s)){LM_LAD_PATH(88);goto lvminlm;}
 ai_ad(s,ITEMP6);ai_ad(s,LATVMETR+i);
 if(ai_bzmf(s)){LM_LAD_PATH(89);goto lvminlm;}
 LM_LAD_PATH(90);
 ai_ad(s,LATVEL+i);ai_su(s,LATVMETR+i);goto zerolsty;
latvpos: /* INDEX ITEMP5; CS LATVEL; EXTEND; BZMF LVMINLM; TCF +5. */
 ai_cs(s,LATVEL+i);
 if(ai_bzmf(s)){LM_LAD_PATH(91);goto lvminlm;}
 LM_LAD_PATH(92);goto w2772;
latvneg: /* INDEX ITEMP5; CA LATVEL; EXTEND; BZMF LVMINLM. */
 ai_ca(s,LATVEL+i);
 if(ai_bzmf(s)){LM_LAD_PATH(93);goto lvminlm;}
 LM_LAD_PATH(94);
w2772: /* INDEX ITEMP5; CS LATVMETR; TCF ZEROLSTY. */
 ai_cs(s,LATVMETR+i);goto zerolsty;
neglmlv: /* INDEX ITEMP5; CA LATVEL; EXTEND; BZMF LVMINLM; CA MAXVBITS; INDEX
  * ITEMP5; AD LATVMETR; COM; INDEX ITEMP5; AD LATVEL; EXTEND; BZMF LVMINLM;
  * EXTEND; INDEX ITEMP5; SU LATVEL; INDEX ITEMP5; AD LATVMETR; COM; TCF
  * ZEROLSTY. */
 ai_ca(s,LATVEL+i);
 if(ai_bzmf(s)){LM_LAD_PATH(95);goto lvminlm;}
 ai_caf(s,MAXVBITS);ai_ad(s,LATVMETR+i);ai_com(s);ai_ad(s,LATVEL+i);
 if(ai_bzmf(s)){LM_LAD_PATH(96);goto lvminlm;}
 LM_LAD_PATH(97);
 ai_su(s,LATVEL+i);ai_ad(s,LATVMETR+i);ai_com(s);goto zerolsty;
lvminlm: /* INDEX ITEMP5; CS LATVMETR; INDEX ITEMP5; AD LATVEL. */
 ai_cs(s,LATVMETR+i);ai_ad(s,LATVEL+i);
zerolsty: /* INDEX ITEMP5; XCH RUPTREG3; CAF ZERO. */
 ai_xch(s,RUPTREG3+i);ai_caf(s,ZERO);
zerolsty3: /* INDEX ITEMP5; TS TRAKLATV; INDEX ITEMP5; CA RUPTREG3; AD NEG0;
  * INDEX ITEMP5; TS CDUTCMD; INDEX ITEMP5; CA RUPTREG3; INDEX ITEMP5; ADS
  * LATVMETR; CCS ITEMP5; TCF VMONITOR. A is +0 or +-1 at TS TRAKLATV, and
  * a word plus -0 at TS CDUTCMD, so neither skips. */
 (void)ai_ts(s,TRAKLATV+i);ai_ca(s,RUPTREG3+i);ai_adf(s,NEG0);(void)ai_ts(s,CDUTCMD+i);
 ai_ca(s,RUPTREG3+i);ai_ads(s,LATVMETR+i);
 c=ai_ccs(s,ITEMP5);
 if(c==0u){LM_LAD_PATH(98);goto vmonitor;}
 if(c==3u){LM_LAD_PATH(113);return leave(s,LM_LAD_ZERODATA-1u,05014u);}  /* -0: WOR CHAN14's word as TC 05014 */
 if(c==1u){LM_LAD_PATH(99);ai_caf(s,PRIO6);}
 else LM_LAD_PATH(114);                          /* - : EXTEND; WOR CHAN14 */
 /* CAF BITSET; EXTEND; WOR CHAN14; TC LADQSAVE. */
 wor(s,&io->chan14);
 switch(tc_ladqsave(s,LM_LAD_ZERODATA)){case 1:LM_LAD_PATH(100);goto altrout1;case 2:LM_LAD_PATH(101);goto altout1;
  default:LM_LAD_PATH(102);return EXEC_OK;}
zerodata: /* CAF ZERO; TS L; TCF ZDATA2. */
 ai_caf(s,ZERO);(void)ai_ts(s,LREG);goto zdata2;

disprset: /* CS FLAGWRD0; MASK R10FLBIT; EXTEND; BZF ABORTON; CAF BIT8; MASK
  * IMODES33; CCS A; CAF BIT2; AD BIT8; COM; EXTEND; WAND CHAN12. */
 ai_cs(s,FLAGWRD0);ai_maskf(s,R10FLBIT);
 if(ai_bzf(s)){LM_LAD_PATH(103);goto aborton;}
 LM_LAD_PATH(104);
 ai_caf(s,BIT8);ai_mask(s,IMODES33);
 c=ai_ccs(s,0u);
 if(c==0u){LM_LAD_PATH(105);ai_caf(s,BIT2);}
 else if(c==1u)LM_LAD_PATH(106);
 else if(c==2u){LM_LAD_PATH(107);goto w3064;}   /* - : COM */
 else{LM_LAD_PATH(108);goto w3065;}             /* -0: EXTEND; WAND */
 ai_adf(s,BIT8);
w3064:
 ai_com(s);
w3065:
 wand(s,&io->chan12);
aborton: /* CS BITS8/7; MASK IMODES33; TS IMODES33; CS DIDFLBIT; MASK FLAGWRD1;
  * TS FLAGWRD1; TCF TASKOVER. */
 ai_csf(s,BITS87);ai_mask(s,IMODES33);
 if(ai_ts(s,IMODES33))LM_LAD_PATH(109);           /* skips CS DIDFLBIT */
 else{LM_LAD_PATH(110);ai_csf(s,DIDFLBIT);}
 ai_mask(s,FLAGWRD1);
 if(ai_ts(s,FLAGWRD1))LM_LAD_PATH(111);           /* skips TCF TASKOVER: BITS8/7 runs as TC 00300 */
 else{LM_LAD_PATH(112);return jump(s,LM_LAD_TASKOVER);}
 return leave(s,LM_LAD_BITS87+1u,0300u);
}

static enum exec_status flow_entry(const agc_native_program *p,agc_image *s,void *v,unsigned at){(void)p;
 if(!entry(s,v))return EXEC_OUT_OF_SCOPE;
 return flow(s,v,at);
}
static enum exec_status landisp(const agc_native_program *p,agc_image *s,void *v){return flow_entry(p,s,v,AT_LANDISP);}
static enum exec_status intlze(const agc_native_program *p,agc_image *s,void *v){return flow_entry(p,s,v,AT_INTLZE);}
static enum exec_status twidret(const agc_native_program *p,agc_image *s,void *v){return flow_entry(p,s,v,AT_TWIDRET);}
static enum exec_status varret(const agc_native_program *p,agc_image *s,void *v){return flow_entry(p,s,v,AT_VARRET);}
const agc_site_operand lm_landing_displays_sites[LM_LAD_SITES]={
#include "lm_landing_displays_sites.inc"
};
static const agc_routine_entry entries[4]={
 {P21(LM_LAD_LANDISP),landisp},{P21(LM_LAD_TWIDRET),twidret},{P21(LM_LAD_INTLZE),intlze},{P21(LM_LAD_VARRET),varret}};
const agc_routine_table lm_landing_displays_routines={entries,4};
