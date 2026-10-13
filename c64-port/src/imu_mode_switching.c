/* SPDX-License-Identifier: MIT */
/* Luminary099 and Comanche055 IMU_MODE_SWITCHING_ROUTINES as native routines
 * of the control-transfer ABI; see include/imu_mode_switching.h and
 * docs/IMU-MODE-SWITCHING.md. Each instruction of the source is one ai_*
 * register effect (or channel operation) on the image, in source order;
 * INDEX is the explicit address offset. Addresses below are Luminary's;
 * the Comanche code is the same word for word except where noted, so its
 * addresses come from the bank layouts (z7, z4, z37, zf). */
#include "imu_mode_switching.h"
#include "agc_image_ops.h"
#include "cm_native_image.h"
#define W s->words
/* Erasable (EB-window addresses are the source's; EB selects the bank). */
#define TIME2 024u
#define CDUX 032u
#define PIPAX 037u
#define GYROCMD 047u
#define CDUXCMD 050u
#define ITEMP1 061u
#define ITEMP2 062u
#define ITEMP4 064u
#define LOCCTR 064u
#define RUPTREG1 070u
#define RUPTREG2 071u
#define RUPTREG3 072u
#define FLAGWRD1 075u
#define FLAGWRD2 076u
#define FLAGWRD3 077u
#define BUF2 0133u
#define MPAC 0154u
#define LOC 0164u
#define PRIORITY 0167u
#define DSPTAB11 01036u
#define COMMAND 01471u
#define CDUIND 01474u
#define EB1400 01400u
/* Fixed constants (values checked against both ropes by
 * tools/imu_mode_switching_source.py). */
#define K_BIT1 01u
#define K_BIT4 010u
#define K_BIT5 020u
#define K_BIT6 040u
#define K_BIT8 0200u
#define K_BIT9 0400u
#define K_BIT10 01000u
#define K_BIT11 02000u
#define K_BIT13 010000u
#define K_BIT14 020000u
#define K_BIT15 040000u
#define K_BITS3_4 014u
#define K_BITS4_5 030u
#define K_BITS4_6 050u
#define K_SUPER011 060u
#define K_OCT40010 040010u
#define K_13_14_15 070000u
#define K_600MS 074u
#define K_2SECS 0310u
#define K_OCT62 062u
#define K_OCT220 0220u
#define K_MCOMMAX 077500u    /* -COMMAX */
#define K_MCOMMAX_ 077477u   /* -COMMAX- */
#define K_COARSTOL 077511u
#define K_GDESELCT 01700u
#define K_MGYROMIN 077601u   /* -GYROMIN */
#define K_LOW5 037u
#define K_LOW7 0177u
#define K_LOW8 0377u
#define K_SEVEN 7u
#define K_PRIO37 037000u
#define K_NEG0 077777u
#define K_MINUS1 077776u
#define K_NEG3 077774u
static const agc_word K_GYROFRAC[2]={0u,034u};
/* Return points of TC Q and TC MPAC +2: the TC stored there runs from Q
 * itself (or erasable) and leaves Q one past it. */
#define RET_Q 3u
#define RET_MPAC2 0157u

typedef struct {
 uint8_t cm;
 agc_word alarm,waitlist,vardelay,fixdelay,taskover,ibnkcall,swreturn,jobsleep,jobwake,makecadr;
 agc_word modabort,poodoo,postjump,tpagree,shortmp,finetime1,upflag,downflag,phaschng,bankcall,gotopooh,varalarm;
 agc_word imodes30,lgyro,modecadr,attcadr,thetad,time2sav,scalsave;
 agc_word gyromin_word,kalebcon; /* fixed 07601 (read by -GYROMIN as MASK); ECADR BCDU */
} ims_mission;
static const ims_mission LM={0,05567u,05203u,05224u,05221u,05261u,04674u,04631u,05133u,05137u,04645u,
 05716u,05652u,04635u,07257u,07307u,04103u,05504u,05516u,05353u,04616u,06001u,05744u,
 01302u,01314u,01304u,01307u,0321u,0314u,0316u,030001u,03276u};
static const ims_mission CM={1,05650u,05245u,05272u,05267u,05324u,04740u,04675u,05175u,05201u,04711u,
 05721u,05721u,04701u,07226u,07256u,04554u,05546u,05560u,05412u,04662u,04106u,05764u,
 01320u,01304u,01322u,01325u,01155u,01150u,01152u,052131u,03261u};

/* The program of the routine running now (set by the table wrappers; the
 * trampoline calls one routine at a time). ENDIMU reads channel 11 from
 * its agc_hardware. */
static const agc_native_program *program_now;
/* Comanche's bank 07 is Luminary's less 0115 up to ENDMODE, 0117 after the
 * BZF Luminary has there, then MODABORT is one word (TC POODOO) where
 * Luminary has DXCH BUF2 / TC BAILOUT1, and BITS4-5 is an EQUALS. */
#define CM7(a) ((a)<=03673u?(a)-0115u:(a)<=03741u?(a)-0117u:(a)==03742u?03622u:(a)==03743u?03623u:(a)-0121u)
static agc_word z7(const ims_mission *m,unsigned a){return (agc_word)(m->cm?CM7(a):a);}
static agc_word z4(const ims_mission *m,unsigned a){return (agc_word)(m->cm?a+0377u:a);}
static agc_word z37(const ims_mission *m,unsigned a){return (agc_word)(m->cm?a+3u:a);}
agc_word ims_comanche_address(unsigned lm_bank,agc_word a){
 return lm_bank==07u?(agc_word)CM7(a):lm_bank==04u?(agc_word)(a+0377u):lm_bank==037u?(agc_word)(a+3u):(agc_word)(a+042u);
}
/* LGWAKE: CADR GWAKE. */
static agc_word lgwake(const ims_mission *m){return (agc_word)(016000u+z7(m,03374u)-02000u);}

static void interrupts(const ims_mission *m,void *io,uint16_t on){
 if(m->cm)((cm_native_io *)io)->interrupt_enabled=on;else ((lm_imu_io *)io)->control.interrupt_enabled=on;
}
static agc_word *channel(const ims_mission *m,void *io,unsigned c){
 if(m->cm){cm_native_io *d=(cm_native_io *)io;return c==012u?&d->channel12:c==013u?&d->channel13:&d->channel14;}
 {lm_imu_io *d=(lm_imu_io *)io;return c==012u?&d->channel12:c==013u?&d->channel13:&d->channel14;}
}
/* A overflow-corrected to 15 bits, as the channel instructions take it. */
static agc_word acc15(const apollo_overlay_state *s){return (agc_word)(((W[0]>>1)&040000u)|(W[0]&037777u));}
/* RAND, WAND, WOR: A gets the 15-bit result sign-extended; a write to
 * channel 13 keeps bits 12-14 clear (they are trap enables, not storage).
 * Channel 11 (READ DSALMOUT) is the program's agc_hardware word. */
static void ch_rand(const ims_mission *m,apollo_overlay_state *s,void *io,unsigned c){ai_caf(s,(agc_word)(acc15(s)&*channel(m,io,c)&077777u));}
static void ch_write(const ims_mission *m,apollo_overlay_state *s,void *io,unsigned c,agc_word v){
 agc_word *p=channel(m,io,c);*p=c==013u?(agc_word)(v&043777u):v;ai_caf(s,v);
}
static void ch_wand(const ims_mission *m,apollo_overlay_state *s,void *io,unsigned c){ch_write(m,s,io,c,(agc_word)(acc15(s)&*channel(m,io,c)&077777u));}
static void ch_wor(const ims_mission *m,apollo_overlay_state *s,void *io,unsigned c){ch_write(m,s,io,c,(agc_word)((acc15(s)|*channel(m,io,c))&077777u));}
/* MSU K: two's-complement difference of A and K, converted to ones'
 * complement (one less when bit 15 is set), sign-extended into A. */
static void ai_msu(apollo_overlay_state *s,unsigned a){
 unsigned d=(unsigned)acc15(s)+((~(unsigned)ai_word(s,a))&077777u)+1u;
 if(d&040000u)d=((d|0100000u)-1u)&0177777u;
 ai_caf(s,(agc_word)(d&077777u));
}
/* INDEX K: the index word as the address offset (-0 indexes as 0). */
static unsigned ix(const apollo_overlay_state *s,unsigned k){agc_word v=ai_word(s,k);return v==077777u?0u:v;}
static unsigned ixa(const apollo_overlay_state *s){agc_word v=acc15(s);return v==077777u?0u:v;}
static enum exec_status jump(apollo_overlay_state *s,agc_word z){W[5]=z;return EXEC_OK;}
static enum exec_status call(apollo_overlay_state *s,agc_word q,agc_word z){ai_tc(s,q);W[5]=z;return EXEC_OK;}
/* TC Q from a subroutine entered from outside the file: Z is the caller's
 * return address, Q is left at 3. */
