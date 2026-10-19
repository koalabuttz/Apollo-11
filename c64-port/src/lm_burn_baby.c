/* SPDX-License-Identifier: MIT */
/* Luminary099 BURN,_BABY,_BURN_--_MASTER_IGNITION_ROUTINE (bank 36, with
 * parts in banks 35, 17 and 27 and KILLTASK in fixed-fixed) as native
 * routines of the control-transfer ABI; see include/lm_burn_baby.h and
 * docs/LM-BURN-BABY.md.
 * Basic code: each instruction is one ai_* effect on the image in source
 * order, under a case label at its physical address; a TC or TCF within the
 * file is a goto, one to another file ends the run with Q and Z set. A
 * transfer to an address computed at run time (INDEX WHICH; TCF n, a TC Q
 * return, DTCB) sets Z and lets the step loop find the file's word there.
 * Interpretive code: each order-pair word is a case of its own, running the
 * compiled lm_interp_steps/lm_interp_orders paths with the pair's words as
 * literal arguments. */
#include "lm_burn_baby.h"
#include "flight_undo.h"
#include "agc_image_ops.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_w.h"
#if !defined(LMBB_C64_GROUP)
#define BB_G1 1
#define BB_G2 1
#else
#define BB_G1 (LMBB_C64_GROUP==1)
#define BB_G2 (LMBB_C64_GROUP==2)
#endif
/* Coverage hook for the host test (empty in flight builds): path n for each
 * instruction reached (bank 36 a - 02022; bank 35 01200 + a - 03747; bank
 * 17 01240 + a - 02000; bank 27 01300 + a - 02200; fixed-fixed 01350 + a -
 * 06027), 01400 + LOC - 02152 for each order pair, and 01500 + n for each
 * outcome of the file's decisions (BR). */
#ifdef LMBB_COUNT_PATHS
extern unsigned lmbb_paths[];
#define BB_PATH(n) ((void)++lmbb_paths[n])
#else
#define BB_PATH(n) ((void)0)
#endif
#define BR(n) BB_PATH(01500u+(n))

/* Central registers, counters and erasable (E7 under EBANK= WHICH, E6
 * under EBANK= AOSQ, E3 under EBANK= LST2). */
#define A 0u
#define LREG 1u
#define QREG 2u
#define EBANK 3u
#define FBANK 4u
#define BBANK 6u
#define TIME2 024u
#define TIME1 025u
#define THRUST 055u
#define ITEMP1 061u
#define ITEMP2 062u
#define ITEMP3 063u
#define ITEMP4 064u
#define EDOP 023u
#define NEWJOB 067u
#define FLAGWRD2 076u
#define FLAGWRD5 0101u
#define FLAGWRD7 0103u
#define FLAGWRD8 0104u
#define FLAGWRD9 0105u
#define FLGWRD10 0106u
#define DAPBOOLS 0111u
#define FIXLOC 0120u
#define MPAC 0154u
#define LOC 0164u
#define BANKSET 0165u
#define PUSHLOC 0166u
#define NVWORD2 0371u        /* NVWORD +2 */
#define MPHASE1 0752u        /* -PHASE1 */
#define MPHASE2 0754u        /* -PHASE2 */
#define MPHASE3 0756u        /* -PHASE3 */
#define PHASE3 0757u
#define MPHASE4 0760u        /* -PHASE4 */
#define PHASE5 0763u
#define MPHASE6 0764u        /* -PHASE6 */
#define PHASE6 0765u
#define TBASE1 01053u
#define TBASE4 01061u
#define TBASE6 01065u
#define NVWORD1 01067u
#define P40RET 01142u        /* P40/RET */
#define DISPDEX 01163u
#define TEMPR60 01164u
#define AVEGEXIT 01252u      /* AVEGEXIT = AVGEXIT */
#define TEVENT 01344u
#define WCHPHASE 01351u
#define LST2 01410u          /* E3 */
#define LST21 01411u         /* LST2 +1 */
#define IGNAOSQ 01412u       /* E6 */
#define IGNAOSR 01413u       /* E6 */
#define ZOOMTIME 01422u
#define TIG 01441u
#define TTOGO 01453u
#define WHICH 01455u
#define SAVET30 01477u       /* SAVET-30 */
#define DVTOTAL 01507u
#define DVTOTAL1 01510u
#define GOBLTIME 01511u
#define DVCNTR 01515u
#define TGO 01516u
#define AOSQ 01537u          /* E6 */
#define AOSR 01541u          /* E6 */
#define PIPTIME1 01560u      /* PIPTIME +1 */
#define WCHPHOLD 01621u
#define FLPASS0 01623u
/* Fixed constants of other files (checked against the rope by
 * tools/lm_burn_baby_source.py). */
#define K_ZERO 0u
#define K_ONE 01u
#define K_TWO 02u
#define K_THREE 03u
#define K_FOUR 04u
#define K_BIT1 01u
#define K_BIT4 010u
#define K_BIT5 020u
#define K_BIT9 0400u
#define K_BIT11 02000u
#define K_BIT13 010000u
#define K_LOW5 037u
#define K_LOW10 01777u
#define K_PRIO12 012000u
#define K_PRIO13 013000u
#define K_PRIO16 016000u
#define K_PRIO17 017000u
#define K_PRIO20 020000u
#define K_PRIO27 027000u
#define K_PRIO30 030000u
#define K_EBANK6 03000u
#define K_EBANK7 03400u
#define K_CNTDNDEX 017u      /* LOW4 */
#define K_BLANKDEX 02u       /* TWO */
#define K_VB99DEX 013u       /* ELEVEN */
#define K_VB97DEX 035u       /* OCT35 */
#define K_5SEC 0764u
#define K_1SEC 0144u
#define K_V16N85B 04125u
#define K_V05N09 01211u
#define K_OCT23 023u
#define K_APSFLBIT 010000u
#define K_IGNFLBIT 010000u
#define K_ASTNBIT 04000u
#define K_ENGONBIT 0100u
#define K_FLUNDBIT 01000u
#define K_LETABBIT 0400u
#define K_SWANDBIT 02000u
#define K_PULSES 040000u
#define K_NOTHRBIT 04000u
#define K_DRIFTBIT 0200u
#define K_IMPULBIT 0400u
#define K_ULLAGER 040u
#define K_TCTSKOVR 05261u
static const agc_word K_NEG0[2] __attribute__((unused))={077777u,0u};       /* NEG0, ZERO */
static const agc_word K_5SECDP[2] __attribute__((unused))={0u,0764u};        /* 5SECDP (bank 36 03755) */
/* Other files. */
#define X_BANKCALL 04616u
#define X_POSTJUMP 04635u
#define X_BANKJUMP 04640u
#define X_MAKECADR 04645u
#define X_IBNKCALL 04674u
#define X_NOVAC 05072u
#define X_FINDVAC 05105u
#define X_ENDOFJOB 05155u
#define X_TWIDDLE 05173u
#define X_WAITLIST 05203u
#define X_FIXDELAY 05221u
#define X_TASKOVER 05261u
#define X_LONGCALL 05277u
#define X_CHECKMM 05321u
#define X_2PHSCHNG 05327u
#define X_PHASCHNG 05353u
#define X_UPFLAG 05504u
#define X_COMFLAG 05511u
#define X_DOWNFLAG 05516u
#define X_DEBIT 05522u
#define X_ALARM 05567u
#define X_GOTOPOOH 06001u
#define X_INTPRET 06037u
#define X_CHANG2 05126u
#define X_TPAGREE 07257u
#define NEWOPS 06070u

/* The ignition tables P12TABLE .. ABRTABLE (bank 36 02022-02125), read by
 * INDEX WHICH; CA/CS/DCA n: VN, four TCFs, the SPOT TCF, the DEC, the
 * 2CADR, three TCFs, and one more in P40TABLE and P42TABLE (P41TABLE -5
 * overlaps P40TABLE). The TCF words also run as code (below). */
#define TABLE0 02022u
static const agc_word TABLE[0104] __attribute__((unused))={
 001512u,012325u,012612u,013062u,015261u,012146u,000000u,003770u,064067u,012376u,
 012563u,012521u,001450u,012325u,012614u,013047u,015261u,012146u,004300u,003666u,
 074066u,012366u,012563u,012504u,013123u,012151u,077776u,003376u,074066u,012400u,
 012567u,001450u,012322u,012614u,013047u,015261u,012146u,005120u,003666u,074066u,
 012366u,012563u,012541u,013420u,001476u,012325u,012612u,012777u,015261u,012151u,
 004300u,003770u,064067u,012376u,012563u,012455u,001477u,012325u,012612u,013062u,
 015261u,012120u,012121u,012122u,012123u,012376u,012563u,012531u,
};

enum {GO=0,STOP=1,REFUSE=2};
/* Helpers stay out of line: the C64 build has to fit the many call sites. */
#define NOINL __attribute__((noinline,unused))
#define RUN_LIMIT 600u
static agc_word oc(agc_register a){return (agc_word)((a&037777u)|((a>>1)&040000u));}
static int tc_word(agc_register w){return w>=02000u&&w<=07777u;}

/* ---- Interpretive helpers (as lm_lunar_landing.c) -------------------- */
enum {VLOAD=06505,DLOAD=06052,DSU=07005,ABVAL=03176,MXV=07334,CALLITA=02507,SWITCH=02516};
/* The fixed operand words ADDRWD names after address formation (n words),
 * or *f=0 for an erasable operand: D29.9SEC (bank 36 03142) only. */
static const agc_word D299SEC[2]={0u,05656u};
static int operand(const apollo_overlay_state *s,unsigned n,const agc_word **f){
 unsigned a=W[0116],bank=W[4]>>10;
 *f=0;
 if(a<02000u)return 1;
 if(bank==036u&&a==03142u&&n==2u){*f=D299SEC;return 1;}
 return 0;
}
#define K(n) do{if(!operand(s,(n),&k))return REFUSE;}while(0)
/* DANZIG at the end of a pair: NEWJOB +0 goes on to NEWOPS; positive goes
 * to CHANG2 (CA BANKSET; TS BBANK; CCS EDOP; CCS NEWJOB; TCF CHANG2). */
