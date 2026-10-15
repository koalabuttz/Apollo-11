/* SPDX-License-Identifier: MIT */
/* Luminary099 THE_LUNAR_LANDING (P63, bank 32; P68's LANDJUNK, bank 31) as
 * native routines of the control-transfer ABI; see
 * include/lm_lunar_landing.h and docs/LM-LUNAR-LANDING.md.
 * Basic code: each instruction is one ai_* effect on the image in source
 * order, under a case label at its address; a TC or TCF within the file is
 * a goto, one to another file ends the run with Q and Z set. Interpretive
 * code: each order-pair word is a case of its own, running the compiled
 * lm_interp_steps/lm_interp_orders paths with the pair's words as literal
 * arguments; branches follow the outcomes those paths return. */
#include "lm_lunar_landing.h"
#include "flight_undo.h"
#include "agc_image_ops.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_w.h"
/* The C64 runs the cases in two PRGs (LLD_C64_GROUP): group 1 registers
 * every routine but DDUMCALC and P63SPOT2, group 2 those two, with the
 * order pairs each reaches (the ignition iteration, DDUMGOOD and
 * P63SPOT2). Host builds register all. */
#if !defined(LLD_C64_GROUP)
#define LLD_G1 1
#define LLD_G2 1
#else
#define LLD_G1 (LLD_C64_GROUP==1)
#define LLD_G2 (LLD_C64_GROUP==2)
#endif
/* Coverage hook for the host test (empty in flight builds): path a - 02144
 * for each bank-31 instruction address a reached, 0100 + a - 02776 in bank
 * 32; 0400 + LOC - 02154 for each bank-31 order pair, 0440 + LOC - 03020
 * in bank 32; and 0700 + n for each outcome of the file's decisions (BR). */
#ifdef LLD_COUNT_PATHS
extern unsigned lld_paths[];
#define LLD_PATH(n) ((void)++lld_paths[n])
#else
#define LLD_PATH(n) ((void)0)
#endif
#define BR(n) LLD_PATH(0700u+(n))

/* Erasable (E7 under EBANK= E2DPS). */
#define NEWJOB 067u
#define FIXLOC 0120u
#define LOC 0164u
#define BANKSET 0165u
#define PUSHLOC 0166u
#define EDOP 023u
#define EBANK 03u
#define DVTHRUSH 01251u
#define WCHPHASE 01351u
#define WHICH 01455u
#define DVCNTR 01515u
#define FLPASS0 01623u
#define NIGNLOOP 01646u
/* Fixed constants (checked against the rope by tools/lm_lunar_landing_source.py). */
#define K_P63ADRES 02076u
#define K_DPSTHRSH 044u
#define K_FOUR 04u
#define K_ONE 01u
#define K_ZERO 0u
#define K_BIT14 020000u
#define K_MD1 025u         /* ASTNDEX = MD1 */
#define K_EBANK7 03400u
#define K_BIT6 040u
#define K_CODE500 0500u
#define K_V06N43 01453u    /* V06N43* */
/* Other files. */
#define X_BANKCALL 04616u
#define X_POSTJUMP 04635u
#define X_IBNKCALL 04674u
#define X_CHANG2 05126u
#define X_ENDOFJOB 05155u
#define X_PHASCHNG 05353u
#define X_GOTOPOOH 06001u
#define X_INTPRET 06037u
#define NEWOPS 06070u
/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,DLOAD=06052,SLOAD=06501,PDDL=06523,PDVL=06557,PUSH=03247,SETPD=07613,SSP=06620,
 DAD=06775,DSU=07005,BDSU=07062,DMPR=07574,DDV=07577,DSQ=03174,ABVAL=03176,UNIT=03023,GSHIFT=07621,
 MXV=07334,VXV=07460,DOT=07331,RTBBHIZ=02447,BZEGOTO=02467,BPLBMN=02474,CALLITA=02507,SWITCH=02516};

/* Bank-32 data the code reads: P63ADRES, CODE500, 99999CON, GUIDDURN and
 * DDUMCRIT (03253-03262). */
static const agc_word B32DATA[010]={02076u,0500u,035u,030373u,04u,01610u,0u,010u};
/* The interpretive constants UNITZ, UNITY, UNITX, ZEROVECS and HI6ZEROS
 * (INTERPRETIVE_CONSTANT.agc, bank 23 02513-02526). */