static enum exec_status tc_q(apollo_overlay_state *s){agc_word q=W[2];ai_tc(s,RET_Q);W[5]=q;return EXEC_OK;}
static int q_returns(const apollo_overlay_state *s){return W[2]>=010u&&W[2]<=07777u;}

/* ---- Shared tails (bank 07) ---- */
/* MODEEXIT: RELINT; TCF SWRETURN. */
static enum exec_status modeexit(const ims_mission *m,apollo_overlay_state *s,void *io){interrupts(m,io,1);return jump(s,m->swreturn);}
/* ENDMODE: CA RUPTREG3; ENDMODE +1: INDEX RUPTREG2 TS MODECADR; TCF TASKOVER. */
static enum exec_status endmode1(const ims_mission *m,apollo_overlay_state *s){(void)ai_ts(s,m->modecadr+ix(s,RUPTREG2));return jump(s,m->taskover);}
static enum exec_status endmode(const ims_mission *m,apollo_overlay_state *s){ai_ca(s,RUPTREG3);return endmode1(m,s);}
/* CAF ZERO; INDEX RUPTREG2 XCH MODECADR; TC JOBWAKE. */
static enum exec_status wake1(const ims_mission *m,apollo_overlay_state *s){ai_xch(s,m->modecadr+ix(s,RUPTREG2));return call(s,z7(m,03702u),m->jobwake);}
static enum exec_status wake(const ims_mission *m,apollo_overlay_state *s){ai_caf(s,0);return wake1(m,s);}
/* GOODEND +2: TS RUPTREG3; INDEX RUPTREG2 CCS MODECADR. Luminary:
 * TCF +4 / TCF ENDMODE / EXTEND BZF ENDMODE +1 (else CAF ZERO) / the BZF
 * word unextended, TCF ENDMODE +1. Comanche: TCF +2 / TCF ENDMODE /
 * CAF ZERO / INDEX RUPTREG2 with A +0. */
static enum exec_status endtail(const ims_mission *m,apollo_overlay_state *s){uint8_t c;
 (void)ai_ts(s,RUPTREG3);
 c=ai_ccs(s,m->modecadr+ix(s,RUPTREG2));
 if(c==1u)return endmode(m,s);
 if(m->cm)return c==3u?wake1(m,s):wake(m,s);
 if(c==0u||(c==2u&&!ai_bzf(s)))return wake(m,s);
 return endmode1(m,s);
}
/* BADEND: TS RUPTREG2; CS ZERO; TCF GOODEND +2. GOODEND: TS RUPTREG2;
 * CS ONE. An overflowing A skips the CS. */
static enum exec_status badend(const ims_mission *m,apollo_overlay_state *s){if(!ai_ts(s,RUPTREG2))ai_csf(s,0);return endtail(m,s);}
static enum exec_status goodend(const ims_mission *m,apollo_overlay_state *s){if(!ai_ts(s,RUPTREG2))ai_csf(s,1u);return endtail(m,s);}
/* IMUBAD: CAF ZERO; TCF BADEND. */
static enum exec_status imubad(const ims_mission *m,apollo_overlay_state *s){ai_caf(s,0);return badend(m,s);}
/* ENDIMU: EXTEND; READ DSALMOUT; MASK BIT1; CCS A; TCF IMUBAD;
 * IMUGOOD TCF GOODEND (A +0). */
static enum exec_status endimu(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;
 ai_caf(s,(agc_word)(program_now->hardware->channel11&077777u));ai_maskf(s,K_BIT1);
 if(ai_ccs(s,0)==0u)return imubad(m,s);
 return goodend(m,s);
}
/* CAGETEST (after the caller's TC): CAF BIT6; MASK IMODES30; CCS A;
 * TCF IMUBAD; TC Q. Returns 1 when it returns (A +0, Q 3). */
static int cagetest(const ims_mission *m,apollo_overlay_state *s,agc_word q){
 ai_tc(s,q);ai_caf(s,K_BIT6);ai_mask(s,m->imodes30);
 if(ai_ccs(s,0)==0u)return 0;
 ai_tc(s,RET_Q);return 1;
}
/* CAGETSTQ: CS IMODES30; MASK BIT6; CCS A; INCR Q; TC Q. Returns 1 for the
 * Q +1 return (cage bit clear). */
static int cagetstq(const ims_mission *m,apollo_overlay_state *s,agc_word q){int r;
 ai_tc(s,q);ai_cs(s,m->imodes30);ai_maskf(s,K_BIT6);
 r=ai_ccs(s,0)==0u;if(r)ai_incr(s,2);
 ai_tc(s,RET_Q);return r;
}
/* CAGETSTJ: CS IMODES30; MASK BIT6; CCS A; TC Q. Returns 0 when caged;
 * the caller then takes jfail. */
static int cagetstj(const ims_mission *m,apollo_overlay_state *s,agc_word q){
 ai_tc(s,q);ai_cs(s,m->imodes30);ai_maskf(s,K_BIT6);
 if(ai_ccs(s,0)!=0u)return 0;
 ai_tc(s,RET_Q);return 1;
}
/* CAGETSTJ +4: CS ZERO; TS IMUCADR; TCF MODEEXIT. */
static enum exec_status jfail(const ims_mission *m,apollo_overlay_state *s,void *io){ai_csf(s,0);(void)ai_ts(s,m->modecadr);return modeexit(m,s,io);}
/* NOATTOFF: CS OCT40010; MASK DSPTAB +11D; AD BIT15; TS DSPTAB +11D. */
static void noattoff(apollo_overlay_state *s){ai_csf(s,K_OCT40010);ai_mask(s,DSPTAB11);ai_adf(s,K_BIT15);(void)ai_ts(s,DSPTAB11);}
/* ZEROICDU: CAF ZERO; TS CDUX; TS CDUY; TS CDUZ. */
static void zeroicdu(apollo_overlay_state *s){ai_caf(s,0);(void)ai_ts(s,CDUX);(void)ai_ts(s,CDUX+1u);(void)ai_ts(s,CDUX+2u);}
/* RNDREFDR: clear TRACKFLG, DRIFTFLG and REFSMFLG. */
static void rndrefdr(apollo_overlay_state *s){
 ai_csf(s,K_BIT5);ai_mask(s,FLAGWRD1);(void)ai_ts(s,FLAGWRD1);
 ai_csf(s,K_BIT15);ai_mask(s,FLAGWRD2);(void)ai_ts(s,FLAGWRD2);
 ai_csf(s,K_BIT13);ai_mask(s,FLAGWRD3);(void)ai_ts(s,FLAGWRD3);
}
/* SETCOARS: CAF BIT4; RAND CHAN12; CCS A; TC Q (coarse align already on);
 * else stop gyro and CDU drive, set coarse align, NO ATT light and the
 * mode bits, then RNDREFDR. Ends at TC Q either way. */
static void setcoars(const ims_mission *m,apollo_overlay_state *s,void *io){
 ai_caf(s,K_BIT4);ch_rand(m,s,io,012);
 if(ai_ccs(s,0)==0u)return;
 ai_csf(s,K_BIT6);ch_wand(m,s,io,012);ai_csf(s,K_BIT10);ch_wand(m,s,io,014);
 ai_csf(s,0);(void)ai_ts(s,GYROCMD);ai_caf(s,K_BIT4);ch_wor(m,s,io,012);
 ai_cs(s,DSPTAB11);ai_maskf(s,K_OCT40010);ai_ads(s,DSPTAB11);
 ai_cs(s,m->imodes30+1u);ai_maskf(s,K_BIT6);ai_ads(s,m->imodes30+1u);
 ai_cs(s,m->imodes30);ai_maskf(s,K_BIT4);ai_ads(s,m->imodes30);
 rndrefdr(s);
}

/* ---- Fixed-fixed ---- */
static enum exec_status r_zeroicdu(const ims_mission *m,apollo_overlay_state *s,void *io){(void)m;(void)io;
 if(!q_returns(s))return EXEC_OUT_OF_SCOPE;
 zeroicdu(s);return tc_q(s);
}

/* ---- Bank 07: IMUZERO ---- */
/* 2714 IMUZERO: INHINT; CS DSPTAB +11D; MASK BITS4&6; CCS A; TCF IMUZEROA;
 * TC ALARM (OCT 206). IMUZEROA: TC CAGETSTJ; clear the zero and coarse
 * mode bits; WAND CHAN12 off coarse and fine; NOATTOFF; WOR CHAN12 zero
 * bit; ZEROICDU; CAF BIT6; TC WAITLIST (2CADR IMUZERO2). */