static NOINL int next(apollo_overlay_state *s){
 agc_word j=W[NEWJOB];
 if(j==0u){BR(0);lmi_danzig_next(s);return GO;}
 if(j>037777u)return REFUSE;
 BR(1);ai_ca(s,BANKSET);(void)ai_ts(s,6);(void)ai_ccs(s,EDOP);(void)ai_ccs(s,NEWJOB);W[5]=X_CHANG2;return STOP;
}
#define NEXT return next(s)
#define DO(x) do{if(x)return REFUSE;}while(0)
static NOINL void right(apollo_overlay_state *s){lmi_danzig_right(s);}
static NOINL void opnd(apollo_overlay_state *s,agc_word word,agc_word target){
 if(word&040000u)lmi_address_pushup(s,word,target);else lmi_direct(s,word,target);
}
static NOINL void shorts(apollo_overlay_state *s){lmi_short_dispatch(s);if(W[5]==02017u)lmi_shortt(s);else lmi_shortv(s);}
static NOINL int store(apollo_overlay_state *s,agc_word code){lmi_store_code(s,code);return next(s);}
static NOINL int stodl(apollo_overlay_state *s,agc_word code,agc_word word){const agc_word *k;
 lmi_store_code(s,code);lmi_dodload(s);lmi_diradres(s,word,DLOAD);K(2);lmi_dload_op(s,k);return next(s);
}
static NOINL int stovl(apollo_overlay_state *s,agc_word code,agc_word word){const agc_word *k;
 lmi_store_code(s,code);lmi_dovload(s);lmi_diradres(s,word,VLOAD);K(6);lmi_vload(s,k);return next(s);
}
/* STCALL: the store, then 15BITADR to CALL and the GOTO. */
static NOINL int stcall(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_15bitadr(s,word,CALLITA);lmi_call(s);lmi_goto(s);return GO;
}
static NOINL int dload(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,DLOAD);K(2);lmi_dload_op(s,k);return GO;}
static NOINL int vload(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,VLOAD);K(6);lmi_vload(s,k);return GO;}
static NOINL int dsu(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,DSU);K(2);lmi_dsu(s,k);return GO;}
static NOINL void mxv(apollo_overlay_state *s,agc_word word){opnd(s,word,MXV);lmi_mxv(s,0);}
static NOINL int call(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,CALLITA);lmi_call(s);lmi_goto(s);return GO;}
static NOINL int abval(apollo_overlay_state *s){lmi_unary(s,ABVAL);if(lmi_abvalabs(s)){BR(2);return STOP;}return GO;}
static NOINL int exit_(apollo_overlay_state *s){lmi_newops(s,077776);lmi_exit(s);return GO;}
#define STEP(x) do{int r_=(x);if(r_)return r_;}while(0)

/* One order-pair word at LOC (Z at NEWOPS). */
static int interpretive(apollo_overlay_state *s,lm_bb_io *io){
 (void)io;
 BB_PATH(01400u+W[LOC]-02152u);
 switch(W[LOC]){
#if BB_G1
 /* P41SPOT: DLOAD DSU TIG D29.9SEC; STCALL TDEC1 INITCDUW; BOFF CALL
  * MUNFLAG GOMIDAV CSMPREC; VLOAD MXV VATT1 REFSMMAT; VSR1; STOVL V(CSM)
  * RATT1; VSL4 MXV REFSMMAT; STCALL R(CSM) MUNGRAV; STODL G(CSM) TAT;
  * STORE TDEC1; GOMIDAV CALRB MIDTOAV1 (CALRB is CALL; MIDTOAV1 returns
  * through BANKJUMP to IRETURN1, the basic word after it, or one past). */
 case 02152:lmi_newops(s,045345);DO(dload(s,003442));right(s);DO(dsu(s,035143));NEXT;
 case 02155:return stcall(s,034041,061104);
 case 02157:lmi_newops(s,045014);lmi_misc(s,003347,SWITCH);
  if(lmi_switch(s,&io->control)){BR(3);lmi_swbranch(s,074200);return GO;}
  BR(4);right(s);return call(s,027043);
 case 02163:lmi_newops(s,064375);DO(vload(s,000025));right(s);mxv(s,001734);NEXT;
 case 02166:lmi_newops(s,077762);shorts(s);NEXT;
 case 02167:return stovl(s,025726,000017);
 case 02171:lmi_newops(s,064312);shorts(s);right(s);mxv(s,001734);NEXT;
 case 02173:return stcall(s,035720,067130);
 case 02175:return stodl(s,016317,000015);
 case 02177:return store(s,000041);
 case 02200:lmi_newops(s,077624);return call(s,027557);
 /* VLOAD ABVAL VN1; STORE ABVEL; EXIT. */
 case 02233:lmi_newops(s,051575);DO(vload(s,003553));right(s);STEP(abval(s));NEXT;
 case 02235:return store(s,003472);
 case 02236:return exit_(s);
#endif
 default:return REFUSE;
 }
}

/* ---- Basic helpers ------------------------------------------------------ */
static NOINL int out(apollo_overlay_state *s,agc_word q,agc_word z){ai_tc(s,q);W[5]=z;return STOP;}
static NOINL int jump(apollo_overlay_state *s,agc_word z){W[5]=z;return STOP;}
/* Continue at z, which the step loop runs if it is the file's word under
 * FB (a table TCF, or code) and leaves the file at otherwise. */
static NOINL int to(apollo_overlay_state *s,agc_word z){W[5]=z;return GO;}
static NOINL void intpret(apollo_overlay_state *s,lm_bb_io *io,agc_word ret){ai_tc(s,ret);W[5]=X_INTPRET;lmi_intpret(s,&io->control);}
/* INDEX WHICH: the table WHICH names, one of the six its producers store
 * (P12ADRES, P40ADRES, P41ADRES, P42ADRES, P63ADRES, WHICHADR). */
static NOINL int which(const apollo_overlay_state *s,agc_word *w){
 agc_word v=ai_word(s,WHICH);
 if(v!=02022u&&v!=02036u&&v!=02046u&&v!=02061u&&v!=02076u&&v!=02112u)return 0;
 *w=v;return 1;
}
#define WHICHW(w) do{if(!which(s,&(w)))return REFUSE;}while(0)
#define TBL(w,n) TABLE[(w)+(n)-TABLE0]
/* Channel orders, as yaAGC's. */
static NOINL void rand_(apollo_overlay_state *s,agc_word chan){W[0]=agc_extend((agc_word)(oc(W[0])&chan));}
static NOINL void write_(apollo_overlay_state *s,agc_word *chan){*chan=oc(W[0]);}
static NOINL void wor(apollo_overlay_state *s,agc_word *chan){agc_word v=(agc_word)(oc(W[0])|*chan);*chan=v;W[0]=agc_extend(v);}
/* TC Q: the word in Q runs as TC (Z = Q, Q = 3); Q must be a TC word into
 * fixed memory. */
static NOINL int tcq(apollo_overlay_state *s){agc_register q=W[QREG];if(!tc_word(q))return REFUSE;ai_tc(s,3u);return to(s,(agc_word)q);}
/* DTCB (DXCH Z) executed at `at`: Z,BB <-> A,L. */
static NOINL void dtcb(apollo_overlay_state *s,agc_word at){agc_word z=agc_correct(W[0]),bb=agc_correct(W[1]);
 W[0]=(agc_word)(at+1u);W[1]=agc_extend(W[6]);W[5]=z;
 W[6]=(agc_word)(bb&076007u);W[3]=(agc_word)((bb&7u)<<8);W[4]=(agc_word)(bb&076000u);
}
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
/* Physical case labels: bank b (octal digits) address a; FF(a) for
 * fixed-fixed. */
#define PH(b,a) (uint16_t)(0##b*1024u+((a)-02000u))
#define PATHOF(b,a) (0##b==036?(a)-02022u:0##b==035?01200u+(a)-03747u:0##b==017?01240u+(a)-02000u:01300u+(a)-02200u)
#define AT(b,a) case PH(b,a): L##b##_##a: __attribute__((unused)) BB_PATH(PATHOF(b,a))
#define FF(a) case (a): LFF_##a: __attribute__((unused)) BB_PATH(01350u+(a)-06027u)
#define GOTO(b,a) goto L##b##_##a
#define TC(r,b,a) do{ai_tc(s,(r));GOTO(b,a);}while(0)

