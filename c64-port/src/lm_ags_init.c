/* SPDX-License-Identifier: MIT */
/* Luminary099 AGS_INITIALIZATION (R47, bank 32) as native routines of the
 * control-transfer ABI; see include/lm_ags_init.h and docs/LM-AGS-INIT.md.
 * Basic code: each instruction is one ai_* effect on the image in source
 * order, under a case label at its address; a TC or TCF within the file is
 * a goto, one to another file ends the run with Q and Z set. Interpretive
 * code: each order-pair word is a case of its own, running the compiled
 * lm_interp_steps/lm_interp_orders paths with the pair's words as literal
 * arguments; branches follow the outcomes those paths return. */
#include "lm_ags_init.h"
#include "agc_image_ops.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#define W s->words
/* Coverage hook for the host test (empty in flight builds): path a - 02005
 * for each basic instruction address a reached, 0200 + the pair's offset
 * for each order pair (02037-02071 and SCALEVEC's 02132-02167 at 0240),
 * and 0300 + n for each outcome of the file's decisions (BR). */
#ifdef AGS_COUNT_PATHS
extern unsigned ags_paths[];
#define AGS_PATH(n) ((void)++ags_paths[n])
#else
#define AGS_PATH(n) ((void)0)
#endif
#define BR(n) AGS_PATH(0300u+(n))

/* Erasable (AGSK in the EBANK= AGSBUFF window, E4). */
#define A_ 0u
#define SAMPTIME 013u
#define NEWJOB 067u
#define FLAGWRD0 074u
#define FLAGWRD3 077u
#define FIXLOC 0120u
#define MPAC 0154u
#define LOC 0164u
#define BANKSET 0165u
#define PUSHLOC 0166u
#define EDOP 023u
#define DNLSTCOD 0332u
#define DSPTEMX 01051u
#define IMUCADR 01304u
#define AGSWORD 01324u
#define AGSK 01420u
/* Fixed constants (checked against the rope by tools/lm_ags_init_source.py). */
#define K_ONE 01u          /* LAGSLIST = ONE */
#define K_TEN 012u
#define K_BIT6 040u
#define K_IMUSEBIT 0200u   /* BIT8 */
#define K_REFSMBIT 010000u /* BIT13 */
#define K_V06N16 01420u
#define K_V50N16 014420u
#define K_20SEC 03720u
/* Other files. */
#define X_BANKCALL 04616u
#define X_CHANG2 05126u
#define X_ENDEXT 05472u
#define X_DOWNFLAG 05516u
#define X_ALARM 05567u
#define X_INTPRET 06037u
#define NEWOPS 06070u
/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,DLOAD=06052,DSU=07005,DDV=07577,VAD=06751,VXSC=07401,MXV=07334,
 LXA=02354,SXA=02364,RVQ=03274,EXIT=06743,RTBBHIZ=02447,CALLITA=02507,SWITCH=02516};

/* Bank-32 data the interpretive operands name: TSCALE, 20SEC, RSCALE,
 * VSCALE, AGSRND1 and AGSRND2 (02174-02216). */
static const agc_word B32DATA[023]={03100u,0u,03720u,015077u,05041u,024402u,025724u,
 0u,060000u,0u,060000u,0u,060000u,0u,037777u,0u,037777u,0u,037777u};

enum {GO=0,STOP=1,REFUSE=2};
#define RUN_LIMIT 400u

/* The fixed operand words ADDRWD names after address formation (n words),
 * or 0 for an erasable operand. Returns 0 for a fixed address outside the
 * data above. */
static int operand(const apollo_overlay_state *s,unsigned n,const agc_word **k){
 unsigned a=W[0116],bank=W[4]>>10;
 *k=0;
 if(a<02000u)return 1;
 if(bank==032u&&a>=02174u&&a+n<=02217u){*k=B32DATA+(a-02174u);return 1;}
 return 0;
}
#define K(n) do{if(!operand(s,(n),&k))return REFUSE;}while(0)

/* ---- Interpretive helpers -------------------------------------------- */
/* DANZIG at the end of a pair: NEWJOB +0 goes on to NEWOPS; positive goes
 * to CHANG2 (CA BANKSET; TS BBANK; CCS EDOP; CCS NEWJOB; TCF CHANG2). */