static enum exec_status r_imuzero(const ims_mission *m,apollo_overlay_state *s,void *io){
 interrupts(m,io,0);
 ai_cs(s,DSPTAB11);ai_maskf(s,K_BITS4_6);
 if(ai_ccs(s,0)!=0u)return call(s,z7(m,02722u),m->alarm);
 if(!cagetstj(m,s,z7(m,02725u)))return jfail(m,s,io);
 ai_cs(s,m->imodes30+1u);ai_maskf(s,K_SUPER011);ai_ads(s,m->imodes30+1u);
 ai_cs(s,m->imodes30);ai_maskf(s,K_BITS3_4);ai_ads(s,m->imodes30);
 ai_csf(s,K_BITS4_6);ch_wand(m,s,io,012);
 ai_tc(s,z7(m,02737u));noattoff(s);ai_tc(s,RET_Q);
 ai_caf(s,K_BIT5);ch_wor(m,s,io,012);
 ai_tc(s,z7(m,02743u));zeroicdu(s);ai_tc(s,RET_Q);
 ai_caf(s,K_BIT6);return call(s,z7(m,02745u),m->waitlist);
}
/* 2723 after ALARM 206: TCF CAGETSTJ +4. */
static enum exec_status r_c2723(const ims_mission *m,apollo_overlay_state *s,void *io){return jfail(m,s,io);}
/* 2747 after WAITLIST: CS IMODES30; MASK BIT9; CCS A; TCF MODEEXIT;
 * TC ALARM (OCT 210), whose return is MODEEXIT. */
static enum exec_status r_c2747(const ims_mission *m,apollo_overlay_state *s,void *io){
 ai_cs(s,m->imodes30);ai_maskf(s,K_BIT9);
 if(ai_ccs(s,0)==0u)return modeexit(m,s,io);
 return call(s,z7(m,02754u),m->alarm);
}
static enum exec_status r_modeexit(const ims_mission *m,apollo_overlay_state *s,void *io){return modeexit(m,s,io);}
/* 2757 IMUZERO2 (task): TC CAGETEST; TC ZEROICDU; CS BIT5; WAND CHAN12;
 * CAF BIT11; TC VARDELAY. */
static enum exec_status r_imuzero2(const ims_mission *m,apollo_overlay_state *s,void *io){
 if(!cagetest(m,s,z7(m,02760u)))return imubad(m,s);
 ai_tc(s,z7(m,02761u));zeroicdu(s);ai_tc(s,RET_Q);
 ai_csf(s,K_BIT5);ch_wand(m,s,io,012);
 ai_caf(s,K_BIT11);return call(s,z7(m,02766u),m->vardelay);
}
/* 2766 IMUZERO3: TC CAGETEST; clear BITS3&4 of IMODES30 and SUPER011 of
 * IMODES33; TC IBNKCALL (CADR SETISSW). */
static enum exec_status r_imuzero3(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;
 if(!cagetest(m,s,z7(m,02767u)))return imubad(m,s);
 ai_csf(s,K_BITS3_4);ai_mask(s,m->imodes30);(void)ai_ts(s,m->imodes30);
 ai_csf(s,K_SUPER011);ai_mask(s,m->imodes30+1u);(void)ai_ts(s,m->imodes30+1u);
 return call(s,z7(m,02776u),m->ibnkcall);
}
/* 2777 after SETISSW: TCF ENDIMU. */
static enum exec_status r_c2777(const ims_mission *m,apollo_overlay_state *s,void *io){
 if(!program_now->hardware)return EXEC_OUT_OF_SCOPE;
 return endimu(m,s,io);
}

/* ---- Coarse align ---- */
/* 3000 IMUCOARS: INHINT; TC CAGETSTJ; TC SETCOARS; CAF SIX; TC WAITLIST
 * (2CADR COARS). */
static enum exec_status r_imucoars(const ims_mission *m,apollo_overlay_state *s,void *io){
 interrupts(m,io,0);
 if(!cagetstj(m,s,z7(m,03002u)))return jfail(m,s,io);
 ai_tc(s,z7(m,03003u));setcoars(m,s,io);ai_tc(s,RET_Q);
 ai_caf(s,6u);return call(s,z7(m,03005u),m->waitlist);
}
/* 3007 after WAITLIST: TCF MODEEXIT. */
#define r_c3007 r_modeexit
/* 3010 COARS (task): TC CAGETEST; CAF BIT6; WOR CHAN12; then per axis
 * (CDUIND 2, 1, 0) COARS1: the commanded change THETAD - CDU (MSU, MP
 * BIT13, XCH L; DOUBLE; TS ITEMP1 / ADS L on overflow) into COMMAND;
 * CCS CDUIND; TC COARS1; then CAF TWO; TC VARDELAY. */
static enum exec_status r_coars(const ims_mission *m,apollo_overlay_state *s,void *io){
 if(!cagetest(m,s,z7(m,03011u)))return imubad(m,s);
 ai_caf(s,K_BIT6);ch_wor(m,s,io,012);
 ai_caf(s,2u);
 for(;;){
  (void)ai_ts(s,CDUIND);
  ai_ca(s,m->thetad+ix(s,CDUIND));ai_msu(s,CDUX+ix(s,CDUIND));
  ai_mpf(s,K_BIT13);ai_xch(s,1);ai_double(s);
  if(ai_ts(s,ITEMP1))ai_ads(s,1);
  ai_lxch(s,COMMAND+ix(s,CDUIND));
  /* CDUIND is 2, 1 or 0: CCS is + (A one less) or +0. */
  if(ai_ccs(s,CDUIND)!=0u)break;
  ai_tc(s,z7(m,03036u));
 }
 ai_caf(s,2u);return call(s,z7(m,03040u),m->vardelay);
}
/* 3040 COARS2 (after VARDELAY): TC CAGETEST; TS ITEMP1; per axis
 * (CDUIND 2, 1, 0) INDEX CDUIND CCS COMMAND: TC COMPOS / TC NEXTCDU +1 /
 * TC COMNEG / TC NEXTCDU +1, the command limited to COMMAX per pass
 * (COMZERO when it fits), INCR ITEMP1 for each axis commanded, AD NEG0;
 * INDEX CDUIND TS CDUXCMD; CCS CDUIND; TC COARS2 +3. Then CCS ITEMP1:
 * SENDPULS (WOR CHAN14 13,14,15; CAF 600MS; TC VARDELAY) or TC FIXDELAY
 * (DEC 150). */
static enum exec_status r_coars2(const ims_mission *m,apollo_overlay_state *s,void *io){uint8_t c,zero;
 if(!cagetest(m,s,z7(m,03041u)))return imubad(m,s);
 (void)ai_ts(s,ITEMP1);ai_caf(s,2u);
 for(;;){
  (void)ai_ts(s,CDUIND);
  c=ai_ccs(s,COMMAND+ix(s,CDUIND));
  if(c==1u||c==3u)ai_tc(s,z7(m,c==1u?03050u:03052u));
  else{
   if(c==0u){ai_tc(s,z7(m,03047u));ai_adf(s,K_MCOMMAX);zero=ai_bzmf(s);
    if(!zero){(void)ai_ts(s,COMMAND+ix(s,CDUIND));ai_csf(s,K_MCOMMAX_);}}
   else{ai_tc(s,z7(m,03051u));ai_adf(s,K_MCOMMAX);zero=ai_bzmf(s);
    if(!zero){ai_com(s);(void)ai_ts(s,COMMAND+ix(s,CDUIND));ai_caf(s,K_MCOMMAX_);ai_tc(s,z7(m,03127u));}}
   if(zero){ai_caf(s,0);ai_xch(s,COMMAND+ix(s,CDUIND));ai_tc(s,z7(m,03133u));}
   ai_incr(s,ITEMP1);
  }
  ai_adf(s,K_NEG0);(void)ai_ts(s,CDUXCMD+ix(s,CDUIND));
  if(ai_ccs(s,CDUIND)!=0u)break;
  ai_tc(s,z7(m,03066u));
 }
 /* ITEMP1 counts the axes commanded (0..3): CCS is + or +0. */
 if(ai_ccs(s,ITEMP1)==0u){ai_caf(s,K_13_14_15);ch_wor(m,s,io,014);ai_caf(s,K_600MS);return call(s,z7(m,03040u),m->vardelay);}
 return call(s,z7(m,03071u),m->fixdelay);
}
/* 3072 after FIXDELAY: CAF TWO; CHKCORS per axis: INDEX A CA CDUX;
 * INDEX ITEMP1 MSU THETAD; CCS A; nonzero: COARSERR AD COARSTOL; BZMF
 * CORSCHK2; TC ALARM (OCT 211); CORSCHK2 CCS ITEMP1; TCF CHKCORS;
 * TCF ENDIMU. */