/* Basic code at Z (physical: the bank FB selects, or fixed-fixed). */
static int basic(apollo_overlay_state *s,lm_bb_io *io,uint16_t ph){
 agc_word w=0;uint8_t c;
 (void)w;
 switch(ph){
#if BB_G1
 /* The tables' TCF words (and ABRTABLE's NOOPs), run by INDEX WHICH;
  * TCF n and BZF 12. */
 AT(36,02023);GOTO(36,02325);
 AT(36,02024);GOTO(36,02612);
 AT(36,02025);GOTO(36,03062);
 AT(36,02026);return jump(s,X_TASKOVER);
 AT(36,02027);GOTO(36,02146);
 AT(36,02033);GOTO(36,02376);
 AT(36,02034);GOTO(36,02563);
 AT(36,02035);GOTO(36,02521);
 AT(36,02037);GOTO(36,02325);
 AT(36,02040);GOTO(36,02614);
 AT(36,02041);GOTO(36,03047);
 AT(36,02042);return jump(s,X_TASKOVER);
 AT(36,02043);GOTO(36,02146);
 AT(36,02047);GOTO(36,02366);
 AT(36,02050);GOTO(36,02563);
 AT(36,02051);GOTO(36,02504);
 AT(36,02052);GOTO(36,03123);
 AT(36,02053);GOTO(36,02151);
 AT(36,02057);GOTO(36,02400);
 AT(36,02060);GOTO(36,02567);
 AT(36,02062);GOTO(36,02322);
 AT(36,02063);GOTO(36,02614);
 AT(36,02064);GOTO(36,03047);
 AT(36,02065);return jump(s,X_TASKOVER);
 AT(36,02066);GOTO(36,02146);
 AT(36,02072);GOTO(36,02366);
 AT(36,02073);GOTO(36,02563);
 AT(36,02074);GOTO(36,02541);
 AT(36,02075);return jump(s,03420u); /* TCF P42STAGE (P40-P47) */
 AT(36,02077);GOTO(36,02325);
 AT(36,02100);GOTO(36,02612);
 AT(36,02101);GOTO(36,02777);
 AT(36,02102);return jump(s,X_TASKOVER);
 AT(36,02103);GOTO(36,02151);
 AT(36,02107);GOTO(36,02376);
 AT(36,02110);GOTO(36,02563);
 AT(36,02111);GOTO(36,02455);
 AT(36,02113);GOTO(36,02325);
 AT(36,02114);GOTO(36,02612);
 AT(36,02115);GOTO(36,03062);
 AT(36,02116);return jump(s,X_TASKOVER);
 AT(36,02117);GOTO(36,02120);
 AT(36,02120);GOTO(36,02121);
 AT(36,02121);GOTO(36,02122);
 AT(36,02122);GOTO(36,02123);
 AT(36,02123);GOTO(36,02376);
 AT(36,02124);GOTO(36,02563);
 AT(36,02125);GOTO(36,02531);
 /* BURNBABY TC PHASCHNG (OCT 04024); CAF ZERO; TS DVTOTAL; TS DVTOTAL +1;
  * TC BANKCALL (CADR P40AUTO). */
 AT(36,02126);return out(s,02127,X_PHASCHNG);
 AT(36,02130);ai_caf(s,K_ZERO);
 AT(36,02131);(void)ai_ts(s,DVTOTAL);   /* TS of +0 never skips */
 AT(36,02132);(void)ai_ts(s,DVTOTAL1);
 AT(36,02133);return out(s,02134,X_BANKCALL);
 /* B*RNB*B* EXTEND; DCA TIG; DXCH GOBLTIME; INHINT; TC IBNKCALL (CADR
  * ENGINOF3); RELINT; INDEX WHICH; TCF 5. */
 AT(36,02135);
 AT(36,02136);ai_dca(s,TIG);
 AT(36,02137);ai_dxch(s,GOBLTIME);
 AT(36,02140);io->control.interrupt_enabled=0;
 AT(36,02141);return out(s,02142,X_IBNKCALL);
 AT(36,02143);io->control.interrupt_enabled=1;
 AT(36,02144);WHICHW(w);
 AT(36,02145);return to(s,(agc_word)(w+5u));
 /* P40SPOT CS CNTDNDEX; TC BANKCALL (CADR STCLOK2). */
 AT(36,02146);ai_csf(s,K_CNTDNDEX);
 AT(36,02147);return out(s,02150,X_BANKCALL);
 /* P41SPOT TC INTPRET. */
 AT(36,02151);intpret(s,io,02152);return GO;
 /* MIDTOAV1's return (IRETURN1): TCF CALLT-35; its error return (IRETURN1
  * +1): EXTEND; DCA PIPTIME1; DXCH TIG; EXTEND; DCA D29.9SEC; DAS TIG. */
 AT(36,02202);GOTO(36,02211);
 AT(36,02203);
 AT(36,02204);ai_dca(s,PIPTIME1);
 AT(36,02205);ai_dxch(s,TIG);
 AT(36,02206);
 AT(36,02207);ai_dcaf(s,D299SEC);
 AT(36,02210);ai_das(s,TIG);
 /* CALLT-35 DXCH MPAC; DXCH SAVET-30; EXTEND; DCS 5SECDP; DAS SAVET-30;
  * EXTEND; DCA SAVET-30; TC LONGCALL (2CADR TIG-35); TC PHASCHNG (OCT
  * 20254); TC CHECKMM (DEC 63); TCF ENDOFJOB; CS CNTDNDEX; TS DISPDEX;
  * TC INTPRET. */
 AT(36,02211);ai_dxch(s,MPAC);
 AT(36,02212);ai_dxch(s,SAVET30);
 AT(36,02213);
 AT(36,02214);ai_dcsf(s,K_5SECDP);
 AT(36,02215);ai_das(s,SAVET30);
 AT(36,02216);
 AT(36,02217);ai_dca(s,SAVET30);
 AT(36,02220);return out(s,02221,X_LONGCALL);
 AT(36,02223);return out(s,02224,X_PHASCHNG);
 AT(36,02225);return out(s,02226,X_CHECKMM);
 AT(36,02227);return jump(s,X_ENDOFJOB);
 AT(36,02230);ai_csf(s,K_CNTDNDEX);
 AT(36,02231);(void)ai_ts(s,DISPDEX);   /* -CNTDNDEX: no overflow */
 AT(36,02232);intpret(s,io,02233);return GO;
 AT(36,02237);return jump(s,X_ENDOFJOB);
 /* TIG-35 CAF 5SEC; TC TWIDDLE (ADRES TIG-30); TC PHASCHNG (OCT 40154);
  * CS BLANKDEX; TS DISPDEX; INDEX WHICH; CS 6; EXTEND; BZMF TASKOVER;
  * CAF 4.9SEC; TC TWIDDLE (ADRES TIG-30.1); CAF PRIO17; TC NOVAC (2CADR
  * P41BLANK); TCF TASKOVER. */
 AT(36,02240);ai_caf(s,K_5SEC);
 AT(36,02241);return out(s,02242,X_TWIDDLE);
 AT(36,02243);return out(s,02244,X_PHASCHNG);
 AT(36,02245);ai_csf(s,K_BLANKDEX);
 AT(36,02246);(void)ai_ts(s,DISPDEX);
 AT(36,02247);WHICHW(w);
 AT(36,02250);ai_csf(s,TBL(w,6u));
 AT(36,02251);
 AT(36,02252);if(ai_bzmf(s)){BR(5);return jump(s,X_TASKOVER);}
 BR(6);
 AT(36,02253);ai_caf(s,0752u);          /* 4.9SEC */
 AT(36,02254);return out(s,02255,X_TWIDDLE);
 AT(36,02256);ai_caf(s,K_PRIO17);
 AT(36,02257);return out(s,02260,X_NOVAC);
 AT(36,02262);return jump(s,X_TASKOVER);
 /* P41BLANK TC BANKCALL (CADR CLEANDSP); TCF ENDOFJOB. */
 AT(36,02263);return out(s,02264,X_BANKCALL);
 AT(36,02265);return jump(s,X_ENDOFJOB);
 /* TIG-30.1 CAF PRIO17; TC NOVAC (2CADR TIG-30A); TCF TASKOVER. */
 AT(36,02266);ai_caf(s,K_PRIO17);
 AT(36,02267);return out(s,02270,X_NOVAC);
 AT(36,02272);return jump(s,X_TASKOVER);
 /* TIG-30A CAF V16N85B; TC BANKCALL (CADR REGODSP). */
 AT(36,02273);ai_caf(s,K_V16N85B);
 AT(36,02274);return out(s,02275,X_BANKCALL);
 /* TIG-30 CAF S24.9SEC; TC TWIDDLE (ADRES TIG-5); CS CNTDNDEX; TS DISPDEX;
  * INDEX WHICH; CAF 6; EXTEND; BZMF ULLGNOT; TS SAVET-30; TC TWIDDLE
  * (ADRES ULLGTASK); CA THREE; TS L; CS THREE; DXCH -PHASE1; CS TIME1;
  * TS TBASE1; INDEX WHICH; TCF 1. */
 AT(36,02276);ai_caf(s,04672u);         /* S24.9SEC */
 AT(36,02277);return out(s,02300,X_TWIDDLE);
 AT(36,02301);ai_csf(s,K_CNTDNDEX);
 AT(36,02302);(void)ai_ts(s,DISPDEX);
 AT(36,02303);WHICHW(w);
 AT(36,02304);ai_caf(s,TBL(w,6u));
 AT(36,02305);
 AT(36,02306);if(ai_bzmf(s)){BR(7);GOTO(36,02325);}
 BR(8);
 AT(36,02307);(void)ai_ts(s,SAVET30);   /* a positive DEC word */
 AT(36,02310);return out(s,02311,X_TWIDDLE);
 AT(36,02312);ai_caf(s,K_THREE);
 AT(36,02313);(void)ai_ts(s,LREG);
 AT(36,02314);ai_csf(s,K_THREE);
 AT(36,02315);ai_dxch(s,MPHASE1);
 AT(36,02316);ai_cs(s,TIME1);
 AT(36,02317);(void)ai_ts(s,TBASE1);    /* CS of a 15-bit word: no overflow */
 AT(36,02320);WHICHW(w);
 AT(36,02321);return to(s,(agc_word)(w+1u));
 /* WANTAPS CS FLGWRD10; MASK APSFLBIT; ADS FLGWRD10. */
 AT(36,02322);ai_cs(s,FLGWRD10);
 AT(36,02323);ai_maskf(s,K_APSFLBIT);
 AT(36,02324);ai_ads(s,FLGWRD10);
 /* ULLGNOT EXTEND; INDEX WHICH; DCA 7; DXCH AVEGEXIT; CAF TWO; TS L;
  * CS TWO; DXCH -PHASE4; CS TIME1; TS TBASE4. REDO2.17 EXTEND; DCA NEG0;
  * DXCH -PHASE2. REDO4.2 CCS PHASE5; TCF TASKOVER; TC POSTJUMP (CADR
  * PREREAD); ULLGTASK ... */
 AT(36,02325);
 AT(36,02326);WHICHW(w);
 AT(36,02327);ai_dcaf(s,&TBL(w,7u));
 AT(36,02330);ai_dxch(s,AVEGEXIT);
 AT(36,02331);ai_caf(s,K_TWO);
 AT(36,02332);(void)ai_ts(s,LREG);
 AT(36,02333);ai_csf(s,K_TWO);
 AT(36,02334);ai_dxch(s,MPHASE4);
 AT(36,02335);ai_cs(s,TIME1);
 AT(36,02336);(void)ai_ts(s,TBASE4);
 AT(36,02337);
 AT(36,02340);ai_dcaf(s,K_NEG0);
 AT(36,02341);ai_dxch(s,MPHASE2);
 AT(36,02342);c=ai_ccs(s,PHASE5);BR(9u+c);
  /* A negative PHASE5 runs the CADR word as MASK: no producer stores one. */
  if(c==2u)return REFUSE;
  if(c==3u)GOTO(36,02346);
  if(c==1u)GOTO(36,02344);
 AT(36,02343);return jump(s,X_TASKOVER);
 AT(36,02344);return out(s,02345,X_POSTJUMP);
 /* ULLGTASK TC ONULLAGE; TC PHASCHNG (OCT 1); TCF TASKOVER. */
 AT(36,02346);TC(02347,36,02657);
 AT(36,02347);return out(s,02350,X_PHASCHNG);
 AT(36,02351);return jump(s,X_TASKOVER);
 /* TIG-5 EXTEND; DCA NEG0; DXCH -PHASE3; CAF 5SEC; TC TWIDDLE (ADRES
  * TIG-0); TC DOWNFLAG (ADRES IGNFLAG); TC DOWNFLAG (ADRES ASTNFLAG);
  * INDEX WHICH; TCF 11. */
 AT(36,02352);
 AT(36,02353);ai_dcaf(s,K_NEG0);
 AT(36,02354);ai_dxch(s,MPHASE3);
 AT(36,02355);ai_caf(s,K_5SEC);
 AT(36,02356);return out(s,02357,X_TWIDDLE);
 AT(36,02360);return out(s,02361,X_DOWNFLAG);
 AT(36,02362);return out(s,02363,X_DOWNFLAG);
 AT(36,02364);WHICHW(w);
 AT(36,02365);return to(s,(agc_word)(w+011u));
 /* P40SJUNK CCS PHASE3; TCF DISPCHNG; CAF PRIO20; TC FINDVAC (2CADR
  * S40.13); TC PHASCHNG (OCT 00053). A negative or -0 PHASE3 would run the
  * FINDVAC with A = |PHASE3| -1 or the 2CADR word: no producer. */
 AT(36,02366);c=ai_ccs(s,PHASE3);BR(13u+c);
  if(c>=2u)return REFUSE;
  if(c==1u)GOTO(36,02370);
 AT(36,02367);GOTO(36,02376);
 AT(36,02370);ai_caf(s,K_PRIO20);
 AT(36,02371);return out(s,02372,X_FINDVAC);
 AT(36,02374);return out(s,02375,X_PHASCHNG);
 /* DISPCHNG CS VB99DEX; TS DISPDEX. COMMON TC PHASCHNG (OCT 40074); TCF
  * TASKOVER. */
 AT(36,02376);ai_csf(s,K_VB99DEX);
 AT(36,02377);(void)ai_ts(s,DISPDEX);
 AT(36,02400);return out(s,02401,X_PHASCHNG);
 AT(36,02402);return jump(s,X_TASKOVER);
 /* TIG-0 CS FLAGWRD7; MASK IGNFLBIT; ADS FLAGWRD7; TC CHECKMM (DEC 63);
  * TCF IGNYET?; CA ZOOMTIME; TC WAITLIST (2CADR P63ZOOM); TC 2PHSCHNG
  * (OCT 40033, 05014, 77777). */
 AT(36,02403);ai_cs(s,FLAGWRD7);
 AT(36,02404);ai_maskf(s,K_IGNFLBIT);
 AT(36,02405);ai_ads(s,FLAGWRD7);
 AT(36,02406);return out(s,02407,X_CHECKMM);
 AT(36,02410);GOTO(36,02421);
 AT(36,02411);ai_ca(s,ZOOMTIME);
 AT(36,02412);return out(s,02413,X_WAITLIST);
 AT(36,02415);return out(s,02416,X_2PHSCHNG);
 /* IGNYET? CAF ASTNBIT; MASK FLAGWRD7; EXTEND; INDEX WHICH; BZF 12. */
 AT(36,02421);ai_caf(s,K_ASTNBIT);
 AT(36,02422);ai_mask(s,FLAGWRD7);
 AT(36,02423);
 AT(36,02424);WHICHW(w);
 AT(36,02425);if(ai_bzf(s)){BR(17);return to(s,(agc_word)(w+012u));}
 BR(18);
 /* IGNITION CS FLAGWRD5; MASK ENGONBIT; ADS FLAGWRD5; CS PRIO30; EXTEND;
  * RAND DSALMOUT; AD BIT13; EXTEND; WRITE DSALMOUT; EXTEND; DCA TIME2;
  * DXCH TEVENT; EXTEND; DCA TGO; DXCH TIG; EXTEND; DCA TIME2; DAS TIG;
  * CS FLUNDBIT; MASK FLAGWRD8; TS FLAGWRD8; INDEX WHICH; TCF 13. */
 AT(36,02426);ai_cs(s,FLAGWRD5);
 AT(36,02427);ai_maskf(s,K_ENGONBIT);
 AT(36,02430);ai_ads(s,FLAGWRD5);
 AT(36,02431);ai_csf(s,K_PRIO30);
 AT(36,02432);
 AT(36,02433);rand_(s,io->chan11);
 AT(36,02434);ai_adf(s,K_BIT13);
 AT(36,02435);
 AT(36,02436);write_(s,&io->chan11);
 AT(36,02437);
 AT(36,02440);ai_dca(s,TIME2);
 AT(36,02441);ai_dxch(s,TEVENT);
 AT(36,02442);
 AT(36,02443);ai_dca(s,TGO);
 AT(36,02444);ai_dxch(s,TIG);
 AT(36,02445);
 AT(36,02446);ai_dca(s,TIME2);
 AT(36,02447);ai_das(s,TIG);
 AT(36,02450);ai_csf(s,K_FLUNDBIT);
 AT(36,02451);ai_mask(s,FLAGWRD8);
 AT(36,02452);(void)ai_ts(s,FLAGWRD8);  /* a MASK result: no overflow */
 AT(36,02453);WHICHW(w);
 AT(36,02454);return to(s,(agc_word)(w+013u));
 /* P63IGN EXTEND; DCA DSP2CADR; DXCH AVGEXIT; CA Z; TS DISPDEX; CS
  * FLAGWRD9; MASK LETABBIT; ADS FLAGWRD9; CS FLAGWRD7; MASK SWANDBIT; ADS
  * FLAGWRD7; CS PULSES; MASK DAPBOOLS; TS DAPBOOLS; EXTEND; DCA TIME2; DXCH
  * TIG; CAF ZERO; TS WCHPHASE; TS WCHPHOLD; CA TWO; TS FLPASS0; TCF
  * P42IGN. (CA Z reads the address of the next instruction.) */
 AT(36,02455);
 AT(36,02456);{static const agc_word d[2]={03471u,062067u};ai_dcaf(s,d);} /* DSP2CADR */
 AT(36,02457);ai_dxch(s,AVEGEXIT);
 AT(36,02460);ai_caf(s,02461u);
 AT(36,02461);(void)ai_ts(s,DISPDEX);
 AT(36,02462);ai_cs(s,FLAGWRD9);
 AT(36,02463);ai_maskf(s,K_LETABBIT);
 AT(36,02464);ai_ads(s,FLAGWRD9);
 AT(36,02465);ai_cs(s,FLAGWRD7);
 AT(36,02466);ai_maskf(s,K_SWANDBIT);
 AT(36,02467);ai_ads(s,FLAGWRD7);
 AT(36,02470);ai_csf(s,K_PULSES);
 AT(36,02471);ai_mask(s,DAPBOOLS);
 AT(36,02472);(void)ai_ts(s,DAPBOOLS);
 AT(36,02473);
 AT(36,02474);ai_dca(s,TIME2);
 AT(36,02475);ai_dxch(s,TIG);
 AT(36,02476);ai_caf(s,K_ZERO);
 AT(36,02477);(void)ai_ts(s,WCHPHASE);
 AT(36,02500);(void)ai_ts(s,WCHPHOLD);
 AT(36,02501);ai_caf(s,K_TWO);
 AT(36,02502);(void)ai_ts(s,FLPASS0);
 AT(36,02503);GOTO(36,02541);
 /* P40IGN CS FLAGWRD5; MASK NOTHRBIT; EXTEND; BZF P42IGN; CA ZOOMTIME;
  * TC WAITLIST (2CADR P40ZOOM); P63IGN1 TC 2PHSCHNG (OCT 40033, 05014,
  * 77777); TCF P42IGN. */
 AT(36,02504);ai_cs(s,FLAGWRD5);
 AT(36,02505);ai_maskf(s,K_NOTHRBIT);
 AT(36,02506);
 AT(36,02507);if(ai_bzf(s)){BR(19);GOTO(36,02541);}
 BR(20);
 AT(36,02510);ai_ca(s,ZOOMTIME);
 AT(36,02511);return out(s,02512,X_WAITLIST);
 AT(36,02514);return out(s,02515,X_2PHSCHNG);
 AT(36,02520);GOTO(36,02541);
 /* P12IGN CAF EBANK6; TS EBANK; CA IGNAOSQ; TS AOSQ; CA IGNAOSR; TS AOSR;
  * CAF EBANK7; TS EBANK. ABRTIGN CA Z; TS DISPDEX; EXTEND; DCA ATMAGADR;
  * DXCH AVGEXIT; CS FLAGWRD7; MASK SWANDBIT; ADS FLAGWRD7. */
 AT(36,02521);ai_caf(s,K_EBANK6);
 AT(36,02522);(void)ai_ts(s,EBANK);
 AT(36,02523);ai_ca(s,IGNAOSQ);
 AT(36,02524);(void)ai_ts(s,AOSQ);      /* CA of a 15-bit word: no overflow */
 AT(36,02525);ai_ca(s,IGNAOSR);
 AT(36,02526);(void)ai_ts(s,AOSR);
 AT(36,02527);ai_caf(s,K_EBANK7);
 AT(36,02530);(void)ai_ts(s,EBANK);
 AT(36,02531);ai_caf(s,02532u);
 AT(36,02532);(void)ai_ts(s,DISPDEX);
 AT(36,02533);
 AT(36,02534);{static const agc_word d[2]={03642u,070067u};ai_dcaf(s,d);} /* ATMAGADR */
 AT(36,02535);ai_dxch(s,AVEGEXIT);
 AT(36,02536);ai_cs(s,FLAGWRD7);
 AT(36,02537);ai_maskf(s,K_SWANDBIT);
 AT(36,02540);ai_ads(s,FLAGWRD7);
 /* P42IGN CS DRIFTBIT; MASK DAPBOOLS; TS DAPBOOLS; CAF IMPULBIT; MASK
  * FLAGWRD2; CCS A; TCF IMPLBURN. DVMONCON TC DOWNFLAG (ADRES IGNFLAG);
  * TC DOWNFLAG (ADRES ASTNFLAG); TC DOWNFLAG (ADRES IDLEFLAG); TC PHASCHNG
  * (OCT 40054); TC FIXDELAY (DEC 50); ULLAGOFF TC NOULLAGE. (CCS of a MASK
  * result is never negative.) */
 AT(36,02541);ai_csf(s,K_DRIFTBIT);
 AT(36,02542);ai_mask(s,DAPBOOLS);
 AT(36,02543);(void)ai_ts(s,DAPBOOLS);
 AT(36,02544);ai_caf(s,K_IMPULBIT);
 AT(36,02545);ai_mask(s,FLAGWRD2);
 AT(36,02546);c=ai_ccs(s,A);BR(21u+c);
  if(c>=2u)return REFUSE;
  if(c==1u)GOTO(36,02550);
 AT(36,02547);return jump(s,03522u);    /* TCF IMPLBURN (P40-P47) */
 AT(36,02550);return out(s,02551,X_DOWNFLAG);
 AT(36,02552);return out(s,02553,X_DOWNFLAG);
 AT(36,02554);return out(s,02555,X_DOWNFLAG);
 AT(36,02556);return out(s,02557,X_PHASCHNG);
 AT(36,02560);return out(s,02561,X_FIXDELAY);
 AT(36,02562);TC(02563,36,02653);
 /* WAITABIT EXTEND; DCA NEG0; DXCH -PHASE4; TCF TASKOVER. */
 AT(36,02563);
 AT(36,02564);ai_dcaf(s,K_NEG0);
 AT(36,02565);ai_dxch(s,MPHASE4);
 AT(36,02566);return jump(s,X_TASKOVER);
 /* TIGTASK TC POSTJUMP (CADR TIGTASK1). */
 AT(36,02567);return out(s,02570,X_POSTJUMP);
 /* COMFAIL TC UPFLAG (ADRES IDLEFLAG); TC UPFLAG (ADRES FLUNDISP); CAF
  * FOUR; TS DVCNTR; CCS PHASE6; TCF +3; TC BANKCALL (CADR STCLOK1); +3 CS
  * VB97DEX; TS DISPDEX; TC PHASCHNG (OCT 00004); TCF ENDOFJOB. A negative
  * PHASE6 would run the CADR word: no producer. */
 AT(36,02571);return out(s,02572,X_UPFLAG);
 AT(36,02573);return out(s,02574,X_UPFLAG);
 AT(36,02575);ai_caf(s,K_FOUR);
 AT(36,02576);(void)ai_ts(s,DVCNTR);
 AT(36,02577);c=ai_ccs(s,PHASE6);BR(25u+c);
  if(c==2u)return REFUSE;
  if(c==3u)GOTO(36,02603);
  if(c==1u)GOTO(36,02601);
 AT(36,02600);GOTO(36,02603);
 AT(36,02601);return out(s,02602,X_BANKCALL);
 AT(36,02603);ai_csf(s,K_VB97DEX);
 AT(36,02604);(void)ai_ts(s,DISPDEX);
 AT(36,02605);return out(s,02606,X_PHASCHNG);
 AT(36,02607);return jump(s,X_ENDOFJOB);
 /* COMFAIL1 INDEX WHICH; TCF 2. COMFAIL3 CA Z; TCF +2. COMFAIL4 CS
  * CNTDNDEX; TS DISPDEX; TC DOWNFLAG (ADRES IDLEFLAG); TC DOWNFLAG (ADRES
  * FLUNDISP); TCF ENDOFJOB. */
 AT(36,02610);WHICHW(w);
 AT(36,02611);return to(s,(agc_word)(w+2u));
 AT(36,02612);ai_caf(s,02613u);
 AT(36,02613);GOTO(36,02615);
 AT(36,02614);ai_csf(s,K_CNTDNDEX);
 AT(36,02615);(void)ai_ts(s,DISPDEX);
 AT(36,02616);return out(s,02617,X_DOWNFLAG);
 AT(36,02620);return out(s,02621,X_DOWNFLAG);
 AT(36,02622);return jump(s,X_ENDOFJOB);
 /* COMFAIL2 TC PHASCHNG (OCT 00003); INHINT; TC KILLTASK (CADR ZOOM); TC
  * IBNKCALL (CADR ENGINOF4); TC UPFLAG (ADRES DRIFTDFL); TC INVFLAG (ADRES
  * AORBTFLG); TC UPFLAG (ADRES ULLAGFLG); CAF BIT1; INHINT; TC TWIDDLE
  * (ADRES TIG-5); TCF ENDOFJOB. */
 AT(36,02623);return out(s,02624,X_PHASCHNG);
 AT(36,02625);io->control.interrupt_enabled=0;
 AT(36,02626);ai_tc(s,02627);goto LFF_06027;
 AT(36,02630);return out(s,02631,X_IBNKCALL);
 AT(36,02632);return out(s,02633,X_UPFLAG);
 AT(36,02634);TC(02635,36,02645);
 AT(36,02636);return out(s,02637,X_UPFLAG);
 AT(36,02640);ai_caf(s,K_BIT1);
 AT(36,02641);io->control.interrupt_enabled=0;
 AT(36,02642);return out(s,02643,X_TWIDDLE);
 AT(36,02644);return jump(s,X_ENDOFJOB);
 /* INVFLAG CA Q; TC DEBIT; COM; EXTEND; RXOR LCHAN; TCF COMFLAG. RXOR of
  * channel 1 is A XOR L, as yaAGC's. */
 AT(36,02645);ai_ca(s,QREG);
 AT(36,02646);return out(s,02647,X_DEBIT);
 AT(36,02647);ai_com(s);
 AT(36,02650);
 AT(36,02651);W[0]=(agc_register)(W[0]^W[1]);
 AT(36,02652);return jump(s,X_COMFLAG);
 /* NOULLAGE CS ULLAGER; MASK DAPBOOLS; TS DAPBOOLS; TC Q. ONULLAGE CS
  * DAPBOOLS; MASK ULLAGER; ADS DAPBOOLS; TC Q. */
 AT(36,02653);ai_csf(s,K_ULLAGER);
 AT(36,02654);ai_mask(s,DAPBOOLS);
 AT(36,02655);(void)ai_ts(s,DAPBOOLS);
 AT(36,02656);return tcq(s);
 AT(36,02657);ai_cs(s,DAPBOOLS);
 AT(36,02660);ai_maskf(s,K_ULLAGER);
 AT(36,02661);ai_ads(s,DAPBOOLS);
 AT(36,02662);return tcq(s);
 /* STCLOK1 CA ZERO. STCLOK2 TS DISPDEX (overflow in A skips to TS
  * TBASE4). STCLOK3 TC MAKECADR; TS TBASE4; EXTEND; DCA TIG; DXCH MPAC;
  * EXTEND; DCS TIME2; DAS MPAC; TC TPAGREE. */
 AT(36,02663);ai_caf(s,K_ZERO);
 AT(36,02664);if(ai_ts(s,DISPDEX)){BR(29);GOTO(36,02666);}
 AT(36,02665);return out(s,02666,X_MAKECADR);
 /* MAKECADR's CADR never overflows; with overflow the TS would skip past
  * the EXTEND. */
 AT(36,02666);if(ai_ts(s,TBASE4))return REFUSE;
 AT(36,02667);
 AT(36,02670);ai_dca(s,TIG);
 AT(36,02671);ai_dxch(s,MPAC);
 AT(36,02672);
 AT(36,02673);ai_dcs(s,TIME2);
 AT(36,02674);ai_das(s,MPAC);
 AT(36,02675);return out(s,02676,X_TPAGREE);
 /* CAF 1SEC; TS Q; DXCH MPAC; MASK LOW5; EXTEND; DV Q; CA L; AD TWO;
  * INHINT; TC TWIDDLE (ADRES CLOKTASK); TC 2PHSCHNG (OCT 40036, 05024,
  * 13000); CA TBASE4; TC BANKJUMP. */
 AT(36,02676);ai_caf(s,K_1SEC);
 AT(36,02677);(void)ai_ts(s,QREG);
 AT(36,02700);ai_dxch(s,MPAC);
 AT(36,02701);ai_maskf(s,K_LOW5);
 AT(36,02702);
 AT(36,02703);ai_dv(s,QREG);
 AT(36,02704);ai_ca(s,LREG);
 AT(36,02705);ai_adf(s,K_TWO);
 AT(36,02706);io->control.interrupt_enabled=0;
 AT(36,02707);return out(s,02710,X_TWIDDLE);
 AT(36,02711);return out(s,02712,X_2PHSCHNG);
 AT(36,02715);ai_ca(s,TBASE4);
 AT(36,02716);return out(s,02717,X_BANKJUMP);
 /* CLOKTASK CS TIME1; TS TBASE6; CCS DISPDEX; TCF KILLCLOK; NOOP; CAF
  * PRIO27; TC NOVAC (2CADR CLOKJOB); TC FIXDELAY (DEC 100); TCF CLOKTASK.
  * KILLCLOK EXTEND; DCA NEG0; DXCH -PHASE6; TCF TASKOVER. */
 AT(36,02717);ai_cs(s,TIME1);
 AT(36,02720);(void)ai_ts(s,TBASE6);
 AT(36,02721);c=ai_ccs(s,DISPDEX);BR(30u+c);
  if(c==1u)GOTO(36,02723);
  if(c==2u)GOTO(36,02724);
  if(c==3u)GOTO(36,02725);
 AT(36,02722);GOTO(36,02733);
 AT(36,02723);
 AT(36,02724);ai_caf(s,K_PRIO27);
 AT(36,02725);return out(s,02726,X_NOVAC);
 AT(36,02730);return out(s,02731,X_FIXDELAY);
 AT(36,02732);GOTO(36,02717);
 AT(36,02733);
 AT(36,02734);ai_dcaf(s,K_NEG0);
 AT(36,02735);ai_dxch(s,MPHASE6);
 AT(36,02736);return jump(s,X_TASKOVER);
 /* CLOKJOB EXTEND; DCS TIG; DXCH TTOGO; EXTEND; DCA TIME2; DAS TTOGO;
  * INHINT; CCS DISPDEX; TCF ENDOFJOB; TCF ENDOFJOB; COM; RELINT; INDEX A;
  * TCF DISPNOT -1. DISPDEX -n selects DISPNOT -n: the file stores -35,
  * -17, -13 and -2 (and the -25 entry is code too). */
 AT(36,02737);
 AT(36,02740);ai_dcs(s,TIG);
 AT(36,02741);ai_dxch(s,TTOGO);
 AT(36,02742);
 AT(36,02743);ai_dca(s,TIME2);
 AT(36,02744);ai_das(s,TTOGO);
 AT(36,02745);io->control.interrupt_enabled=0;
 AT(36,02746);c=ai_ccs(s,DISPDEX);BR(34u+c);
  if(c==3u)return REFUSE;               /* -0: TCF DISPNOT -1, a CADR word */
  if(c==2u)GOTO(36,02751);
  if(c==1u)GOTO(36,02750);
 AT(36,02747);return jump(s,X_ENDOFJOB);
 AT(36,02750);return jump(s,X_ENDOFJOB);
 AT(36,02751);ai_com(s);
 AT(36,02752);io->control.interrupt_enabled=1;
 AT(36,02753);{agc_word t=(agc_word)(03011u-(~oc(W[0])&037777u));
  if(t!=02755u&&t!=02765u&&t!=02773u&&t!=02777u&&t!=03010u)return REFUSE;
  BB_PATH(PATHOF(36,02754));return to(s,t);}
 /* -35: CS ZERO; TS NVWORD1; CA NVWORD +2; TC BANKCALL (CADR CLOCPLAY);
  * TCF STOPCLOK; TCF COMFAIL1; TCF COMFAIL2. */
 AT(36,02755);ai_csf(s,K_ZERO);
 AT(36,02756);(void)ai_ts(s,NVWORD1);
 AT(36,02757);ai_ca(s,NVWORD2);
 AT(36,02760);return out(s,02761,X_BANKCALL);
 AT(36,02762);GOTO(36,03013);
 AT(36,02763);GOTO(36,02610);
 AT(36,02764);GOTO(36,02623);
 /* -25: CAF V06N61; TC BANKCALL (CADR REFLASH); TCF STOPCLOK; TCF
  * ASTNRETN; TCF -6. */
 AT(36,02765);ai_caf(s,01475u);         /* V06N61 */
 AT(36,02766);return out(s,02767,X_BANKCALL);
 AT(36,02770);GOTO(36,03013);
 AT(36,02771);GOTO(36,03030);
 AT(36,02772);GOTO(36,02764);
 /* -17: INDEX WHICH; CAF 0; TC BANKCALL (CADR REGODSP). */
 AT(36,02773);WHICHW(w);
 AT(36,02774);ai_caf(s,TBL(w,0u));
 AT(36,02775);return out(s,02776,X_BANKCALL);
 /* -13 (V99RECYC): CS BIT9; TS NVWORD1; INDEX WHICH; CAF 0; TC BANKCALL
  * (CADR CLOCPLAY); TCF STOPCLOK; TCF *PROCEED; TCF *ENTER. */
 AT(36,02777);ai_csf(s,K_BIT9);
 AT(36,03000);(void)ai_ts(s,NVWORD1);
 AT(36,03001);WHICHW(w);
 AT(36,03002);ai_caf(s,TBL(w,0u));
 AT(36,03003);return out(s,03004,X_BANKCALL);
 AT(36,03005);GOTO(36,03013);
 AT(36,03006);GOTO(36,03041);
 AT(36,03007);GOTO(36,03044);
 /* -2: TC BANKCALL (CADR CLEANDSP). DISPNOT TCF ENDOFJOB. */
 AT(36,03010);return out(s,03011,X_BANKCALL);
 AT(36,03012);return jump(s,X_ENDOFJOB);
 /* STOPCLOK TC NULLCLOK; TCF GOTOPOOH. */
 AT(36,03013);TC(03014,36,03015);
 AT(36,03014);return jump(s,X_GOTOPOOH);
 /* NULLCLOK INHINT; EXTEND; QXCH P40/RET; TC NOULLAGE; TC KILLTASK (CADR
  * ULLGTASK); TC PHASCHNG (OCT 1); CA Z; TS DISPDEX; TC P40/RET. The word
  * in P40/RET runs as TC: the QXCH stored one of NULLCLOK's three returns
  * there. */
 AT(36,03015);io->control.interrupt_enabled=0;
 AT(36,03016);
 AT(36,03017);ai_qxch(s,P40RET);
 AT(36,03020);TC(03021,36,02653);
 AT(36,03021);ai_tc(s,03022);goto LFF_06027;
 AT(36,03023);return out(s,03024,X_PHASCHNG);
 AT(36,03025);ai_caf(s,03026u);
 AT(36,03026);(void)ai_ts(s,DISPDEX);
 AT(36,03027);w=ai_word(s,P40RET);
  if(w!=03014u&&w!=03057u&&w!=03074u)return REFUSE;
  ai_tc(s,P40RET+1u);return to(s,w);
 /* ASTNRETN TC PHASCHNG (OCT 04024); CAF ZERO; TS DISPDEX; CAF PRIO13; TC
  * FINDVAC (2CADR ASTNRET); TCF ENDOFJOB. */
 AT(36,03030);return out(s,03031,X_PHASCHNG);
 AT(36,03032);ai_caf(s,K_ZERO);
 AT(36,03033);(void)ai_ts(s,DISPDEX);
 AT(36,03034);ai_caf(s,K_PRIO13);
 AT(36,03035);return out(s,03036,X_FINDVAC);
 AT(36,03040);return jump(s,X_ENDOFJOB);
 /* *PROCEED TC UPFLAG (ADRES ASTNFLAG); TCF IGNITE. *ENTER INHINT; INDEX
  * WHICH; TCF 3. */
 AT(36,03041);return out(s,03042,X_UPFLAG);
 AT(36,03043);GOTO(36,03102);
 AT(36,03044);io->control.interrupt_enabled=0;
 AT(36,03045);WHICHW(w);
 AT(36,03046);return to(s,(agc_word)(w+3u));
 /* GOPOST CAF PRIO12; TC FINDVAC (2CADR POSTBURN); INHINT; TC IBNKCALL
  * (CADR ALLCOAST); TC NULLCLOK; TC PHASCHNG (OCT 00134); TCF ENDOFJOB. */
 AT(36,03047);ai_caf(s,K_PRIO12);
 AT(36,03050);return out(s,03051,X_FINDVAC);
 AT(36,03053);io->control.interrupt_enabled=0;
 AT(36,03054);return out(s,03055,X_IBNKCALL);
 AT(36,03056);TC(03057,36,03015);
 AT(36,03057);return out(s,03060,X_PHASCHNG);
 AT(36,03061);return jump(s,X_ENDOFJOB);
 /* GOCUTOFF CAF PRIO17; TC FINDVAC (2CADR CUTOFF); TC DOWNFLAG (ADRES
  * FLUNDISP); INHINT; TC IBNKCALL (CADR ALLCOAST); TC NULLCLOK; TC
  * PHASCHNG (OCT 07024, 17000, 2CADR CUTOFF); TCF ENDOFJOB. */
 AT(36,03062);ai_caf(s,K_PRIO17);
 AT(36,03063);return out(s,03064,X_FINDVAC);
 AT(36,03066);return out(s,03067,X_DOWNFLAG);
 AT(36,03070);io->control.interrupt_enabled=0;
 AT(36,03071);return out(s,03072,X_IBNKCALL);
 AT(36,03073);TC(03074,36,03015);
 AT(36,03074);return out(s,03075,X_PHASCHNG);
 AT(36,03101);return jump(s,X_ENDOFJOB);
 /* IGNITE CS FLAGWRD7; MASK IGNFLBIT; CCS A; TCF IGNITE1; CAF BIT1;
  * INHINT; TC TWIDDLE (ADRES IGNITION); CAF OCT23; TS L; COM; DXCH
  * -PHASE4. IGNITE1 CS CNTDNDEX; TS DISPDEX; TCF ENDOFJOB. */
 AT(36,03102);ai_cs(s,FLAGWRD7);
 AT(36,03103);ai_maskf(s,K_IGNFLBIT);
 AT(36,03104);c=ai_ccs(s,A);BR(38u+c);
  if(c>=2u)return REFUSE;               /* a MASK result is never negative */
  if(c==1u)GOTO(36,03106);
 AT(36,03105);GOTO(36,03116);
 AT(36,03106);ai_caf(s,K_BIT1);
 AT(36,03107);io->control.interrupt_enabled=0;
 AT(36,03110);return out(s,03111,X_TWIDDLE);
 AT(36,03112);ai_caf(s,K_OCT23);
 AT(36,03113);(void)ai_ts(s,LREG);
 AT(36,03114);ai_com(s);
 AT(36,03115);ai_dxch(s,MPHASE4);
 AT(36,03116);ai_csf(s,K_CNTDNDEX);
 AT(36,03117);(void)ai_ts(s,DISPDEX);
 AT(36,03120);return jump(s,X_ENDOFJOB);
 /* P40ALM TC ALARM (OCT 1706). REP40ALM CAF V05N09; TC BANKCALL (CADR
  * GOFLASH); TCF GOTOPOOH; TCF +2; TCF REP40ALM; INDEX WHICH; TCF 14. */
 AT(36,03121);return out(s,03122,X_ALARM);
 AT(36,03123);ai_caf(s,K_V05N09);
 AT(36,03124);return out(s,03125,X_BANKCALL);
 AT(36,03126);return jump(s,X_GOTOPOOH);
 AT(36,03127);GOTO(36,03131);
 AT(36,03130);GOTO(36,03123);
 AT(36,03131);WHICHW(w);
 AT(36,03132);return to(s,(agc_word)(w+014u));
#endif
#if BB_G2
 /* P40AUTO (bank 35) TC MAKECADR; TS TEMPR60. P40A/P TC BANKCALL (CADR
  * G+N,AUTO); CCS A; TCF TURNITON; CAF APSFLBIT; MASK FLGWRD10; CCS A;
  * TCF GOBACK; CAF BIT5; EXTEND; RAND CHAN30; EXTEND; BZF GOBACK. */
 AT(35,03747);return out(s,03750,X_MAKECADR);
 AT(35,03750);if(ai_ts(s,TEMPR60))return REFUSE; /* a CADR never overflows */
 AT(35,03751);return out(s,03752,X_BANKCALL);
 AT(35,03753);c=ai_ccs(s,A);BR(42u+c);
  if(c==1u)GOTO(35,03755);
  if(c==2u)GOTO(35,03756);
  if(c==3u)GOTO(35,03757);
 AT(35,03754);GOTO(35,03766);
 AT(35,03755);ai_caf(s,K_APSFLBIT);
 AT(35,03756);ai_mask(s,FLGWRD10);
 AT(35,03757);c=ai_ccs(s,A);BR(46u+c);
  if(c>=2u)return REFUSE;               /* a MASK result is never negative */
  if(c==1u)GOTO(35,03761);
 AT(35,03760);GOTO(35,03773);
 AT(35,03761);ai_caf(s,K_BIT5);
 AT(35,03762);
 AT(35,03763);rand_(s,io->chan30);
 AT(35,03764);
 AT(35,03765);if(ai_bzf(s)){BR(50);GOTO(35,03773);}
 BR(51);
 /* TURNITON CAF P40A/PMD; TC BANKCALL (CADR GOPERF1); TCF GOTOPOOH; TCF
  * P40A/P. GOBACK CA TEMPR60; TC BANKJUMP. */
 AT(35,03766);ai_caf(s,0203u);          /* P40A/PMD */
 AT(35,03767);return out(s,03770,X_BANKCALL);
 AT(35,03771);return jump(s,X_GOTOPOOH);
 AT(35,03772);GOTO(35,03751);
 AT(35,03773);ai_ca(s,TEMPR60);
 AT(35,03774);return out(s,03775,X_BANKJUMP);
 /* TIGTASK1 (bank 17) CAF PRIO16; TC NOVAC (2CADR TIGNOW); TC PHASCHNG
  * (OCT 6); TCF TASKOVER. */
 AT(17,02000);ai_caf(s,K_PRIO16);
 AT(17,02001);return out(s,02002,X_NOVAC);
 AT(17,02004);return out(s,02005,X_PHASCHNG);
 AT(17,02006);return jump(s,X_TASKOVER);
 /* P63ZOOM EXTEND; DCA LUNLANAD; DXCH AVEGEXIT; TC IBNKCALL (CADR
  * FLATOUT); TCF P40ZOOMA. P40ZOOM CAF BIT13; TS THRUST; CAF BIT4;
  * EXTEND; WOR CHAN14. P40ZOOMA TC PHASCHNG (OCT 3); TCF TASKOVER. */
 AT(17,02007);
 AT(17,02010);{static const agc_word d[2]={02462u,062067u};ai_dcaf(s,d);} /* LUNLANAD */
 AT(17,02011);ai_dxch(s,AVEGEXIT);
 AT(17,02012);return out(s,02013,X_IBNKCALL);
 AT(17,02014);GOTO(17,02022);
 AT(17,02015);ai_caf(s,K_BIT13);
 AT(17,02016);(void)ai_ts(s,THRUST);
 AT(17,02017);ai_caf(s,K_BIT4);
 AT(17,02020);
 AT(17,02021);wor(s,&io->chan14);
 AT(17,02022);return out(s,02023,X_PHASCHNG);
 AT(17,02024);return jump(s,X_TASKOVER);
#endif
#if BB_G1
 /* KILLTASK (fixed-fixed) CA KILLBB; INHINT; LXCH A; INDEX Q; CA 0; LXCH
  * BBANK; TCF KILLTSK2. CA 0 under INDEX Q reads the caller's CADR (the
  * compiled site table). */
 FF(06027);ai_caf(s,056063u);           /* KILLBB: BBCON KILLTSK2 */
 FF(06030);io->control.interrupt_enabled=0;
 FF(06031);ai_lxch(s,A);
 FF(06032);{agc_register q=W[QREG];uint16_t p;
  if(!tc_word(q))return REFUSE;
  p=agc_fixed_physical((agc_word)q,W[FBANK],io->control.superbank);
  if(!agc_site_lookup(lm_burn_baby_sites,LMBB_SITES,p,&w))return REFUSE;}
 FF(06033);ai_caf(s,w);
 FF(06034);ai_lxch(s,BBANK);
 FF(06035);GOTO(27,02200);
 /* KILLTSK2 (bank 27, EBANK3) LXCH ITEMP2; INCR Q; EXTEND; QXCH ITEMP1;
  * TS ITEMP3; MASK LOW10; AD BIT11; TS ITEMP4; CS LOW10; MASK ITEMP3; TS
  * ITEMP3; ZL. ADRSCAN INDEX L; CS LST2; AD ITEMP4; EXTEND; BZF TSTFBANK.
  * LETITLIV CS LSTLIM; AD L; EXTEND; BZF DEAD; INCR L; INCR L; TCF
  * ADRSCAN. DEAD DXCH ITEMP1; DTCB. TSTFBANK CS LOW10; INDEX L; MASK LST2
  * +1; EXTEND; SU ITEMP3; EXTEND; BZF KILLDEAD; TCF LETITLIV. KILLDEAD CA
  * TCTSKOVR; INDEX L; TS LST2; TCF DEAD. (L counts 0, 2, ... 16; the TS
  * of a CADR or a MASK result never skips.) */
 AT(27,02200);ai_lxch(s,ITEMP2);
 AT(27,02201);ai_incr(s,QREG);
 AT(27,02202);
 AT(27,02203);ai_qxch(s,ITEMP1);
 /* KILLTASK's CA 0 leaves no overflow; with one the TS would skip the
  * MASK. */
 AT(27,02204);if(ai_ts(s,ITEMP3))return REFUSE;
 AT(27,02205);ai_maskf(s,K_LOW10);
 AT(27,02206);ai_adf(s,K_BIT11);
 AT(27,02207);(void)ai_ts(s,ITEMP4);
 AT(27,02210);ai_csf(s,K_LOW10);
 AT(27,02211);ai_mask(s,ITEMP3);
 AT(27,02212);(void)ai_ts(s,ITEMP3);
 AT(27,02213);ai_zl(s);
 /* INDEX L reads the list only for L 0 .. 16, as KILLTSK2's ZL and
  * LETITLIV leave it. A larger L (DEAD's DTCB to a word of this file with
  * the caller's BBANK still in L) would carry into the opcode, so it is
  * refused at each INDEX L (#147). */
 AT(27,02214);if(W[1]>16u)return REFUSE;
 ai_cs(s,LST2+W[1]);
 AT(27,02216);ai_ad(s,ITEMP4);
 AT(27,02217);
 AT(27,02220);if(ai_bzf(s)){BR(52);GOTO(27,02232);}
 BR(53);
 AT(27,02221);ai_csf(s,K_BIT5);         /* LSTLIM = BIT5 */
 AT(27,02222);ai_ad(s,LREG);
 AT(27,02223);
 AT(27,02224);if(ai_bzf(s)){BR(54);GOTO(27,02230);}
 BR(55);
 AT(27,02225);ai_incr(s,LREG);
 AT(27,02226);ai_incr(s,LREG);
 AT(27,02227);GOTO(27,02214);
 AT(27,02230);ai_dxch(s,ITEMP1);
 AT(27,02231);dtcb(s,02231u);return GO;
 AT(27,02232);ai_csf(s,K_LOW10);
 AT(27,02234);if(W[1]>16u)return REFUSE;
 ai_mask(s,LST21+W[1]);
 AT(27,02235);
 AT(27,02236);ai_su(s,ITEMP3);
 AT(27,02237);
 AT(27,02240);if(ai_bzf(s)){BR(56);GOTO(27,02242);}
 BR(57);
 AT(27,02241);GOTO(27,02221);
 AT(27,02242);ai_caf(s,K_TCTSKOVR);
 AT(27,02244);if(W[1]>16u)return REFUSE;
 (void)ai_ts(s,LST2+W[1]);
 AT(27,02245);GOTO(27,02230);
#endif
 default:return REFUSE;
 }
}