static int next(apollo_overlay_state *s){
 agc_word j=W[NEWJOB];
 if(j==0u){BR(16);lmi_danzig_next(s);return GO;}
 if(j>037777u)return REFUSE;
 BR(17);ai_ca(s,BANKSET);(void)ai_ts(s,6);(void)ai_ccs(s,EDOP);(void)ai_ccs(s,NEWJOB);W[5]=X_CHANG2;return STOP;
}
#define NEXT return next(s)
static void right(apollo_overlay_state *s){lmi_danzig_right(s);}
static void opnd(apollo_overlay_state *s,agc_word word,agc_word target){lmi_direct(s,word,target);}
static void shorts(apollo_overlay_state *s){lmi_short_dispatch(s);if(W[5]==02017u)lmi_shortt(s);else lmi_shortv(s);}
static int store(apollo_overlay_state *s,agc_word code){lmi_store_code(s,code);return next(s);}
static int stodl(apollo_overlay_state *s,agc_word code,agc_word word){const agc_word *k;
 lmi_store_code(s,code);lmi_dodload(s);lmi_diradres(s,word,DLOAD);K(2);lmi_dload_op(s,k);return next(s);
}
static int stovl(apollo_overlay_state *s,agc_word code,agc_word word){const agc_word *k;
 lmi_store_code(s,code);lmi_dovload(s);lmi_diradres(s,word,VLOAD);K(6);lmi_vload(s,k);return next(s);
}
/* SCALEVEC's two MXV REFSMMAT orders share one call of the matrix body. */
static __attribute__((noinline)) void mxv_refsmmat(apollo_overlay_state *s,agc_word word){opnd(s,word,MXV);lmi_mxv(s,0);}
static int vad(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,VAD);K(6);lmi_vad(s,k);return GO;}
static int vxsc(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,VXSC);K(2);lmi_vxsc(s,k);return GO;}
#define DO(x) do{if(x)return REFUSE;}while(0)
static void misc(apollo_overlay_state *s,agc_word word,agc_word target){lmi_misc(s,word,target);}
static int rtb(apollo_overlay_state *s,agc_word word){misc(s,word,RTBBHIZ);(void)ai_ccs(s,020);lmi_rtb(s);return GO;}
static int call(apollo_overlay_state *s,agc_word word){misc(s,word,CALLITA);lmi_call(s);lmi_goto(s);return GO;}
/* STCALL: the store, then 15BITADR to CALL and the GOTO. */
static int stcall(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_15bitadr(s,word,CALLITA);lmi_call(s);lmi_goto(s);return GO;
}
/* SET: the switch order, which never branches. */
static int set(apollo_overlay_state *s,lm_ags_io *io,agc_word word){misc(s,word,SWITCH);return lmi_switch(s,&io->control)?REFUSE:GO;}
static int rvq(apollo_overlay_state *s){
 if(!(W[(W[FIXLOC]+052u)&03777u]&074000u))return REFUSE;
 lmi_unary(s,RVQ);lmi_rvq(s);return GO;
}
static int exit_(apollo_overlay_state *s){lmi_newops(s,077776);lmi_exit(s);return GO;}