static enum exec_status r_c3072(const ims_mission *m,apollo_overlay_state *s,void *io){
 uint8_t c;
 if(!program_now->hardware)return EXEC_OUT_OF_SCOPE;
 ai_caf(s,2u);
 for(;;){
  (void)ai_ts(s,ITEMP1);
  ai_ca(s,CDUX+ixa(s));ai_msu(s,m->thetad+ix(s,ITEMP1));
  c=ai_ccs(s,0);
  if(c==0u||c==2u){ai_adf(s,K_COARSTOL);if(!ai_bzmf(s))return call(s,z7(m,03114u),m->alarm);}
  if(ai_ccs(s,ITEMP1)!=0u)return endimu(m,s,io);
 }
}
/* 3115 after ALARM 211: TCF IMUBAD. */
static enum exec_status r_c3115(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;return imubad(m,s);}
/* 3140 CA+ECE: CAF BIT6; WOR CHAN12; TC TASKOVER. */
static enum exec_status r_ca_ece(const ims_mission *m,apollo_overlay_state *s,void *io){
 ai_caf(s,K_BIT6);ch_wor(m,s,io,012);return call(s,z7(m,03144u),m->taskover);
}
/* 3144 SETCOARS and 3175 RNDREFDR entered by TC from other files. */
static enum exec_status r_setcoars(const ims_mission *m,apollo_overlay_state *s,void *io){
 if(!q_returns(s))return EXEC_OUT_OF_SCOPE;
 setcoars(m,s,io);return tc_q(s);
}
static enum exec_status r_rndrefdr(const ims_mission *m,apollo_overlay_state *s,void *io){(void)m;(void)io;
 if(!q_returns(s))return EXEC_OUT_OF_SCOPE;
 rndrefdr(s);return tc_q(s);
}

/* ---- Fine align ---- */
/* 3210 IMUFINE: INHINT; TC CAGETSTJ; CS BITS4-5; WAND CHAN12; clear BIT6
 * of IMODES33; TC NOATTOFF; CAF BIT10; TC WAITLIST (2CADR IFAILOK). */
static enum exec_status r_imufine(const ims_mission *m,apollo_overlay_state *s,void *io){
 interrupts(m,io,0);
 if(!cagetstj(m,s,z7(m,03212u)))return jfail(m,s,io);
 ai_csf(s,K_BITS4_5);ch_wand(m,s,io,012);
 ai_csf(s,K_BIT6);ai_mask(s,m->imodes30+1u);(void)ai_ts(s,m->imodes30+1u);
 ai_tc(s,z7(m,03221u));noattoff(s);ai_tc(s,RET_Q);
 ai_caf(s,K_BIT10);return call(s,z7(m,03223u),m->waitlist);
}
/* 3225: CAF 2SECS; TC WAITLIST (2CADR IMUFINED). */
static enum exec_status r_c3225(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;ai_caf(s,K_2SECS);return call(s,z7(m,03227u),m->waitlist);}
#define r_c3231 r_modeexit
/* 3232 IMUFINED: TC CAGETEST; TCF ENDIMU. */
static enum exec_status imufined(const ims_mission *m,apollo_overlay_state *s,void *io){
 if(!program_now->hardware)return EXEC_OUT_OF_SCOPE;
 if(!cagetest(m,s,z7(m,03233u)))return imubad(m,s);
 return endimu(m,s,io);
}
#define r_imufined imufined
/* PFAILOK2: MASK IMODES30; TS IMODES30; TC IBNKCALL (CADR SETISSW). */
static enum exec_status pfailok2(const ims_mission *m,apollo_overlay_state *s){
 ai_mask(s,m->imodes30);(void)ai_ts(s,m->imodes30);return call(s,z7(m,03252u),m->ibnkcall);
}
/* 3234 IFAILOK (task): TC CAGETSTQ; TCF TASKOVER; CAF BIT4; RAND CHAN12;
 * CCS A; TCF TASKOVER; set BIT13 of IMODES30; CS BIT4; PFAILOK2. */
static enum exec_status r_ifailok(const ims_mission *m,apollo_overlay_state *s,void *io){
 if(!cagetstq(m,s,z7(m,03235u)))return jump(s,m->taskover);
 ai_caf(s,K_BIT4);ch_rand(m,s,io,012);
 if(ai_ccs(s,0)==0u)return jump(s,m->taskover);
 ai_cs(s,m->imodes30);ai_maskf(s,K_BIT13);ai_ads(s,m->imodes30);
 ai_csf(s,K_BIT4);return pfailok2(m,s);
}
/* 3253 after SETISSW: TCF TASKOVER. */
static enum exec_status r_c3253(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;return jump(s,m->taskover);}
/* 3254 PFAILOK (task): TC CAGETSTQ; TCF TASKOVER; set BIT10 of IMODES30
 * and BIT13 of IMODES33; CS BIT5; TCF PFAILOK2. */
static enum exec_status r_pfailok(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;
 if(!cagetstq(m,s,z7(m,03255u)))return jump(s,m->taskover);
 ai_cs(s,m->imodes30);ai_maskf(s,K_BIT10);ai_ads(s,m->imodes30);
 ai_cs(s,m->imodes30+1u);ai_maskf(s,K_BIT13);ai_ads(s,m->imodes30+1u);
 ai_csf(s,K_BIT5);return pfailok2(m,s);
}
/* 3266 NOATTOFF entered by TC from T4RUPT. */
static enum exec_status r_noattoff(const ims_mission *m,apollo_overlay_state *s,void *io){(void)m;(void)io;
 if(!q_returns(s))return EXEC_OUT_OF_SCOPE;
 noattoff(s);return tc_q(s);
}

/* ---- PIPA use ---- */
/* PIPFREE2: TC IBNKCALL (CADR SETISSW). */
static enum exec_status pipfree2(const ims_mission *m,apollo_overlay_state *s){return call(s,z7(m,03306u),m->ibnkcall);}
/* 3277 PIPUSE1: TC CAGETSTQ; TCF SWRETURN; INHINT; clear BIT1 of IMODES30;
 * PIPFREE2. */
static enum exec_status pipuse1(const ims_mission *m,apollo_overlay_state *s,void *io){
 if(!cagetstq(m,s,z7(m,03300u)))return jump(s,m->swreturn);
 interrupts(m,io,0);
 ai_csf(s,K_BIT1);ai_mask(s,m->imodes30);(void)ai_ts(s,m->imodes30);
 return pipfree2(m,s);
}
#define r_pipuse1 pipuse1
/* 3273 PIPUSE: CS ZERO; TS PIPAX; TS PIPAY; TS PIPAZ; PIPUSE1. */
static enum exec_status r_pipuse(const ims_mission *m,apollo_overlay_state *s,void *io){
 ai_csf(s,0);(void)ai_ts(s,PIPAX);(void)ai_ts(s,PIPAX+1u);(void)ai_ts(s,PIPAX+2u);
 return pipuse1(m,s,io);
}
#define r_c3307 r_modeexit
/* 3310 PIPFREE: INHINT; set BIT1 of IMODES30; MASK BIT10; CCS A;
 * TCF MODEEXIT; TC ALARM (OCT 212). */
static enum exec_status r_pipfree(const ims_mission *m,apollo_overlay_state *s,void *io){
 interrupts(m,io,0);
 ai_cs(s,m->imodes30);ai_maskf(s,K_BIT1);ai_ads(s,m->imodes30);ai_maskf(s,K_BIT10);
 if(ai_ccs(s,0)==0u)return modeexit(m,s,io);
 return call(s,z7(m,03320u),m->alarm);
}
/* 3321 after ALARM 212: INHINT; TCF PIPFREE2. */
static enum exec_status r_c3321(const ims_mission *m,apollo_overlay_state *s,void *io){interrupts(m,io,0);return pipfree2(m,s);}

/* ---- Gyro torquing ---- */
/* GWAKE2: TC WAITLIST (2CADR STRTGYRO). REGSLEEP: CAF LGWAKE; TCF JOBSLEEP. */
static enum exec_status gwake2(const ims_mission *m,apollo_overlay_state *s){return call(s,z7(m,03335u),m->waitlist);}
static enum exec_status regsleep(const ims_mission *m,apollo_overlay_state *s){ai_caf(s,lgwake(m));return jump(s,m->jobsleep);}
/* 3323 IMUPULSE: TS MPAC +5 (an overflowing A skips the TC); TC CAGETSTJ;
 * CCS LGYRO: TC GYROBUSY (EXTEND DCA BUF2; DXCH MPAC; REGSLEEP) /
 * TS MPAC +2 / CAF BIT6 / EXTEND; WOR CHAN14; CAF FOUR; GWAKE2. */
static enum exec_status r_imupulse(const ims_mission *m,apollo_overlay_state *s,void *io){uint8_t c;
 if(!ai_ts(s,MPAC+5u)&&!cagetstj(m,s,z7(m,03325u)))return jfail(m,s,io);
 c=ai_ccs(s,m->lgyro);
 if(c==0u){ai_tc(s,z7(m,03327u));ai_dca(s,BUF2);ai_dxch(s,MPAC);return regsleep(m,s);}
 if(c==1u)(void)ai_ts(s,MPAC+2u);
 if(c!=3u)ai_caf(s,K_BIT6);
 ch_wor(m,s,io,014);ai_caf(s,4u);return gwake2(m,s);
}
/* An EBANK of 0 makes 1400-1407 the central registers: a DCA or DXCH
 * there at index 0-7 reads or writes A, L, Q, the banks, Z or ZERO. */