/* The file's words: bank 36 02022-03146, bank 35 03747-03775, bank 17
 * 02000-02026, bank 27 02200-02246 and fixed-fixed 06027-06036. A data
 * word among them reached as code is refused (basic's default). */
static int in_code(uint16_t ph){
 return (ph>=PH(36,02022)&&ph<=PH(36,03146))||(ph>=PH(35,03747)&&ph<=PH(35,03775))||
        (ph>=PH(17,02000)&&ph<=PH(17,02026))||(ph>=PH(27,02200)&&ph<=PH(27,02246))||
        (ph>=06027u&&ph<=06036u);
}
/* The words that run only after the EXTEND or INDEX before them (an
 * extracode, an indexed operand). A computed transfer (a TC Q return,
 * DTCB, TC P40/RET) that lands on one would run a different instruction:
 * refused. */
static const uint16_t dependent[]={
 06033u,PH(17,02010),PH(17,02021),PH(27,02203),PH(27,02215),PH(27,02220),PH(27,02224),PH(27,02234),
 PH(27,02236),PH(27,02240),PH(27,02244),PH(35,03763),PH(35,03765),PH(36,02136),PH(36,02145),PH(36,02204),
 PH(36,02207),PH(36,02214),PH(36,02217),PH(36,02250),PH(36,02252),PH(36,02304),PH(36,02306),PH(36,02321),
 PH(36,02326),PH(36,02327),PH(36,02340),PH(36,02353),PH(36,02365),PH(36,02424),PH(36,02425),PH(36,02433),
 PH(36,02436),PH(36,02440),PH(36,02443),PH(36,02446),PH(36,02454),PH(36,02456),PH(36,02474),PH(36,02507),
 PH(36,02534),PH(36,02564),PH(36,02611),PH(36,02651),PH(36,02670),PH(36,02673),PH(36,02703),PH(36,02734),
 PH(36,02740),PH(36,02743),PH(36,02754),PH(36,02774),PH(36,03002),PH(36,03017),PH(36,03046),PH(36,03132)};