/* One order-pair word at LOC (Z at NEWOPS). */
static int interpretive(apollo_overlay_state *s,lm_ags_io *io){
 const agc_word *k;
 if(W[LOC]>=02037u&&W[LOC]<=02071u)AGS_PATH(0200u+W[LOC]-02037u);
 else if(W[LOC]>=02132u&&W[LOC]<=02167u)AGS_PATH(0240u+W[LOC]-02132u);
 switch(W[LOC]){
 /* AGSVCALC SET NODOFLAG; SET EXIT XDSPFLAG. */
 case 02037:lmi_newops(s,077614);DO(set(s,io,001076));NEXT;
 case 02041:lmi_newops(s,077414);DO(set(s,io,002076));right(s);lmi_unary(s,EXIT);lmi_exit(s);return GO;
 /* RTB LOADTIME; STCALL TDEC1 LEMPREC; CALL SCALEVEC; STODL AGSBUFF TAT;
  * STCALL TDEC1 CSMPREC; CALL SCALEVEC; STODL AGSBUFF +6 TAT; DSU DDV AGSK
  * TSCALE; STORE AGSBUFF +12D; EXIT. */
 case 02047:lmi_newops(s,077634);return rtb(s,021573);
 case 02051:return stcall(s,034041,027057);
 case 02053:lmi_newops(s,077624);return call(s,064132);
 case 02055:return stodl(s,016201,000015);
 case 02057:return stcall(s,034041,027043);
 case 02061:lmi_newops(s,077624);return call(s,064132);
 case 02063:return stodl(s,016207,000015);
 case 02065:lmi_newops(s,056225);opnd(s,002021,DSU);K(2);lmi_dsu(s,k);right(s);opnd(s,024175,DDV);K(2);lmi_ddv(s,k);NEXT;
 case 02070:return store(s,002215);
 case 02071:return exit_(s);
 /* SCALEVEC VLOAD MXV VATT1 REFSMMAT; VXSC VSL2 VSCALE; VAD VAD AGSRND1
  * AGSRND2; RTB VECSGNAG; STOVL VATT1 RATT1; MXV VXSC REFSMMAT RSCALE;
  * VSL8 VAD AGSRND1; VAD RTB AGSRND2 VECSGNAG; LXA,1 VATT1; SXA,1 LXA,1
  * MPAC +1 VATT1 +2; SXA,1 LXA,1 MPAC +4 VATT1 +4; SXA,1 RVQ MPAC +6. */
 case 02132:lmi_newops(s,064375);opnd(s,000025,VLOAD);K(6);lmi_vload(s,k);right(s);mxv_refsmmat(s,001734);NEXT;
 case 02135:lmi_newops(s,072561);DO(vxsc(s,024202));right(s);shorts(s);NEXT;
 case 02137:lmi_newops(s,053255);DO(vad(s,024204));right(s);DO(vad(s,024212));NEXT;
 case 02142:lmi_newops(s,077634);return rtb(s,021772);
 case 02144:return stovl(s,024025,000017);
 case 02146:lmi_newops(s,074321);mxv_refsmmat(s,001734);right(s);DO(vxsc(s,024200));NEXT;
 case 02151:lmi_newops(s,053212);shorts(s);right(s);DO(vad(s,024204));NEXT;
 case 02153:lmi_newops(s,047055);DO(vad(s,024212));right(s);return rtb(s,021772);
 case 02156:lmi_newops(s,077750);misc(s,000024,LXA);lmi_lxa(s);NEXT;
 case 02160:lmi_newops(s,072130);misc(s,000155,SXA);lmi_sxa(s);right(s);misc(s,000026,LXA);lmi_lxa(s);NEXT;
 case 02163:lmi_newops(s,072130);misc(s,000160,SXA);lmi_sxa(s);right(s);misc(s,000030,LXA);lmi_lxa(s);NEXT;
 case 02166:lmi_newops(s,043530);misc(s,000162,SXA);lmi_sxa(s);right(s);return rvq(s);
 default:return REFUSE;
 }
}

/* ---- Basic helpers ------------------------------------------------------ */
static int out(apollo_overlay_state *s,agc_word q,agc_word z){ai_tc(s,q);W[5]=z;return STOP;}
static int jump(apollo_overlay_state *s,agc_word z){W[5]=z;return STOP;}
static void intpret(apollo_overlay_state *s,lm_ags_io *io,agc_word ret,agc_word z){ai_tc(s,ret);W[5]=z;lmi_intpret(s,&io->control);}
/* A case label (and goto label) at each instruction address; straight-line
 * code falls through from one instruction to the next, as the source does. */
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#define AT(a) case a: L##a: __attribute__((unused)) AGS_PATH((a)-02005u)
#define GOTO(a) goto L##a
#define TC(r,a) do{ai_tc(s,(r));GOTO(a);}while(0)

/* Basic code at Z (bank 32). The CCS exits of a MASK result past the
 * first two run the next words as the original does: AGSINIT's -0 exit
 * reaches TC ENDEXT, its negative exit OCT 220 (TC 00220); the IMUSE
 * test's reach CKSTALL's CCS IMUCADR and TCF +3. */