static int e0_registers(agc_word ebank,agc_word index){return ((ebank>>8)&7u)==0u&&index<=7u;}
/* GYROAGRE: TS MPAC +3; DOUBLE; AD ITEMP1; TS MPAC +4; EXTEND;
 * INDEX A DCA 1400; DXCH MPAC; TC TPAGREE. */
static enum exec_status gyroagre(const ims_mission *m,apollo_overlay_state *s){
 (void)ai_ts(s,MPAC+3u);ai_double(s);ai_ad(s,ITEMP1);(void)ai_ts(s,MPAC+4u);
 ai_dca(s,EB1400+ixa(s));ai_dxch(s,MPAC);return call(s,z7(m,03357u),m->tpagree);
}
/* 3337 after WAITLIST: CA MPAC +5; XCH EBANK; XCH MPAC +5; TS LGYRO;
 * MASK LOW8; TS ITEMP1; CAF TWO; GYROAGRE. */
static enum exec_status r_c3337(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;
 if((W[MPAC+5u]&K_LOW8)>0372u||e0_registers(W[MPAC+5u],(agc_word)(W[MPAC+5u]&K_LOW8)))return EXEC_OUT_OF_SCOPE;
 ai_ca(s,MPAC+5u);ai_xch(s,3);ai_xch(s,MPAC+5u);(void)ai_ts(s,m->lgyro);
 ai_maskf(s,K_LOW8);(void)ai_ts(s,ITEMP1);ai_caf(s,2u);
 return gyroagre(m,s);
}
/* 3357 after TPAGREE: DXCH MPAC; INDEX MPAC +4 DXCH 1400; CCS MPAC +3;
 * TCF GYROAGRE / CA MPAC +5; TS EBANK; TCF MODEEXIT. */
static enum exec_status r_c3357(const ims_mission *m,apollo_overlay_state *s,void *io){uint8_t c;agc_word n=W[MPAC+3u];
 if(W[MPAC+4u]>0376u)return EXEC_OUT_OF_SCOPE;
 if(n>=1u&&n<=037777u&&(W[ITEMP1]>0376u||2ul*(n-1u)+W[ITEMP1]>0376u))return EXEC_OUT_OF_SCOPE;
 if(e0_registers(W[3],W[MPAC+4u])||(n>=1u&&n<=037777u&&e0_registers(W[3],W[ITEMP1])))return EXEC_OUT_OF_SCOPE;
 /* (Under EBANK 0 the DXCH can land on MPAC +3 or ITEMP1, which the checks
  * above read first: GYROAGRE's TS MPAC +4 could then overflow.) */
 if(((W[3]>>8)&7u)==0u&&(W[MPAC+4u]+1u==MPAC+3u||W[MPAC+4u]==MPAC+3u||W[MPAC+4u]+1u==ITEMP1||W[MPAC+4u]==ITEMP1))return EXEC_OUT_OF_SCOPE;
 ai_dxch(s,MPAC);ai_dxch(s,EB1400+ix(s,MPAC+4u));
 c=ai_ccs(s,MPAC+3u);
 if(c==0u)return gyroagre(m,s);
 if(c==1u)ai_ca(s,MPAC+5u);
 if(c!=3u)(void)ai_ts(s,3);
 return modeexit(m,s,io);
}
/* 3374 GWAKE (the job JOBSLEEP left): CCS LGYRO; TCF REGSLEEP / TS MPAC +2 /
 * EXTEND; DCA MPAC / (the DCA word unextended) CA MPAC +1; DXCH BUF2;
 * CAF ONE; TCF GWAKE2. */
static enum exec_status r_gwake(const ims_mission *m,apollo_overlay_state *s,void *io){uint8_t c;(void)io;
 c=ai_ccs(s,m->lgyro);
 if(c==0u)return regsleep(m,s);
 if(c==1u)(void)ai_ts(s,MPAC+2u);
 if(c==3u)ai_ca(s,MPAC+1u);else ai_dca(s,MPAC);
 ai_dxch(s,BUF2);ai_caf(s,1u);return gwake2(m,s);
}
/* LASTSEG +1: EXTEND; MP BIT10; AD THREE; TC WAITLIST (2CADR STRTGYRO).
 * AUG3: EXTEND; MP BIT10; AD NEG3; TC WAITLIST (2CADR 8192AUG). */
static enum exec_status lastseg1(const ims_mission *m,apollo_overlay_state *s){ai_mpf(s,K_BIT10);ai_adf(s,3u);return call(s,z7(m,03535u),m->waitlist);}
static enum exec_status aug3(const ims_mission *m,apollo_overlay_state *s){ai_mpf(s,K_BIT10);ai_adf(s,K_NEG3);return call(s,z7(m,03554u),m->waitlist);}
/* LGYRO's GSELECT index stays inside the gyro command words from here:
 * no step count past 7 and no LOW8 index of 0377 (DCA/DXCH 1400 would
 * reach 02000). The steps from count c: gyro X adds 2, Y adds 2, Z
 * subtracts 4 and adds 0, each with BIT13. */
static int strtgyro_domain(agc_word v){unsigned a;
 if(v>037777u)return 0;
 for(;;){a=v>>11;
  if((a&1u)||a>=6u)return 1;
  if(a==4u)v=(agc_word)(v-4u);
  v=(agc_word)(v+(a==4u?0u:2u)+K_BIT13);
  if((v&K_LOW8)==K_LOW8||e0_registers(v,(agc_word)(v&K_LOW8)))return 0;
 }
}
/* GSELECT (after TC GSELECT with the OCT at Q): INDEX Q CAF 0; TS ITEMP4;
 * MASK SEVEN; AD BIT13; ADS LGYRO; TS EBANK; MASK LOW8; TS ITEMP1;
 * CS SEVEN; MASK ITEMP4; TS ITEMP4; EXTEND; INDEX ITEMP1 DCA 1400;
 * DXCH RUPTREG1; then the major/minor tests, MAJ+ / MAJ- channel-14
 * select, GMERGE, and LONGGYRO, the short exit or the +0/-0 fixes before
 * LASTSEG. Returns 1 to continue at STRTGYR2. */
static int gselect(const ims_mission *m,apollo_overlay_state *s,void *io,agc_word oct,enum exec_status *out){uint8_t c,neg;
 ai_caf(s,oct);(void)ai_ts(s,ITEMP4);ai_maskf(s,K_SEVEN);ai_adf(s,K_BIT13);ai_ads(s,m->lgyro);
 if(!ai_ts(s,3))ai_maskf(s,K_LOW8);
 (void)ai_ts(s,ITEMP1);ai_csf(s,K_SEVEN);ai_mask(s,ITEMP4);(void)ai_ts(s,ITEMP4);
 ai_dca(s,EB1400+ix(s,ITEMP1));ai_dxch(s,RUPTREG1);
 c=ai_ccs(s,RUPTREG1);
 if(c==1u||c==3u){
  c=ai_ccs(s,RUPTREG2);
  if(c==1u||c==3u)return 1;
  ai_adf(s,K_MGYROMIN);
  if(ai_bzmf(s))return 1;
 }
 neg=c==2u;
 if(!neg){
  ai_dcaf(s,K_GYROFRAC);ai_das(s,RUPTREG1);ai_ca(s,ITEMP4);ch_wor(m,s,io,014);
  ai_caf(s,K_LOW7);ai_mask(s,RUPTREG2);ai_xch(s,RUPTREG2);
 }else{
  ai_dcsf(s,K_GYROFRAC);ai_das(s,RUPTREG1);ai_ca(s,ITEMP4);ai_adf(s,K_BIT9);ch_wor(m,s,io,014);
  ai_cs(s,RUPTREG1);(void)ai_ts(s,RUPTREG1);ai_cs(s,RUPTREG2);ai_maskf(s,K_LOW7);ai_com(s);
  ai_xch(s,RUPTREG2);ai_com(s);
 }
 /* GMERGE */
 ai_mpf(s,K_BIT8);(void)ai_ts(s,ITEMP2);ai_ca(s,RUPTREG1);ai_mpf(s,K_BIT9);(void)ai_ts(s,RUPTREG1);
 ai_ca(s,1);ai_mpf(s,K_BIT14);ai_ads(s,ITEMP2);ai_dca(s,RUPTREG1);ai_adf(s,K_MINUS1);
 c=ai_ccs(s,0);
 if(c==0u){
  /* LONGGYRO: INDEX ITEMP1 DXCH 1400; CAF BIT14; AD ITEMP2; TS GYROCMD; AUG3. */
  ai_dxch(s,EB1400+ix(s,ITEMP1));ai_caf(s,K_BIT14);ai_ad(s,ITEMP2);(void)ai_ts(s,GYROCMD);
  *out=aug3(m,s);return 0;
 }
 /* +0: -GYROMIN as MASK 07601; -: TCF +4; -0: CAF BIT14; ADS ITEMP2;
  * CAF ZERO. Then INDEX ITEMP1 DXCH 1400; CA ITEMP2; LASTSEG TS GYROCMD.
  * The +0 outcome cannot occur: A is RUPTREG1 as DCA loaded it (no
  * overflow), and in ones' complement x + (-1) is +0 for no x (x = +1
  * gives -0). The MASK stays as the source has it. */
 if(c==1u)ai_maskf(s,m->gyromin_word);
 if(c==3u){ai_caf(s,K_BIT14);ai_ads(s,ITEMP2);ai_caf(s,0);}
 ai_dxch(s,EB1400+ix(s,ITEMP1));ai_ca(s,ITEMP2);(void)ai_ts(s,GYROCMD);
 *out=lastseg1(m,s);return 0;
}
/* 3405 STRTGYRO (task): CS GDESELCT; WAND CHAN14; TC CAGETEST; STRTGYR2:
 * CA LGYRO; EXTEND; MP BIT4; INDEX A TCF +1 into TC GSELECT (OCT 202),
 * TC GSELECT (OCT 302), TC GSELECT -2 (CS FOUR; ADS LGYRO; OCT 100), or
 * the end: CAF ZERO; TS LGYRO; CAF LGWAKE; TC JOBWAKE. An odd index lands
 * on an OCT word, a TC into erasable. */