static const agc_word B23DATA[014]={0u,0u,0u,0u,020000u,0u,0u,0u,0u,0u,0u,0u};

enum {GO=0,STOP=1,REFUSE=2};
/* Helpers stay out of line: the C64 build has to fit the many call sites. */
#define NOINL __attribute__((noinline,unused))
#define RUN_LIMIT 600u

/* The fixed operand words ADDRWD names after address formation (n words),
 * or *f=0 for an erasable operand. Returns 0 for a fixed address outside
 * the data above. */
static int operand(const apollo_overlay_state *s,unsigned n,const agc_word **f){
 unsigned a=W[0116],bank=W[4]>>10;
 *f=0;
 if(a<02000u)return 1;
 if(a+n>04000u)return 0;
 if(bank==032u&&a>=03253u&&a+n<=03263u){*f=B32DATA+(a-03253u);return 1;}
 if(bank==023u&&a>=02513u&&a+n<=02527u){*f=B23DATA+(a-02513u);return 1;}
 return 0;
}
#define K(n) do{if(!operand(s,(n),&k))return REFUSE;}while(0)

/* ---- Interpretive helpers -------------------------------------------- */
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
/* Address formation: direct, or PUSHUP when the next word is an order pair. */
static NOINL void opnd(apollo_overlay_state *s,agc_word word,agc_word target){
 if(word&040000u)lmi_address_pushup(s,word,target);else lmi_direct(s,word,target);
}
static NOINL void shorts(apollo_overlay_state *s){lmi_short_dispatch(s);if(W[5]==02017u)lmi_shortt(s);else lmi_shortv(s);}
static NOINL void push(apollo_overlay_state *s){lmi_unary(s,PUSH);lmi_push(s);}
static NOINL void gshift(apollo_overlay_state *s,agc_word word){lmi_direct(s,word,GSHIFT);lmi_gshift(s);}
static NOINL int store(apollo_overlay_state *s,agc_word code){lmi_store_code(s,code);return next(s);}
static NOINL int stodl(apollo_overlay_state *s,agc_word code,agc_word word){const agc_word *k;
 lmi_store_code(s,code);lmi_dodload(s);
 if(word&040000u)lmi_diradres_pushup(s,word,DLOAD);else lmi_diradres(s,word,DLOAD);
 K(2);lmi_dload_op(s,k);return next(s);
}
static NOINL int stovl(apollo_overlay_state *s,agc_word code,agc_word word){const agc_word *k;
 lmi_store_code(s,code);lmi_dovload(s);lmi_diradres(s,word,VLOAD);K(6);lmi_vload(s,k);return next(s);
}
/* STCALL: the store, then 15BITADR to CALL and the GOTO. */
static NOINL int stcall(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_15bitadr(s,word,CALLITA);lmi_call(s);lmi_goto(s);return GO;
}
/* Loads and arithmetic with an operand (fixed data through K). */
static NOINL int dload(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,DLOAD);K(2);lmi_dload_op(s,k);return GO;}
static NOINL int vload(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,VLOAD);K(6);lmi_vload(s,k);return GO;}
static NOINL int pddl(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,PDDL);K(2);lmi_pddl_op(s,k);return GO;}
static NOINL int pdvl(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,PDVL);K(6);lmi_pdvl(s,k);return GO;}
static NOINL int dad(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,DAD);K(2);lmi_dad(s,k);return GO;}
static NOINL int dsu(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,DSU);K(2);lmi_dsu(s,k);return GO;}
static NOINL int bdsu(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,BDSU);K(2);lmi_bdsu(s,k);return GO;}
static NOINL int dmpr(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,DMPR);K(2);lmi_dmpr(s,k);return GO;}
static NOINL int ddv(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,DDV);K(2);lmi_ddv(s,k);return GO;}
static NOINL int vxv(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,VXV);K(6);lmi_vxv(s,k);return GO;}
static NOINL int dot(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,DOT);K(6);lmi_dot(s,k);return GO;}
/* The two VSL4 MXV REFSMMAT pairs share one call of the matrix body. */
static NOINL void vsl4_mxv(apollo_overlay_state *s,agc_word word){shorts(s);right(s);opnd(s,word,MXV);lmi_mxv(s,0);}
static NOINL void ssp(apollo_overlay_state *s,agc_word word,agc_word value){lmi_direct(s,word,SSP);lmi_ssp(s,value);}
static NOINL void misc(apollo_overlay_state *s,agc_word word,agc_word target){lmi_misc(s,word,target);}
static NOINL int rtb(apollo_overlay_state *s,agc_word word){misc(s,word,RTBBHIZ);(void)ai_ccs(s,020);lmi_rtb(s);return GO;}
static NOINL int call(apollo_overlay_state *s,agc_word word){misc(s,word,CALLITA);lmi_call(s);lmi_goto(s);return GO;}
/* SET/CLEAR: the switch order, which never branches. */
static NOINL int sw(apollo_overlay_state *s,lm_lld_io *io,agc_word word){misc(s,word,SWITCH);return lmi_switch(s,&io->control)?REFUSE:GO;}
/* UNIT and ABVAL/ABS: GO at DANZIG, STOP at POODOO1 (alarm 1302). */
static NOINL int unit(apollo_overlay_state *s){lmi_unary(s,UNIT);if(lmi_unit(s)){BR(2);return STOP;}return GO;}
static NOINL int abval(apollo_overlay_state *s){lmi_unary(s,ABVAL);if(lmi_abvalabs(s)){BR(3);return STOP;}return GO;}
static NOINL int exit_(apollo_overlay_state *s){lmi_newops(s,077776);lmi_exit(s);return GO;}
/* GO or STOP from a step; REFUSE passes through. */
#define STEP(x) do{int r_=(x);if(r_)return r_;}while(0)