static int basic(apollo_overlay_state *s,lm_ags_io *io){
 uint8_t c;
 switch(W[5]){
 /* AGSINIT CAF REFSMBIT; MASK FLAGWRD3; CCS A; TC REDSPTEM; TC ALARM (OCT
  * 220); TC ENDEXT. */
 AT(02005);ai_caf(s,K_REFSMBIT);
 AT(02006);ai_mask(s,FLAGWRD3);
 AT(02007);c=ai_ccs(s,A_);BR(c);
  if(c==1u)GOTO(02011);
  if(c==2u)return out(s,02013,00220u);
  if(c==3u)GOTO(02013);
 AT(02010);TC(02011,02017);
 AT(02011);return out(s,02012,X_ALARM);
 AT(02013);return out(s,02014,X_ENDEXT);
 /* NEWAGS EXTEND; DCA SAMPTIME; DXCH AGSK. */
 AT(02014);
 AT(02015);ai_dca(s,SAMPTIME);
 AT(02016);ai_dxch(s,AGSK);
 /* REDSPTEM EXTEND; DCA AGSK; DXCH DSPTEMX. */
 AT(02017);
 AT(02020);ai_dca(s,AGSK);
 AT(02021);ai_dxch(s,DSPTEMX);
 /* AGSDISPK CAF V06N16; TC BANKCALL (CADR GOMARKF). */
 AT(02022);ai_caf(s,K_V06N16);
 AT(02023);return out(s,02024,X_BANKCALL);
 /* GOMARKF's returns: TC ENDEXT; TC AGSVCALC; CS BIT6; AD MPAC; EXTEND;
  * BZF NEWAGS; EXTEND; DCA DSPTEMX; TC REDSPTEM -1. */
 AT(02025);return out(s,02026,X_ENDEXT);
 AT(02026);TC(02027,02036);
 AT(02027);ai_csf(s,K_BIT6);
 AT(02030);ai_ad(s,MPAC);
 AT(02031);
 AT(02032);if(ai_bzf(s)){BR(4);GOTO(02014);}
 BR(5);
 AT(02033);
 AT(02034);ai_dca(s,DSPTEMX);
 AT(02035);TC(02036,02016);
 /* AGSVCALC TC INTPRET. */
 AT(02036);intpret(s,io,02037,X_INTPRET);return GO;
 /* After SET EXIT: CAF V06N16; TC BANKCALL (CADR EXDSPRET); TC INTPRET. */
 AT(02043);ai_caf(s,K_V06N16);
 AT(02044);return out(s,02045,X_BANKCALL);
 AT(02046);intpret(s,io,02047,X_INTPRET);return GO;
 /* After EXIT: CAF LAGSLIST; TS DNLSTCOD; CAF 20SEC; TC BANKCALL (CADR
  * DELAYJOB). TS of a CAF'd constant does not skip. */
 AT(02072);ai_caf(s,K_ONE);
 AT(02073);(void)ai_ts(s,DNLSTCOD);
 AT(02074);ai_caf(s,K_20SEC);
 AT(02075);return out(s,02076,X_BANKCALL);
 /* CA AGSWORD; TS DNLSTCOD (skips CAF IMUSEBIT on overflow); CAF IMUSEBIT;
  * MASK FLAGWRD0; CCS A; TC AGSEND. */
 AT(02077);ai_ca(s,AGSWORD);
 AT(02100);if(ai_ts(s,DNLSTCOD)){BR(6);GOTO(02102);}
 BR(7);
 AT(02101);ai_caf(s,K_IMUSEBIT);
 AT(02102);ai_mask(s,FLAGWRD0);
 AT(02103);c=ai_ccs(s,A_);BR(8u+c);
  if(c==1u)GOTO(02105);
  if(c==2u)GOTO(02106);
  if(c==3u)GOTO(02107);
 AT(02104);TC(02105,02122);
 /* CKSTALL CCS IMUCADR; TCF +3; TCF +6; TCF +1; +3 CAF TEN; TC BANKCALL
  * (CADR DELAYJOB); TCF CKSTALL. */
 AT(02105);c=ai_ccs(s,IMUCADR);BR(12u+c);
  if(c==1u)GOTO(02107);
  if(c==2u)GOTO(02110);
  if(c==3u)GOTO(02111);
 AT(02106);GOTO(02111);
 AT(02107);GOTO(02115);
 AT(02110);GOTO(02111);
 AT(02111);ai_caf(s,K_TEN);
 AT(02112);return out(s,02113,X_BANKCALL);
 AT(02114);GOTO(02105);
 /* +6 TC BANKCALL (CADR IMUZERO); TC BANKCALL (CADR IMUSTALL); TC AGSEND. */
 AT(02115);return out(s,02116,X_BANKCALL);
 AT(02117);return out(s,02120,X_BANKCALL);
 AT(02121);TC(02122,02122);
 /* AGSEND TC DOWNFLAG (ADRES NODOFLAG); CAF V50N16; TC BANKCALL (CADR
  * GOMARK3); TCF ENDEXT; TCF ENDEXT; TC ENDEXT. */
 AT(02122);return out(s,02123,X_DOWNFLAG);
 AT(02124);ai_caf(s,K_V50N16);
 AT(02125);return out(s,02126,X_BANKCALL);
 AT(02127);return jump(s,X_ENDEXT);
 AT(02130);return jump(s,X_ENDEXT);
 AT(02131);return out(s,02132,X_ENDEXT);
 default:return REFUSE;
 }
}