static int dependent_word(uint16_t ph){unsigned i;
 for(i=0;i<sizeof dependent/sizeof dependent[0];++i)if(dependent[i]==ph)return 1;
 return 0;
}
/* Is Z (or LOC at NEWOPS) the file's code? *ph is its physical address. */
static int here(const apollo_overlay_state *s,const lm_bb_io *io,int *interp,uint16_t *ph){
 agc_word z=W[5];
 *interp=0;
 if(z==NEWOPS){*interp=1;*ph=agc_fixed_physical(W[LOC],W[BANKSET],io->control.superbank);}
 else *ph=agc_fixed_physical(z,W[4],io->control.superbank);
 return *ph!=AGC_TRANSFER_NONE&&in_code(*ph);
}
/* FIXLOC one of the five VAC areas, PUSHLOC inside it, no order code left
 * over in EDOP. */
static int work_area(const apollo_overlay_state *s){
 agc_word f=W[FIXLOC],p=W[PUSHLOC];
 if(W[EDOP])return 0;
 if(f!=0401u&&f!=0455u&&f!=0531u&&f!=0605u&&f!=0661u)return 0;
 return p>=f&&p<=f+042u;
}
static enum exec_status steps(apollo_overlay_state *s,lm_bb_io *io){
 unsigned n;int st,interp;uint16_t ph;
 for(n=0;n<RUN_LIMIT;++n){
  if(!here(s,io,&interp,&ph))return n?EXEC_OK:EXEC_OUT_OF_SCOPE;
  /* NEWOPS reads the order word under FB, which DANZIG set from BANKSET. */
#ifdef AGC_FLIGHT_INTERPRETER
  /* Flight: the native interpreter runs the file's interpretive code
   * from NEWOPS, so a chain stops where it reaches it. */
  if(interp)return n?EXEC_OK:EXEC_OUT_OF_SCOPE;
#endif
  if(interp&&(((W[4]^W[BANKSET])&076000u)||!work_area(s)))return EXEC_OUT_OF_SCOPE;
  if(!interp&&dependent_word(ph))return EXEC_OUT_OF_SCOPE;
  st=interp?interpretive(s,io):basic(s,io,ph);
  if(st==REFUSE)return EXEC_OUT_OF_SCOPE;
  if(st==STOP)return EXEC_OK;
 }
 return EXEC_OUT_OF_SCOPE;
}
static enum exec_status run(apollo_overlay_state *s,void *v){
 lm_bb_io *io=v,entry;enum exec_status st;
 if (!s || !io || !io->undo || FLIGHT_UNDO_ALIAS(io, s)) return EXEC_OUT_OF_SCOPE;
 FLIGHT_UNDO_SAVE(io, s);
 entry = *io;
 st=steps(s,io);
 if (st != EXEC_OK) {
     FLIGHT_UNDO_RESTORE(&entry, s);
     *io = entry;
 }
 return st;
}
static enum exec_status routine(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;return run(s,io);}
#define K17(a) (uint16_t)(017u*1024u+((a)-02000u))
#define K27(a) (uint16_t)(027u*1024u+((a)-02000u))
#define K35(a) (uint16_t)(035u*1024u+((a)-02000u))
#define K36(a) (uint16_t)(036u*1024u+((a)-02000u))
#define KFF(a) (uint16_t)(a)
#define R17(a) {K17(a),routine}
#define R27(a) {K27(a),routine}
#define R35(a) {K35(a),routine}
#define R36(a) {K36(a),routine}
#define RFF(a) {KFF(a),routine}
/* Flight build only: 36,2237 is the basic word the EXIT at 36,2236 returns
 * to. On the C64 the native interpreter runs the interpretive code and its
 * EXIT hands that word to the flight program as a basic key
 * (tools/flight_exit_returns.py). The host build runs on across EXIT in one
 * call and does not key it. */
