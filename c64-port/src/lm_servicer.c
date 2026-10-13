/* SPDX-License-Identifier: MIT */
/* Luminary099 SERVICER (banks 37, 33, 22, 27, 32, 23, 21 and 34) as native
 * routines of the control-transfer ABI; see include/lm_servicer.h and
 * docs/LM-SERVICER.md.
 * Basic code: each instruction is one ai_* effect on the image in source
 * order, under a case label at its physical address; a TC or TCF within the
 * file is a goto, one to another file ends the run with Q and Z set. A
 * transfer to an address computed at run time (a TC Q or TC A return, DTCB,
 * the TC of LRADRET) sets Z and lets the step loop find the file's word
 * there. Interpretive code: each order-pair word is a case of its own,
 * running the compiled lm_interp_steps/lm_interp_orders paths with the
 * pair's words as literal arguments. */
#include "lm_servicer.h"
#include "agc_image_ops.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#define W s->words
/* C64 segment groups (LMSV_C64_GROUP, docs/LM-SERVICER.md): a group build
 * holds only its group's code and keys and ends a run, as a stop, before
 * the first instruction or order pair of the file outside it (SV_END and
 * the segment-end labels in basic). The host build holds every group. */
#if !defined(LMSV_C64_GROUP)
#define SV_G(g) 1
#define SV_END(ph,interp) REFUSE
#else
#define SV_G(g) ((g)==LMSV_C64_GROUP)
#define SV_END(ph,interp) seg_end((ph),(interp))
#endif
/* Coverage hook for the host test (empty in flight builds): path n for each
 * instruction or order pair reached (bank 33 a - 02206; bank 37 01600 + a -
 * 03410; bank 22 02100 + a - 03661; bank 27 02140 + a - 03321; bank 32
 * 02200 + a - 03770; bank 23 02210 + a - 02461; bank 21 02240 + a - 02130;
 * bank 34 02270 + a - 03716) and 02400 + n for each outcome of the file's
 * decisions (BR). */
#ifdef LMSV_COUNT_PATHS
extern unsigned lmsv_paths[];
#define SV_PATH(n) ((void)++lmsv_paths[n])
#else
#define SV_PATH(n) ((void)0)
#endif
#define BR(n) SV_PATH(02400u+(n))

/* Central registers. */
#define A 0u
#define LREG 1u
#define QREG 2u
#define EBANK 3u
#define FBANK 4u
#define BBANK 6u
#define EDOP 023u
#define NEWJOB 067u
#define LOC 0164u
#define BANKSET 0165u
/* Erasable (S addresses; banked ones under the EBANK the code sets). */
#define TIME2 024u    /* TIME2 */
#define TIME1 025u    /* TIME1 */
#define TIME5 030u    /* TIME5 */
#define CDUX 032u    /* CDUX */
#define CDUY 033u    /* CDUY */
#define CDUZ 034u    /* CDUZ */
#define PIPAX 037u    /* PIPAX */
#define PIPAY 040u    /* PIPAY */
#define PIPAZ 041u    /* PIPAZ */
#define X1 046u    /* X1 */
#define ITEMP1 061u    /* ITEMP1 */
#define FLAGWRD2 076u    /* FLAGWRD2 */
#define FLAGWRD3 077u    /* FLAGWRD3 */
#define FLAGWRD5 0101u    /* FLAGWRD5 */
#define FLAGWRD6 0102u    /* FLAGWRD6 */
#define FLAGWRD7 0103u    /* FLAGWRD7 */
#define FLAGWRD8 0104u    /* FLAGWRD8 */
#define FLGWRD10 0106u    /* FLGWRD10 */
#define FLGWRD11 0107u    /* FLGWRD11 */
#define RADMODES 0110u    /* RADMODES */
#define FLGWRD12 0110u    /* FLGWRD12 */
#define DAPBOOLS 0111u    /* DAPBOOLS */
#define FIXLOC 0120u    /* FIXLOC */
#define ABVELS 0130u    /* ABVEL* */
#define BUF 0130u    /* BUF */
#define VSELECTS 0131u    /* VSELECT* */
#define BUF2 0133u    /* BUF2 */
#define MPTEMP 0135u    /* MPTEMP */
#define MPAC 0154u    /* MPAC */
#define MODE 0163u    /* MODE */
#define PUSHLOC 0166u    /* PUSHLOC */
#define PRIORITY 0167u    /* PRIORITY */
#define DELVX 0324u    /* DELVX */
#define DELVY 0326u    /* DELVY */
#define DELVZ 0330u    /* DELVZ */
#define MPHASE1 0752u    /* -PHASE1 */
#define MPHASE2 0754u    /* -PHASE2 */
#define PHASE2 0755u    /* PHASE2 */
#define MPHASE3 0756u    /* -PHASE3 */
#define MPHASE4 0760u    /* -PHASE4 */
#define PHASE4 0761u    /* PHASE4 */
#define MPHASE5 0762u    /* -PHASE5 */
#define MPHASE6 0764u    /* -PHASE6 */
#define CDUSPOTY 0766u    /* CDUSPOTY */
#define CDUSPOT 0766u    /* CDUSPOT */
#define CDUSPOTZ 0770u    /* CDUSPOTZ */
#define CDUSPOTX 0772u    /* CDUSPOTX */
#define MODREG 01011u    /* MODREG */
#define TBASE2 01055u    /* TBASE2 */
#define PIPCTR 01056u    /* PIPCTR */
#define TBASE5 01063u    /* TBASE5 */
#define N1_PIPADT 01075u    /* 1/PIPADT */
#define SAMPLSUM 01101u    /* SAMPLSUM */
#define CDUTEMPX 01155u    /* CDUTEMPX */
#define CDUTEMPY 01156u    /* CDUTEMPY */
#define CDUTEMPZ 01157u    /* CDUTEMPZ */
#define PIPATMPX 01160u    /* PIPATMPX */
#define PIPATMPY 01161u    /* PIPATMPY */
#define PIPATMPZ 01162u    /* PIPATMPZ */
#define RN 01220u    /* RN */
#define PIPTIME 01234u    /* PIPTIME */
#define MASS 01244u    /* MASS */
#define ABDELV 01246u    /* ABDELV */
#define PGUIDE 01247u    /* PGUIDE */
#define DVTHRUSH 01251u    /* DVTHRUSH */
#define AVGEXIT 01252u    /* AVGEXIT */
#define TEMX 01254u    /* TEMX */
#define TEMY 01255u    /* TEMY */
#define TEMZ 01256u    /* TEMZ */
#define PIPAGE 01257u    /* PIPAGE */
#define OUTROUTE 01260u    /* OUTROUTE */
#define SKALSKAL 01356u    /* SKALSKAL */
#define LRHMAX 01420u    /* LRHMAX */
#define LRWH 01421u    /* LRWH */
#define RPCRTIME 01427u    /* RPCRTIME */
#define RPCRTQSW 01430u    /* RPCRTQSW */
#define PHSNAME5 01446u    /* PHSNAME5 */
#define AIG 01457u    /* AIG */
#define AOG 01461u    /* AOG */
#define ABVEL 01471u    /* ABVEL */
#define DVTOTAL 01507u    /* DVTOTAL */
#define ABDVCONV 01513u    /* ABDVCONV */
#define DVCNTR 01515u    /* DVCNTR */
#define R 01520u    /* R */
#define V 01526u    /* V */
#define LRVMAX 01526u    /* LRVMAX */
#define LRVF 01527u    /* LRVF */
#define LRWVFZ 01533u    /* LRWVFZ */
#define HCALC 01534u    /* HCALC */
#define UNIT_R_ 01536u    /* UNIT/R/ */
#define LRWVFF 01536u    /* LRWVFF */
#define XNBPIP 01545u    /* XNBPIP */
#define PIPTIME1 01560u    /* PIPTIME1 */
#define MASS1 01570u    /* MASS1 */
#define DVCNTR1 01570u    /* DVCNTR1 */
#define R1S 01572u    /* R1S */
#define V1S 01600u    /* V1S */
#define TTF_8 01642u    /* TTF/8 */
#define LRADRET 01650u    /* LRADRET */
#define VSELECT 01651u    /* VSELECT */
#define VMEAS 01652u    /* VMEAS */
#define LRVTIME 01652u    /* LRVTIME */
#define LRXCDU 01654u    /* LRXCDU */
#define HMEAS 01654u    /* HMEAS */
#define LRYCDU 01655u    /* LRYCDU */
#define LRZCDU 01656u    /* LRZCDU */
#define GNUR 01656u    /* GNUR */
#define LRADRET1 01656u    /* LRADRET1 */
#define PIPTEM 01657u    /* PIPTEM */
#define DELTAH 01664u    /* DELTAH */
#define LRLCTR 01670u    /* LRLCTR */
#define LRRCTR 01671u    /* LRRCTR */
#define LRMCTR 01672u    /* LRMCTR */
#define LRSCTR 01673u    /* LRSCTR */
#define STILBADH 01674u    /* STILBADH */
#define STILBADV 01675u    /* STILBADV */
#define DALTRATE 01716u    /* DALTRATE */
#define LRXCDUDL 01734u    /* LRXCDUDL */
#define LRZCDUDL 01736u    /* LRZCDUDL */
#define LRVTIMDL 01737u    /* LRVTIMDL */
#define RUNIT 01743u    /* RUNIT */
#define MKTIME 01754u    /* MKTIME */
#define RN1 01544u    /* RN1 */
#define LRALPHA 01522u    /* LRALPHA */
#define LRWVZ 01530u    /* LRWVZ */
#define K_DPSVEX 042341u    /* DPSVEX (33,2001) */
#define K_ALTSCBIT 0400u    /* ALTSCBIT */
#define K_APSFLBIT 010000u    /* APSFLBIT */
#define K_AUXFLBIT 02u    /* AUXFLBIT */
#define K_AVEGFBIT 020u    /* AVEGFBIT */
#define K_BIT13 010000u    /* BIT13 */
#define K_BIT6 040u    /* BIT6 */
#define K_BIT7 0100u    /* BIT7 */
#define K_BIT9 0400u    /* BIT9 */
#define K_BITS4M7 0110u    /* BITS4-7 */
#define K_DEC17 021u    /* DEC17 */
#define K_DESIGBIT 01000u    /* DESIGBIT */
#define K_DONEADR 03451u    /* DONEADR */
#define K_DRFTBIT 040000u    /* DRFTBIT */
#define K_EBANK3 01400u    /* EBANK3 */
#define K_EBANK4 02000u    /* EBANK4 */
#define K_EBANK5 02400u    /* EBANK5 */
#define K_EBANK7 03400u    /* EBANK7 */
#define K_EIGHTEEN 022u    /* EIGHTEEN */
#define K_ENDJBCAD 065772u    /* ENDJBCAD */
#define K_FIVE 05u    /* FIVE */
#define K_FOUR 04u    /* FOUR */
#define K_IDLEFBIT 0100u    /* IDLEFBIT */
#define K_KPIP 01507u    /* KPIP */
#define K_KPIP1 0321u    /* KPIP1 */
#define K_LOW9 0777u    /* LOW9 */
#define K_LRBYBIT 040000u    /* LRBYBIT */
#define K_LRINHBIT 0200u    /* LRINHBIT */
#define K_MUNFLBIT 0200u    /* MUNFLBIT */
#define K_N170MS 021u    /* 170MS */
#define K_N2KFT_SEC 01414u    /* 2KFT/SEC */
#define K_N2SECS 0310u    /* 2SECS */
#define K_NEG1 077776u    /* NEG1 */
#define K_NEG1_2 057777u    /* NEG1/2 */
#define K_NEG2 077775u    /* NEG2 */
#define K_NEG3 077774u    /* NEG3 */
#define K_NOLRRBIT 01000u    /* NOLRRBIT */
#define K_NR29_RDR 02400u    /* NR29&RDR */
#define K_OCT10002 010002u    /* OCT10002 */
#define K_OCT24 024u    /* OCT24 */
#define K_OCT33 033u    /* OCT33 */
#define K_OCT37771 037771u    /* OCT37771 */
#define K_OCT523 0523u    /* OCT523 */
#define K_ONE 01u    /* ONE */
#define K_PRIO20 020000u    /* PRIO20 */
#define K_PRIO21 021000u    /* PRIO21 */
#define K_PRIO22 022000u    /* PRIO22 */
#define K_PRIO23 023000u    /* PRIO23 */
#define K_PRIO25 025000u    /* PRIO25 */
#define K_PRIO3 03000u    /* PRIO3 */
#define K_PRIO31 031000u    /* PRIO31 */
#define K_PRIO32 032000u    /* PRIO32 */
#define K_PSTHIBIT 02000u    /* PSTHIBIT */
#define K_READLBIT 040u    /* READLBIT */
#define K_READVBIT 020u    /* READVBIT */
#define K_RNGEDBIT 010u    /* RNGEDBIT */
#define K_RNGSCBIT 01000u    /* RNGSCBIT */
#define K_SEVEN 07u    /* SEVEN */
#define K_SIX 06u    /* SIX */
#define K_STEERBIT 02000u    /* STEERBIT */
#define K_SUPER011 060u    /* SUPER011 */
#define K_SURFFBIT 0200u    /* SURFFBIT */
#define K_TWO 02u    /* TWO */
#define K_USEQRJTS 020000u    /* USEQRJTS */
#define K_VELDABIT 0100u    /* VELDABIT */
#define K_VXINHBIT 04000u    /* VXINHBIT */
#define K_XNBPIPAD 02145u    /* XNBPIPAD */
#define K_XORFLBIT 0400u    /* XORFLBIT */
#define K_ZERO 00u    /* ZERO */
static const agc_word __attribute__((unused)) K_APSVEX[2]={041545u,042341u};  /* APSVEX */
static const agc_word __attribute__((unused)) K_AVOUTCAD[2]={03661u,044067u};  /* AVOUTCAD */
static const agc_word __attribute__((unused)) K_NEG0[2]={077777u,00u};  /* NEG0 */
static const agc_word __attribute__((unused)) K_SVEXTADR[2]={03770u,064067u};  /* SVEXTADR */
#define X_R10_R11A 02021u    /* R10,R11A */
#define X_R29 02045u    /* R29 */
#define X_BANKCALL 04616u    /* BANKCALL */
#define X_SWRETURN 04631u    /* SWRETURN */
#define X_POSTJUMP 04635u    /* POSTJUMP */
#define X_BANKJUMP 04640u    /* BANKJUMP */
#define X_MAKECADR 04645u    /* MAKECADR */
#define X_IBNKCALL 04674u    /* IBNKCALL */
#define X_NOVAC 05072u    /* NOVAC */
#define X_FINDVAC 05105u    /* FINDVAC */
#define X_PRIOCHNG 05146u    /* PRIOCHNG */
#define X_ENDOFJOB 05155u    /* ENDOFJOB */
#define X_WAITLIST 05203u    /* WAITLIST */
#define X_VARDELAY 05224u    /* VARDELAY */
#define X_TASKOVER 05261u    /* TASKOVER */
#define X_N2PHSCHNG 05327u    /* 2PHSCHNG */
#define X_PHASCHNG 05353u    /* PHASCHNG */
#define X_UPFLAG 05504u    /* UPFLAG */
#define X_DOWNFLAG 05516u    /* DOWNFLAG */
#define X_ALARM 05567u    /* ALARM */
#define X_WHIMPER 05644u    /* WHIMPER */
#define X_GOTOPOOH 06001u    /* GOTOPOOH */
#define X_INTPRET 06037u    /* INTPRET */
#define X_BRANCH 06723u    /* BRANCH */
#define X_BRANCHP4 06727u    /* BRANCH     +4 */
#define X_SHORTMP 07307u    /* SHORTMP */
#define X_SHORTMP2 07313u    /* SHORTMP2 */
#define X_ALSIGNAG 07544u    /* ALSIGNAG */
#define PHX(b,a) (uint16_t)((b)*1024u+((a)-02000u))
/* Physical case labels: bank b (octal digits) address a. */
#define PH(b,a) PHX(0##b,a)
#define PATHOF(b,a) (0##b==033?(a)-02206u:0##b==037?01600u+(a)-03410u:0##b==022?02100u+(a)-03661u:\
 0##b==027?02140u+(a)-03321u:0##b==032?02200u+(a)-03770u:0##b==023?02210u+(a)-02461u:\
 0##b==021?02240u+(a)-02130u:02270u+(a)-03716u)
#define X_CHANG2 05126u
#define NEWOPS 06070u

enum {GO=0,STOP=1,REFUSE=2};
/* Helpers stay out of line: the C64 build has to fit the many call sites. */
#define NOINL __attribute__((noinline,unused))
#define RUN_LIMIT 2000u
static __attribute__((unused)) agc_word oc(agc_register a){return (agc_word)((a&037777u)|((a>>1)&040000u));}
static __attribute__((unused)) int tc_word(agc_register w){return w>=02000u&&w<=07777u;}

/* ---- Interpretive helpers (as lm_burn_baby.c) ------------------------ */
/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,DLOAD=06052,PDDL=06523,PDVL=06557,VAD=06751,DAD=06775,DSU=07005,BDSU=07062,DMP=07572,
 DMPR=07574,DDV=07577,BDDV=07603,VXSC=07401,VXV=07460,DOT=07331,MXV=07334,VXM=07337,SLOAD=06501,TSLC=07616,
 GSHIFT=07621,STADR=06354,EXIT=06743,PUSH=03247,ABVAL=03176,UNIT=03023,DSQ=03174,COMP=07670,RVQ=03274,
 CALLITA=02507,SWITCH=02516,BPLBMN=02474,LXC=02360,RTBBHIZ=02447};
/* The fixed operands the file's orders name (checked against the rope by
 * tools/lm_servicer_source.py). */
/* bank 33 02000-02044: APSVEX .. 2J */
static const agc_word __attribute__((unused)) FX33A[045]={041545u,042341u,061000u,071210u,0,0,072333u,
 045546u,065363u,064451u,021241u,03216u,030153u,023101u,063105u,061733u,
 01507u,0321u,026706u,0150u,033343u,02630u,025010u,024402u,026003u,061377u,
 055754u,077644u,065556u,064453u,055670u,02u,011777u,01023u,037155u,065u,
 06244u};
/* bank 33 02677-02707: ALTCRIT (25KFT, 50KFT, 50FT, 30KFT) and 2KFT/SEC */
static const agc_word __attribute__((unused)) FX33B[011]={07u,016100u,016u,034200u,0,0364u,010u,
 035600u,01414u};
/* bank 33 03026-03031: DP1/20, SHIFT11 */
static const agc_word __attribute__((unused)) FX33C[04]={01463u,06315u,010u,0};
/* bank 33 03143-03153: 1.95SECS, 7.5, 2SEC(18), 2SEC(28), 4SEC(28) */
static const agc_word __attribute__((unused)) FX33D[011]={0303u,05u,033212u,014u,020000u,0,0310u,0,
 0620u};