/* One order-pair word at LOC (Z at NEWOPS). */
static int interpretive(apollo_overlay_state *s,lm_lld_io *io){
 (void)io;
 if(W[LOC]>=02154u&&W[LOC]<=02213u)LLD_PATH(0400u+W[LOC]-02154u);
 else if(W[LOC]>=03020u&&W[LOC]<=03222u)LLD_PATH(0440u+W[LOC]-03020u);
 switch(W[LOC]){
#if LLD_G1
 /* LANDJUNK (bank 31): SET CLEAR SURFFLAG LETABORT; SET VLOAD APSFLAG RN;
  * STODL ALPHAV PIPTIME; SET CALL LUNAFLAG LAT-LONG. */
 case 02154:lmi_newops(s,043014);DO(sw(s,io,004067));right(s);DO(sw(s,io,004666));NEXT;
 case 02157:lmi_newops(s,077214);DO(sw(s,io,005062));right(s);DO(vload(s,001221));NEXT;
 case 02162:return stodl(s,016032,001235);
 case 02164:lmi_newops(s,045014);DO(sw(s,io,001463));right(s);return call(s,026351);
 /* SETPD VLOAD 0 RN; VSL2 PDDL PIPTIME; PUSH CALL R-TO-RP; STORE RLS; EXIT. */
 case 02167:lmi_newops(s,077201);lmi_direct(s,000001,SETPD);lmi_setpd(s);lmi_noibnksw_right(s);DO(vload(s,001221));NEXT;
 case 02172:lmi_newops(s,065352);shorts(s);right(s);DO(pddl(s,001235));NEXT;
 case 02174:lmi_newops(s,045006);push(s);right(s);return call(s,051670);
 case 02176:return store(s,002023);
 case 02177:return exit_(s);
 /* VLOAD UNITX; STCALL GSAV REFMF; EXIT. */
 case 02207:lmi_newops(s,077775);DO(vload(s,006520));NEXT;
 case 02211:return stcall(s,036231,033506);
 case 02213:return exit_(s);
 /* FLAGORGY (bank 32): CLEAR CLEAR NOTHROTL REDFLAG; CLEAR SET LRBYPASS
  * MUNFLAG; CLEAR CLEAR P25FLAG RNDVZFLG. */
 case 03020:lmi_newops(s,043014);DO(sw(s,io,002663));right(s);DO(sw(s,io,003271));NEXT;
 case 03023:lmi_newops(s,043014);DO(sw(s,io,005660));right(s);DO(sw(s,io,003067));NEXT;
 case 03026:lmi_newops(s,043014);DO(sw(s,io,000266));right(s);DO(sw(s,io,000270));NEXT;
 /* IGNALG SETPD VLOAD 0 RLS; PDDL PUSH TLAND; STCALL TPIP RP-TO-R; VSL4 MXV
  * REFSMMAT; STCALL LAND GUIDINIT; DLOAD DSU TLAND GUIDDURN; STCALL TDEC1
  * LEMPREC; SSP VLOAD NIGNLOOP 40D UNITX; STOVL CG UNITY; STOVL CG +6
  * UNITZ; STODL CG +14 99999CON; STOVL DELTAH ZEROVECS; STODL UNFC/2
  * HI6ZEROS; STORE TTF/8. */
 case 03031:lmi_newops(s,077201);lmi_direct(s,000001,SETPD);lmi_setpd(s);lmi_noibnksw_right(s);DO(vload(s,002023));NEXT;
 case 03034:lmi_newops(s,041525);DO(pddl(s,002401));right(s);push(s);NEXT;
 case 03036:return stcall(s,037625,055716);
 case 03040:lmi_newops(s,064312);vsl4_mxv(s,001734);NEXT;
 case 03042:return stcall(s,037635,046432);
 case 03044:lmi_newops(s,045345);DO(dload(s,002401));right(s);DO(dsu(s,025260));NEXT;
 case 03047:return stcall(s,034041,027057);
 case 03051:lmi_newops(s,077331);ssp(s,003647,000050);right(s);DO(vload(s,006520));NEXT;
 case 03055:return stovl(s,026603,006516);
 case 03057:return stovl(s,026611,006514);
 case 03061:return stodl(s,016617,025256);
 case 03063:return stovl(s,027665,006522);
 case 03065:return stodl(s,017254,006522);
 case 03067:return store(s,003643);
 /* IGNALOOP DLOAD TAT; STOVL PIPTIME1 RATT1; VSL4 MXV REFSMMAT; STCALL R
  * MUNGRAV; STCALL GDT/2 ?GUIDSUB. */
 case 03070:lmi_newops(s,077745);DO(dload(s,000015));NEXT;
 case 03072:return stovl(s,027561,000017);
 case 03074:lmi_newops(s,064312);vsl4_mxv(s,001734);NEXT;
 case 03076:return stcall(s,037521,067130);
 case 03100:return stcall(s,035237,062454);
#endif
#if LLD_G2
 /* After DDUMCALC: DLOAD DMPR VGU KIGNX/B4; SL4R BDSU VGU +4; PDDL DSU
  * RIGNZ RGU +4; SR4R PDDL RGU +2; DSQ DMPR KIGNY/B8; SL4R PDDL RGU; DSU
  * DMPR RIGNX KIGNX/B4; PDVL ABVAL VGU; DSU DMPR VIGN KIGNV/B4; DAD DAD;
  * DAD DDV; SRR 10D; PUSH DAD PIPTIME1; STODL TDEC1; ABS DSU DDUMCRIT; BMN
  * CALL DDUMGOOD INTSTALL. (The four orders after PDVL ABVAL take their
  * operands from the pushdown list.) */
 case 03104:lmi_newops(s,057345);DO(dload(s,003627));right(s);DO(dmpr(s,002501));NEXT;
 case 03107:lmi_newops(s,044232);shorts(s);right(s);DO(bdsu(s,003633));NEXT;
 case 03111:lmi_newops(s,045325);DO(pddl(s,002477));right(s);DO(dsu(s,002643));NEXT;
 case 03114:lmi_newops(s,065222);shorts(s);right(s);DO(pddl(s,002641));NEXT;
 case 03116:lmi_newops(s,057316);lmi_unary(s,DSQ);lmi_dsq(s);right(s);DO(dmpr(s,002503));NEXT;
 case 03120:lmi_newops(s,065232);shorts(s);right(s);DO(pddl(s,002637));NEXT;
 case 03122:lmi_newops(s,057225);DO(dsu(s,002475));right(s);DO(dmpr(s,002501));NEXT;
 case 03125:lmi_newops(s,051515);DO(pdvl(s,003627));right(s);STEP(abval(s));NEXT;
 case 03127:lmi_newops(s,057225);DO(dsu(s,002473));right(s);DO(dmpr(s,002505));NEXT;
 case 03132:lmi_newops(s,043215);DO(dad(s,056215));right(s);DO(dad(s,056215));NEXT;
 case 03133:lmi_newops(s,056215);DO(dad(s,077661));right(s);DO(ddv(s,077661));NEXT;
 case 03134:lmi_newops(s,077661);gshift(s,021613);NEXT;
 case 03136:lmi_newops(s,043206);push(s);right(s);DO(dad(s,003561));NEXT;
 case 03140:return stodl(s,014041,045246);
 case 03141:lmi_newops(s,045246);STEP(abval(s));right(s);DO(dsu(s,025262));NEXT;
 case 03143:lmi_newops(s,045040);misc(s,065163,BPLBMN);if(lmi_bplbmn(s)){BR(4);lmi_goto(s);return GO;}BR(5);right(s);return call(s,027414);
#endif
#if LLD_G1
 /* SET SET INTYPFLG MOONFLAG; DLOAD PIPTIME1; STOVL TET RATT1; STOVL RCV
  * VATT1; STCALL VCV INTEGRVS; GOTO IGNALOOP. */
 case 03146:lmi_newops(s,043014);DO(sw(s,io,001473));right(s);DO(sw(s,io,000063));NEXT;
 case 03151:lmi_newops(s,077745);DO(dload(s,003561));NEXT;
 case 03153:return stovl(s,025517,000017);
 case 03155:return stovl(s,025535,000025);
 case 03157:return stcall(s,035543,027107);
 case 03161:lmi_newops(s,077650);misc(s,065070,BZEGOTO);lmi_goto_order(s);return GO;
#endif
#if LLD_G2
 /* DDUMGOOD SLOAD SR ZOOMTIME 14D; BDSU TDEC1; STOVL TIG V; VXV UNIT R;
  * DOT SL1 LAND; R60INIT STOVL OUTOFPLN UNFC/2; STORE R60VSAVE; EXIT. */
 case 03163:lmi_newops(s,054335);lmi_direct(s,003423,SLOAD);lmi_sload(s);right(s);gshift(s,020617);NEXT;
 case 03166:lmi_newops(s,077621);DO(bdsu(s,000041));NEXT;
 case 03170:return stovl(s,027442,003527);
 case 03172:lmi_newops(s,053435);DO(vxv(s,003521));right(s);STEP(unit(s));NEXT;
 case 03174:lmi_newops(s,072441);DO(dot(s,003635));right(s);shorts(s);NEXT;
 case 03176:return stovl(s,026627,003254);
 case 03200:return store(s,002631);
 case 03201:return exit_(s);
#endif
#if LLD_G1
 /* ASTNRET SSP RTB QMAJ (FCADR P63SPOT2) R51P63. */
 case 03211:lmi_newops(s,047131);ssp(s,002747,065215);right(s);return rtb(s,031135);
#endif
#if LLD_G2
 /* P63SPOT2 VLOAD UNIT R60VSAVE; STOVL POINTVSM UNITX; STORE SCAXIS; EXIT. */
 case 03215:lmi_newops(s,053575);DO(vload(s,002631));right(s);STEP(unit(s));NEXT;
 case 03217:return stovl(s,027773,006520);
 case 03221:return store(s,003765);
 case 03222:return exit_(s);
#endif
 default:return REFUSE;
 }
}