#ifdef AGC_FLIGHT_INTERPRETER
#define X36(a) R36(a),
#else
#define X36(a)
#endif
static const agc_routine_entry basic_entries[] = {
#if BB_G1
    RFF(06027),
#endif
#if BB_G2
    R17(02000), R17(02004), R17(02006), R17(02007), R17(02014),
    R17(02015), R17(02022), R17(02024),
#endif
#if BB_G1
    R27(02200), R27(02214), R27(02221), R27(02230), R27(02232),
    R27(02242),
#endif
#if BB_G2
    R35(03747), R35(03750), R35(03751), R35(03753), R35(03766),
    R35(03771), R35(03772), R35(03773),
#endif
#if BB_G1
    R36(02053), R36(02126), R36(02130), R36(02135), R36(02143),
    R36(02146), R36(02151), R36(02202), R36(02203), R36(02211),
    R36(02223), R36(02225), R36(02227), R36(02230), X36(02237) R36(02240),
    R36(02243), R36(02245), R36(02256), R36(02262), R36(02263),
    R36(02265), R36(02266), R36(02272), R36(02273), R36(02276),
    R36(02301), R36(02312), R36(02322), R36(02325), R36(02337),
    R36(02342), R36(02346), R36(02351), R36(02352), R36(02360),
    R36(02362), R36(02364), R36(02366), R36(02374), R36(02376),
    R36(02400), R36(02402), R36(02403), R36(02410), R36(02411),
    R36(02415), R36(02421), R36(02426), R36(02455), R36(02504),
    R36(02514), R36(02520), R36(02521), R36(02531), R36(02541),
    R36(02550), R36(02552), R36(02554), R36(02556), R36(02560),
    R36(02562), R36(02563), R36(02567), R36(02571), R36(02573),
    R36(02575), R36(02603), R36(02607), R36(02610), R36(02612),
    R36(02614), R36(02620), R36(02622), R36(02623), R36(02625),
    R36(02632), R36(02634), R36(02636), R36(02640), R36(02644),
    R36(02645), R36(02647), R36(02653), R36(02657), R36(02663),
    R36(02664), R36(02665), R36(02666), R36(02676), R36(02711),
    R36(02715), R36(02717), R36(02730), R36(02732), R36(02733),
    R36(02737), R36(02755), R36(02762), R36(02763), R36(02764),
    R36(02765), R36(02770), R36(02771), R36(02772), R36(02773),
    R36(02777), R36(03005), R36(03006), R36(03007), R36(03010),
    R36(03012), R36(03013), R36(03015), R36(03025), R36(03030),
    R36(03032), R36(03040), R36(03041), R36(03043), R36(03044),
    R36(03047), R36(03053), R36(03056), R36(03061), R36(03062),
    R36(03066), R36(03070), R36(03073), R36(03101), R36(03102),
    R36(03112), R36(03116), R36(03121), R36(03123), R36(03126),
    R36(03127), R36(03130),
#endif
};
static const agc_routine_entry interp_entries[]={
#if BB_G1
 R36(02152),R36(02155),R36(02157),R36(02163),R36(02166),R36(02167),R36(02171),R36(02173),R36(02175),
 R36(02177),R36(02200),R36(02233),R36(02235),R36(02236),
#endif
};
#define COUNT(t) (uint16_t)(sizeof t/sizeof t[0])
const agc_routine_table lm_burn_baby_routines={basic_entries,COUNT(basic_entries)};
const agc_routine_table lm_burn_baby_interpretive={interp_entries,COUNT(interp_entries)};
const agc_site_operand lm_burn_baby_sites[LMBB_SITES]={
#include "lm_burn_baby_sites.inc"
};
const agc_site_operand lm_burn_baby_flag_sites[LMBB_FLAG_SITES]={
#include "lm_burn_baby_flag_sites.inc"
};
/* Every key, basic then interpretive, in source order. */
const uint16_t lmbb_routine_keys[LMBB_ROUTINES]={
 K17(02000),K17(02004),K17(02006),K17(02007),K17(02014),K17(02015),K17(02022),K17(02024),
 K27(02200),K27(02214),K27(02221),K27(02230),K27(02232),K27(02242),K35(03747),K35(03750),
 K35(03751),K35(03753),K35(03766),K35(03771),K35(03772),K35(03773),K36(02053),K36(02126),
 K36(02130),K36(02135),K36(02143),K36(02146),K36(02151),K36(02202),K36(02203),K36(02211),
 K36(02223),K36(02225),K36(02227),K36(02230),K36(02240),K36(02243),K36(02245),K36(02256),
 K36(02262),K36(02263),K36(02265),K36(02266),K36(02272),K36(02273),K36(02276),K36(02301),
 K36(02312),K36(02322),K36(02325),K36(02337),K36(02342),K36(02346),K36(02351),K36(02352),
 K36(02360),K36(02362),K36(02364),K36(02366),K36(02374),K36(02376),K36(02400),K36(02402),
 K36(02403),K36(02410),K36(02411),K36(02415),K36(02421),K36(02426),K36(02455),K36(02504),
 K36(02514),K36(02520),K36(02521),K36(02531),K36(02541),K36(02550),K36(02552),K36(02554),
 K36(02556),K36(02560),K36(02562),K36(02563),K36(02567),K36(02571),K36(02573),K36(02575),
 K36(02603),K36(02607),K36(02610),K36(02612),K36(02614),K36(02620),K36(02622),K36(02623),
 K36(02625),K36(02632),K36(02634),K36(02636),K36(02640),K36(02644),K36(02645),K36(02647),
 K36(02653),K36(02657),K36(02663),K36(02664),K36(02665),K36(02666),K36(02676),K36(02711),
 K36(02715),K36(02717),K36(02730),K36(02732),K36(02733),K36(02737),K36(02755),K36(02762),
 K36(02763),K36(02764),K36(02765),K36(02770),K36(02771),K36(02772),K36(02773),K36(02777),
 K36(03005),K36(03006),K36(03007),K36(03010),K36(03012),K36(03013),K36(03015),K36(03025),
 K36(03030),K36(03032),K36(03040),K36(03041),K36(03043),K36(03044),K36(03047),K36(03053),
 K36(03056),K36(03061),K36(03062),K36(03066),K36(03070),K36(03073),K36(03101),K36(03102),
 K36(03112),K36(03116),K36(03121),K36(03123),K36(03126),K36(03127),K36(03130),KFF(06027),
 K36(02152),K36(02155),K36(02157),K36(02163),K36(02166),K36(02167),K36(02171),K36(02173),K36(02175),
 K36(02177),K36(02200),K36(02233),K36(02235),K36(02236)};