/* bank 23 02511-02526: DP1/4TH, UNITZ, UNITY, UNITX, ZEROVECS */
static const agc_word __attribute__((unused)) FX23[016]={010000u,0,0,0,0,0,020000u,0,0,0,0,0,0,0};
static const struct {uint16_t ph;uint8_t n;const agc_word *w;} FIXED[]={
 {PHX(033,02000),045,FX33A},{PHX(033,02677),011,FX33B},{PHX(033,03026),04,FX33C},
 {PHX(033,03143),011,FX33D},{PHX(023,02511),016,FX23}};
/* The n operand words ADDRWD names after address formation: *f=0 for an
 * erasable operand, else the words of one of the tables above. */
static NOINL int operand(const apollo_overlay_state *s,unsigned n,const agc_word **f){
 unsigned a=W[0116],i;uint16_t ph;
 *f=0;
 if(a<02000u)return a+n<=02000u;   /* erasable, not running on into fixed */
 if(a>03777u)return 0;
 ph=(uint16_t)((W[FBANK]>>10)*1024u+a-02000u);
 for(i=0;i<sizeof FIXED/sizeof FIXED[0];++i)
  if(ph>=FIXED[i].ph&&ph+n<=FIXED[i].ph+FIXED[i].n){*f=FIXED[i].w+(ph-FIXED[i].ph);return 1;}
 return 0;
}
#define K(n) do{if(!operand(s,(n),&k))return REFUSE;}while(0)
/* DANZIG at the end of a pair: NEWJOB +0 goes on to NEWOPS; positive goes
 * to CHANG2 (CA BANKSET; TS BBANK; CCS EDOP; CCS NEWJOB; TCF CHANG2). */
static NOINL int next(apollo_overlay_state *s){
 agc_word j=W[NEWJOB];
 if(j==0u){BR(0);lmi_danzig_next(s);return GO;}
 if(j>037777u)return REFUSE;
 BR(1);ai_ca(s,BANKSET);(void)ai_ts(s,BBANK);(void)ai_ccs(s,EDOP);(void)ai_ccs(s,NEWJOB);W[5]=X_CHANG2;return STOP;
}
#define NEXT return next(s)
#define DO(x) do{if(x)return REFUSE;}while(0)
#define STEP(x) do{int r_=(x);if(r_)return r_;}while(0)
static NOINL void right(apollo_overlay_state *s){lmi_danzig_right(s);}
/* An address order's operand: the given word, or PUSHUP when the next word
 * (passed instead, negative) is an order pair. */
static NOINL void opnd(apollo_overlay_state *s,agc_word word,agc_word target){
 if(word&040000u)lmi_address_pushup(s,word,target);else lmi_direct(s,word,target);
}
static NOINL void shorts(apollo_overlay_state *s){lmi_short_dispatch(s);if(W[5]==02017u)lmi_shortt(s);else lmi_shortv(s);}
static NOINL int store(apollo_overlay_state *s,agc_word code){lmi_store_code(s,code);return next(s);}
/* STODL/STOVL: the store, then the load of the given word (or PUSHUP). */
static NOINL int stodl(apollo_overlay_state *s,agc_word code,agc_word word){const agc_word *k;
 lmi_store_code(s,code);lmi_dodload(s);
 if(word&040000u)lmi_diradres_pushup(s,word,DLOAD);else lmi_diradres(s,word,DLOAD);
 K(2);lmi_dload_op(s,k);return next(s);
}
static NOINL int stovl(apollo_overlay_state *s,agc_word code,agc_word word){const agc_word *k;
 lmi_store_code(s,code);lmi_dovload(s);
 if(word&040000u)lmi_diradres_pushup(s,word,VLOAD);else lmi_diradres(s,word,VLOAD);
 K(6);lmi_vload(s,k);return next(s);
}
/* STCALL: the store, then 15BITADR to CALL and the GOTO (an erasable
 * address holds the CADR to go to: STCALL VN1 31D returns through the QPRET
 * STQ saved there). */
static NOINL int stcall(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_15bitadr(s,word,CALLITA);lmi_call(s);
 if(word<04000u)lmi_goto_any(s);else lmi_goto(s);
 return GO;
}
static NOINL int call(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,CALLITA);lmi_call(s);lmi_goto(s);return GO;}
/* RTB as the right order: the routine (SGNAGREE) returns to DANZIG. */
static NOINL int rtb(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,RTBBHIZ);(void)ai_ccs(s,020);lmi_rtb(s);return GO;}
static NOINL int rvq(apollo_overlay_state *s){lmi_unary(s,RVQ);lmi_rvq(s);return GO;}
static NOINL int exit_(apollo_overlay_state *s){lmi_exit(s);return GO;}
static NOINL int rexit(apollo_overlay_state *s){lmi_unary(s,EXIT);lmi_exit(s);return GO;}
static NOINL void push(apollo_overlay_state *s){lmi_unary(s,PUSH);lmi_push(s);}
static NOINL void dsq(apollo_overlay_state *s){lmi_unary(s,DSQ);lmi_dsq(s);}
static NOINL void comp(apollo_overlay_state *s){lmi_unary(s,COMP);lmi_comp(s);}
/* ABVAL, ABS and UNIT: STOP at POODOO1 (alarm 1302). */
static NOINL int abval(apollo_overlay_state *s){lmi_unary(s,ABVAL);if(lmi_abvalabs(s)){BR(2);return STOP;}return GO;}
static NOINL int abs_(apollo_overlay_state *s){lmi_unary(s,ABVAL);if(lmi_abvalabs(s)){BR(3);return STOP;}return GO;}
static NOINL int unit(apollo_overlay_state *s){lmi_unary(s,UNIT);if(lmi_unit(s)){BR(4);return STOP;}return GO;}
/* Address orders: the operand word (or PUSHUP), *x the indexed forms. */
static NOINL int vload(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,VLOAD);K(6);lmi_vload(s,k);return GO;}
static NOINL int vloadx(apollo_overlay_state *s,agc_word word){const agc_word *k;lmi_indexed(s,word,VLOAD);K(6);lmi_vload(s,k);return GO;}
static NOINL int dload(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,DLOAD);K(2);lmi_dload_op(s,k);return GO;}
static NOINL int pddl(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,PDDL);K(2);lmi_pddl_op(s,k);return GO;}
static NOINL int pdvl(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,PDVL);K(6);lmi_pdvl(s,k);return GO;}
static NOINL int vad(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,VAD);K(6);lmi_vad(s,k);return GO;}
static NOINL int dad(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,DAD);K(2);lmi_dad(s,k);return GO;}
static NOINL int dsu(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,DSU);K(2);lmi_dsu(s,k);return GO;}
static NOINL int bdsu(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,BDSU);K(2);lmi_bdsu(s,k);return GO;}
static NOINL int dmp(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,DMP);K(2);lmi_dmp(s,k);return GO;}
static NOINL int dmpx(apollo_overlay_state *s,agc_word word){const agc_word *k;lmi_indexed(s,word,DMP);K(2);lmi_dmp(s,k);return GO;}
static NOINL int dmpr(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,DMPR);K(2);lmi_dmpr(s,k);return GO;}
static NOINL int ddv(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,DDV);K(2);lmi_ddv(s,k);return GO;}
static NOINL int bddv(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,BDDV);K(2);lmi_bddv(s,k);return GO;}
static NOINL int bddvx(apollo_overlay_state *s,agc_word word){const agc_word *k;lmi_indexed(s,word,BDDV);K(2);lmi_bddv(s,k);return GO;}
/* VXSC: a scalar operand in vector mode (MODE -1), a vector otherwise. */
static NOINL int vxsc(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,VXSC);K(W[MODE]==077776u?2u:6u);lmi_vxsc(s,k);return GO;}
static NOINL int vxv(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,VXV);K(6);lmi_vxv(s,k);return GO;}
static NOINL int dot(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,DOT);K(6);lmi_dot(s,k);return GO;}
static NOINL int mxv(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,MXV);K(18);lmi_mxv(s,k);return GO;}
static NOINL int vxm(apollo_overlay_state *s,agc_word word){const agc_word *k;opnd(s,word,VXM);K(18);lmi_vxm(s,k);return GO;}
static NOINL int sload(apollo_overlay_state *s,agc_word word){opnd(s,word,SLOAD);if(W[0116]>=02000u)return REFUSE;lmi_sload(s);return GO;}
static NOINL int norm(apollo_overlay_state *s,agc_word word){opnd(s,word,TSLC);lmi_tslc(s);return GO;}
static NOINL int gsh(apollo_overlay_state *s,agc_word word){opnd(s,word,GSHIFT);lmi_gshift(s);return GO;}
static NOINL void gshx(apollo_overlay_state *s,agc_word word){lmi_indexed(s,word,GSHIFT);lmi_gshift(s);}
static NOINL void lxc(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,LXC);lmi_lxc(s);}
static NOINL void stq(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,CALLITA);lmi_ita(s);}
/* SET/CLEAR (never a branch) and BON/BOFF (1 when the branch is taken). */
static NOINL int sw(apollo_overlay_state *s,lm_sv_io *io,agc_word word){lmi_misc(s,word,SWITCH);return lmi_switch(s,&io->control)?REFUSE:GO;}
static NOINL int swb(apollo_overlay_state *s,lm_sv_io *io,agc_word word,agc_word branch){
 lmi_misc(s,word,SWITCH);if(!lmi_switch(s,&io->control))return 0;
 lmi_swbranch(s,branch);return 1;
}
/* BMN: 1 when the branch is taken (to its NEWOPS). */
static NOINL int bmn(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,BPLBMN);if(!lmi_bplbmn(s))return 0;lmi_goto(s);return 1;}
#ifdef LMSV_C64_GROUP
/* Every instruction (0) and order-pair or store word (1) of the file, by
 * path number (PATHOF): the words another group's code runs. */
static const uint8_t SEG_LABELS[2][160]={{
 0xe1,0xe2,0xff,0xff,0xff,0x6f,0xc1,0xff,0xff,0xff,0xff,0xff,0xff,0x9b,0xbf,0xef,
 0x3f,0xfc,0xff,0xfd,0xff,0xf4,0xff,0xff,0xff,0xff,0xff,0x7d,0xff,0xff,0x07,0x00,
 0x00,0x00,0xfc,0xff,0xff,0xff,0xff,0x01,0x6c,0xdd,0x02,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x30,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xff,0x03,0xf0,
 0x07,0xe0,0x83,0xef,0xe3,0xff,0xff,0x80,0xff,0xff,0x7f,0x07,0xf0,0xff,0x03,0x00,
 0x80,0xef,0xfb,0xff,0xff,0xff,0xff,0x3f,0xf8,0xff,0xe6,0xff,0xff,0xff,0xf7,0xff,
 0xbe,0xf3,0xf5,0xff,0xff,0xff,0xd7,0x77,0xff,0xfb,0xff,0xff,0xff,0x00,0x00,0x01,
 0xcf,0xff,0xe7,0xff,0x7f,0xfe,0xff,0xff,0xe7,0xff,0x1f,0xfe,0xff,0xff,0xff,0xff,
 0xff,0xff,0xff,0xff,0x0f,0x00,0x00,0x00,0xd7,0x23,0xc0,0x00,0xff,0xff,0xff,0x1f,
 0x05,0x01,0x00,0x78,0xff,0x3f,0x0f,0xf5,0xff,0xff,0xef,0xff,0x7f,0x00,0x00,0x00,
},{
 0x00,0x14,0x00,0x00,0x00,0x00,0x22,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x80,0x02,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xa8,0x53,
 0x55,0x75,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x9c,0xb2,0xac,0xd4,0xa4,0xe9,
 0x52,0x5a,0x50,0xaa,0x4a,0x35,0xa9,0xac,0x5d,0x55,0x2d,0x19,0x00,0x00,0xa4,0x04,
 0x48,0x19,0x54,0x00,0x14,0x00,0x00,0x53,0x00,0x00,0x00,0x48,0x0a,0x00,0x24,0xd5,
 0x36,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xa9,0xd5,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x12,0x00,0x00,0x00,0x00,0x00,
 0x00,0xa2,0xaa,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
}};
static int seg_end(uint16_t ph,int interp){unsigned b=ph/1024u,a=ph%1024u+02000u,p;
 p=b==033u?a-02206u:b==037u?01600u+a-03410u:b==022u?02100u+a-03661u:b==027u?02140u+a-03321u:
   b==032u?02200u+a-03770u:b==023u?02210u+a-02461u:b==021u?02240u+a-02130u:02270u+a-03716u;
 return p<1280u&&(SEG_LABELS[interp][p>>3]>>(p&7)&1u)?STOP:REFUSE;}