static enum exec_status r_strtgyro(const ims_mission *m,apollo_overlay_state *s,void *io){unsigned a;enum exec_status out;
 if(!strtgyro_domain(W[m->lgyro]))return EXEC_OUT_OF_SCOPE;
 ai_csf(s,K_GDESELCT);ch_wand(m,s,io,014);
 if(!cagetest(m,s,z7(m,03411u)))return imubad(m,s);
 for(;;){
  ai_ca(s,m->lgyro);ai_mpf(s,K_BIT4);a=ixa(s);
  switch(a){
   case 0u:ai_tc(s,z7(m,03417u));if(!gselect(m,s,io,0202u,&out))return out;break;
   case 2u:ai_tc(s,z7(m,03421u));if(!gselect(m,s,io,0302u,&out))return out;break;
   case 4u:ai_tc(s,z7(m,03423u));ai_csf(s,4u);ai_ads(s,m->lgyro);if(!gselect(m,s,io,0100u,&out))return out;break;
   case 1u:return call(s,z7(m,03420u),0202u);
   case 3u:return call(s,z7(m,03422u),0302u);
   case 5u:return call(s,z7(m,03424u),0100u);
   default:
    if(a==6u)ai_caf(s,0);
    (void)ai_ts(s,m->lgyro);ai_caf(s,lgwake(m));return call(s,z7(m,03430u),m->jobwake);
  }
 }
}
/* 3430 NORESET (after JOBWAKE): TCF IMUFINED. */
#define r_noreset imufined
/* 3537 GYROEXIT: CAF BIT10; WOR CHAN14; TCF TASKOVER. */
static enum exec_status r_gyroexit(const ims_mission *m,apollo_overlay_state *s,void *io){ai_caf(s,K_BIT10);ch_wor(m,s,io,014);return jump(s,m->taskover);}
#define r_c3556 r_gyroexit
/* 3557 8192AUG (task): TC CAGETEST; CAF BIT4; RAND CHAN12; CCS A;
 * TCF IMUBAD; CA LGYRO; TS EBANK; MASK LOW8; TS ITEMP1; INDEX ITEMP1
 * CCS 1400: AUG2 (INDEX ITEMP1 TS 1400; CAF BIT14; ADS GYROCMD; AUG3) /
 * CAF BIT14 / ADS GYROCMD / TCF LASTSEG +1. */
static enum exec_status r_8192aug(const ims_mission *m,apollo_overlay_state *s,void *io){uint8_t c;
 /* (LGYRO's EBANK 0 with an index of 4-6: CCS finds FB, Z or BB positive
  * and the TS writes it, so the AGC fetches from elsewhere.) */
 if(((W[m->lgyro]>>8)&7u)==0u&&(W[m->lgyro]&K_LOW8)>=4u&&(W[m->lgyro]&K_LOW8)<=6u)return EXEC_OUT_OF_SCOPE;
 if(!cagetest(m,s,z7(m,03560u)))return imubad(m,s);
 ai_caf(s,K_BIT4);ch_rand(m,s,io,012);
 if(ai_ccs(s,0)==0u)return imubad(m,s);
 ai_ca(s,m->lgyro);(void)ai_ts(s,3);ai_maskf(s,K_LOW8);(void)ai_ts(s,ITEMP1);
 c=ai_ccs(s,EB1400+ix(s,ITEMP1));
 if(c==0u){(void)ai_ts(s,EB1400+ix(s,ITEMP1));ai_caf(s,K_BIT14);ai_ads(s,GYROCMD);return aug3(m,s);}
 if(c==1u)ai_caf(s,K_BIT14);
 if(c!=3u)ai_ads(s,GYROCMD);
 return lastseg1(m,s);
}

/* ---- Mode ends and stalls ---- */
static enum exec_status r_endimu(const ims_mission *m,apollo_overlay_state *s,void *io){
 if(!program_now->hardware)return EXEC_OUT_OF_SCOPE;
 return endimu(m,s,io);
}
static enum exec_status r_imubad(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;return imubad(m,s);}
static int end_index(const apollo_overlay_state *s){return W[0]<=2u;}
static enum exec_status r_badend(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;if(!end_index(s))return EXEC_OUT_OF_SCOPE;return badend(m,s);}
static enum exec_status r_goodend(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;if(!end_index(s))return EXEC_OUT_OF_SCOPE;return goodend(m,s);}
/* 3702 after JOBWAKE: CS RUPTREG3; INDEX LOCCTR ADS LOC; TCF TASKOVER. */
static enum exec_status r_c3702(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;
 if(W[LOCCTR]>01613u)return EXEC_OUT_OF_SCOPE;
 ai_cs(s,RUPTREG3);ai_ads(s,LOC+ix(s,LOCCTR));return jump(s,m->taskover);
}
/* MODABORT: Luminary DXCH BUF2; TC BAILOUT1 (OCT 1210); Comanche
 * TC POODOO (OCT 1210). */
static enum exec_status modabort(const ims_mission *m,apollo_overlay_state *s){
 if(!m->cm)ai_dxch(s,BUF2);
 return call(s,z7(m,03743u),m->modabort);
}
/* MG2: INDEX RUPTREG2 TS MODECADR; TCF MODEEXIT. */
static enum exec_status mg2(const ims_mission *m,apollo_overlay_state *s,void *io){(void)ai_ts(s,m->modecadr+ix(s,RUPTREG2));return modeexit(m,s,io);}
/* STALL (A = 0 IMU, 1 optics, 2 radar): INHINT; TS RUPTREG2; INDEX A
 * CCS MODECADR: MODABORT / MODESLP (TC MAKECADR) / MODEGOOD (CCS A;
 * MODABORT / INCR BUF2; MG2 / MG2 / MODESLP) / MG2. */
static enum exec_status stall(const ims_mission *m,apollo_overlay_state *s,void *io){uint8_t c;
 interrupts(m,io,0);
 (void)ai_ts(s,RUPTREG2);
 c=ai_ccs(s,m->modecadr+ixa(s));
 if(c==2u){
  c=ai_ccs(s,0);
  if(c==1u){ai_incr(s,BUF2);c=3u;}
  else if(c==2u)c=3u;
  else if(c==3u)c=1u;
 }
 if(c==0u)return modabort(m,s);
 if(c==1u)return call(s,z7(m,03736u),m->makecadr);
 return mg2(m,s,io);
}
static enum exec_status r_aotstall(const ims_mission *m,apollo_overlay_state *s,void *io){ai_caf(s,1u);ai_tc(s,z7(m,03714u));return stall(m,s,io);}
static enum exec_status r_radstall(const ims_mission *m,apollo_overlay_state *s,void *io){ai_caf(s,2u);return stall(m,s,io);}
static enum exec_status r_imustall(const ims_mission *m,apollo_overlay_state *s,void *io){ai_caf(s,0);return stall(m,s,io);}
/* 3736 after MAKECADR: INDEX RUPTREG2 TS MODECADR; TCF JOBSLEEP (an
 * overflowing A skips it into MODABORT). */
static enum exec_status r_c3736(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;
 if(W[RUPTREG2]>2u)return EXEC_OUT_OF_SCOPE;
 if(ai_ts(s,m->modecadr+ix(s,RUPTREG2)))return modabort(m,s);
 return jump(s,m->jobsleep);
}
/* 3750 GOMANUR: CA ATTCADR; EXTEND; BZF +3; TC POODOO (OCT 1210);
 * EXTEND; DCA BUF2; DXCH ATTCADR; CA BBANK; MASK SEVEN; ADS ATTCADR +1;
 * CA PRIORITY; MASK PRIO37; TS ATTPRIO; CAF KALEBCON; TS EBANK;
 * TC POSTJUMP (CADR KALCMAN3). */