/* ---- Basic helpers ------------------------------------------------------ */
static NOINL int out(apollo_overlay_state *s,agc_word q,agc_word z){ai_tc(s,q);W[5]=z;return STOP;}
static NOINL int jump(apollo_overlay_state *s,agc_word z){W[5]=z;return STOP;}
static NOINL void intpret(apollo_overlay_state *s,lm_lld_io *io,agc_word ret,agc_word z){ai_tc(s,ret);W[5]=z;lmi_intpret(s,&io->control);}
static agc_word oc(agc_register a){return (agc_word)((a&037777u)|((a>>1)&040000u));}
/* Channel orders, as yaAGC's: RAND and WAND. */
static NOINL void rand_(apollo_overlay_state *s,agc_word chan){W[0]=agc_extend((agc_word)(oc(W[0])&chan));}
static NOINL void wand(apollo_overlay_state *s,agc_word *chan){agc_word v=(agc_word)(oc(W[0])&*chan);*chan=v;W[0]=agc_extend(v);}
/* A case label (and goto label) at each instruction address; straight-line
 * code falls through from one instruction to the next, as the source does. */
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#define AT(a) case a: L##a: __attribute__((unused)) LLD_PATH((a)<02776u?(a)-02144u:0100u+(a)-02776u)
#define GOTO(a) goto L##a
#define TC(r,a) do{ai_tc(s,(r));GOTO(a);}while(0)