#endif
#define IC(b,a) case PH(b,a): SV_PATH(PATHOF(b,a))
/* One order-pair word at LOC (Z at NEWOPS), by its physical address. */
static int interpretive(apollo_overlay_state *s,lm_sv_io *io,uint16_t ph){
 (void)s;(void)io;
 switch(ph){
#if SV_G(6)
 /* VLOAD ABVAL; DELV */
 IC(33,02220);lmi_newops(s,051575);DO(vload(s,000325));right(s);STEP(abval(s));NEXT;
 /* EXIT */
 IC(33,02222);lmi_newops(s,077776);return exit_(s);
#endif
#if SV_G(5)
 /* AVERAGEG BON CALL; MUNFLAG; RVBOTH; CALCRVG */
 IC(33,02267);lmi_newops(s,045014);if(swb(s,io,003307,067032)){BR(188);return GO;}BR(189);right(s);return call(s,066776);
 /* EXIT */
 IC(33,02273);lmi_newops(s,077776);return exit_(s);
 /* SET CLEAR; NOR29FLG; SWANDISP */
 IC(22,03677);lmi_newops(s,043014);DO(sw(s,io,001464));right(s);DO(sw(s,io,003664));NEXT;
 /* CLEAR CALL; MUNFLAG; AVETOMID */
 IC(22,03702);lmi_newops(s,045014);DO(sw(s,io,003267));right(s);return call(s,027467);
 /* CLEAR EXIT; V37FLAG */
 IC(22,03705);lmi_newops(s,077414);DO(sw(s,io,003671));right(s);return rexit(s);
 /* VLOAD BOFF; RN1; MUNFLAG; NORMLIZ1 */
 IC(23,02462);lmi_newops(s,043175);DO(vload(s,003545));right(s);if(swb(s,io,003347,066415)){BR(190);return GO;}BR(191);NEXT;
 /* VSL6 MXV; REFSMMAT */
 IC(23,02466);lmi_newops(s,064252);shorts(s);right(s);DO(mxv(s,001734));NEXT;
 /* STCALL R; MUNGRAV */
 IC(23,02470);return stcall(s,037521,067130);
 /* VLOAD VSL1; VN1 */
 IC(23,02472);lmi_newops(s,076575);DO(vload(s,003553));right(s);shorts(s);NEXT;
 /* MXV; REFSMMAT */
 IC(23,02474);lmi_newops(s,077721);DO(mxv(s,001734));NEXT;
 /* STOVL V; V(CSM) */
 IC(23,02476);return stovl(s,027527,001726);
 /* VXV UNIT; R(CSM) */
 IC(23,02500);lmi_newops(s,053435);DO(vxv(s,001720));right(s);STEP(unit(s));NEXT;
 /* STORE UHYP */
 IC(23,02502);return store(s,003720);
#endif
#if SV_G(7)
 /* ASCSPOT EXIT */
 IC(23,02503);lmi_newops(s,077776);return exit_(s);
#endif
#if SV_G(4)
 /* NORMLIZ1 CALL; CALCGRAV */
 IC(33,02415);lmi_newops(s,077624);return call(s,066730);
 /* EXIT */
 IC(33,02417);lmi_newops(s,077776);return exit_(s);
 /* MUNRETRN EXIT */
 IC(33,02461);lmi_newops(s,077776);return exit_(s);
 /* VLOAD ABVAL; R1S */
 IC(33,02571);lmi_newops(s,051575);DO(vload(s,003573));right(s);STEP(abval(s));NEXT;
 /* PUSH DSU; /LAND/ */
 IC(33,02573);lmi_newops(s,045206);push(s);right(s);DO(dsu(s,002333));NEXT;
 /* STORE HCALC */
 IC(33,02575);return store(s,003535);
 /* STORE HCALC1 */
 IC(33,02576);return store(s,003775);
 /* DMPR RTB; ALTCONV; SGNAGREE */
 IC(33,02577);lmi_newops(s,047075);DO(dmpr(s,026026));right(s);return rtb(s,021612);
#endif
#if SV_G(5)
 /* STOVL ALTBITS; UNIT/R/ */
 IC(33,02602);return stovl(s,027742,003537);
 /* VXV UNIT; UHYP */
 IC(33,02604);lmi_newops(s,053435);DO(vxv(s,003720));right(s);STEP(unit(s));NEXT;
 /* STOVL UHZP; R1S */
 IC(33,02606);return stovl(s,027726,003573);
 /* VXM VSR4; REFSMMAT */
 IC(33,02610);lmi_newops(s,060505);DO(vxm(s,001734));right(s);shorts(s);NEXT;
 /* STOVL RN1; V1S */
 IC(33,02612);return stovl(s,027545,003601);
 /* VXM VSL1; REFSMMAT */
 IC(33,02614);lmi_newops(s,076505);DO(vxm(s,001734));right(s);shorts(s);NEXT;
 /* STOVL VN1; UNIT/R/ */
 IC(33,02616);return stovl(s,027553,003537);
 /* VXV ABVAL; V1S */
 IC(33,02620);lmi_newops(s,051435);DO(vxv(s,003601));right(s);STEP(abval(s));NEXT;
 /* SL1 DSQ */
 IC(33,02622);lmi_newops(s,063552);shorts(s);right(s);dsq(s);NEXT;
 /* DDV; (pushup) */
 IC(33,02623);lmi_newops(s,077671);DO(ddv(s,047075));NEXT;
 /* DMPR RTB; ARCONV1; SGNAGREE */
 IC(33,02624);lmi_newops(s,047075);DO(dmpr(s,026030));right(s);return rtb(s,021612);
#endif
#if SV_G(6)
 /* COPYCYC2 EXIT */
 IC(33,02627);lmi_newops(s,077776);return exit_(s);
#endif
#if SV_G(2)
 /* CALCGRAV UNIT PUSH */
 IC(33,02730);lmi_newops(s,041456);STEP(unit(s));right(s);push(s);NEXT;
 /* STORE UNIT/R/ */
 IC(33,02731);return store(s,003537);
 /* LXC,1 SLOAD; RTX2; RTX2 */
 IC(33,02732);lmi_newops(s,067340);lxc(s,002777);right(s);DO(sload(s,003000));NEXT;
 /* DCOMP BMN; CALCGRV1 */
 IC(33,02735);lmi_newops(s,050076);comp(s);right(s);if(bmn(s,066765)){BR(192);return GO;}BR(193);NEXT;
 /* VLOAD DOT; UNITZ; UNIT/R/ */
 IC(33,02737);lmi_newops(s,050375);DO(vload(s,006514));right(s);DO(dot(s,003537));NEXT;
 /* SL1 PUSH */
 IC(33,02742);lmi_newops(s,041552);shorts(s);right(s);push(s);NEXT;
 /* DSQ BDSU; DP1/20 */
 IC(33,02743);lmi_newops(s,044316);dsq(s);right(s);DO(bdsu(s,027027));NEXT;
 /* PDDL DDV; RESQ; 34D */
 IC(33,02745);lmi_newops(s,056325);DO(pddl(s,026040));right(s);DO(ddv(s,000043));NEXT;
 /* STORE 32D */
 IC(33,02750);return store(s,000041);
 /* DMP DMP; 20J; (pushup) */
 IC(33,02751);lmi_newops(s,041205);DO(dmp(s,026042));right(s);DO(dmp(s,065361));NEXT;
 /* VXSC PDDL; UNIT/R/; (pushup) */
 IC(33,02753);lmi_newops(s,065361);DO(vxsc(s,003537));right(s);DO(pddl(s,041205));NEXT;
 /* DMP DMP; 2J; 32D */
 IC(33,02755);lmi_newops(s,041205);DO(dmp(s,026044));right(s);DO(dmp(s,000041));NEXT;
 /* VXSC VSL1; UNITZ */
 IC(33,02760);lmi_newops(s,076561);DO(vxsc(s,006514));right(s);shorts(s);NEXT;
 /* VAD STADR; (pushup); STORE UNITGOBL */
 IC(33,02762);lmi_newops(s,045455);DO(vad(s,074256));right(s);lmi_unary(s,STADR);lmi_stadr(s,074256);NEXT;
 /* VAD PUSH; (pushup) */
 IC(33,02764);lmi_newops(s,041455);DO(vad(s,060345));right(s);push(s);NEXT;
#endif
#if SV_G(3)
 /* CALCGRV1 DLOAD NORM; 34D; X2 */
 IC(33,02765);lmi_newops(s,060345);DO(dload(s,000043));right(s);DO(norm(s,000050));NEXT;
 /* BDDV* SLR*; -MUDT,1; 0          -21D,2 */
 IC(33,02770);lmi_newops(s,053663);DO(bddvx(s,026032));right(s);gshx(s,056623);NEXT;
 /* VXSC STADR; (pushup); STORE GDT1/2 */
 IC(33,02773);lmi_newops(s,045561);DO(vxsc(s,074214));right(s);lmi_unary(s,STADR);lmi_stadr(s,074214);NEXT;
 /* RVQ */
 IC(33,02775);lmi_newops(s,077616);return rvq(s);
#endif
#if SV_G(7)
 /* CALCRVG VLOAD VXM; DELV; REFSMMAT */
 IC(33,02776);lmi_newops(s,061375);DO(vload(s,000325));right(s);DO(vxm(s,001734));NEXT;
 /* VXSC VSL1; KPIP1 */
 IC(33,03001);lmi_newops(s,076561);DO(vxsc(s,026022));right(s);shorts(s);NEXT;
 /* STORE DELVREF */
 IC(33,03003);return store(s,003527);
 /* VSR1 PUSH */
 IC(33,03004);lmi_newops(s,041562);shorts(s);right(s);push(s);NEXT;
 /* VAD PUSH; GDT/2 */
 IC(33,03005);lmi_newops(s,041455);DO(vad(s,001237));right(s);push(s);NEXT;
 /* VAD PDDL; VN; PGUIDE */
 IC(33,03007);lmi_newops(s,065255);DO(vad(s,001227));right(s);DO(pddl(s,001250));NEXT;
 /* SL VXSC; 6D; (pushup) */
 IC(33,03012);lmi_newops(s,074261);DO(gsh(s,020207));right(s);DO(vxsc(s,044055));NEXT;
 /* VAD STQ; RN; 31D */
 IC(33,03014);lmi_newops(s,044055);DO(vad(s,001221));right(s);stq(s,000037);NEXT;
 /* STCALL RN1; CALCGRAV */
 IC(33,03017);return stcall(s,037545,066730);
 /* VAD VAD; (pushup); (pushup) */
 IC(33,03021);lmi_newops(s,053255);DO(vad(s,077655));right(s);DO(vad(s,077655));NEXT;
 /* VAD; VN */
 IC(33,03022);lmi_newops(s,077655);DO(vad(s,001227));NEXT;
 /* STCALL VN1; 31D */
 IC(33,03024);return stcall(s,037553,000037);
 /* RVBOTH VLOAD PUSH; G(CSM) */
 IC(33,03032);lmi_newops(s,041575);DO(vload(s,002317));right(s);push(s);NEXT;
 /* VAD PDDL; V(CSM); PGUIDE */
 IC(33,03034);lmi_newops(s,065255);DO(vad(s,001726));right(s);DO(pddl(s,001250));NEXT;
 /* DDV VXSC; SHIFT11; (pushup) */
 IC(33,03037);lmi_newops(s,074271);DO(ddv(s,027031));right(s);DO(vxsc(s,077655));NEXT;
 /* VAD; R(CSM) */
 IC(33,03041);lmi_newops(s,077655);DO(vad(s,001720));NEXT;
 /* STCALL R1S; MUNGRAV */
 IC(33,03043);return stcall(s,037573,067130);
 /* VAD VAD; V(CSM); (pushup) */
 IC(33,03045);lmi_newops(s,053255);DO(vad(s,001726));right(s);DO(vad(s,077626));NEXT;
 /* STADR; STORE V1S */
 IC(33,03047);lmi_newops(s,077626);lmi_unary(s,STADR);lmi_stadr(s,074176);NEXT;
 /* EXIT */
 IC(33,03051);lmi_newops(s,077776);return exit_(s);
 /* VLOAD; GDT1/2 */
 IC(33,03054);lmi_newops(s,077775);DO(vload(s,003563));NEXT;
 /* STOVL G(CSM); R1S */
 IC(33,03056);return stovl(s,026317,003573);
 /* STOVL R(CSM); V1S */
 IC(33,03060);return stovl(s,025720,003601);
 /* STORE V(CSM) */
 IC(33,03062);return store(s,001726);
 /* EXIT */
 IC(33,03063);lmi_newops(s,077776);return exit_(s);
#endif
#if SV_G(4)
 /* MUNRVG VLOAD VXSC; DELV; KPIP2 */
 IC(33,03066);lmi_newops(s,074375);DO(vload(s,000325));right(s);DO(vxsc(s,026024));NEXT;
 /* PUSH VAD; GDT/2 */
 IC(33,03071);lmi_newops(s,053206);push(s);right(s);DO(vad(s,001237));NEXT;
 /* PUSH VAD; V */
 IC(33,03073);lmi_newops(s,053206);push(s);right(s);DO(vad(s,003527));NEXT;
 /* PDDL DDV; PGUIDE; SHIFT11 */
 IC(33,03075);lmi_newops(s,056325);DO(pddl(s,001250));right(s);DO(ddv(s,027031));NEXT;
 /* VXSC; (pushup) */
 IC(33,03100);lmi_newops(s,077761);DO(vxsc(s,077655));NEXT;
 /* VAD; R */
 IC(33,03101);lmi_newops(s,077655);DO(vad(s,003521));NEXT;
 /* STCALL R1S; MUNGRAV */
 IC(33,03103);return stcall(s,037573,067130);
 /* VAD VAD; (pushup); (pushup) */
 IC(33,03105);lmi_newops(s,053255);DO(vad(s,077655));right(s);DO(vad(s,077655));NEXT;
 /* VAD; V */
 IC(33,03106);lmi_newops(s,077655);DO(vad(s,003527));NEXT;
 /* STORE V1S */
 IC(33,03110);return store(s,003601);
 /* ABVAL */
 IC(33,03111);lmi_newops(s,077646);STEP(abval(s));NEXT;
 /* STOVL ABVEL; UNIT/R/ */
 IC(33,03112);return stovl(s,027472,003537);
 /* DOT SL1; V1S */
 IC(33,03114);lmi_newops(s,072441);DO(dot(s,003601));right(s);shorts(s);NEXT;
 /* STOVL HDOTDISP; R1S */
 IC(33,03116);return stovl(s,027474,003573);
 /* VXV VSL2; WM */
 IC(33,03120);lmi_newops(s,072435);DO(vxv(s,002325));right(s);shorts(s);NEXT;
 /* STODL DELVS; 36D */
 IC(33,03122);return stodl(s,017734,000045);
 /* DSU; /LAND/ */
 IC(33,03124);lmi_newops(s,077625);DO(dsu(s,002333));NEXT;
 /* STCALL HCALC; MUNRETRN */
 IC(33,03126);return stcall(s,037535,066461);
#endif
#if SV_G(6)
 /* MUNGRAV UNIT */
 IC(33,03130);lmi_newops(s,077656);STEP(unit(s));NEXT;
 /* STODL UNIT/R/; 34D */
 IC(33,03131);return stodl(s,017537,000043);
 /* SL BDDV; 6D; -MUDTMUN */
 IC(33,03133);lmi_newops(s,055261);DO(gsh(s,020207));right(s);DO(bddv(s,026036));NEXT;
 /* DMP VXSC; SHIFT11; UNIT/R/ */
 IC(33,03136);lmi_newops(s,074205);DO(dmp(s,027031));right(s);DO(vxsc(s,003537));NEXT;
 /* STORE GDT1/2 */
 IC(33,03141);return store(s,003563);
 /* RVQ */
 IC(33,03142);lmi_newops(s,077616);return rvq(s);
#endif
#if SV_G(7)
 /* VLOAD VXM; HBEAMNB; XNBPIP */
 IC(33,03170);lmi_newops(s,061375);DO(vload(s,002273));right(s);DO(vxm(s,002146));NEXT;
 /* PDVL VSL2; V1S */
 IC(33,03173);lmi_newops(s,072515);DO(pdvl(s,003601));right(s);shorts(s);NEXT;
 /* VAD DOT; DELVS; 0D */
 IC(33,03175);lmi_newops(s,050255);DO(vad(s,003734));right(s);DO(dot(s,000001));NEXT;
 /* DMP EXIT; RADSKAL */
 IC(33,03200);lmi_newops(s,077405);DO(dmp(s,001355));right(s);return rexit(s);
#endif
#if SV_G(6)
 /* DAD SL; HMEAS; 7D */
 IC(33,03211);lmi_newops(s,054215);DO(dad(s,003655));right(s);DO(gsh(s,020210));NEXT;
 /* DMP VXSC; HSCAL; (pushup) */
 IC(33,03214);lmi_newops(s,074205);DO(dmp(s,026011));right(s);DO(vxsc(s,045241));NEXT;
 /* DOT DSU; UNIT/R/; HCALC */
 IC(33,03216);lmi_newops(s,045241);DO(dot(s,003537));right(s);DO(dsu(s,003535));NEXT;
 /* STORE DELTAH */
 IC(33,03221);return store(s,003665);
 /* EXIT */
 IC(33,03222);lmi_newops(s,077776);return exit_(s);
 /* ABS DSU; DELQFIX */
 IC(33,03230);lmi_newops(s,045246);abs_(s);right(s);DO(dsu(s,002521));NEXT;
 /* SL3 DSU; HCALC */
 IC(33,03232);lmi_newops(s,045252);shorts(s);right(s);DO(dsu(s,003535));NEXT;
 /* EXIT */
 IC(33,03234);lmi_newops(s,077776);return exit_(s);
#endif
#if SV_G(2)
 /* DLOAD SR4; HCALC */
 IC(33,03250);lmi_newops(s,040545);DO(dload(s,003535));right(s);shorts(s);NEXT;
 /* EXIT */
 IC(33,03252);lmi_newops(s,077776);return exit_(s);
#endif
#if SV_G(7)
 /* SL1 */
 IC(33,03276);lmi_newops(s,077752);shorts(s);NEXT;
 /* VXSC VAD; UNIT/R/; R1S */
 IC(33,03277);lmi_newops(s,053361);DO(vxsc(s,003537));right(s);DO(vad(s,003573));NEXT;
 /* STCALL GNUR; MUNGRAV */
 IC(33,03302);return stcall(s,037657,067130);
 /* EXIT */
 IC(33,03304);lmi_newops(s,077776);return exit_(s);
#endif
#if SV_G(2)
 /* VLOAD* CALL; VZBEAMNB,1; *NBSM* */
 IC(33,03341);lmi_newops(s,045173);DO(vloadx(s,002231));right(s);return call(s,047673);
#endif
#if SV_G(3)
 /* PDDL SL; VMEAS; 12D */
 IC(33,03344);lmi_newops(s,054325);DO(pddl(s,003653));right(s);DO(gsh(s,020215));NEXT;
 /* DMP* PUSH; VZSCAL,2 */
 IC(33,03347);lmi_newops(s,041403);DO(dmpx(s,051764));right(s);push(s);NEXT;
 /* EXIT */
 IC(33,03351);lmi_newops(s,077776);return exit_(s);
 /* VXSC PDDL; KPIP1; LRVTIME */
 IC(33,03370);lmi_newops(s,065361);DO(vxsc(s,026022));right(s);DO(pddl(s,002253));NEXT;
 /* DSU DDV; PIPTIME; 2SEC(28) */
 IC(33,03373);lmi_newops(s,056225);DO(dsu(s,001235));right(s);DO(ddv(s,027151));NEXT;
 /* VXSC VSL1; GDT/2 */
 IC(33,03376);lmi_newops(s,076561);DO(vxsc(s,001237));right(s);shorts(s);NEXT;
 /* VAD VAD; V; (pushup) */
 IC(33,03400);lmi_newops(s,053255);DO(vad(s,003527));right(s);DO(vad(s,053352));NEXT;
 /* VSL2 VAD; DELVS */
 IC(33,03402);lmi_newops(s,053352);shorts(s);right(s);DO(vad(s,003734));NEXT;
 /* PUSH ABVAL */
 IC(33,03404);lmi_newops(s,051406);push(s);right(s);STEP(abval(s));NEXT;
 /* SR4 DAD; 7.5 */
 IC(33,03405);lmi_newops(s,043202);shorts(s);right(s);DO(dad(s,027145));NEXT;
 /* STOVL 20D; (pushup) */
 IC(33,03407);return stovl(s,024025,044241);
 /* DOT BDSU; 0; (pushup) */
 IC(33,03410);lmi_newops(s,044241);DO(dot(s,000001));right(s);DO(bdsu(s,051406));NEXT;
 /* PUSH ABS */
 IC(33,03412);lmi_newops(s,051406);push(s);right(s);abs_(s);NEXT;
 /* DSU EXIT; 20D */
 IC(33,03413);lmi_newops(s,077425);DO(dsu(s,000025));right(s);return rexit(s);
#endif
#if SV_G(7)
 /* DMP VXSC; (pushup); (pushup) */
 IC(33,03504);lmi_newops(s,074205);DO(dmp(s,077655));right(s);DO(vxsc(s,077655));NEXT;
 /* VAD; V1S */
 IC(33,03505);lmi_newops(s,077655);DO(vad(s,003601));NEXT;
 /* STORE GNUV */
 IC(33,03507);return store(s,003657);
 /* EXIT */
 IC(33,03510);lmi_newops(s,077776);return exit_(s);
 /* VLOAD CALL; UNITY; TRG*SMNB */
 IC(33,03756);lmi_newops(s,045175);DO(vload(s,006516));right(s);return call(s,047651);
 /* STOVL VYBEAMNB; UNITX */
 IC(33,03761);return stovl(s,026237,006520);
 /* CALL; *SMNB* */
 IC(33,03763);lmi_newops(s,077624);return call(s,047671);
#endif
#if SV_G(4)
 /* STORE VXBEAMNB */
 IC(33,03765);return store(s,002245);
 /* VXV VSL1; VYBEAMNB */
 IC(33,03766);lmi_newops(s,076435);DO(vxv(s,002237));right(s);shorts(s);NEXT;
 /* STOVL VZBEAMNB; HBEAMANT */
 IC(33,03770);return stovl(s,026231,026003);
 /* CALL; *SMNB* */
 IC(33,03772);lmi_newops(s,077624);return call(s,047671);
#endif
#if SV_G(6)
 /* STORE HBEAMNB */
 IC(33,03774);return store(s,002273);
 /* EXIT */
 IC(33,03775);lmi_newops(s,077776);return exit_(s);
#endif
 default:return SV_END(ph,1);
 }
}

/* ---- Basic helpers ------------------------------------------------------ */
static NOINL int out(apollo_overlay_state *s,agc_word q,agc_word z){ai_tc(s,q);W[5]=z;return STOP;}
static NOINL int jump(apollo_overlay_state *s,agc_word z){W[5]=z;return STOP;}
/* Continue at z, which the step loop runs if it is the file's word under
 * FB and leaves the file at otherwise. */
static NOINL int to(apollo_overlay_state *s,agc_word z){W[5]=z;return GO;}
static NOINL void intpret(apollo_overlay_state *s,lm_sv_io *io,agc_word ret){ai_tc(s,ret);W[5]=X_INTPRET;lmi_intpret(s,&io->control);}
/* Channel orders, as yaAGC's. */
static NOINL void rand_(apollo_overlay_state *s,agc_word chan){W[0]=agc_extend((agc_word)(oc(W[0])&chan));}
static NOINL void wand(apollo_overlay_state *s,agc_word *chan){agc_word v=(agc_word)(oc(W[0])&*chan);*chan=v;W[0]=agc_extend(v);}
static NOINL void wor(apollo_overlay_state *s,agc_word *chan){agc_word v=(agc_word)(oc(W[0])|*chan);*chan=v;W[0]=agc_extend(v);}
/* TC Q: the word in Q runs as TC (Z = Q, Q = 3); TC A (QUIKFAZ5's return):
 * the word in A runs as TC (Q = 1). Either must be a TC word into fixed
 * memory. */
static NOINL int tcq(apollo_overlay_state *s){agc_register q=W[QREG];if(!tc_word(q))return REFUSE;ai_tc(s,3u);return to(s,(agc_word)q);}
static NOINL int tca(apollo_overlay_state *s){agc_register a=W[A];if(!tc_word(a))return REFUSE;ai_tc(s,1u);return to(s,(agc_word)a);}
/* TC LRADRET: the word SETPOS stored there runs as TC (Q = LRADRET +1). */
static NOINL int tclradret(apollo_overlay_state *s){agc_word v=ai_word(s,LRADRET);if(!tc_word(v))return REFUSE;ai_tc(s,LRADRET+1u);return to(s,v);}
/* DTCB (DXCH Z) executed at `at`: Z,BB <-> A,L (Z keeps 12 bits). */
static NOINL void dtcb(apollo_overlay_state *s,agc_word at){agc_word z=agc_correct(W[0]),bb=agc_correct(W[1]);
 W[0]=(agc_word)(at+1u);W[1]=agc_extend(W[6]);W[5]=(agc_word)(z&07777u);
 W[6]=(agc_word)(bb&076007u);W[3]=(agc_word)((bb&7u)<<8);W[4]=(agc_word)(bb&076000u);
}
/* INDEX: the value an indexed instruction adds to its address, one of the
 * n the file's producers store there (0, step, .. step(n-1)): from a word
 * of erasable (ix) or a register (ixr). */