static enum exec_status r_gomanur(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;
 ai_ca(s,m->attcadr);
 if(!ai_bzf(s))return call(s,z7(m,03754u),m->poodoo);
 ai_dca(s,BUF2);ai_dxch(s,m->attcadr);ai_ca(s,6);ai_maskf(s,K_SEVEN);ai_ads(s,m->attcadr+1u);
 ai_ca(s,PRIORITY);ai_maskf(s,K_PRIO37);(void)ai_ts(s,m->attcadr+2u);
 ai_caf(s,m->kalebcon);(void)ai_ts(s,3);return call(s,z7(m,03771u),m->postjump);
}

/* ---- R02BOTH (Luminary bank 04, Comanche bank 07) ---- */
/* 3254 R02BOTH: CAF REFSMBIT; MASK FLAGWRD3; CCS A; TC R02ZERO (TC UPFLAG,
 * ADRES IMUSE); CA IMODES30; MASK BIT9; EXTEND; BZF +2; CS BIT4;
 * AD OCT220; TC VARALARM. */
static enum exec_status r_r02both(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;
 ai_caf(s,K_BIT13);ai_mask(s,FLAGWRD3);
 if(ai_ccs(s,0)==0u){ai_tc(s,z4(m,03260u));return call(s,z4(m,03271u),m->upflag);}
 ai_ca(s,m->imodes30);ai_maskf(s,K_BIT9);
 if(!ai_bzf(s))ai_csf(s,K_BIT4);
 ai_adf(s,K_OCT220);return call(s,z4(m,03267u),m->varalarm);
}
/* 3267 after VARALARM: TC GOTOPOOH. 3272 after UPFLAG: TCF SWRETURN. */
static enum exec_status r_c3267(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;return call(s,z4(m,03270u),m->gotopooh);}
/* 3270 R02ZERO (TC R02ZERO from R02BOTH, and TC BANKCALL CADR R02ZERO in
 * P51-P53): TC UPFLAG (ADRES IMUSE). */
static enum exec_status r_r02zero(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;return call(s,z4(m,03271u),m->upflag);}
static enum exec_status r_c3272(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;return jump(s,m->swreturn);}

/* ---- P06 / P05 (Luminary bank 37, Comanche bank 26) ---- */
/* SCALPREP (after the caller's TC): EXTEND; QXCH MPAC +2; TC FINETIME +1. */
static enum exec_status scalprep(const ims_mission *m,apollo_overlay_state *s){ai_qxch(s,MPAC+2u);return call(s,z37(m,03717u),m->finetime1);}
/* 3655 P06: TC UPFLAG (ADRES NODOFLAG). */
static enum exec_status r_p06(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;return call(s,z37(m,03656u),m->upflag);}
/* 3657 PRESTAND: INHINT; EXTEND; DCA TIME2; DXCH TIME2SAV; TC SCALPREP. */
static enum exec_status prestand(const ims_mission *m,apollo_overlay_state *s,void *io){
 interrupts(m,io,0);ai_dca(s,TIME2);ai_dxch(s,m->time2sav);ai_tc(s,z37(m,03664u));return scalprep(m,s);
}
#define r_prestand prestand
/* 3664 (SCALPREP's retry return): TC PRESTAND. */
static enum exec_status r_c3664(const ims_mission *m,apollo_overlay_state *s,void *io){ai_tc(s,z37(m,03665u));return prestand(m,s,io);}
/* 3665: DXCH MPAC; DXCH SCALSAVE; INHINT; TC BANKCALL (CADR RNDREFDR). */
static enum exec_status r_c3665(const ims_mission *m,apollo_overlay_state *s,void *io){
 ai_dxch(s,MPAC);ai_dxch(s,m->scalsave);interrupts(m,io,0);return call(s,z37(m,03671u),m->bankcall);
}
/* 3672: TC DOWNFLAG (ADRES IMUSE). 3674: TC DOWNFLAG (ADRES RNDVZFLG). */
static enum exec_status r_c3672(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;return call(s,z37(m,03673u),m->downflag);}
static enum exec_status r_c3674(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;return call(s,z37(m,03675u),m->downflag);}
static int ch13_domain(const ims_mission *m,void *io){return (*channel(m,io,013)&034000u)==0u;}
/* 3676: CAF BIT11; EXTEND; WOR CHAN13; TC PHASCHNG (OCT 07024, OCT 20000,
 * 2CADR POSTAND). */
static enum exec_status r_c3676(const ims_mission *m,apollo_overlay_state *s,void *io){
 if(!ch13_domain(m,io))return EXEC_OUT_OF_SCOPE;
 ai_caf(s,K_BIT11);ch_wor(m,s,io,013);return call(s,z37(m,03702u),m->phaschng);
}
/* 3706: CAF OCT62; TC BANKCALL (CADR GOPERF1); its three returns 3711-3713
 * are TCF -3, -4, -5 back to 3706. */
static enum exec_status r_c3706(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;ai_caf(s,K_OCT62);return call(s,z37(m,03710u),m->bankcall);}
#define r_c3711 r_c3706
#define r_c3712 r_c3706
#define r_c3713 r_c3706
static enum exec_status r_scalprep(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;return scalprep(m,s);}
/* 3717 after FINETIME: RELINT; DXCH MPAC; CA BIT5; TS L; CA ZERO;
 * DAS MPAC; CS LOW5; MASK MPAC +1; XCH MPAC +1; MASK LOW5; CCS A;
 * INCR MPAC +2; TC MPAC +2. */
static enum exec_status r_c3717(const ims_mission *m,apollo_overlay_state *s,void *io){agc_word t;
 if(W[MPAC+2u]<010u||W[MPAC+2u]>07776u)return EXEC_OUT_OF_SCOPE;
 interrupts(m,io,1);
 ai_dxch(s,MPAC);ai_caf(s,K_BIT5);(void)ai_ts(s,1);ai_caf(s,0);ai_das(s,MPAC);
 ai_csf(s,K_LOW5);ai_mask(s,MPAC+1u);ai_xch(s,MPAC+1u);ai_maskf(s,K_LOW5);
 /* MASK LOW5 leaves A >= +0: CCS is + or +0. */
 if(ai_ccs(s,0)==0u)ai_incr(s,MPAC+2u);
 t=W[MPAC+2u];ai_tc(s,RET_MPAC2);return jump(s,t);
}
/* POSTAND +3: INHINT; CA ZERO; TS L; DXCH TIME2; TC SCALPREP. */
static enum exec_status postand3(const ims_mission *m,apollo_overlay_state *s,void *io){
 interrupts(m,io,0);ai_caf(s,0);(void)ai_ts(s,1);ai_dxch(s,TIME2);ai_tc(s,z37(m,03744u));return scalprep(m,s);
}
/* 3734 POSTAND (restart 2CADR): CS BIT11; EXTEND; WAND CHAN13; POSTAND +3. */
static enum exec_status r_postand(const ims_mission *m,apollo_overlay_state *s,void *io){
 if(!ch13_domain(m,io))return EXEC_OUT_OF_SCOPE;
 ai_csf(s,K_BIT11);ch_wand(m,s,io,013);return postand3(m,s,io);
}
/* 3744 (SCALPREP's retry return): TC POSTAND +3. */
static enum exec_status r_c3744(const ims_mission *m,apollo_overlay_state *s,void *io){ai_tc(s,z37(m,03745u));return postand3(m,s,io);}
/* 3745: EXTEND; DCS SCALSAVE; DAS MPAC; CAF BIT10; TC SHORTMP. */
static enum exec_status r_c3745(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;
 ai_dcs(s,m->scalsave);ai_das(s,MPAC);ai_caf(s,K_BIT10);return call(s,z37(m,03752u),m->shortmp);
}
/* 3752 after SHORTMP: CAF ZERO; TS MPAC +2; TC TPAGREE. */
static enum exec_status r_c3752(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;ai_caf(s,0);(void)ai_ts(s,MPAC+2u);return call(s,z37(m,03755u),m->tpagree);}
/* 3755 after TPAGREE: CCS MPAC; TC POSTCOM; TC POSTCOM; TC +1; CAF BIT10;
 * ADS MPAC; POSTCOM EXTEND; DCA TIME2SAV; DAS MPAC; TC TPAGREE. */