/* Basic code at Z (bank 31 or 32, which here() has checked against FB). */
static int basic(apollo_overlay_state *s,lm_lld_io *io){
 switch(W[5]){
#if LLD_G1
 /* LANDJUNK TC PHASCHNG (OCT 04024); INHINT; TC BANKCALL (CADR ZATTEROR);
  * TC BANKCALL (CADR SETMAXDB); TC INTPRET. */
 AT(02144);return out(s,02145,X_PHASCHNG);
 AT(02146);io->control.interrupt_enabled=0;
 AT(02147);return out(s,02150,X_BANKCALL);
 AT(02151);return out(s,02152,X_BANKCALL);
 AT(02153);intpret(s,io,02154,X_INTPRET);return GO;
 /* After EXIT: CAF V06N43*; TC BANKCALL (CADR GOFLASH); TCF GOTOPOOH;
  * TCF +2; TCF -5; TC INTPRET. */
 AT(02200);ai_caf(s,K_V06N43);
 AT(02201);return out(s,02202,X_BANKCALL);
 AT(02203);return jump(s,X_GOTOPOOH);
 AT(02204);GOTO(02206);
 AT(02205);GOTO(02200);
 AT(02206);intpret(s,io,02207,X_INTPRET);return GO;
 /* After EXIT: TCF GOTOPOOH. */
 AT(02214);return jump(s,X_GOTOPOOH);
 /* P63LM TC PHASCHNG (OCT 04024); TC BANKCALL (CADR R02BOTH). */
 AT(02776);return out(s,02777,X_PHASCHNG);
 AT(03000);return out(s,03001,X_BANKCALL);
 /* CAF P63ADRES; TS WHICH; CAF DPSTHRSH; TS DVTHRUSH; CAF FOUR; TS
  * DVCNTR; CS ONE; TS WCHPHASE; CA ZERO; TS FLPASS0; CS BIT14; EXTEND;
  * WAND CHAN12; FLAGORGY TC INTPRET. TS of a loaded constant does not
  * skip. */
 AT(03002);ai_caf(s,K_P63ADRES);
 AT(03003);(void)ai_ts(s,WHICH);
 AT(03004);ai_caf(s,K_DPSTHRSH);
 AT(03005);(void)ai_ts(s,DVTHRUSH);
 AT(03006);ai_caf(s,K_FOUR);
 AT(03007);(void)ai_ts(s,DVCNTR);
 AT(03010);ai_csf(s,K_ONE);
 AT(03011);(void)ai_ts(s,WCHPHASE);
 AT(03012);ai_caf(s,K_ZERO);
 AT(03013);(void)ai_ts(s,FLPASS0);
 AT(03014);ai_csf(s,K_BIT14);
 AT(03015);
 AT(03016);wand(s,&io->chan12);
 AT(03017);intpret(s,io,03020,X_INTPRET);return GO;
#endif
 /* DDUMCALC TS NIGNLOOP; TC INTPRET. With overflow in A the TS would skip
  * into the order words; no producer does that. */
 AT(03102);if(oc(W[0])!=(agc_word)(W[0]&077777u))return REFUSE;
  (void)ai_ts(s,NIGNLOOP);
 AT(03103);intpret(s,io,03104,X_INTPRET);return GO;
 /* After EXIT: IGNALGRT TC PHASCHNG (OCT 04024); ASTNCLOK CS ASTNDEX; TC
  * BANKCALL (CADR STCLOK2); TCF ENDOFJOB; ASTNRET TC INTPRET. */
 AT(03202);return out(s,03203,X_PHASCHNG);
#if LLD_G1
 AT(03204);ai_csf(s,K_MD1);
 AT(03205);return out(s,03206,X_BANKCALL);
 AT(03207);return jump(s,X_ENDOFJOB);
 AT(03210);intpret(s,io,03211,X_INTPRET);return GO;
#endif
 /* After EXIT: CAF EBANK7; TS EBANK; INHINT; TC IBNKCALL (CADR PFLITEDB);
  * RELINT; TC BANKCALL (CADR R60LEM); TC PHASCHNG (OCT 04024). */
 AT(03223);ai_caf(s,K_EBANK7);
 AT(03224);(void)ai_ts(s,EBANK);
 AT(03225);io->control.interrupt_enabled=0;
 AT(03226);return out(s,03227,X_IBNKCALL);
#if LLD_G1
 AT(03230);io->control.interrupt_enabled=1;
 AT(03231);return out(s,03232,X_BANKCALL);
 AT(03233);return out(s,03234,X_PHASCHNG);
 /* P63SPOT3 CA BIT6; EXTEND; RAND CHAN33; EXTEND; BZF P63SPOT4; CAF
  * CODE500; TC BANKCALL (CADR GOPERF1); TCF GOTOPOOH; TCF P63SPOT3;
  * P63SPOT4 TC BANKCALL (CADR SETPOS1); TC POSTJUMP (CADR BURNBABY). */
 AT(03235);ai_caf(s,K_BIT6);
 AT(03236);
 AT(03237);rand_(s,io->chan33);
 AT(03240);
 AT(03241);if(ai_bzf(s)){BR(6);GOTO(03247);}
 BR(7);
 AT(03242);ai_caf(s,K_CODE500);
 AT(03243);return out(s,03244,X_BANKCALL);
 AT(03245);return jump(s,X_GOTOPOOH);
 AT(03246);GOTO(03235);
 AT(03247);return out(s,03250,X_BANKCALL);
 AT(03251);return out(s,03252,X_POSTJUMP);
#endif
 default:return REFUSE;
 }
}