static NOINL int ixr(agc_register w,unsigned n,unsigned step,unsigned *v){
 if(w%step||w/step>=n)return 0;
 *v=w;return 1;
}
static NOINL int ix(const apollo_overlay_state *s,unsigned x,unsigned n,unsigned step,unsigned *v){return ixr(ai_word(s,x),n,step,v);}
#define IX(x,n,step) do{if(!ix(s,(x),(n),(step),&v))return REFUSE;}while(0)
static __attribute__((unused)) int vac(agc_word f){return f==0401u||f==0455u||f==0531u||f==0605u||f==0661u;}
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#define AT(b,a) case PH(b,a): L##b##_##a: __attribute__((unused)) SV_PATH(PATHOF(b,a))
#define GOTO(b,a) goto L##b##_##a
#define TC(r,b,a) do{ai_tc(s,(r));GOTO(b,a);}while(0)

/* Basic code at Z (physical: the bank FB selects). */
static int basic(apollo_overlay_state *s,lm_sv_io *io,uint16_t ph){
 unsigned v=0;uint8_t c;
 (void)s;(void)io;(void)v;(void)c;
 switch(ph){
#if SV_G(1)
 /* PREREAD CAF SEVEN; TC GNUFAZE5; CAF PRIO21; TC NOVAC */
 AT(37,03410);ai_caf(s,K_SEVEN);
 AT(37,03411);TC(03412,37,03531);
 AT(37,03412);ai_caf(s,K_PRIO21);
 AT(37,03413);return out(s,03414,X_NOVAC);
 /* BIBIBIAS TC PIPASR +3; CS FLAGWRD7; MASK SUPER011; ADS FLAGWRD7; CS
  * DRFTBIT; MASK FLAGWRD2; TS FLAGWRD2; CAF FOUR; TS PIPAGE; CAF ENDJBCAD; TS
  * OUTROUTE; CAF PRIO22; TC FINDVAC */
 AT(37,03416);TC(03417,37,03544);
 AT(37,03417);ai_cs(s,FLAGWRD7);
 AT(37,03420);ai_maskf(s,K_SUPER011);
 AT(37,03421);ai_ads(s,FLAGWRD7);
 AT(37,03422);ai_csf(s,K_DRFTBIT);
 AT(37,03423);ai_mask(s,FLAGWRD2);
 AT(37,03424);if(ai_ts(s,FLAGWRD2)){GOTO(37,03426);}
 AT(37,03425);ai_caf(s,K_FOUR);
 AT(37,03426);if(ai_ts(s,PIPAGE)){GOTO(37,03430);}
 AT(37,03427);ai_caf(s,K_ENDJBCAD);
 AT(37,03430);if(ai_ts(s,OUTROUTE)){GOTO(37,03432);}
 AT(37,03431);ai_caf(s,K_PRIO22);
 AT(37,03432);return out(s,03433,X_FINDVAC);
 /* CA TWO */
 AT(37,03435);ai_caf(s,K_TWO);
 /* GOREADAX TC GNUTFAZ5; CA 2SECS; TC VARDELAY */
 AT(37,03436);TC(03437,37,03525);
 AT(37,03437);ai_caf(s,K_N2SECS);
 AT(37,03440);return out(s,03441,X_VARDELAY);
 /* READACCS CS OCT37771; AD TIME5; CCS A; CS ONE; TCF +2; CA ONE */
 AT(37,03441);ai_csf(s,K_OCT37771);
 AT(37,03442);ai_ad(s,TIME5);
 AT(37,03443);c=ai_ccs(s,A);BR(8u+c);switch(c){case 1:GOTO(37,03445);case 2:GOTO(37,03446);case 3:GOTO(37,03447);}
 AT(37,03444);ai_csf(s,K_ONE);
 AT(37,03445);GOTO(37,03447);
 AT(37,03446);ai_caf(s,K_ONE);
 /* +2 ADS TIME5; TC PIPASR */
 AT(37,03447);ai_ads(s,TIME5);
 AT(37,03450);TC(03451,37,03541);
 /* PIPSDONE CA FIVE; TC GNUFAZE5 */
 AT(37,03451);ai_caf(s,K_FIVE);
 AT(37,03452);TC(03453,37,03531);
 /* REDO5.5 CAF ONE; TS PIPAGE; CA PRIO20; TC FINDVAC */
 AT(37,03453);ai_caf(s,K_ONE);
 AT(37,03454);if(ai_ts(s,PIPAGE)){GOTO(37,03456);}
 AT(37,03455);ai_caf(s,K_PRIO20);
 AT(37,03456);return out(s,03457,X_FINDVAC);
 /* CA BIT9; EXTEND; WOR DSALMOUT; CA FLAGWRD7; MASK AVEGFBIT; EXTEND; BZF
  * AVEGOUT; CA FLAGWRD6; MASK MUNFLBIT; EXTEND; BZF MAKEACCS; CCS PHASE2; TCF
  * MAKEACCS; CAF SEVEN; TS PIPCTR; CS TIME1; AD FIVE; AD NEG1/2; AD NEG1/2;
  * XCH TBASE2; CAF DEC17; TS L; COM; DXCH -PHASE2; CAF OCT24; TC WAITLIST */
 AT(37,03461);ai_caf(s,K_BIT9);
 AT(37,03462);
 AT(37,03463);wor(s,&io->chan11);
 AT(37,03464);ai_ca(s,FLAGWRD7);
 AT(37,03465);ai_maskf(s,K_AVEGFBIT);
 AT(37,03466);
 AT(37,03467);if(ai_bzf(s)){BR(12);GOTO(37,03517);}BR(13);
 AT(37,03470);ai_ca(s,FLAGWRD6);
 AT(37,03471);ai_maskf(s,K_MUNFLBIT);
 AT(37,03472);
 AT(37,03473);if(ai_bzf(s)){BR(14);GOTO(37,03515);}BR(15);
 AT(37,03474);c=ai_ccs(s,PHASE2);BR(16u+c);switch(c){case 1:GOTO(37,03476);case 2:GOTO(37,03477);case 3:GOTO(37,03500);}
 AT(37,03475);GOTO(37,03515);
 AT(37,03476);ai_caf(s,K_SEVEN);
 AT(37,03477);if(ai_ts(s,PIPCTR)){GOTO(37,03501);}
 AT(37,03500);ai_cs(s,TIME1);
 AT(37,03501);ai_adf(s,K_FIVE);
 AT(37,03502);ai_adf(s,K_NEG1_2);
 AT(37,03503);ai_adf(s,K_NEG1_2);
 AT(37,03504);ai_xch(s,TBASE2);
 AT(37,03505);ai_caf(s,K_DEC17);
 AT(37,03506);if(ai_ts(s,LREG)){GOTO(37,03510);}
 AT(37,03507);ai_com(s);
 AT(37,03510);ai_dxch(s,MPHASE2);
 AT(37,03511);ai_caf(s,K_OCT24);
 AT(37,03512);return out(s,03513,X_WAITLIST);
 /* MAKEACCS CA FOUR; TCF GOREADAX */
 AT(37,03515);ai_caf(s,K_FOUR);
 AT(37,03516);GOTO(37,03436);
 /* AVEGOUT EXTEND; DCA AVOUTCAD; DXCH AVGEXIT; CA FOUR; TC GNUTFAZ5; TC
  * TASKOVER */
 AT(37,03517);
 AT(37,03520);ai_dcaf(s,K_AVOUTCAD);
 AT(37,03521);ai_dxch(s,AVGEXIT);
 AT(37,03522);ai_caf(s,K_FOUR);
 AT(37,03523);TC(03524,37,03525);
 AT(37,03524);return out(s,03525,X_TASKOVER);
 /* GNUTFAZ5 TS L; CS TIME1; TS TBASE5; TCF +2 */
 AT(37,03525);if(ai_ts(s,LREG)){GOTO(37,03527);}
 AT(37,03526);ai_cs(s,TIME1);
 AT(37,03527);if(ai_ts(s,TBASE5)){GOTO(37,03531);}
 AT(37,03530);GOTO(37,03532);
 /* GNUFAZE5 TS L; CS L; DXCH -PHASE5; TC Q */
 AT(37,03531);if(ai_ts(s,LREG)){GOTO(37,03533);}
 AT(37,03532);ai_cs(s,LREG);
 AT(37,03533);ai_dxch(s,MPHASE5);
 AT(37,03534);return tcq(s);
 /* SERVICER TC PHASCHNG */
 AT(33,02206);return out(s,02207,X_PHASCHNG);
 /* CAF PRIO31; TS 1/PIPADT; TC BANKCALL */
 AT(33,02213);ai_caf(s,K_PRIO31);
 AT(33,02214);if(ai_ts(s,N1_PIPADT)){return to(s,02216u);}
 AT(33,02215);return out(s,02216,X_BANKCALL);
#endif
#if SV_G(6)
 /* GETABVAL TC INTPRET */
 AT(33,02217);intpret(s,io,02220);return GO;
 /* CA MPAC; TS ABDELV; EXTEND; MP KPIP; DXCH ABDVCONV; EXTEND; DCA MASS; DXCH
  * MASS1 */
 AT(33,02223);ai_ca(s,MPAC);
 AT(33,02224);if(ai_ts(s,ABDELV)){return REFUSE;}
 AT(33,02225);
 AT(33,02226);ai_mpf(s,K_KPIP);
 AT(33,02227);ai_dxch(s,ABDVCONV);
 AT(33,02230);
 AT(33,02231);ai_dca(s,MASS);
 AT(33,02232);ai_dxch(s,MASS1);
#endif
#if !SV_G(1) && (SV_G(6))
 L33_02233: __attribute__((unused)) W[5]=02233u;return STOP;
#endif
#if SV_G(1)
 /* MASSMON CS FLAGWRD8; MASK SURFFBIT; EXTEND; BZF MOONSPOT; CA FLGWRD10;
  * MASK APSFLBIT; CCS A; EXTEND; DCA APSVEX; TS Q; EXTEND; DCA ABDVCONV;
  * EXTEND */
 AT(33,02233);ai_cs(s,FLAGWRD8);
 AT(33,02234);ai_maskf(s,K_SURFFBIT);
 AT(33,02235);
 AT(33,02236);if(ai_bzf(s)){BR(20);GOTO(33,02254);}BR(21);
 AT(33,02237);ai_ca(s,FLGWRD10);
 AT(33,02240);ai_maskf(s,K_APSFLBIT);
 AT(33,02241);c=ai_ccs(s,A);BR(22u+c);switch(c){case 1:GOTO(33,02243);case 2:GOTO(33,02244);case 3:GOTO(33,02245);}
 AT(33,02242);ai_dcaf(s,K_APSVEX);GOTO(33,02244);   /* EXTEND; DCA APSVEX */
 /* CCS A's +0 outcome runs the DCA word without its EXTEND: CA DPSVEX. */
 AT(33,02243);ai_caf(s,K_DPSVEX);
 AT(33,02244);if(ai_ts(s,QREG)){return REFUSE;}
 AT(33,02245);
 AT(33,02246);ai_dca(s,ABDVCONV);
 AT(33,02247);
 /* OCT10002 DV Q; EXTEND; MP MASS; DAS MASS1 */
 AT(33,02250);ai_dv(s,QREG);
 AT(33,02251);
 AT(33,02252);ai_mp(s,MASS);
 AT(33,02253);ai_das(s,MASS1);
 /* MOONSPOT CA KPIP1; TC SHORTMP; DXCH MPAC; DAS DVTOTAL; TC TMPTOSPT; TC
  * BANKCALL */
 AT(33,02254);ai_caf(s,K_KPIP1);
 AT(33,02255);return out(s,02256,X_SHORTMP);
 AT(33,02256);ai_dxch(s,MPAC);
 AT(33,02257);ai_das(s,DVTOTAL);
 AT(33,02260);TC(02261,33,02440);
 AT(33,02261);return out(s,02262,X_BANKCALL);
 /* CAF XNBPIPAD; TC BANKCALL */
 AT(33,02263);ai_caf(s,K_XNBPIPAD);
 AT(33,02264);return out(s,02265,X_BANKCALL);
 /* TC INTPRET */
 AT(33,02266);intpret(s,io,02267);return GO;
 /* GOSERV TC QUIKFAZ5 */
 AT(33,02274);TC(02275,33,03552);
 /* COPYCYCL TC COPYCYC; TS PIPATMPX; TS PIPATMPY; TS PIPATMPZ; CS STEERBIT;
  * MASK FLAGWRD2; TS FLAGWRD2; CAF IDLEFBIT; MASK FLAGWRD7; CCS A; TCF
  * NODVMON1; CS FLAGWRD6; MASK AUXFLBIT; CCS A; TCF NODVMON2 */
 AT(33,02275);TC(02276,33,02423);
 AT(33,02276);if(ai_ts(s,PIPATMPX)){GOTO(33,02300);}
 AT(33,02277);if(ai_ts(s,PIPATMPY)){GOTO(33,02301);}
 AT(33,02300);if(ai_ts(s,PIPATMPZ)){GOTO(33,02302);}
 AT(33,02301);ai_csf(s,K_STEERBIT);
 AT(33,02302);ai_mask(s,FLAGWRD2);
 AT(33,02303);if(ai_ts(s,FLAGWRD2)){GOTO(33,02305);}
 AT(33,02304);ai_caf(s,K_IDLEFBIT);
 AT(33,02305);ai_mask(s,FLAGWRD7);
 AT(33,02306);c=ai_ccs(s,A);BR(26u+c);switch(c){case 1:GOTO(33,02310);case 2:GOTO(33,02311);case 3:GOTO(33,02312);}
 AT(33,02307);GOTO(33,02342);
 AT(33,02310);ai_cs(s,FLAGWRD6);
 AT(33,02311);ai_maskf(s,K_AUXFLBIT);
 AT(33,02312);c=ai_ccs(s,A);BR(30u+c);switch(c){case 1:GOTO(33,02314);case 2:GOTO(33,02315);case 3:GOTO(33,02316);}
 AT(33,02313);GOTO(33,02346);
 /* DVMON CS DVTHRUSH; AD ABDELV; EXTEND; BZMF LOTHRUST; CS FLAGWRD2; MASK
  * STEERBIT; ADS FLAGWRD2 */
 AT(33,02314);ai_cs(s,DVTHRUSH);
 AT(33,02315);ai_ad(s,ABDELV);
 AT(33,02316);
 AT(33,02317);if(ai_bzmf(s)){BR(34);GOTO(33,02352);}BR(35);
 AT(33,02320);ai_cs(s,FLAGWRD2);
 AT(33,02321);ai_maskf(s,K_STEERBIT);
 AT(33,02322);ai_ads(s,FLAGWRD2);
 /* DVCNTSET CAF ONE; TS DVCNTR; CA FLGWRD10; MASK APSFLBIT; CCS A; TCF
  * USEJETS; CA BIT9; EXTEND; RAND CHAN32; EXTEND; BZF USEJETS */
 AT(33,02323);ai_caf(s,K_ONE);
 AT(33,02324);if(ai_ts(s,DVCNTR)){GOTO(33,02326);}
 AT(33,02325);ai_ca(s,FLGWRD10);
 AT(33,02326);ai_maskf(s,K_APSFLBIT);
 AT(33,02327);c=ai_ccs(s,A);BR(36u+c);switch(c){case 1:GOTO(33,02331);case 2:GOTO(33,02332);case 3:return REFUSE;}
 AT(33,02330);GOTO(33,02375);
 AT(33,02331);ai_caf(s,K_BIT9);
 AT(33,02332);
 AT(33,02333);rand_(s,io->chan32);
 AT(33,02334);
 AT(33,02335);if(ai_bzf(s)){BR(40);GOTO(33,02375);}BR(41);
 /* USEGTS CS USEQRJTS; MASK DAPBOOLS; TS DAPBOOLS; TCF SERVOUT */
 AT(33,02336);ai_csf(s,K_USEQRJTS);
 AT(33,02337);ai_mask(s,DAPBOOLS);
 AT(33,02340);if(ai_ts(s,DAPBOOLS)){GOTO(33,02342);}
 AT(33,02341);GOTO(33,02400);
 /* NODVMON1 CS AUXFLBIT; MASK FLAGWRD6; TS FLAGWRD6; TCF USEJETS */
 AT(33,02342);ai_csf(s,K_AUXFLBIT);
 AT(33,02343);ai_mask(s,FLAGWRD6);
 AT(33,02344);if(ai_ts(s,FLAGWRD6)){GOTO(33,02346);}
 AT(33,02345);GOTO(33,02375);
 /* NODVMON2 CS FLAGWRD6; MASK AUXFLBIT; ADS FLAGWRD6; TCF USEJETS */
 AT(33,02346);ai_cs(s,FLAGWRD6);
 AT(33,02347);ai_maskf(s,K_AUXFLBIT);
 AT(33,02350);ai_ads(s,FLAGWRD6);
 AT(33,02351);GOTO(33,02375);
 /* LOTHRUST TC QUIKFAZ5; CCS DVCNTR; TCF DECCNTR; CCS PHASE4; TCF SERVOUT; TC
  * PHASCHNG */
 AT(33,02352);TC(02353,33,03552);
 AT(33,02353);c=ai_ccs(s,DVCNTR);BR(42u+c);switch(c){case 1:GOTO(33,02355);case 2:GOTO(33,02356);case 3:GOTO(33,02357);}
 AT(33,02354);GOTO(33,02366);
 AT(33,02355);c=ai_ccs(s,PHASE4);BR(46u+c);switch(c){case 1:GOTO(33,02357);case 2:return to(s,02360u);case 3:GOTO(33,02361);}
 AT(33,02356);GOTO(33,02400);
 AT(33,02357);return out(s,02360,X_PHASCHNG);
 /* CAF PRIO25; TC NOVAC */
 AT(33,02361);ai_caf(s,K_PRIO25);
 AT(33,02362);return out(s,02363,X_NOVAC);
 /* TCF SERVOUT */
 AT(33,02365);GOTO(33,02400);
 /* DECCNTR TS DVCNTR1; TC QUIKFAZ5; CA DVCNTR1; TS DVCNTR; INHINT; TC
  * IBNKCALL */
 AT(33,02366);if(ai_ts(s,DVCNTR1)){GOTO(33,02370);}
 AT(33,02367);TC(02370,33,03552);
 AT(33,02370);ai_ca(s,DVCNTR1);
 AT(33,02371);if(ai_ts(s,DVCNTR)){GOTO(33,02373);}
 AT(33,02372);io->control.interrupt_enabled=0;
 AT(33,02373);return out(s,02374,X_IBNKCALL);
 /* USEJETS CS DAPBOOLS; MASK USEQRJTS; ADS DAPBOOLS */
 AT(33,02375);ai_cs(s,DAPBOOLS);
 AT(33,02376);ai_maskf(s,K_USEQRJTS);
 AT(33,02377);ai_ads(s,DAPBOOLS);
 /* SERVOUT RELINT; TC BANKCALL */
 AT(33,02400);io->control.interrupt_enabled=1;
 AT(33,02401);return out(s,02402,X_BANKCALL);
 /* CA PRIORITY; MASK LOW9; TS PUSHLOC; ZL; DXCH FIXLOC; TC QUIKFAZ5; EXTEND;
  * DCA AVGEXIT; DXCH Z */
 AT(33,02403);ai_ca(s,PRIORITY);
 AT(33,02404);ai_maskf(s,K_LOW9);
 AT(33,02405);if(ai_ts(s,PUSHLOC)){GOTO(33,02407);}
 AT(33,02406);ai_zl(s);
 AT(33,02407);ai_dxch(s,FIXLOC);
 AT(33,02410);TC(02411,33,03552);
 AT(33,02411);
 AT(33,02412);ai_dca(s,AVGEXIT);
 AT(33,02413);dtcb(s,02413);return GO;
 /* AVGEND CA PIPTIME +1; TS 1/PIPADT; TC UPFLAG */
 AT(22,03661);ai_ca(s,PIPTIME+1u);
 AT(22,03662);if(ai_ts(s,N1_PIPADT)){return to(s,03664u);}
 AT(22,03663);return out(s,03664,X_UPFLAG);
 /* TC BANKCALL */
 AT(22,03665);return out(s,03666,X_BANKCALL);
 /* CS BIT9; EXTEND; WAND DSALMOUT; TC 2PHSCHNG */
 AT(22,03667);ai_csf(s,K_BIT9);
 AT(22,03670);
 AT(22,03671);wand(s,&io->chan11);
 AT(22,03672);return out(s,03673,X_N2PHSCHNG);
#endif
#if SV_G(5)
 /* TC INTPRET */
 AT(22,03676);intpret(s,io,03677);return GO;
#endif
#if SV_G(1)
 /* AVERTRN CA OUTROUTE; TC BANKJUMP */
 AT(22,03707);ai_ca(s,OUTROUTE);
 AT(22,03710);return out(s,03711,X_BANKJUMP);
 /* SERVIDLE EXTEND; DCA SVEXTADR; DXCH AVGEXIT; CS FLAGWRD7; MASK IDLEFBIT;
  * ADS FLAGWRD7; CAF LRBYBIT; TS FLGWRD11; EXTEND; DCA NEG0; DXCH -PHASE1; CA
  * FLAGWRD6; MASK MUNFLBIT; CCS A; TCF +4; EXTEND; DCA NEG0; DXCH -PHASE2 */
 AT(27,03321);
 AT(27,03322);ai_dcaf(s,K_SVEXTADR);
 AT(27,03323);ai_dxch(s,AVGEXIT);
 AT(27,03324);ai_cs(s,FLAGWRD7);
 AT(27,03325);ai_maskf(s,K_IDLEFBIT);
 AT(27,03326);ai_ads(s,FLAGWRD7);
 AT(27,03327);ai_caf(s,K_LRBYBIT);
 AT(27,03330);if(ai_ts(s,FLGWRD11)){return REFUSE;}
 AT(27,03331);
 AT(27,03332);ai_dcaf(s,K_NEG0);
 AT(27,03333);ai_dxch(s,MPHASE1);
 AT(27,03334);ai_ca(s,FLAGWRD6);
 AT(27,03335);ai_maskf(s,K_MUNFLBIT);
 AT(27,03336);c=ai_ccs(s,A);BR(50u+c);switch(c){case 1:GOTO(27,03340);case 2:return REFUSE;case 3:GOTO(27,03342);}
 AT(27,03337);GOTO(27,03343);
 AT(27,03340);
 AT(27,03341);ai_dcaf(s,K_NEG0);
 AT(27,03342);ai_dxch(s,MPHASE2);
 /* +4 EXTEND; DCA NEG0; DXCH -PHASE3; EXTEND; DCA NEG0; DXCH -PHASE6; CAF
  * OCT33; TS L; COM; DXCH -PHASE4; TCF WHIMPER */
 AT(27,03343);
 AT(27,03344);ai_dcaf(s,K_NEG0);
 AT(27,03345);ai_dxch(s,MPHASE3);
 AT(27,03346);
 AT(27,03347);ai_dcaf(s,K_NEG0);
 AT(27,03350);ai_dxch(s,MPHASE6);
 AT(27,03351);ai_caf(s,K_OCT33);
 AT(27,03352);if(ai_ts(s,LREG)){GOTO(27,03354);}
 AT(27,03353);ai_com(s);
 AT(27,03354);ai_dxch(s,MPHASE4);
 AT(27,03355);return jump(s,X_WHIMPER);
 /* SERVEXIT TC PHASCHNG */
 AT(32,03770);return out(s,03771,X_PHASCHNG);
 /* +2 TCF ENDOFJOB */
 AT(32,03772);return jump(s,X_ENDOFJOB);
#endif
#if SV_G(5)
 /* NORMLIZE TC INTPRET */
 AT(23,02461);intpret(s,io,02462);return GO;
#endif
#if SV_G(7)
 /* EXTEND; DCA NEG0; DXCH -PHASE2; TC POSTJUMP */
 AT(23,02504);
 AT(23,02505);ai_dcaf(s,K_NEG0);
 AT(23,02506);ai_dxch(s,MPHASE2);
 AT(23,02507);return out(s,02510,X_POSTJUMP);
#endif
#if SV_G(1)
 /* NORMLIZ2 CA EIGHTEEN; TC COPYCYC +1; TC ENDOFJOB */
 AT(33,02420);ai_caf(s,K_EIGHTEEN);
 AT(33,02421);TC(02422,33,02424);
 AT(33,02422);return out(s,02423,X_ENDOFJOB);
 /* COPYCYC CA OCT24 */
 AT(33,02423);ai_caf(s,K_OCT24);
 /* +1 INHINT */
 AT(33,02424);io->control.interrupt_enabled=0;
 /* +2 MASK NEG1; TS ITEMP1; EXTEND; INDEX ITEMP1; DCA RN1; INDEX ITEMP1; DXCH
  * RN; CCS ITEMP1; TCF COPYCYC +2; TC Q */
 AT(33,02425);ai_maskf(s,K_NEG1);
 AT(33,02426);if(ai_ts(s,ITEMP1)){return REFUSE;}
 AT(33,02427);
 AT(33,02430);IX(ITEMP1,11u,2u);
 AT(33,02431);ai_dca(s,RN1+v);
 AT(33,02432);IX(ITEMP1,11u,2u);
 AT(33,02433);ai_dxch(s,RN+v);
 AT(33,02434);c=ai_ccs(s,ITEMP1);BR(54u+c);switch(c){case 1:GOTO(33,02436);case 2:return to(s,02437u);case 3:GOTO(33,02440);}
 AT(33,02435);GOTO(33,02425);
 AT(33,02436);return tcq(s);
 /* PIPASR EXTEND; DCA TIME2; DXCH PIPTIME1 */
 AT(37,03541);
 AT(37,03542);ai_dca(s,TIME2);
 AT(37,03543);ai_dxch(s,PIPTIME1);
 /* +3 CS ZERO; TS TEMX; TS TEMY; TS TEMZ; CA ZERO; TS DELVZ; TS DELVZ +1; TS
  * DELVY; TS DELVY +1; TS DELVX +1; TS PIPAGE */
 AT(37,03544);ai_csf(s,K_ZERO);
 AT(37,03545);if(ai_ts(s,TEMX)){GOTO(37,03547);}
 AT(37,03546);if(ai_ts(s,TEMY)){GOTO(37,03550);}
 AT(37,03547);if(ai_ts(s,TEMZ)){GOTO(37,03551);}
 AT(37,03550);ai_caf(s,K_ZERO);
 AT(37,03551);if(ai_ts(s,DELVZ)){GOTO(37,03553);}
 AT(37,03552);if(ai_ts(s,DELVZ+1u)){GOTO(37,03554);}
 AT(37,03553);if(ai_ts(s,DELVY)){GOTO(37,03555);}
 AT(37,03554);if(ai_ts(s,DELVY+1u)){GOTO(37,03556);}
 AT(37,03555);if(ai_ts(s,DELVX+1u)){GOTO(37,03557);}
 AT(37,03556);if(ai_ts(s,PIPAGE)){return REFUSE;}
 /* REPIP1 EXTEND; DCS PIPAX; DXCH TEMX; DXCH PIPAX; TS DELVX; LXCH DELVY */
 AT(37,03557);
 AT(37,03560);ai_dcs(s,PIPAX);
 AT(37,03561);ai_dxch(s,TEMX);
 AT(37,03562);ai_dxch(s,PIPAX);
 AT(37,03563);if(ai_ts(s,DELVX)){GOTO(37,03565);}
 AT(37,03564);ai_lxch(s,DELVY);
 /* REPIP3 CS PIPAZ; XCH TEMZ; XCH PIPAZ */
 AT(37,03565);ai_cs(s,PIPAZ);
 AT(37,03566);ai_xch(s,TEMZ);
 AT(37,03567);ai_xch(s,PIPAZ);
 /* DODELVZ TS DELVZ */
 AT(37,03570);if(ai_ts(s,DELVZ)){return REFUSE;}
 /* REPIP4 EXTEND; DCA PIPTIME1; DXCH PGUIDE; EXTEND; DCS PIPTIME; DAS PGUIDE;
  * CA CDUX; TS CDUTEMPX; CA CDUY; TS CDUTEMPY; CA CDUZ; TS CDUTEMPZ; CA
  * DELVX; TS PIPATMPX; CA DELVY; TS PIPATMPY; CA DELVZ; TS PIPATMPZ; TC Q */
 AT(37,03571);
 AT(37,03572);ai_dca(s,PIPTIME1);
 AT(37,03573);ai_dxch(s,PGUIDE);
 AT(37,03574);
 AT(37,03575);ai_dcs(s,PIPTIME);
 AT(37,03576);ai_das(s,PGUIDE);
 AT(37,03577);ai_ca(s,CDUX);
 AT(37,03600);if(ai_ts(s,CDUTEMPX)){GOTO(37,03602);}
 AT(37,03601);ai_ca(s,CDUY);
 AT(37,03602);if(ai_ts(s,CDUTEMPY)){GOTO(37,03604);}
 AT(37,03603);ai_ca(s,CDUZ);
 AT(37,03604);if(ai_ts(s,CDUTEMPZ)){GOTO(37,03606);}
 AT(37,03605);ai_ca(s,DELVX);
 AT(37,03606);if(ai_ts(s,PIPATMPX)){GOTO(37,03610);}
 AT(37,03607);ai_ca(s,DELVY);
 AT(37,03610);if(ai_ts(s,PIPATMPY)){GOTO(37,03612);}
 AT(37,03611);ai_ca(s,DELVZ);
 AT(37,03612);if(ai_ts(s,PIPATMPZ)){GOTO(37,03614);}
 AT(37,03613);return tcq(s);
 /* REREADAC CCS PIPAGE; TCF READACCS; CAF DONEADR; TS Q; CCS DELVZ; TCF
  * REPIP4; TCF +3; TCF REPIP4; TCF REPIP4; ZL; CCS DELVY; TCF +3; TCF
  * CHKTEMX; TCF +1; LXCH PIPAZ; CCS TEMZ; CS TEMZ; TCF DODELVZ; TCF -2; LXCH
  * DELVZ; TCF REPIP4 */
 AT(37,03614);c=ai_ccs(s,PIPAGE);BR(58u+c);switch(c){case 1:GOTO(37,03616);case 2:GOTO(37,03617);case 3:GOTO(37,03620);}
 AT(37,03615);GOTO(37,03441);
 AT(37,03616);ai_caf(s,K_DONEADR);
 AT(37,03617);if(ai_ts(s,QREG)){GOTO(37,03621);}
 AT(37,03620);c=ai_ccs(s,DELVZ);BR(62u+c);switch(c){case 1:GOTO(37,03622);case 2:GOTO(37,03623);case 3:GOTO(37,03624);}
 AT(37,03621);GOTO(37,03571);
 AT(37,03622);GOTO(37,03625);
 AT(37,03623);GOTO(37,03571);
 AT(37,03624);GOTO(37,03571);
 AT(37,03625);ai_zl(s);
 AT(37,03626);c=ai_ccs(s,DELVY);BR(66u+c);switch(c){case 1:GOTO(37,03630);case 2:GOTO(37,03631);case 3:GOTO(37,03632);}
 AT(37,03627);GOTO(37,03632);
 AT(37,03630);GOTO(37,03641);
 AT(37,03631);GOTO(37,03632);
 AT(37,03632);ai_lxch(s,PIPAZ);
 AT(37,03633);c=ai_ccs(s,TEMZ);BR(70u+c);switch(c){case 1:GOTO(37,03635);case 2:GOTO(37,03636);case 3:GOTO(37,03637);}
 AT(37,03634);ai_cs(s,TEMZ);
 AT(37,03635);GOTO(37,03570);
 AT(37,03636);GOTO(37,03634);
 AT(37,03637);ai_lxch(s,DELVZ);
 AT(37,03640);GOTO(37,03571);
 /* CHKTEMX CCS TEMX; CS TEMX; TCF +3; TCF -2; TCF REPIP1; TS DELVX; CS TEMY;
  * TS DELVY; CS ZERO; DXCH PIPAX; TCF REPIP3 */
 AT(37,03641);c=ai_ccs(s,TEMX);BR(74u+c);switch(c){case 1:GOTO(37,03643);case 2:GOTO(37,03644);case 3:GOTO(37,03645);}
 AT(37,03642);ai_cs(s,TEMX);
 AT(37,03643);GOTO(37,03646);
 AT(37,03644);GOTO(37,03642);
 AT(37,03645);GOTO(37,03557);
 AT(37,03646);if(ai_ts(s,DELVX)){GOTO(37,03650);}
 AT(37,03647);ai_cs(s,TEMY);
 AT(37,03650);if(ai_ts(s,DELVY)){GOTO(37,03652);}
 AT(37,03651);ai_csf(s,K_ZERO);
 AT(37,03652);ai_dxch(s,PIPAX);
 AT(37,03653);GOTO(37,03565);
 /* TMPTOSPT CA CDUTEMPY; TS CDUSPOTY; CA CDUTEMPZ; TS CDUSPOTZ; CA CDUTEMPX;
  * TS CDUSPOTX; TC Q */
 AT(33,02440);ai_ca(s,CDUTEMPY);
 AT(33,02441);if(ai_ts(s,CDUSPOTY)){GOTO(33,02443);}
 AT(33,02442);ai_ca(s,CDUTEMPZ);
 AT(33,02443);if(ai_ts(s,CDUSPOTZ)){GOTO(33,02445);}
 AT(33,02444);ai_ca(s,CDUTEMPX);
 AT(33,02445);if(ai_ts(s,CDUSPOTX)){GOTO(33,02447);}
 AT(33,02446);return tcq(s);
 /* LRHTASK CS FLGWRD11; MASK LRBYBIT; EXTEND; BZF GRP2OFF; CA READLBIT; MASK
  * FLGWRD11; EXTEND; BZF GRP2OFF; CS FLGWRD11; MASK NOLRRBIT; EXTEND; BZF
  * GRP2OFF; CA PRIO32; TC NOVAC */
 AT(21,02130);ai_cs(s,FLGWRD11);
 AT(21,02131);ai_maskf(s,K_LRBYBIT);
 AT(21,02132);
 AT(21,02133);if(ai_bzf(s)){BR(78);GOTO(21,02150);}BR(79);
 AT(21,02134);ai_caf(s,K_READLBIT);
 AT(21,02135);ai_mask(s,FLGWRD11);
 AT(21,02136);
 AT(21,02137);if(ai_bzf(s)){BR(80);GOTO(21,02150);}BR(81);
 AT(21,02140);ai_cs(s,FLGWRD11);
 AT(21,02141);ai_maskf(s,K_NOLRRBIT);
 AT(21,02142);
 AT(21,02143);if(ai_bzf(s)){BR(82);GOTO(21,02150);}BR(83);
 AT(21,02144);ai_caf(s,K_PRIO32);
 AT(21,02145);return out(s,02146,X_NOVAC);
 /* GRP2OFF EXTEND; DCA NEG0; DXCH -PHASE2; TCF R10,R11A */
 AT(21,02150);
 AT(21,02151);ai_dcaf(s,K_NEG0);
 AT(21,02152);ai_dxch(s,MPHASE2);
 AT(21,02153);return jump(s,X_R10_R11A);
 /* HIGATASK INHINT; CS PRIO3; MASK FLGWRD11; AD PRIO3; TS FLGWRD11; CAF
  * PRIO32; TC FINDVAC */
 AT(33,02447);io->control.interrupt_enabled=0;
 AT(33,02450);ai_csf(s,K_PRIO3);
 AT(33,02451);ai_mask(s,FLGWRD11);
 AT(33,02452);ai_adf(s,K_PRIO3);
 AT(33,02453);if(ai_ts(s,FLGWRD11)){GOTO(33,02455);}
 AT(33,02454);ai_caf(s,K_PRIO32);
 AT(33,02455);return out(s,02456,X_FINDVAC);
 /* TCF CONTSERV */
 AT(33,02460);GOTO(33,02540);
#endif
#if SV_G(4)
 /* CS FLGWRD11; MASK LRBYBIT; EXTEND; BZF COPYCYC1; CA READLBIT; MASK
  * FLGWRD11; EXTEND; BZF 35KCHK; CAF XORFLBIT; MASK FLGWRD11; EXTEND; BZF
  * XORCHK */
 AT(33,02462);ai_cs(s,FLGWRD11);
 AT(33,02463);ai_maskf(s,K_LRBYBIT);
 AT(33,02464);
 AT(33,02465);if(ai_bzf(s)){BR(84);GOTO(33,02546);}BR(85);
 AT(33,02466);ai_caf(s,K_READLBIT);
 AT(33,02467);ai_mask(s,FLGWRD11);
 AT(33,02470);
 AT(33,02471);if(ai_bzf(s)){BR(86);GOTO(33,02721);}BR(87);
 AT(33,02472);ai_caf(s,K_XORFLBIT);
 AT(33,02473);ai_mask(s,FLGWRD11);
 AT(33,02474);
 AT(33,02475);if(ai_bzf(s)){BR(88);GOTO(33,02710);}BR(89);
#endif
#if !SV_G(1) && (SV_G(4))
 L33_02476: __attribute__((unused)) W[5]=02476u;return STOP;
#endif
#if SV_G(1)
 /* HITEST CAF PSTHIBIT; MASK FLGWRD11; EXTEND; BZF HIGATCHK */
 AT(33,02476);ai_caf(s,K_PSTHIBIT);
 AT(33,02477);ai_mask(s,FLGWRD11);
 AT(33,02500);
 AT(33,02501);if(ai_bzf(s)){BR(90);GOTO(33,02515);}BR(91);
 /* POS2CHK CAF BIT7; EXTEND; RAND CHAN33; EXTEND; BZF UPDATCHK; CAF BIT13;
  * EXTEND; RAND CHAN12; EXTEND; BZF LRPOSALM; TCF CONTSERV */
 AT(33,02502);ai_caf(s,K_BIT7);
 AT(33,02503);
 AT(33,02504);rand_(s,io->chan33);
 AT(33,02505);
 AT(33,02506);if(ai_bzf(s)){BR(92);GOTO(33,03155);}BR(93);
 AT(33,02507);ai_caf(s,K_BIT13);
 AT(33,02510);
 AT(33,02511);rand_(s,io->chan12);
 AT(33,02512);
 AT(33,02513);if(ai_bzf(s)){BR(94);GOTO(33,02536);}BR(95);
 AT(33,02514);GOTO(33,02540);
 /* HIGATCHK CA TTF/8; AD RPCRTIME; EXTEND; BZMF POS1CHK; CA EBANK4; XCH
  * EBANK; TS L; CS XNBPIP; LXCH EBANK; AD RPCRTQSW; EXTEND; BZMF HIGATASK */
 AT(33,02515);ai_ca(s,TTF_8);
 AT(33,02516);ai_ad(s,RPCRTIME);
 AT(33,02517);
 AT(33,02520);if(ai_bzmf(s)){BR(96);GOTO(33,02531);}BR(97);
 AT(33,02521);ai_caf(s,K_EBANK4);
 AT(33,02522);ai_xch(s,EBANK);
 AT(33,02523);if(ai_ts(s,LREG)){GOTO(33,02525);}
 AT(33,02524);ai_cs(s,XNBPIP);
 AT(33,02525);ai_lxch(s,EBANK);
 AT(33,02526);ai_ad(s,RPCRTQSW);
 AT(33,02527);
 AT(33,02530);if(ai_bzmf(s)){BR(98);GOTO(33,02447);}BR(99);
 /* POS1CHK CAF BIT6; EXTEND; RAND 33; EXTEND; BZF UPDATCHK */
 AT(33,02531);ai_caf(s,K_BIT6);
 AT(33,02532);
 AT(33,02533);rand_(s,io->chan33);
 AT(33,02534);
 AT(33,02535);if(ai_bzf(s)){BR(100);GOTO(33,03155);}BR(101);
 /* LRPOSALM TC ALARM */
 AT(33,02536);return out(s,02537,X_ALARM);
 /* CONTSERV INHINT; CS BITS4-7; MASK FLGWRD11; TS FLGWRD11; TC IBNKCALL */
 AT(33,02540);io->control.interrupt_enabled=0;
 AT(33,02541);ai_csf(s,K_BITS4M7);
 AT(33,02542);ai_mask(s,FLGWRD11);
 AT(33,02543);if(ai_ts(s,FLGWRD11)){return to(s,02545u);}
 AT(33,02544);return out(s,02545,X_IBNKCALL);
 /* COPYCYC1 TC QUIKFAZ5 */
 AT(33,02546);TC(02547,33,03552);
 /* R29? CA FLAGWRD3; MASK NR29&RDR; CCS A; TCF R29NODES; CA RADMODES; MASK
  * OCT10002; CCS A; TCF R29NODES; CA RADMODES; MASK PRIO22; CCS A; TCF
  * NOR29NOW; TCF R29 */
 AT(33,02547);ai_ca(s,FLAGWRD3);
 AT(33,02550);ai_maskf(s,K_NR29_RDR);
 AT(33,02551);c=ai_ccs(s,A);BR(102u+c);switch(c){case 1:GOTO(33,02553);case 2:GOTO(33,02554);case 3:GOTO(33,02555);}
 AT(33,02552);GOTO(33,02564);
 AT(33,02553);ai_ca(s,RADMODES);
 AT(33,02554);ai_maskf(s,K_OCT10002);
 AT(33,02555);c=ai_ccs(s,A);BR(106u+c);switch(c){case 1:GOTO(33,02557);case 2:GOTO(33,02560);case 3:GOTO(33,02561);}
 AT(33,02556);GOTO(33,02564);
 AT(33,02557);ai_ca(s,RADMODES);
 AT(33,02560);ai_maskf(s,K_PRIO22);
 AT(33,02561);c=ai_ccs(s,A);BR(110u+c);switch(c){case 1:GOTO(33,02563);case 2:GOTO(33,02564);case 3:GOTO(33,02565);}
 AT(33,02562);GOTO(33,02570);
 AT(33,02563);return jump(s,X_R29);
 /* R29NODES INHINT; CS DESIGBIT; MASK RADMODES; TS RADMODES */
 AT(33,02564);io->control.interrupt_enabled=0;
 AT(33,02565);ai_csf(s,K_DESIGBIT);
 AT(33,02566);ai_mask(s,RADMODES);
 AT(33,02567);if(ai_ts(s,RADMODES)){return to(s,02571u);}
#endif
#if !SV_G(4) && (SV_G(1))
 L33_02570: __attribute__((unused)) W[5]=02570u;return STOP;
#endif
#if SV_G(4)
 /* NOR29NOW TC INTPRET */
 AT(33,02570);intpret(s,io,02571);return GO;
#endif
#if SV_G(6)
 /* INHINT; CA UNIT/R/; TS RUNIT; CA UNIT/R/ +2; TS RUNIT +1; CA UNIT/R/ +4;
  * TS RUNIT +2; CA MPAC; TS DALTRATE; EXTEND; DCA R1S; DXCH R; EXTEND; DCA
  * R1S +2; DXCH R +2; EXTEND; DCA R1S +4; DXCH R +4; EXTEND; DCA V1S; DXCH V;
  * EXTEND; DCA V1S +2; DXCH V +2; EXTEND; DCA V1S +4; DXCH V +4; TCF COPYCYCL */
 AT(33,02630);io->control.interrupt_enabled=0;
 AT(33,02631);ai_ca(s,UNIT_R_);
 AT(33,02632);if(ai_ts(s,RUNIT)){GOTO(33,02634);}
 AT(33,02633);ai_ca(s,UNIT_R_+2u);
 AT(33,02634);if(ai_ts(s,RUNIT+1u)){GOTO(33,02636);}
 AT(33,02635);ai_ca(s,UNIT_R_+4u);
 AT(33,02636);if(ai_ts(s,RUNIT+2u)){GOTO(33,02640);}
 AT(33,02637);ai_ca(s,MPAC);
 AT(33,02640);if(ai_ts(s,DALTRATE)){return REFUSE;}
 AT(33,02641);
 AT(33,02642);ai_dca(s,R1S);
 AT(33,02643);ai_dxch(s,R);
 AT(33,02644);
 AT(33,02645);ai_dca(s,R1S+2u);
 AT(33,02646);ai_dxch(s,R+2u);
 AT(33,02647);
 AT(33,02650);ai_dca(s,R1S+4u);
 AT(33,02651);ai_dxch(s,R+4u);
 AT(33,02652);
 AT(33,02653);ai_dca(s,V1S);
 AT(33,02654);ai_dxch(s,V);
 AT(33,02655);
 AT(33,02656);ai_dca(s,V1S+2u);
 AT(33,02657);ai_dxch(s,V+2u);
 AT(33,02660);
 AT(33,02661);ai_dca(s,V1S+4u);
 AT(33,02662);ai_dxch(s,V+4u);
 AT(33,02663);GOTO(33,02275);
#endif
#if SV_G(1)
 /* ALTCHK EXTEND; INDEX A; DCA ALTCRIT; DXCH MPAC +1; EXTEND; DCS HCALC; DAS
  * MPAC +1; TC BRANCH +4; CAF ZERO; INCR BUF2; TCF SWRETURN */
 AT(33,02664);
 AT(33,02665);if(!ixr(W[A],4u,2u,&v))return REFUSE;
 AT(33,02666);ai_dcaf(s,FX33B+v);    /* ALTCRIT +A */
 AT(33,02667);ai_dxch(s,MPAC+1u);
 AT(33,02670);
 AT(33,02671);ai_dcs(s,HCALC);
 AT(33,02672);ai_das(s,MPAC+1u);
 AT(33,02673);return out(s,02674,X_BRANCHP4);
 AT(33,02674);ai_caf(s,K_ZERO);
 AT(33,02675);ai_incr(s,BUF2);
 AT(33,02676);return jump(s,X_SWRETURN);
 /* XORCHK CAF SIX; TC BANKCALL */
 AT(33,02710);ai_caf(s,K_SIX);
 AT(33,02711);return out(s,02712,X_BANKCALL);
 /* TCF HITEST; TC UPFLAG */
 AT(33,02713);GOTO(33,02476);
 AT(33,02714);return out(s,02715,X_UPFLAG);
 /* TC UPFLAG */
 AT(33,02716);return out(s,02717,X_UPFLAG);
 /* TCF HITEST */
 AT(33,02720);GOTO(33,02476);
 /* 35KCHK CAF TWO; TC BANKCALL */
 AT(33,02721);ai_caf(s,K_TWO);
 AT(33,02722);return out(s,02723,X_BANKCALL);
 /* TCF CONTSERV; TC UPFLAG */
 AT(33,02724);GOTO(33,02540);
 AT(33,02725);return out(s,02726,X_UPFLAG);
 /* TCF CONTSERV */
 AT(33,02727);GOTO(33,02540);
#endif
#if SV_G(7)
 /* TC QUIKFAZ5; TC INTPRET */
 AT(33,03052);TC(03053,33,03552);
 AT(33,03053);intpret(s,io,03054);return GO;
 /* TC QUIKFAZ5; TC INTPRET */
 AT(33,03064);TC(03065,33,03552);
 AT(33,03065);intpret(s,io,03066);return GO;
#endif
#if SV_G(1)
 /* UPDATCHK CAF NOLRRBIT; MASK FLGWRD11; CCS A; TCF CONTSERV; CAF RNGEDBIT;
  * MASK FLGWRD11; EXTEND; BZF VMEASCHK */
 AT(33,03155);ai_caf(s,K_NOLRRBIT);
 AT(33,03156);ai_mask(s,FLGWRD11);
 AT(33,03157);c=ai_ccs(s,A);BR(114u+c);switch(c){case 1:GOTO(33,03161);case 2:GOTO(33,03162);case 3:GOTO(33,03163);}
 AT(33,03160);GOTO(33,02540);
 AT(33,03161);ai_caf(s,K_RNGEDBIT);
 AT(33,03162);ai_mask(s,FLGWRD11);
 AT(33,03163);
 AT(33,03164);if(ai_bzf(s)){BR(118);GOTO(33,03310);}BR(119);
#endif
#if !SV_G(7) && (SV_G(1))
 L33_03165: __attribute__((unused)) W[5]=03165u;return STOP;
#endif
#if SV_G(7)
 /* POSUPDAT CA FIXLOC; TS PUSHLOC; TC INTPRET */
 AT(33,03165);ai_ca(s,FIXLOC);
 AT(33,03166);if(ai_ts(s,PUSHLOC)){return to(s,03170u);}
 AT(33,03167);intpret(s,io,03170);return GO;
 /* CS FLGWRD12; MASK ALTSCBIT; EXTEND; BZF +3; CA SKALSKAL; TC SHORTMP */
 AT(33,03202);ai_cs(s,FLGWRD12);
 AT(33,03203);ai_maskf(s,K_ALTSCBIT);
 AT(33,03204);
 AT(33,03205);if(ai_bzf(s)){BR(120);GOTO(33,03210);}BR(121);
 AT(33,03206);ai_ca(s,SKALSKAL);
 AT(33,03207);return out(s,03210,X_SHORTMP);
#endif
#if SV_G(6)
 /* +3 TC INTPRET */
 AT(33,03210);intpret(s,io,03211);return GO;
 /* CA FLGWRD11; MASK PSTHIBIT; EXTEND; BZF NOREASON; TC INTPRET */
 AT(33,03223);ai_ca(s,FLGWRD11);
 AT(33,03224);ai_maskf(s,K_PSTHIBIT);
 AT(33,03225);
 AT(33,03226);if(ai_bzf(s)){BR(122);GOTO(33,03243);}BR(123);
 AT(33,03227);intpret(s,io,03230);return GO;
 /* INCR LRLCTR; TC BRANCH; TCF HFAIL; TCF HFAIL; TC DOWNFLAG */
 AT(33,03235);ai_incr(s,LRLCTR);
 AT(33,03236);return out(s,03237,X_BRANCH);
#endif
#if SV_G(1)
 AT(33,03237);GOTO(33,03560);
 AT(33,03240);GOTO(33,03560);
 AT(33,03241);return out(s,03242,X_DOWNFLAG);
#endif
#if SV_G(2)
 /* NOREASON CS FLGWRD11; MASK LRINHBIT; CCS A; TCF VMEASCHK; TC INTPRET */
 AT(33,03243);ai_cs(s,FLGWRD11);
 AT(33,03244);ai_maskf(s,K_LRINHBIT);
 AT(33,03245);c=ai_ccs(s,A);BR(124u+c);switch(c){case 1:GOTO(33,03247);case 2:return to(s,03250u);case 3:return to(s,03251u);}
 AT(33,03246);GOTO(33,03310);
 AT(33,03247);intpret(s,io,03250);return GO;
 /* EXTEND; DCA DELTAH; DXCH MPAC; TC ALSIGNAG; EXTEND; BZF +2; TCF VMEASCHK;
  * TS MPAC +2; CS L; AD LRHMAX; EXTEND; BZMF VMEASCHK; EXTEND; MP LRWH;
  * EXTEND; DV LRHMAX; TS MPTEMP; TC SHORTMP2; TC INTPRET */
 AT(33,03253);
 AT(33,03254);ai_dca(s,DELTAH);
 AT(33,03255);ai_dxch(s,MPAC);
 AT(33,03256);return out(s,03257,X_ALSIGNAG);
#endif
#if SV_G(1)
 AT(33,03257);
 AT(33,03260);if(ai_bzf(s)){BR(128);GOTO(33,03262);}BR(129);
 AT(33,03261);GOTO(33,03310);
 AT(33,03262);if(ai_ts(s,MPAC+2u)){GOTO(33,03264);}
 AT(33,03263);ai_cs(s,LREG);
 AT(33,03264);ai_ad(s,LRHMAX);
 AT(33,03265);
 AT(33,03266);if(ai_bzmf(s)){BR(130);GOTO(33,03310);}BR(131);
 AT(33,03267);
 AT(33,03270);ai_mp(s,LRWH);
 AT(33,03271);
 AT(33,03272);ai_dv(s,LRHMAX);
 AT(33,03273);if(ai_ts(s,MPTEMP)){GOTO(33,03275);}
 AT(33,03274);return out(s,03275,X_SHORTMP2);
#endif
#if SV_G(7)
 AT(33,03275);intpret(s,io,03276);return GO;
 /* TC QUIKFAZ5; CA ZERO */
 AT(33,03305);TC(03306,33,03552);
 AT(33,03306);ai_caf(s,K_ZERO);
#endif
#if !SV_G(1) && (SV_G(7))
 L33_03307: __attribute__((unused)) W[5]=03307u;return STOP;
#endif
#if SV_G(1)
 /* RUPDATED TC GNURVST */
 AT(33,03307);TC(03310,33,03534);
 /* VMEASCHK TC QUIKFAZ5; CS FLGWRD11; MASK VELDABIT; CCS A; TCF VALTCHK */
 AT(33,03310);TC(03311,33,03552);
 AT(33,03311);ai_cs(s,FLGWRD11);
 AT(33,03312);ai_maskf(s,K_VELDABIT);
 AT(33,03313);c=ai_ccs(s,A);BR(132u+c);switch(c){case 1:GOTO(33,03315);case 2:GOTO(33,03316);case 3:GOTO(33,03317);}
 AT(33,03314);GOTO(33,03514);
 /* VELUPDAT CS VSELECT; TS L; ADS L; AD L; AD L; INDEX FIXLOC; DXCH X1; CA
  * EBANK4; TS EBANK; CA LRYCDU; TS CDUSPOT; CA LRZCDU; TS CDUSPOT +2; CA
  * LRXCDU; TS CDUSPOT +4; TC BANKCALL */
 AT(33,03315);ai_cs(s,VSELECT);
 AT(33,03316);if(ai_ts(s,LREG)){GOTO(33,03320);}
 AT(33,03317);ai_ads(s,LREG);
 AT(33,03320);ai_ad(s,LREG);
 AT(33,03321);ai_ad(s,LREG);
 AT(33,03322);if(!vac(W[FIXLOC]))return REFUSE;v=W[FIXLOC];
 AT(33,03323);ai_dxch(s,X1+v);
 AT(33,03324);ai_caf(s,K_EBANK4);
 AT(33,03325);if(ai_ts(s,EBANK)){GOTO(33,03327);}
 AT(33,03326);ai_ca(s,LRYCDU);
 AT(33,03327);if(ai_ts(s,CDUSPOT)){GOTO(33,03331);}
 AT(33,03330);ai_ca(s,LRZCDU);
 AT(33,03331);if(ai_ts(s,CDUSPOT+2u)){GOTO(33,03333);}
 AT(33,03332);ai_ca(s,LRXCDU);
 AT(33,03333);if(ai_ts(s,CDUSPOT+4u)){return to(s,03335u);}
 AT(33,03334);return out(s,03335,X_BANKCALL);
#endif
#if SV_G(2)
 /* CA FIXLOC; TS PUSHLOC; TC INTPRET */
 AT(33,03336);ai_ca(s,FIXLOC);
 AT(33,03337);if(ai_ts(s,PUSHLOC)){return to(s,03341u);}
 AT(33,03340);intpret(s,io,03341);return GO;
#endif
#if SV_G(3)
 /* CS ONE; TS MODE; CA PIPTEM; ZL; DXCH MPAC; CA PIPTEM +1; ZL; DXCH MPAC +3;
  * CA PIPTEM +2; ZL; DXCH MPAC +5; CA EBANK7; TS EBANK; TC INTPRET */
 AT(33,03352);ai_csf(s,K_ONE);
 AT(33,03353);if(ai_ts(s,MODE)){GOTO(33,03355);}
 AT(33,03354);ai_ca(s,PIPTEM);
 AT(33,03355);ai_zl(s);
 AT(33,03356);ai_dxch(s,MPAC);
 AT(33,03357);ai_ca(s,PIPTEM+1u);
 AT(33,03360);ai_zl(s);
 AT(33,03361);ai_dxch(s,MPAC+3u);
 AT(33,03362);ai_ca(s,PIPTEM+2u);
 AT(33,03363);ai_zl(s);
 AT(33,03364);ai_dxch(s,MPAC+5u);
 AT(33,03365);ai_caf(s,K_EBANK7);
 AT(33,03366);if(ai_ts(s,EBANK)){return to(s,03370u);}
 AT(33,03367);intpret(s,io,03370);return GO;
 /* INCR LRMCTR; TC BRANCH; TCF VFAIL; TCF VFAIL; TC DOWNFLAG */
 AT(33,03415);ai_incr(s,LRMCTR);
 AT(33,03416);return out(s,03417,X_BRANCH);
#endif
#if SV_G(1)
 AT(33,03417);GOTO(33,03575);
 AT(33,03420);GOTO(33,03575);
 AT(33,03421);return out(s,03422,X_DOWNFLAG);
 /* CA FLGWRD11; MASK VXINHBIT; EXTEND; BZF VUPDAT; TC DOWNFLAG */
 AT(33,03423);ai_ca(s,FLGWRD11);
 AT(33,03424);ai_maskf(s,K_VXINHBIT);
 AT(33,03425);
 AT(33,03426);if(ai_bzf(s)){BR(136);GOTO(33,03435);}BR(137);
 AT(33,03427);return out(s,03430,X_DOWNFLAG);
 /* CA VSELECT; AD NEG2; EXTEND; BZF ENDVDAT */
 AT(33,03431);ai_ca(s,VSELECT);
 AT(33,03432);ai_adf(s,K_NEG2);
 AT(33,03433);
 AT(33,03434);if(ai_bzf(s)){BR(138);GOTO(33,03514);}BR(139);
 /* VUPDAT CS FLGWRD11; MASK LRINHBIT; CCS A; TCF VALTCHK; TS MPAC +1; CA
  * ABVEL; TS ABVEL*; CA VSELECT; TS VSELECT*; CA EBANK5; TS EBANK; CS LRVF;
  * AD ABVEL*; EXTEND; BZMF USEVF; CS ABVEL*; AD LRVMAX; EXTEND; BZMF WSTOR
  * -1; EXTEND; INDEX VSELECT*; MP LRWVZ; EXTEND; DV LRVMAX; TCF WSTOR */
 AT(33,03435);ai_cs(s,FLGWRD11);
 AT(33,03436);ai_maskf(s,K_LRINHBIT);
 AT(33,03437);c=ai_ccs(s,A);BR(140u+c);switch(c){case 1:GOTO(33,03441);case 2:GOTO(33,03442);case 3:GOTO(33,03443);}
 AT(33,03440);GOTO(33,03514);
 AT(33,03441);if(ai_ts(s,MPAC+1u)){GOTO(33,03443);}
 AT(33,03442);ai_ca(s,ABVEL);
 AT(33,03443);if(ai_ts(s,ABVELS)){GOTO(33,03445);}
 AT(33,03444);ai_ca(s,VSELECT);
 AT(33,03445);if(ai_ts(s,VSELECTS)){GOTO(33,03447);}
 AT(33,03446);ai_caf(s,K_EBANK5);
 AT(33,03447);if(ai_ts(s,EBANK)){GOTO(33,03451);}
 AT(33,03450);ai_cs(s,LRVF);
 AT(33,03451);ai_ad(s,ABVELS);
 AT(33,03452);
 AT(33,03453);if(ai_bzmf(s)){BR(144);GOTO(33,03466);}BR(145);
 AT(33,03454);ai_cs(s,ABVELS);
 AT(33,03455);ai_ad(s,LRVMAX);
 AT(33,03456);
 AT(33,03457);if(ai_bzmf(s)){BR(146);GOTO(33,03471);}BR(147);
 AT(33,03460);
 AT(33,03461);IX(VSELECTS,3u,1u);
 AT(33,03462);ai_mp(s,LRWVZ+v);
 AT(33,03463);
 AT(33,03464);ai_dv(s,LRVMAX);
 AT(33,03465);GOTO(33,03472);
 /* USEVF INDEX VSELECT*; CA LRWVFZ; TCF WSTOR */
 AT(33,03466);IX(VSELECTS,3u,1u);
 AT(33,03467);ai_ca(s,LRWVFZ+v);
 AT(33,03470);GOTO(33,03472);
 /* -1 CA ZERO */
 AT(33,03471);ai_caf(s,K_ZERO);
#endif
#if !SV_G(7) && (SV_G(1))
 L33_03472: __attribute__((unused)) W[5]=03472u;return STOP;
#endif
#if SV_G(7)
 /* WSTOR TS MPAC; CS BIT7; AD MODREG; EXTEND; BZMF +3; CA LRWVFF; TS MPAC */
 AT(33,03472);if(ai_ts(s,MPAC)){GOTO(33,03474);}
 AT(33,03473);ai_csf(s,K_BIT7);
 AT(33,03474);ai_ad(s,MODREG);
 AT(33,03475);
 AT(33,03476);if(ai_bzmf(s)){BR(148);GOTO(33,03501);}BR(149);
 AT(33,03477);ai_ca(s,LRWVFF);
 AT(33,03500);if(ai_ts(s,MPAC)){GOTO(33,03502);}
 /* +3 CA EBANK7; TS EBANK; TC INTPRET */
 AT(33,03501);ai_caf(s,K_EBANK7);
 AT(33,03502);if(ai_ts(s,EBANK)){return to(s,03504u);}
 AT(33,03503);intpret(s,io,03504);return GO;
 /* TC QUIKFAZ5; CA SIX */
 AT(33,03511);TC(03512,33,03552);
 AT(33,03512);ai_caf(s,K_SIX);
#endif
#if !SV_G(1) && (SV_G(7))
 L33_03513: __attribute__((unused)) W[5]=03513u;return STOP;
#endif
#if SV_G(1)
 /* VUPDATED TC GNURVST */
 AT(33,03513);TC(03514,33,03534);
 /* VALTCHK TC QUIKFAZ5; CAF READVBIT; MASK FLGWRD11; CCS A; TCF READV; CS
  * ABVEL; AD 2KFT/SEC; EXTEND; BZMF CONTSERV; TC UPFLAG */
 AT(33,03514);TC(03515,33,03552);
 AT(33,03515);ai_caf(s,K_READVBIT);
 AT(33,03516);ai_mask(s,FLGWRD11);
 AT(33,03517);c=ai_ccs(s,A);BR(150u+c);switch(c){case 1:GOTO(33,03521);case 2:GOTO(33,03522);case 3:GOTO(33,03523);}
 AT(33,03520);GOTO(33,03527);
 AT(33,03521);ai_cs(s,ABVEL);
 AT(33,03522);ai_adf(s,K_N2KFT_SEC);
 AT(33,03523);
 AT(33,03524);if(ai_bzmf(s)){BR(154);GOTO(33,02540);}BR(155);
 AT(33,03525);return out(s,03526,X_UPFLAG);
 /* READV CAF PRIO32; TC NOVAC */
 AT(33,03527);ai_caf(s,K_PRIO32);
 AT(33,03530);return out(s,03531,X_NOVAC);
 /* TCF CONTSERV */
 AT(33,03533);GOTO(33,02540);
 /* GNURVST TS BUF; EXTEND; DCA GNUR; INDEX BUF; DXCH R1S; EXTEND; DCA GNUR
  * +2; INDEX BUF; DXCH R1S +2; EXTEND; DCA GNUR +4; INDEX BUF; DXCH R1S +4;
  * TC Q */
 AT(33,03534);if(ai_ts(s,BUF)){return REFUSE;}
 AT(33,03535);
 AT(33,03536);ai_dca(s,GNUR);
 AT(33,03537);IX(BUF,2u,6u);
 AT(33,03540);ai_dxch(s,R1S+v);
 AT(33,03541);
 AT(33,03542);ai_dca(s,GNUR+2u);
 AT(33,03543);IX(BUF,2u,6u);
 AT(33,03544);ai_dxch(s,R1S+2u+v);
 AT(33,03545);
 AT(33,03546);ai_dca(s,GNUR+4u);
 AT(33,03547);IX(BUF,2u,6u);
 AT(33,03550);ai_dxch(s,R1S+4u+v);
 AT(33,03551);return tcq(s);
 /* QUIKFAZ5 CA EBANK3; XCH EBANK; DXCH L; TS PHSNAME5; LXCH EBANK; TC A */
 AT(33,03552);ai_caf(s,K_EBANK3);
 AT(33,03553);ai_xch(s,EBANK);
 AT(33,03554);ai_dxch(s,LREG);
 AT(33,03555);if(ai_ts(s,PHSNAME5)){GOTO(33,03557);}
 AT(33,03556);ai_lxch(s,EBANK);
 AT(33,03557);return tca(s);
 /* HFAIL CS LRRCTR; EXTEND; BZF NORLITE; AD LRLCTR; MASK NEG3; EXTEND; BZF
  * +2; TCF NORLITE; TC UPFLAG */
 AT(33,03560);ai_cs(s,LRRCTR);
 AT(33,03561);
 AT(33,03562);if(ai_bzf(s)){BR(156);GOTO(33,03572);}BR(157);
 AT(33,03563);ai_ad(s,LRLCTR);
 AT(33,03564);ai_maskf(s,K_NEG3);
 AT(33,03565);
 AT(33,03566);if(ai_bzf(s)){BR(158);GOTO(33,03570);}BR(159);
 AT(33,03567);GOTO(33,03572);
 AT(33,03570);return out(s,03571,X_UPFLAG);
 /* NORLITE CA LRLCTR; TS LRRCTR; TCF VMEASCHK */
 AT(33,03572);ai_ca(s,LRLCTR);
 AT(33,03573);if(ai_ts(s,LRRCTR)){GOTO(33,03575);}
 AT(33,03574);GOTO(33,03310);
 /* VFAIL CS LRSCTR; EXTEND; BZF NOLITE; AD LRMCTR; MASK NEG3; EXTEND; BZF +2;
  * TCF NOLITE; TC UPFLAG */
 AT(33,03575);ai_cs(s,LRSCTR);
 AT(33,03576);
 AT(33,03577);if(ai_bzf(s)){BR(160);GOTO(33,03607);}BR(161);
 AT(33,03600);ai_ad(s,LRMCTR);
 AT(33,03601);ai_maskf(s,K_NEG3);
 AT(33,03602);
 AT(33,03603);if(ai_bzf(s)){BR(162);GOTO(33,03605);}BR(163);
 AT(33,03604);GOTO(33,03607);
 AT(33,03605);return out(s,03606,X_UPFLAG);
 /* NOLITE CA LRMCTR; TS LRSCTR; CCS VSELECT; TCF ENDVDAT; TC UPFLAG */
 AT(33,03607);ai_ca(s,LRMCTR);
 AT(33,03610);if(ai_ts(s,LRSCTR)){GOTO(33,03612);}
 AT(33,03611);c=ai_ccs(s,VSELECT);BR(164u+c);switch(c){case 1:GOTO(33,03613);case 2:return to(s,03614u);case 3:GOTO(33,03615);}
 AT(33,03612);GOTO(33,03514);
 AT(33,03613);return out(s,03614,X_UPFLAG);
 /* TCF ENDVDAT */
 AT(33,03615);GOTO(33,03514);
 /* LRVJOB CA 170MS; TC WAITLIST */
 AT(33,03616);ai_caf(s,K_N170MS);
 AT(33,03617);return out(s,03620,X_WAITLIST);
 /* CCS VSELECT; TCF +2; CAF TWO; DOUBLE; TC BANKCALL */
 AT(33,03622);c=ai_ccs(s,VSELECT);BR(168u+c);switch(c){case 1:GOTO(33,03624);case 2:GOTO(33,03625);case 3:GOTO(33,03626);}
 AT(33,03623);GOTO(33,03625);
 AT(33,03624);ai_caf(s,K_TWO);
 AT(33,03625);ai_double(s);
 AT(33,03626);return out(s,03627,X_BANKCALL);
 /* TC BANKCALL */
 AT(33,03630);return out(s,03631,X_BANKCALL);
 /* TCF VBAD; CCS STILBADV; TCF VSTILBAD; INHINT; EXTEND; DCA SAMPLSUM; DXCH
  * VMEAS; CA EBANK4; TS EBANK; EXTEND; DCA LRVTIME; DXCH LRVTIMDL; EXTEND;
  * DCA LRXCDU; DXCH LRXCDUDL; CA LRZCDU; TS LRZCDUDL; CA EBANK7; TS EBANK; CS
  * FLGWRD11; MASK VELDABIT; ADS FLGWRD11 */
 AT(33,03632);GOTO(33,03665);
 AT(33,03633);c=ai_ccs(s,STILBADV);BR(172u+c);switch(c){case 1:GOTO(33,03635);case 2:GOTO(33,03636);case 3:return REFUSE;}
 AT(33,03634);GOTO(33,03666);
 AT(33,03635);io->control.interrupt_enabled=0;
 AT(33,03636);
 AT(33,03637);ai_dca(s,SAMPLSUM);
 AT(33,03640);ai_dxch(s,VMEAS);
 AT(33,03641);ai_caf(s,K_EBANK4);
 AT(33,03642);if(ai_ts(s,EBANK)){return REFUSE;}
 AT(33,03643);
 AT(33,03644);ai_dca(s,LRVTIME);
 AT(33,03645);ai_dxch(s,LRVTIMDL);
 AT(33,03646);
 AT(33,03647);ai_dca(s,LRXCDU);
 AT(33,03650);ai_dxch(s,LRXCDUDL);
 AT(33,03651);ai_ca(s,LRZCDU);
 AT(33,03652);if(ai_ts(s,LRZCDUDL)){GOTO(33,03654);}
 AT(33,03653);ai_caf(s,K_EBANK7);
 AT(33,03654);if(ai_ts(s,EBANK)){GOTO(33,03656);}
 AT(33,03655);ai_cs(s,FLGWRD11);
 AT(33,03656);ai_maskf(s,K_VELDABIT);
 AT(33,03657);ai_ads(s,FLGWRD11);
 /* ENDLRV CCS VSELECT; TCF +2; CA TWO; TS VSELECT; TCF ENDOFJOB */
 AT(33,03660);c=ai_ccs(s,VSELECT);BR(176u+c);switch(c){case 1:GOTO(33,03662);case 2:GOTO(33,03663);case 3:GOTO(33,03664);}
 AT(33,03661);GOTO(33,03663);
 AT(33,03662);ai_caf(s,K_TWO);
 AT(33,03663);if(ai_ts(s,VSELECT)){GOTO(33,03665);}
 AT(33,03664);return jump(s,X_ENDOFJOB);
 /* VBAD CAF TWO */
 AT(33,03665);ai_caf(s,K_TWO);
 /* VSTILBAD TS STILBADV; TCF ENDLRV */
 AT(33,03666);if(ai_ts(s,STILBADV)){GOTO(33,03670);}
 AT(33,03667);GOTO(33,03660);
 /* LRHJOB TC BANKCALL */
 AT(34,03716);return out(s,03717,X_BANKCALL);
 /* TC BANKCALL */
 AT(34,03720);return out(s,03721,X_BANKCALL);
 /* TCF HBAD; CCS STILBADH; TCF HSTILBAD; INHINT; EXTEND; DCA SAMPLSUM; DXCH
  * HMEAS; EXTEND; DCA PIPTIME1; DXCH MKTIME; EXTEND; DCA CDUTEMPY; DXCH AIG;
  * CA CDUTEMPX; TS AOG; CS FLGWRD11; MASK RNGEDBIT; ADS FLGWRD11 */
 AT(34,03722);GOTO(34,03745);
 AT(34,03723);c=ai_ccs(s,STILBADH);BR(180u+c);switch(c){case 1:GOTO(34,03725);case 2:GOTO(34,03726);case 3:return REFUSE;}
 AT(34,03724);GOTO(34,03755);
 AT(34,03725);io->control.interrupt_enabled=0;
 AT(34,03726);
 AT(34,03727);ai_dca(s,SAMPLSUM);
 AT(34,03730);ai_dxch(s,HMEAS);
 AT(34,03731);
 AT(34,03732);ai_dca(s,PIPTIME1);
 AT(34,03733);ai_dxch(s,MKTIME);
 AT(34,03734);
 AT(34,03735);ai_dca(s,CDUTEMPY);
 AT(34,03736);ai_dxch(s,AIG);
 AT(34,03737);ai_ca(s,CDUTEMPX);
 AT(34,03740);if(ai_ts(s,AOG)){GOTO(34,03742);}
 AT(34,03741);ai_cs(s,FLGWRD11);
 AT(34,03742);ai_maskf(s,K_RNGEDBIT);
 AT(34,03743);ai_ads(s,FLGWRD11);
 /* ENDLRH TC ENDOFJOB */
 AT(34,03744);return out(s,03745,X_ENDOFJOB);
 /* HBAD CA FLAGWRD5; MASK RNGSCBIT; EXTEND; BZF HSTILBAD -1; TC DOWNFLAG */
 AT(34,03745);ai_ca(s,FLAGWRD5);
 AT(34,03746);ai_maskf(s,K_RNGSCBIT);
 AT(34,03747);
 AT(34,03750);if(ai_bzf(s)){BR(184);GOTO(34,03754);}BR(185);
 AT(34,03751);return out(s,03752,X_DOWNFLAG);
 /* TC ENDOFJOB; CAF TWO */
 AT(34,03753);return out(s,03754,X_ENDOFJOB);
 AT(34,03754);ai_caf(s,K_TWO);
 /* HSTILBAD TS STILBADH; TC ENDOFJOB */
 AT(34,03755);if(ai_ts(s,STILBADH)){GOTO(34,03757);}
 AT(34,03756);return out(s,03757,X_ENDOFJOB);
 /* RDGIMS EXTEND; DCA TIME2; DXCH LRVTIME; EXTEND; DCA CDUX; DXCH LRXCDU; CA
  * CDUZ; TS LRZCDU; CA PIPAX; TS PIPTEM; EXTEND; DCA PIPAY; DXCH PIPTEM +1;
  * TC TASKOVER */
 AT(34,03757);
 AT(34,03760);ai_dca(s,TIME2);
 AT(34,03761);ai_dxch(s,LRVTIME);
 AT(34,03762);
 AT(34,03763);ai_dca(s,CDUX);
 AT(34,03764);ai_dxch(s,LRXCDU);
 AT(34,03765);ai_ca(s,CDUZ);
 AT(34,03766);if(ai_ts(s,LRZCDU)){GOTO(34,03770);}
 AT(34,03767);ai_ca(s,PIPAX);
 AT(34,03770);if(ai_ts(s,PIPTEM)){return REFUSE;}
 AT(34,03771);
 AT(34,03772);ai_dca(s,PIPAY);
 AT(34,03773);ai_dxch(s,PIPTEM+1u);
 AT(34,03774);return out(s,03775,X_TASKOVER);
 /* HIGATJOB TC BANKCALL */
 AT(33,03670);return out(s,03671,X_BANKCALL);
 /* TC BANKCALL */
 AT(33,03672);return out(s,03673,X_BANKCALL);
 /* TCF POSALARM */
 AT(33,03674);GOTO(33,03703);
 /* POSGOOD CA PRIO23; TC PRIOCHNG; TC SETPOS2; TC DOWNFLAG */
 AT(33,03675);ai_caf(s,K_PRIO23);
 AT(33,03676);return out(s,03677,X_PRIOCHNG);
 AT(33,03677);TC(03700,33,03737);
 AT(33,03700);return out(s,03701,X_DOWNFLAG);
 /* TC ENDOFJOB */
 AT(33,03702);return out(s,03703,X_ENDOFJOB);
 /* POSALARM CA OCT523; TC BANKCALL */
 AT(33,03703);ai_caf(s,K_OCT523);
 AT(33,03704);return out(s,03705,X_BANKCALL);
 /* TCF GOTOPOOH; TCF +3; TCF ENDOFJOB; TC ENDOFJOB */
 AT(33,03706);return jump(s,X_GOTOPOOH);
 AT(33,03707);GOTO(33,03712);
 AT(33,03710);return jump(s,X_ENDOFJOB);
 AT(33,03711);return out(s,03712,X_ENDOFJOB);
 /* +3 CA BIT7; EXTEND; RAND CHAN33; EXTEND; BZF POSGOOD; TCF POSALARM */
 AT(33,03712);ai_caf(s,K_BIT7);
 AT(33,03713);
 AT(33,03714);rand_(s,io->chan33);
 AT(33,03715);
 AT(33,03716);if(ai_bzf(s)){BR(186);GOTO(33,03675);}BR(187);
 AT(33,03717);GOTO(33,03703);
 /* SETPOS1 TC MAKECADR; TS LRADRET1; CAF TWO; TS STILBADH; TS STILBADV; CA
  * ZERO; TS LRLCTR; TS LRMCTR; TS LRRCTR; TS LRSCTR; TS VSELECT; TC SETPOS;
  * CA LRADRET1; TC BANKJUMP */
 AT(33,03721);return out(s,03722,X_MAKECADR);
 AT(33,03722);if(ai_ts(s,LRADRET1)){GOTO(33,03724);}
 AT(33,03723);ai_caf(s,K_TWO);
 AT(33,03724);if(ai_ts(s,STILBADH)){GOTO(33,03726);}
 AT(33,03725);if(ai_ts(s,STILBADV)){GOTO(33,03727);}
 AT(33,03726);ai_caf(s,K_ZERO);
 AT(33,03727);if(ai_ts(s,LRLCTR)){GOTO(33,03731);}
 AT(33,03730);if(ai_ts(s,LRMCTR)){GOTO(33,03732);}
 AT(33,03731);if(ai_ts(s,LRRCTR)){GOTO(33,03733);}
 AT(33,03732);if(ai_ts(s,LRSCTR)){GOTO(33,03734);}
 AT(33,03733);if(ai_ts(s,VSELECT)){GOTO(33,03735);}
 AT(33,03734);TC(03735,33,03740);
 AT(33,03735);ai_ca(s,LRADRET1);
 AT(33,03736);return out(s,03737,X_BANKJUMP);
 /* SETPOS2 CA TWO */
 AT(33,03737);ai_caf(s,K_TWO);
#endif
#if !SV_G(7) && (SV_G(1))
 L33_03740: __attribute__((unused)) W[5]=03740u;return STOP;
#endif
#if SV_G(7)
 /* SETPOS XCH Q; TS LRADRET; CA EBANK5; TS EBANK; EXTEND; INDEX Q; DCA
  * LRALPHA; TS CDUSPOT +4; LXCH CDUSPOT; CA ZERO; TS CDUSPOT +2; CA EBANK7;
  * TS EBANK; TC INTPRET */
 AT(33,03740);ai_xch(s,QREG);
 AT(33,03741);if(ai_ts(s,LRADRET)){GOTO(33,03743);}
 AT(33,03742);ai_caf(s,K_EBANK5);
 AT(33,03743);if(ai_ts(s,EBANK)){return REFUSE;}
 AT(33,03744);
 AT(33,03745);if(!ixr(W[QREG],2u,2u,&v))return REFUSE;
 AT(33,03746);ai_dca(s,LRALPHA+v);
 AT(33,03747);if(ai_ts(s,CDUSPOT+4u)){GOTO(33,03751);}
 AT(33,03750);ai_lxch(s,CDUSPOT);
 AT(33,03751);ai_caf(s,K_ZERO);
 AT(33,03752);if(ai_ts(s,CDUSPOT+2u)){GOTO(33,03754);}
 AT(33,03753);ai_caf(s,K_EBANK7);
 AT(33,03754);if(ai_ts(s,EBANK)){return to(s,03756u);}
 AT(33,03755);intpret(s,io,03756);return GO;
#endif
#if SV_G(6)
 /* TC LRADRET */
 AT(33,03776);return tclradret(s);
#endif
 /* Segment ends (C64 groups): a jump from this group's code into another
  * group's (or a fall-through, above) stops there, before that instruction. */
#if !SV_G(1) && (SV_G(6))
 L33_02275: __attribute__((unused)) W[5]=02275u;return STOP;
#endif
#if !SV_G(1) && (SV_G(4))
 L33_02546: __attribute__((unused)) W[5]=02546u;return STOP;
#endif
#if !SV_G(1) && (SV_G(4))
 L33_02710: __attribute__((unused)) W[5]=02710u;return STOP;
#endif
#if !SV_G(1) && (SV_G(4))
 L33_02721: __attribute__((unused)) W[5]=02721u;return STOP;
#endif
#if !SV_G(6) && (SV_G(7))
 L33_03210: __attribute__((unused)) W[5]=03210u;return STOP;
#endif
#if !SV_G(2) && (SV_G(6))
 L33_03243: __attribute__((unused)) W[5]=03243u;return STOP;
#endif
#if !SV_G(7) && (SV_G(1))
 L33_03275: __attribute__((unused)) W[5]=03275u;return STOP;
#endif
#if !SV_G(1) && (SV_G(2))
 L33_03310: __attribute__((unused)) W[5]=03310u;return STOP;
#endif
#if !SV_G(1) && (SV_G(7))
 L33_03552: __attribute__((unused)) W[5]=03552u;return STOP;
#endif
 default:return SV_END(ph,0);
 }
}
/* The file's words (code and data). A data word reached as code is
 * refused (basic's default). */