static enum exec_status r_c3755(const ims_mission *m,apollo_overlay_state *s,void *io){uint8_t c;(void)io;
 c=ai_ccs(s,MPAC);
 if(c<2u)ai_tc(s,z37(m,c==0u?03757u:03760u));
 else{if(c==2u)ai_tc(s,z37(m,03761u));ai_caf(s,K_BIT10);ai_ads(s,MPAC);}
 ai_dca(s,m->time2sav);ai_das(s,MPAC);return call(s,z37(m,03767u),m->tpagree);
}
/* 3767 after TPAGREE: DXCH MPAC; DAS TIME2; TC DOWNFLAG (ADRES NODOFLAG). */
static enum exec_status r_c3767(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;ai_dxch(s,MPAC);ai_das(s,TIME2);return call(s,z37(m,03772u),m->downflag);}
/* 3773 after DOWNFLAG: TC GOTOPOOH. */
static enum exec_status r_c3773(const ims_mission *m,apollo_overlay_state *s,void *io){(void)io;return call(s,z37(m,03774u),m->gotopooh);}

/* ---- Routine tables ---- */
#define FF_ROUTINES(X) X(zeroicdu,02,IMS_ZEROICDU)
#define B07_ROUTINES(X) B07A_ROUTINES(X) B07B_ROUTINES(X) B07C_ROUTINES(X)
#define B07A_ROUTINES(X) \
 X(imuzero,07,IMS_IMUZERO) X(c2723,07,02723u) X(c2747,07,02747u) X(modeexit,07,IMS_MODEEXIT) \
 X(imuzero2,07,IMS_IMUZERO2) X(imuzero3,07,IMS_IMUZERO3) X(c2777,07,02777u) \
 X(imucoars,07,IMS_IMUCOARS) X(c3007,07,03007u) X(coars,07,IMS_COARS) X(coars2,07,IMS_COARS2) \
 X(c3072,07,03072u) X(c3115,07,03115u) X(ca_ece,07,IMS_CA_ECE) X(setcoars,07,IMS_SETCOARS) \
 X(rndrefdr,07,IMS_RNDREFDR) X(imufine,07,IMS_IMUFINE) X(c3225,07,03225u) X(c3231,07,03231u) \
 X(imufined,07,IMS_IMUFINED) X(ifailok,07,IMS_IFAILOK) X(c3253,07,03253u) X(pfailok,07,IMS_PFAILOK)
#define B07B_ROUTINES(X) \
 X(noattoff,07,IMS_NOATTOFF) X(pipuse,07,IMS_PIPUSE) X(pipuse1,07,IMS_PIPUSE1) X(c3307,07,03307u) \
 X(pipfree,07,IMS_PIPFREE) X(c3321,07,03321u) X(imupulse,07,IMS_IMUPULSE) X(c3337,07,03337u) \
 X(c3357,07,03357u)
#define B07C_ROUTINES(X) \
 X(gwake,07,IMS_GWAKE) X(strtgyro,07,IMS_STRTGYRO) X(noreset,07,IMS_NORESET) \
 X(gyroexit,07,IMS_GYROEXIT) X(c3556,07,03556u) X(8192aug,07,IMS_8192AUG) X(endimu,07,IMS_ENDIMU) \
 X(imubad,07,IMS_IMUBAD) X(badend,07,IMS_BADEND) X(goodend,07,IMS_GOODEND) X(c3702,07,03702u) \
 X(aotstall,07,IMS_AOTSTALL) X(radstall,07,IMS_RADSTALL) X(imustall,07,IMS_IMUSTALL) X(c3736,07,03736u) \
 X(gomanur,07,IMS_GOMANUR)
#define R02_ROUTINES(X) X(r02both,04,IMS_R02BOTH) X(c3267,04,03267u) X(r02zero,04,IMS_R02ZERO) X(c3272,04,03272u)
#define P06_ROUTINES(X) \
 X(p06,037,IMS_P06) X(prestand,037,IMS_PRESTAND) X(c3664,037,03664u) X(c3665,037,03665u) \
 X(c3672,037,03672u) X(c3674,037,03674u) X(c3676,037,03676u) X(c3706,037,03706u) \
 X(c3711,037,03711u) X(c3712,037,03712u) X(c3713,037,03713u) X(scalprep,037,IMS_SCALPREP) \
 X(c3717,037,03717u) X(postand,037,IMS_POSTAND) X(c3744,037,03744u) X(c3745,037,03745u) \
 X(c3752,037,03752u) X(c3755,037,03755u) X(c3767,037,03767u) X(c3773,037,03773u)
#define WRAP(name,bank,addr) \
 static enum exec_status lm_##name(const agc_native_program *p,apollo_overlay_state *s,void *io){program_now=p;return r_##name(&LM,s,io);} \
 static enum exec_status cm_##name(const agc_native_program *p,apollo_overlay_state *s,void *io){program_now=p;return r_##name(&CM,s,io);}
#if !defined IMS_C64_PART
FF_ROUTINES(WRAP) B07_ROUTINES(WRAP) R02_ROUTINES(WRAP) P06_ROUTINES(WRAP)
#elif IMS_C64_PART==1
FF_ROUTINES(WRAP) B07A_ROUTINES(WRAP) R02_ROUTINES(WRAP)
#elif IMS_C64_PART==2
B07B_ROUTINES(WRAP) P06_ROUTINES(WRAP)
#else
B07C_ROUTINES(WRAP)
#endif
/* Physical keys: fixed-fixed 04000-05777 is bank 02; Comanche's R02 is in
 * bank 07 and its P05P06 in bank 26. */
#define LM_KEY(bank,z) (uint16_t)((bank)==02?2u*1024u+((z)-04000u):(bank)*1024u+((z)-02000u))
#define CM_BANK(bank) ((bank)==04?07u:(bank)==037?026u:(unsigned)(bank))
#define CM_Z(bank,z) ((bank)==02?(z)+042u:(bank)==04?(z)+0377u:(bank)==037?(z)+3u:CM7(z))
#define CM_KEY(bank,z) (uint16_t)((bank)==02?2u*1024u+(CM_Z(bank,z)-04000u):CM_BANK(bank)*1024u+(CM_Z(bank,z)-02000u))
#define LM_ENTRY(name,bank,addr) {LM_KEY(bank,addr),lm_##name},
#define CM_ENTRY(name,bank,addr) {CM_KEY(bank,addr),cm_##name},
#define LM_PHYS(name,bank,addr) LM_KEY(bank,addr),
#define CM_PHYS(name,bank,addr) CM_KEY(bank,addr),
const uint16_t ims_routine_keys[2u*IMS_LM_ROUTINES]={
 FF_ROUTINES(LM_PHYS) R02_ROUTINES(LM_PHYS) B07_ROUTINES(LM_PHYS) P06_ROUTINES(LM_PHYS)
 FF_ROUTINES(CM_PHYS) B07_ROUTINES(CM_PHYS) R02_ROUTINES(CM_PHYS) P06_ROUTINES(CM_PHYS)};
/* Sorted by physical address, the registry's lookup order: Luminary's R02
 * (bank 04) comes before bank 07, Comanche's (bank 07) after it. The C64
 * diagnostics link a third of the routines at a time (IMS_C64_PART 1:
 * fixed-fixed, IMUZERO to PFAILOK, R02; 2: NOATTOFF to the TPAGREE return,
 * P06; 3: GWAKE to GOMANUR) so each program fits in memory. */
#if !defined IMS_C64_PART
static const agc_routine_entry lm_entries[]={FF_ROUTINES(LM_ENTRY) R02_ROUTINES(LM_ENTRY) B07_ROUTINES(LM_ENTRY) P06_ROUTINES(LM_ENTRY)};
static const agc_routine_entry cm_entries[]={FF_ROUTINES(CM_ENTRY) B07_ROUTINES(CM_ENTRY) R02_ROUTINES(CM_ENTRY) P06_ROUTINES(CM_ENTRY)};
#elif IMS_C64_PART==1
static const agc_routine_entry lm_entries[]={FF_ROUTINES(LM_ENTRY) R02_ROUTINES(LM_ENTRY) B07A_ROUTINES(LM_ENTRY)};
static const agc_routine_entry cm_entries[]={FF_ROUTINES(CM_ENTRY) B07A_ROUTINES(CM_ENTRY) R02_ROUTINES(CM_ENTRY)};
#elif IMS_C64_PART==2
static const agc_routine_entry lm_entries[]={B07B_ROUTINES(LM_ENTRY) P06_ROUTINES(LM_ENTRY)};
static const agc_routine_entry cm_entries[]={B07B_ROUTINES(CM_ENTRY) P06_ROUTINES(CM_ENTRY)};
#else
static const agc_routine_entry lm_entries[]={B07C_ROUTINES(LM_ENTRY)};
static const agc_routine_entry cm_entries[]={B07C_ROUTINES(CM_ENTRY)};
#endif
const agc_routine_table lm_imu_mode_switching_routines={lm_entries,(uint16_t)(sizeof lm_entries/sizeof *lm_entries)};
const agc_routine_table cm_imu_mode_switching_routines={cm_entries,(uint16_t)(sizeof cm_entries/sizeof *cm_entries)};