/* The file's words: bank-31 code 02144-02214, bank-32 code 02776-03252. */
static int in_code(uint16_t ph){
 return (ph>=031u*1024u+0144u&&ph<=031u*1024u+0214u)||(ph>=032u*1024u+0776u&&ph<=032u*1024u+01252u);
}
/* Is Z (or LOC at NEWOPS) the file's code? */
static int here(const apollo_overlay_state *s,const lm_lld_io *io,int *interp){
 agc_word z=W[5];
 *interp=0;
 if(z==NEWOPS){*interp=1;return in_code(agc_fixed_physical(W[LOC],W[BANKSET],io->control.superbank));}
 if(z<02000u||z>=04000u)return 0;
 return in_code(agc_fixed_physical(z,W[4],io->control.superbank));
}
/* FIXLOC one of the five VAC areas, PUSHLOC inside it, no order code left
 * over in EDOP (every NEWOPS the file reaches follows a whole pair or a
 * store code). */
static int work_area(const apollo_overlay_state *s){
 agc_word f=W[FIXLOC],p=W[PUSHLOC];
 if(W[EDOP])return 0;
 if(f!=0401u&&f!=0455u&&f!=0531u&&f!=0605u&&f!=0661u)return 0;
 return p>=f&&p<=f+042u;
}
static enum exec_status steps(apollo_overlay_state *s,lm_lld_io *io){
 unsigned n;int st,interp;
 for(n=0;n<RUN_LIMIT;++n){
  if(!here(s,io,&interp))return n?EXEC_OK:EXEC_OUT_OF_SCOPE;
  /* NEWOPS reads the order word under FB, which DANZIG set from BANKSET. */
#ifdef AGC_FLIGHT_INTERPRETER
  /* Flight: the native interpreter runs the file's interpretive code
   * from NEWOPS, so a chain stops where it reaches it. */
  if(interp)return n?EXEC_OK:EXEC_OUT_OF_SCOPE;
#endif
  if(interp&&(((W[4]^W[BANKSET])&076000u)||!work_area(s)))return EXEC_OUT_OF_SCOPE;
  st=interp?interpretive(s,io):basic(s,io);
  if(st==REFUSE)return EXEC_OUT_OF_SCOPE;
  if(st==STOP)return EXEC_OK;
 }
 return EXEC_OUT_OF_SCOPE;
}
static enum exec_status run(apollo_overlay_state *s,void *v){
 lm_lld_io *io=v,entry;enum exec_status st;
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
#define B31(a) (uint16_t)(031u*1024u+((a)-02000u))
#define B32(a) (uint16_t)(032u*1024u+((a)-02000u))
#define R31(a) {B31(a),routine}
#define R32(a) {B32(a),routine}
/* Flight builds also key the basic word each EXIT returns to: there the
 * native interpreter runs the interpretive code and its EXIT hands the word
 * to the flight program as a basic key (tools/flight_exit_returns.py). The
 * host build runs on across EXIT in one call and keys none of them. */
#ifdef AGC_FLIGHT_INTERPRETER
#define X31(a) R31(a),
#define X32(a) R32(a),
#else
#define X31(a)
#define X32(a)
#endif
static const agc_routine_entry basic_entries[]={
#if LLD_G1
 R31(02144),R31(02146),R31(02151),R31(02153),X31(02200)R31(02203),R31(02204),R31(02205),X31(02214)
 R32(02776),R32(03000),R32(03002),
#endif
#if LLD_G2
 R32(03102),
#endif
#if LLD_G1
 X32(03202)R32(03204),R32(03207),R32(03210),X32(03223)R32(03230),R32(03233),
 R32(03235),R32(03245),R32(03246),R32(03247),R32(03251),
#endif
};
static const agc_routine_entry interp_entries[]={
#if LLD_G1
 R31(02154),R31(02157),R31(02162),R31(02164),R31(02167),R31(02172),R31(02174),R31(02176),
 R31(02177),R31(02207),R31(02211),R31(02213),R32(03020),R32(03023),R32(03026),R32(03031),
 R32(03034),R32(03036),R32(03040),R32(03042),R32(03044),R32(03047),R32(03051),R32(03055),
 R32(03057),R32(03061),R32(03063),R32(03065),R32(03067),R32(03070),R32(03072),R32(03074),
 R32(03076),R32(03100),
#endif
#if LLD_G2
 R32(03104),R32(03107),R32(03111),R32(03114),R32(03116),R32(03120),R32(03122),R32(03125),
 R32(03127),R32(03132),R32(03133),R32(03134),R32(03136),R32(03140),R32(03141),R32(03143),
#endif
#if LLD_G1
 R32(03146),R32(03151),R32(03153),R32(03155),R32(03157),R32(03161),
#endif
#if LLD_G2
 R32(03163),R32(03166),R32(03170),R32(03172),R32(03174),R32(03176),R32(03200),R32(03201),
#endif
#if LLD_G1
 R32(03211),
#endif
#if LLD_G2
 R32(03215),R32(03217),R32(03221),R32(03222),
#endif
};
#define COUNT(t) (uint16_t)(sizeof t/sizeof t[0])
const agc_routine_table lm_lunar_landing_routines={basic_entries,COUNT(basic_entries)};
const agc_routine_table lm_lunar_landing_interpretive={interp_entries,COUNT(interp_entries)};
const uint16_t lld_routine_keys[LLD_ROUTINES]={
 B31(02144),B31(02146),B31(02151),B31(02153),B31(02203),B31(02204),B31(02205),
 B32(02776),B32(03000),B32(03002),B32(03102),B32(03204),B32(03207),B32(03210),B32(03230),B32(03233),
 B32(03235),B32(03245),B32(03246),B32(03247),B32(03251),
 B31(02154),B31(02157),B31(02162),B31(02164),B31(02167),B31(02172),B31(02174),B31(02176),
 B31(02177),B31(02207),B31(02211),B31(02213),B32(03020),B32(03023),B32(03026),B32(03031),
 B32(03034),B32(03036),B32(03040),B32(03042),B32(03044),B32(03047),B32(03051),B32(03055),
 B32(03057),B32(03061),B32(03063),B32(03065),B32(03067),B32(03070),B32(03072),B32(03074),
 B32(03076),B32(03100),B32(03104),B32(03107),B32(03111),B32(03114),B32(03116),B32(03120),
 B32(03122),B32(03125),B32(03127),B32(03132),B32(03133),B32(03134),B32(03136),B32(03140),
 B32(03141),B32(03143),B32(03146),B32(03151),B32(03153),B32(03155),B32(03157),B32(03161),
 B32(03163),B32(03166),B32(03170),B32(03172),B32(03174),B32(03176),B32(03200),B32(03201),
 B32(03211),B32(03215),B32(03217),B32(03221),B32(03222)};