static __attribute__((unused)) int in_code(uint16_t ph){
 return (ph>=PH(21,02130)&&ph<=PH(21,02153))||
        (ph>=PH(22,03661)&&ph<=PH(22,03710))||
        (ph>=PH(23,02461)&&ph<=PH(23,02510))||
        (ph>=PH(27,03321)&&ph<=PH(27,03357))||
        (ph>=PH(32,03770)&&ph<=PH(32,03772))||
        (ph>=PH(33,02206)&&ph<=PH(33,03776))||
        (ph>=PH(34,03716)&&ph<=PH(34,03774))||
        (ph>=PH(37,03410)&&ph<=PH(37,03654));
}
/* The words that run only after the EXTEND or INDEX before them (an
 * extracode, an indexed operand): a computed transfer that lands on one
 * would run a different instruction (refused). */
static const uint16_t dependent[]={
 PH(21,02133),PH(21,02137),PH(21,02143),PH(21,02151),PH(22,03671),PH(23,02505),PH(27,03322),PH(27,03332),PH(27,03341),PH(27,03344),PH(27,03347),PH(33,02226),PH(33,02231),PH(33,02236),PH(33,02243),PH(33,02246),PH(33,02250),PH(33,02252),PH(33,02317),PH(33,02333),PH(33,02335),PH(33,02412),PH(33,02430),PH(33,02431),PH(33,02433),PH(33,02465),PH(33,02471),PH(33,02475),PH(33,02501),PH(33,02504),PH(33,02506),PH(33,02511),PH(33,02513),PH(33,02520),PH(33,02530),PH(33,02533),PH(33,02535),PH(33,02642),PH(33,02645),PH(33,02650),PH(33,02653),PH(33,02656),PH(33,02661),PH(33,02665),PH(33,02666),PH(33,02671),PH(33,03164),PH(33,03205),PH(33,03226),PH(33,03254),PH(33,03260),PH(33,03266),PH(33,03270),PH(33,03272),PH(33,03323),PH(33,03426),PH(33,03434),PH(33,03453),PH(33,03457),PH(33,03461),PH(33,03462),PH(33,03464),PH(33,03467),PH(33,03476),PH(33,03524),PH(33,03536),PH(33,03540),PH(33,03542),PH(33,03544),PH(33,03546),PH(33,03550),PH(33,03562),PH(33,03566),PH(33,03577),PH(33,03603),PH(33,03637),PH(33,03644),PH(33,03647),PH(33,03714),PH(33,03716),PH(33,03745),PH(33,03746),PH(34,03727),PH(34,03732),PH(34,03735),PH(34,03750),PH(34,03760),PH(34,03763),PH(34,03772),PH(37,03463),PH(37,03467),PH(37,03473),PH(37,03520),PH(37,03542),PH(37,03560),PH(37,03572),PH(37,03575)};