/* Is Z (or LOC at NEWOPS) the file's code? */
static int here(const apollo_overlay_state *s,const lm_ags_io *io,int *interp){
 agc_word z=W[5];uint16_t ph;
 *interp=0;
 if(z==NEWOPS){ph=agc_fixed_physical(W[LOC],W[BANKSET],io->control.superbank);*interp=1;
  return (ph>=032u*1024u+037u&&ph<=032u*1024u+071u)||(ph>=032u*1024u+0132u&&ph<=032u*1024u+0167u);}
 if(z<02000u||z>=04000u)return 0;
 ph=agc_fixed_physical(z,W[4],io->control.superbank);
 return ph>=032u*1024u+05u&&ph<=032u*1024u+0131u;
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
static enum exec_status steps(apollo_overlay_state *s,lm_ags_io *io){
 unsigned n;int st,interp;
 for(n=0;n<RUN_LIMIT;++n){
  if(!here(s,io,&interp))return n?EXEC_OK:EXEC_OUT_OF_SCOPE;
  /* NEWOPS reads the order word under FB, which DANZIG set from BANKSET. */
  if(interp&&(((W[4]^W[BANKSET])&076000u)||!work_area(s)))return EXEC_OUT_OF_SCOPE;
  st=interp?interpretive(s,io):basic(s,io);
  if(st==REFUSE)return EXEC_OUT_OF_SCOPE;
  if(st==STOP)return EXEC_OK;
 }
 return EXEC_OUT_OF_SCOPE;
}
static enum exec_status run(apollo_overlay_state *s,void *v){
 lm_ags_io *io=v,entry;enum exec_status st;
 if(!s||!io||!io->undo||io->undo==s)return EXEC_OUT_OF_SCOPE;
 *io->undo=*s;entry=*io;
 st=steps(s,io);
 if(st!=EXEC_OK){*s=*entry.undo;*io=entry;}
 return st;
}
static enum exec_status routine(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;return run(s,io);}
#define B32(a) (uint16_t)(032u*1024u+((a)-02000u))
#define RT(a) {B32(a),routine}
static const agc_routine_entry basic_entries[AGS_BASIC_ROUTINES]={
 RT(02005),RT(02013),RT(02025),RT(02026),RT(02027),RT(02046),RT(02077),RT(02114),RT(02117),RT(02121),
 RT(02122),RT(02124),RT(02127),RT(02130),RT(02131)};
static const agc_routine_entry interp_entries[AGS_INTERPRETIVE_ROUTINES]={
 RT(02037),RT(02041),RT(02047),RT(02051),RT(02053),RT(02055),RT(02057),RT(02061),RT(02063),RT(02065),
 RT(02070),RT(02071),RT(02132),RT(02135),RT(02137),RT(02142),RT(02144),RT(02146),RT(02151),RT(02153),
 RT(02156),RT(02160),RT(02163),RT(02166)};
const agc_routine_table lm_ags_init_routines={basic_entries,AGS_BASIC_ROUTINES};
const agc_routine_table lm_ags_init_interpretive={interp_entries,AGS_INTERPRETIVE_ROUTINES};
const uint16_t ags_routine_keys[AGS_ROUTINES]={
 B32(02005),B32(02013),B32(02025),B32(02026),B32(02027),B32(02046),B32(02077),B32(02114),B32(02117),B32(02121),
 B32(02122),B32(02124),B32(02127),B32(02130),B32(02131),
 B32(02037),B32(02041),B32(02047),B32(02051),B32(02053),B32(02055),B32(02057),B32(02061),B32(02063),B32(02065),
 B32(02070),B32(02071),B32(02132),B32(02135),B32(02137),B32(02142),B32(02144),B32(02146),B32(02151),B32(02153),
 B32(02156),B32(02160),B32(02163),B32(02166)};
const agc_site_operand lm_ags_init_sites[LM_AGS_SITES]={
#include "lm_ags_init_sites.inc"
};