static __attribute__((unused)) int dependent_word(uint16_t ph){unsigned i;
 for(i=0;i<sizeof dependent/sizeof dependent[0];++i)if(dependent[i]==ph)return 1;
 return 0;
}
/* Is Z (or LOC at NEWOPS) the file's code? *ph is its physical address. */
static __attribute__((unused)) int here(const apollo_overlay_state *s,const lm_sv_io *io,int *interp,uint16_t *ph){
 agc_word z=W[5];
 *interp=0;
 if(z==NEWOPS){*interp=1;*ph=agc_fixed_physical(W[LOC],W[BANKSET],io->control.superbank);}
 else *ph=agc_fixed_physical(z,W[FBANK],io->control.superbank);
 return *ph!=AGC_TRANSFER_NONE&&in_code(*ph);
}
/* FIXLOC one of the five VAC areas, PUSHLOC inside it, no order code left
 * over in EDOP. */
static __attribute__((unused)) int work_area(const apollo_overlay_state *s){
 agc_word f=W[FIXLOC],p=W[PUSHLOC];
 if(W[EDOP])return 0;
 if(f!=0401u&&f!=0455u&&f!=0531u&&f!=0605u&&f!=0661u)return 0;
 return p>=f&&p<=f+042u;
}
/* A computed transfer (TC Q, TC A, TC LRADRET, DTCB) out of the file onto
 * the interpreter's own code (fixed-fixed 06037-07707 but NEWOPS, SWCALL
 * 04621-04630, the interpreter's parts of banks 0 and 1) or an erasable
 * instruction: the original runs on there, the routines do not (refused). */
static __attribute__((unused)) int inner(const apollo_overlay_state *s,const lm_sv_io *io){
 agc_word z=W[5];uint16_t ph;
 if(z==NEWOPS)return 0;
 if(z<02000u||(z>=04621u&&z<=04630u)||(z>=06037u&&z<=07707u))return 1;
 if(z>=04000u)return 0;
 ph=agc_fixed_physical(z,W[FBANK],io->control.superbank);
 return ph<=03734u-02000u||(ph>=1024u+0344u&&ph<=1024u+0600u);
}
static enum exec_status steps(apollo_overlay_state *s,lm_sv_io *io){
 unsigned n;int st,interp;uint16_t ph;
 for(n=0;n<RUN_LIMIT;++n){
  if(!here(s,io,&interp,&ph))return n&&!inner(s,io)?EXEC_OK:EXEC_OUT_OF_SCOPE;
  /* NEWOPS reads the order word under FB, which DANZIG set from BANKSET. */
  if(interp&&(((W[FBANK]^W[BANKSET])&076000u)||!work_area(s)))return EXEC_OUT_OF_SCOPE;
  if(!interp&&dependent_word(ph))return EXEC_OUT_OF_SCOPE;
  st=interp?interpretive(s,io,ph):basic(s,io,ph);
  if(st==REFUSE)return EXEC_OUT_OF_SCOPE;
  if(st==STOP)return EXEC_OK;
 }
 return EXEC_OUT_OF_SCOPE;
}
static enum exec_status run(apollo_overlay_state *s,void *v){
 lm_sv_io *io=v,entry;enum exec_status st;
 if(!s||!io||!io->undo||io->undo==s)return EXEC_OUT_OF_SCOPE;
 *io->undo=*s;entry=*io;
 st=steps(s,io);
 if(st!=EXEC_OK){*s=*entry.undo;*io=entry;}
 return st;
}
static enum exec_status routine(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;return run(s,io);}
#define K21(a) PH(21,a)
#define K22(a) PH(22,a)
#define K23(a) PH(23,a)
#define K27(a) PH(27,a)
#define K32(a) PH(32,a)
#define K33(a) PH(33,a)
#define K34(a) PH(34,a)
#define K37(a) PH(37,a)
#define R21(a) {K21(a),routine}
#define R22(a) {K22(a),routine}
#define R23(a) {K23(a),routine}
#define R27(a) {K27(a),routine}
#define R32(a) {K32(a),routine}
#define R33(a) {K33(a),routine}
#define R34(a) {K34(a),routine}
#define R37(a) {K37(a),routine}
static const agc_routine_entry basic_entries[]={
#if SV_G(1)
 R21(02130),R21(02150),R22(03661),R22(03665),R22(03667),
#endif
#if SV_G(5)
 R22(03676),
#endif
#if SV_G(1)
 R22(03707),
#endif
#if SV_G(5)
 R23(02461),
#endif
#if SV_G(1)
 R27(03321),R32(03770),R32(03772),R33(02206),R33(02213),
#endif
#if SV_G(6)
 R33(02217),
#endif
#if SV_G(1)
 R33(02233),R33(02254),R33(02256),R33(02261),R33(02263),R33(02266),R33(02274),R33(02275),
 R33(02276),R33(02314),R33(02323),R33(02336),R33(02342),R33(02346),R33(02352),R33(02353),
 R33(02361),R33(02365),R33(02366),R33(02370),R33(02375),R33(02400),R33(02403),R33(02411),
 R33(02420),R33(02422),R33(02423),R33(02440),R33(02447),R33(02460),R33(02476),R33(02502),
 R33(02515),R33(02531),R33(02536),R33(02540),R33(02546),R33(02547),R33(02564),
#endif
#if SV_G(4)
 R33(02570),
#endif
#if SV_G(1)
 R33(02664),R33(02674),R33(02675),R33(02676),R33(02710),R33(02713),R33(02714),R33(02716),
 R33(02720),R33(02721),R33(02724),R33(02725),R33(02727),
#endif
#if SV_G(7)
 R33(03053),R33(03065),
#endif
#if SV_G(1)
 R33(03155),
#endif
#if SV_G(7)
 R33(03165),
#endif
#if SV_G(6)
 R33(03210),
#endif
#if SV_G(1)
 R33(03237),R33(03240),R33(03241),
#endif
#if SV_G(2)
 R33(03243),
#endif
#if SV_G(1)
 R33(03257),
#endif
#if SV_G(7)
 R33(03275),R33(03306),
#endif
#if SV_G(1)
 R33(03307),R33(03310),R33(03311),R33(03315),
#endif
#if SV_G(2)
 R33(03336),
#endif
#if SV_G(1)
 R33(03417),R33(03420),R33(03421),R33(03423),R33(03431),R33(03435),R33(03466),
#endif
#if SV_G(7)
 R33(03472),R33(03512),
#endif
#if SV_G(1)
 R33(03513),R33(03514),R33(03515),R33(03527),R33(03533),R33(03534),R33(03552),R33(03560),
 R33(03572),R33(03575),R33(03607),R33(03615),R33(03616),R33(03622),R33(03630),R33(03632),
 R33(03633),R33(03660),R33(03665),R33(03666),R33(03670),R33(03672),R33(03674),R33(03675),
 R33(03677),R33(03700),R33(03702),R33(03703),R33(03706),R33(03707),R33(03710),R33(03711),
 R33(03712),R33(03721),R33(03722),R33(03735),R33(03737),
#endif
#if SV_G(7)
 R33(03740),
#endif
#if SV_G(1)
 R34(03716),R34(03720),R34(03722),R34(03723),R34(03744),R34(03745),R34(03753),R34(03754),
 R34(03755),R34(03757),R37(03410),R37(03412),R37(03416),R37(03417),R37(03435),R37(03436),
 R37(03437),R37(03441),R37(03451),R37(03453),R37(03461),R37(03515),R37(03517),R37(03524),
 R37(03525),R37(03531),R37(03541),R37(03557),R37(03565),R37(03570),R37(03571),R37(03614),
 R37(03641),
#endif
};
static const agc_routine_entry interp_entries[]={
#if SV_G(5)
 R22(03705),R23(02472),
#endif
#if SV_G(7)
 R23(02503),
#endif
#if SV_G(5)
 R33(02267),R33(02273),
#endif
#if SV_G(4)
 R33(02415),R33(02417),R33(02461),
#endif
#if SV_G(5)
 R33(02602),
#endif
#if SV_G(6)
 R33(02627),
#endif
#if SV_G(2)
 R33(02730),
#endif
#if SV_G(3)
 R33(02765),
#endif
#if SV_G(7)
 R33(02776),R33(03021),R33(03032),R33(03045),
#endif
#if SV_G(4)
 R33(03066),R33(03105),
#endif
#if SV_G(6)
 R33(03130),
#endif
#if SV_G(7)
 R33(03304),
#endif
#if SV_G(3)
 R33(03344),
#endif
#if SV_G(7)
 R33(03761),
#endif
#if SV_G(4)
 R33(03765),
#endif
#if SV_G(6)
 R33(03774),
#endif
};
#define COUNT(t) (uint16_t)(sizeof t/sizeof t[0])
const agc_routine_table lm_servicer_routines={basic_entries,COUNT(basic_entries)};
const agc_routine_table lm_servicer_interpretive={interp_entries,COUNT(interp_entries)};
const agc_site_operand lm_servicer_flag_sites[LMSV_FLAG_SITES]={
#include "lm_servicer_flag_sites.inc"
};
/* Every key, basic then interpretive, in source order. */
const uint16_t lmsv_routine_keys[LMSV_ROUTINES]={
 K21(02130),K21(02150),K22(03661),K22(03665),K22(03667),K22(03676),K22(03707),K23(02461),
 K27(03321),K32(03770),K32(03772),K33(02206),K33(02213),K33(02217),K33(02233),K33(02254),
 K33(02256),K33(02261),K33(02263),K33(02266),K33(02274),K33(02275),K33(02276),K33(02314),
 K33(02323),K33(02336),K33(02342),K33(02346),K33(02352),K33(02353),K33(02361),K33(02365),
 K33(02366),K33(02370),K33(02375),K33(02400),K33(02403),K33(02411),K33(02420),K33(02422),
 K33(02423),K33(02440),K33(02447),K33(02460),K33(02476),K33(02502),K33(02515),K33(02531),
 K33(02536),K33(02540),K33(02546),K33(02547),K33(02564),K33(02570),K33(02664),K33(02674),
 K33(02675),K33(02676),K33(02710),K33(02713),K33(02714),K33(02716),K33(02720),K33(02721),
 K33(02724),K33(02725),K33(02727),K33(03053),K33(03065),K33(03155),K33(03165),K33(03210),
 K33(03237),K33(03240),K33(03241),K33(03243),K33(03257),K33(03275),K33(03306),K33(03307),
 K33(03310),K33(03311),K33(03315),K33(03336),K33(03417),K33(03420),K33(03421),K33(03423),
 K33(03431),K33(03435),K33(03466),K33(03472),K33(03512),K33(03513),K33(03514),K33(03515),
 K33(03527),K33(03533),K33(03534),K33(03552),K33(03560),K33(03572),K33(03575),K33(03607),
 K33(03615),K33(03616),K33(03622),K33(03630),K33(03632),K33(03633),K33(03660),K33(03665),
 K33(03666),K33(03670),K33(03672),K33(03674),K33(03675),K33(03677),K33(03700),K33(03702),
 K33(03703),K33(03706),K33(03707),K33(03710),K33(03711),K33(03712),K33(03721),K33(03722),
 K33(03735),K33(03737),K33(03740),K34(03716),K34(03720),K34(03722),K34(03723),K34(03744),
 K34(03745),K34(03753),K34(03754),K34(03755),K34(03757),K37(03410),K37(03412),K37(03416),
 K37(03417),K37(03435),K37(03436),K37(03437),K37(03441),K37(03451),K37(03453),K37(03461),
 K37(03515),K37(03517),K37(03524),K37(03525),K37(03531),K37(03541),K37(03557),K37(03565),
 K37(03570),K37(03571),K37(03614),K37(03641),K22(03705),K23(02472),K23(02503),K33(02267),
 K33(02273),K33(02415),K33(02417),K33(02461),K33(02602),K33(02627),K33(02730),K33(02765),
 K33(02776),K33(03021),K33(03032),K33(03045),K33(03066),K33(03105),K33(03130),K33(03304),
 K33(03344),K33(03761),K33(03765),K33(03774)
};
