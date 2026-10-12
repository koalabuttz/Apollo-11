/* SPDX-License-Identifier: MIT */
/* Luminary099 P34/P74 (P34-35_P74-75.agc, bank 35 02545-02724) as routines
 * of the control-transfer ABI; see include/lm_p34p74_routines.h and
 * docs/LM-P34P74.md. Every interpretive pair is compiled with the shared
 * bodies (src/lm_interp_steps.c, src/lm_interp_orders.c): its original order
 * and operand words appear below only as the literal bookkeeping arguments
 * of those paths, and each runtime branch follows the outcome they return.
 * SIN and COS run the original bodies (src/sband_trig.c). Each basic
 * instruction is one ai_* register effect on the image, in source order. */
#include "lm_p34p74_routines.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"
#include "fixed_data.h"
#include "sband_trig.h"
#define W s->words
/* The routines a C64 build registers (C64 RAM): group 1 all but two, group 2
 * P34/P74E's INTPRET return (SIN, COS) and S34/35.2's return (ABVAL). Host
 * builds register all. */
#ifndef LM_P34P74_C64_GROUP
#define HAS_G1 1
#define HAS_G2 1
#else
#define HAS_G1 (LM_P34P74_C64_GROUP==1)
#define HAS_G2 (LM_P34P74_C64_GROUP==2)
#endif
#if defined __clang__ && defined LM_P34P74_C64_GROUP
/* A C64 group build compiles only its routines, so helpers that only the
 * other group's routines use go unused there. */
#pragma clang diagnostic ignored "-Wunused-function"
#endif
/* INDJUMP/MISCJUMP/UNAJUMP body entries. */
enum {VLOAD=06505,PDVL=06557,DLOAD=06052,DAD=06775,VSU=06747,GSHIFT=07621,SETPD=07613,PUSH=03247,ABVAL=03176,
 COSINE=03517,SINE=03530,CALLITA=02507,BZEGOTO=02467,SWITCH=02516};
/* Erasable (EB-window addresses as the source names them, EBANK=SUBEXIT;
 * DXCH carries K+1). */
#define CENTANG 01620u
#define NN 01466u
/* Fixed constants (35,3723, 35,3725, 35,3745-3746, 35,2417, 2,1006). */
#define K_V06N37 01445u
#define K_V06N58 01472u
#define K_130DEG_HI 013434u
#define K_130DEG_LO 016162u
#define K_P30ZERO 0u
#define K_V05N09 01211u
/* Destinations outside the routines: AVFLAGA, AVFLAGP and P20FLGON
 * (P32-P35_P72-P75), DISPLAYE and VNPOOH (lm-p34), and ALARM, BANKCALL,
 * GOTOPOOH and INTPRET. */
#define AVFLAGA 02347u
#define AVFLAGP 02354u
#define P20FLGON 02361u
#define DISPLAYE 03674u
#define VNPOOH 03712u
#define ALARM 05567u
#define BANKCALL 04616u
#define GOTOPOOH 06001u
#define INTPRET 06037u

static void next(apollo_overlay_state *s){lmi_danzig_next(s);}
static void right(apollo_overlay_state *s){lmi_danzig_right(s);}
static void opnd(apollo_overlay_state *s,agc_word word,agc_word target){
 if(word&040000u)lmi_address_pushup(s,word,target);else lmi_direct(s,word,target);
}
static void store(apollo_overlay_state *s,agc_word code){lmi_store_code(s,code);next(s);}
static void dload(apollo_overlay_state *s,agc_word word){opnd(s,word,DLOAD);lmi_dload_op(s,0);}
static void stodl(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_dodload(s);
 if(word&040000u)lmi_diradres_pushup(s,word,DLOAD);else lmi_diradres(s,word,DLOAD);
 lmi_dload_op(s,0);next(s);
}
static void stovl(apollo_overlay_state *s,agc_word code,agc_word word){
 lmi_store_code(s,code);lmi_dovload(s);
 if(word&040000u)lmi_diradres_pushup(s,word,VLOAD);else lmi_diradres(s,word,VLOAD);
 lmi_vload(s,0);next(s);
}
/* STCALL code label (fixed CADR) to the callee's NEWOPS. */
static void stcall(apollo_overlay_state *s,agc_word code,agc_word word){lmi_store_code(s,code);lmi_15bitadr(s,word,CALLITA);lmi_call(s);lmi_goto(s);}
static void gshift(apollo_overlay_state *s,agc_word word){lmi_indexed(s,word,GSHIFT);lmi_gshift(s);}
static uint8_t sw(apollo_overlay_state *s,p66_bias_io *io,agc_word word){lmi_misc(s,word,SWITCH);return lmi_switch(s,io);}
static void call(apollo_overlay_state *s,agc_word word){lmi_misc(s,word,CALLITA);lmi_call(s);lmi_goto(s);}
/* ABVAL: 1 when it ends at POODOO1 (alarm 1302). */
static uint8_t abval(apollo_overlay_state *s){lmi_unary(s,ABVAL);return lmi_abvalabs(s);}
static uint8_t trig(apollo_overlay_state *s,uint8_t cosine){lmi_unary(s,cosine?COSINE:SINE);return sband_trig(s,APOLLO_FIXED_LM,cosine);}
/* The certified fixed words that ADDRWD names after address formation (a
 * fixed-fixed address, or the FBANK the formation switched to). Banks 30-37
 * are the plain ones: the entry contract admits only SUPERBNK settings
 * below 4. */
static uint8_t fixed_operand(const apollo_overlay_state *s,agc_word *k,unsigned n){
 unsigned i,a=W[0116],bank;
 if(a>=04000u&&a+n<=010000u){bank=a>>10;a&=01777u;}
 else if(a>=02000u&&a+n<=04000u){bank=W[4]>>10;a-=02000u;}
 else return 0;
 if(bank>=040u)return 0;
 for(i=0;i<n;++i)if(!apollo_fixed_data_read(APOLLO_FIXED_LM,(uint16_t)(bank*1024u+a+i),&k[i]))return 0;
 return 1;
}

/* Canonical image at NEWOPS with LOC `loc` in FB 35, coherent banks with
 * BANKSET = BB, INTBIT15 set, EDOP, ZERO and NEWJOB zero, the VAC one of the
 * five with PUSHLOC no further than 32D into it, MODE one of the three, IRQ
 * 0 or 1 and an allowed SUPERBNK. */
static uint8_t newops_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word loc){
 unsigned i,vac=W[0120],pd=W[0166];agc_word mode=W[0163];
 if(!s||!io||W[5]!=06070u||W[4]!=072000u||W[0164]!=loc||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[0165]!=W[6])return 0;
 if(W[0115]!=040000u||W[023]||W[7]||W[067])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u)||io->superbank>=0100u)return 0;
 if(vac<0401u||vac>0661u||(vac-0401u)%054u||pd<vac||pd>vac+040u)return 0;
 if(mode!=0u&&mode!=1u&&mode!=077776u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 return 1;
}
#define INTERP(name,loc) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *v){\
 p66_bias_io *io=v;(void)p;(void)io;if(!newops_ok(s,io,(loc)))return EXEC_OUT_OF_SCOPE;
/* Every interpretive routine ends at a NEWOPS, at POODOO1, or after EXIT at
 * the basic word that follows: the trampoline continues there or stops
 * outside the registered routines. */

/* CLEAR DLOAD ETPIFLAG TTPI; STODL TIG ELEV; BZE SET P34/P74B ETPIFLAG
 * (both paths reach P34/P74B's NEWOPS). */
INTERP(p34_start,02562u)
 lmi_newops(s,071214);if(sw(s,io,001270))return EXEC_INVALID_ACTION;right(s);dload(s,003636);next(s);
 stodl(s,017442,002257);
 lmi_newops(s,043054);lmi_misc(s,072572,BZEGOTO);
 if(lmi_bze(s))return EXEC_OK;
 right(s);if(sw(s,io,001070))return EXEC_INVALID_ACTION;next(s);
 return EXEC_OK;
}
/* P34/P74B CALL SELECTMU. */
INTERP(p34b,02572u)
 lmi_newops(s,077624);call(s,020000);
 return EXEC_OK;
}
/* INTLOOP DLOAD DAD TTPI NOMTPI; STCALL TDEC1 PRECSET. */
static void intloop(apollo_overlay_state *s){
 lmi_newops(s,043345);dload(s,003636);right(s);opnd(s,002317,DAD);lmi_dad(s,0);next(s);
 stcall(s,034041,046360);
}
/* P34/P74C DLOAD SET ZEROVECS ITSWICH; BON CLEAR ETPIFLAG SWCHSET ITSWICH;
 * SWCHSET STORE NOMTPI; on into INTLOOP. */
INTERP(p34c,02574u)
 agc_word k[2];
 lmi_newops(s,043145);lmi_direct(s,006522,DLOAD);if(!fixed_operand(s,k,2))return EXEC_INVALID_ACTION;lmi_dload_op(s,k);
 right(s);if(sw(s,io,003460))return EXEC_INVALID_ACTION;next(s);
 lmi_newops(s,043014);
 if(sw(s,io,001310))lmi_swbranch(s,072603);
 else{right(s);if(sw(s,io,003660))return EXEC_INVALID_ACTION;next(s);}
 store(s,002317);
 intloop(s);
 return EXEC_OK;
}
INTERP(r_intloop,02604u)
 intloop(s);
 return EXEC_OK;
}
/* CALL S33/34.1 (where PRECSET returns). */
INTERP(call_s331,02611u)
 lmi_newops(s,077624);call(s,072764);
 return EXEC_OK;
}
/* BZE EXIT SWCHCLR (where S33/34.1 returns). */
INTERP(s331_ret,02613u)
 lmi_newops(s,077454);lmi_misc(s,072625,BZEGOTO);
 if(lmi_bze(s))return EXEC_OK;
 right(s);lmi_exit(s);
 return EXEC_OK;
}
/* SWCHCLR BONCLR BON ITSWICH INTLOOP ETPIFLAG P34/P74D; EXIT. */
INTERP(swchclr,02625u)
 lmi_newops(s,043014);
 if(sw(s,io,003600)){lmi_swbranch(s,072604);return EXEC_OK;}
 right(s);
 if(sw(s,io,001310)){lmi_swbranch(s,072635);return EXEC_OK;}
 next(s);
 lmi_newops(s,077776);lmi_exit(s);
 return EXEC_OK;
}
/* P34/P74D EXIT. */
INTERP(p34d,02635u)
 lmi_newops(s,077776);lmi_exit(s);
 return EXEC_OK;
}
#if HAS_G2
/* SETPD DLOAD 0D RTX1; STODL X1 CENTANG; PUSH COS; STODL CSTH (pushes up);
 * SIN; STOVL SNTH RPASS3; VSR* 0,2; STOVL RVEC VPASS3; VSR* SET 0,2 RVSW;
 * STCALL VVEC TIMETHET (where P34/P74E's INTPRET returns). */
INTERP(p34e,02641u)
 lmi_newops(s,071201);lmi_direct(s,000001,SETPD);lmi_setpd(s);lmi_noibnksw_right(s);dload(s,002777);next(s);
 stodl(s,014047,003621);
 lmi_newops(s,071406);lmi_unary(s,PUSH);lmi_push(s);right(s);if(!trig(s,1))return EXEC_INVALID_ACTION;next(s);
 stodl(s,016732,077756);
 lmi_newops(s,077756);if(!trig(s,0))return EXEC_INVALID_ACTION;next(s);
 stovl(s,026730,003552);
 lmi_newops(s,077657);gshift(s,057176);next(s);
 stovl(s,026655,003560);
 lmi_newops(s,043057);gshift(s,057176);right(s);if(sw(s,io,003466))return EXEC_INVALID_ACTION;next(s);
 stcall(s,036744,024732);
 return EXEC_OK;
}
#endif
/* DLOAD TTPI; STORE INTIME; DAD T; STCALL TPASS4 S34/35.2 (where TIMETHET
 * returns). */
INTERP(timethet_ret,02664u)
 lmi_newops(s,077745);dload(s,003636);next(s);
 store(s,003610);
 lmi_newops(s,077615);opnd(s,000037,DAD);lmi_dad(s,0);next(s);
 stcall(s,037631,073306);
 return EXEC_OK;
}
#if HAS_G2
/* VLOAD ABVAL DELVEET3; STOVL DELVTPI VPASS4; VSU ABVAL VTPRIME; STOVL
 * DELVTPF RACT3; PDVL CALL VIPRIME PERIAPO1 (where S34/35.2 returns). */
INTERP(s3435_2_ret,02673u)
 lmi_newops(s,051575);opnd(s,002366,VLOAD);lmi_vload(s,0);right(s);if(abval(s))return EXEC_OK;next(s);
 stovl(s,027576,003506);
 lmi_newops(s,051451);opnd(s,003566,VSU);lmi_vsu(s,0);right(s);if(abval(s))return EXEC_OK;next(s);
 stovl(s,026350,003536);
 lmi_newops(s,045115);opnd(s,002337,PDVL);lmi_pdvl(s,0);right(s);call(s,046316);
 return EXEC_OK;
}
#endif
/* CALL SHIFTR1 (where PERIAPO1 returns). */
INTERP(periapo_ret,02706u)
 lmi_newops(s,077624);call(s,046426);
 return EXEC_OK;
}
/* STODL POSTTPI TTPI; STORE TIG; EXIT (where SHIFTR1 returns). */
INTERP(shiftr1_ret,02710u)
 stodl(s,017606,003636);
 store(s,003442);
 lmi_newops(s,077776);lmi_exit(s);
 return EXEC_OK;
}
/* The CALLs of S34/35.5 and VN1645, each where the one before returns. */
INTERP(call_s5,02717u)
 lmi_newops(s,077624);call(s,073522);
 return EXEC_OK;
}
INTERP(call_vn1645,02721u)
 lmi_newops(s,077624);call(s,073606);
 return EXEC_OK;
}
/* GOTO P34/P74C (where VN1645 returns). */
INTERP(loop,02723u)
 lmi_newops(s,077650);lmi_misc(s,072574,BZEGOTO);lmi_goto_order(s);
 return EXEC_OK;
}
static const agc_routine_entry interpretive[]={
#if HAS_G1
 {LM_P34P74_B35(02562),p34_start},{LM_P34P74_B35(02572),p34b},{LM_P34P74_B35(02574),p34c},
 {LM_P34P74_B35(02604),r_intloop},{LM_P34P74_B35(02611),call_s331},{LM_P34P74_B35(02613),s331_ret},
 {LM_P34P74_B35(02625),swchclr},{LM_P34P74_B35(02635),p34d},
#endif
#if HAS_G2
 {LM_P34P74_B35(02641),p34e},
#endif
#if HAS_G1
 {LM_P34P74_B35(02664),timethet_ret},
#endif
#if HAS_G2
 {LM_P34P74_B35(02673),s3435_2_ret},
#endif
#if HAS_G1
 {LM_P34P74_B35(02706),periapo_ret},{LM_P34P74_B35(02710),shiftr1_ret},{LM_P34P74_B35(02717),call_s5},
 {LM_P34P74_B35(02721),call_vn1645},{LM_P34P74_B35(02723),loop},
#endif
};
const agc_routine_table lm_p34p74_interpretive_routines={interpretive,(uint16_t)(sizeof interpretive/sizeof *interpretive)};

static void tc(apollo_overlay_state *s,agc_word ret,agc_word target){ai_tc(s,ret);W[5]=target;}
/* Basic entry: canonical image at Z in bank 35 with coherent banks. */
static uint8_t basic_ok(const apollo_overlay_state *s,const p66_bias_io *io,agc_word z){unsigned i;
 if(!s||!io||W[5]!=z||W[4]!=072000u||W[3]>03400u||(W[3]&0377u)||W[6]!=(agc_word)(W[4]|(W[3]>>8))||W[7])return 0;
 if(io->interrupt_enabled>1u||(io->superbank&~0160u)||io->superbank>=0100u)return 0;
 for(i=2;i<2048u;++i)if(W[i]>AGC_MASK)return 0;
 return 1;
}
#define ENTRY(z) do{if(!basic_ok(s,(const p66_bias_io *)io,(z)))return EXEC_OUT_OF_SCOPE;(void)p;}while(0)
#define ROUTINE(name) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io)
/* P34/P74A TC P20FLGON. */
static void p34a(apollo_overlay_state *s){tc(s,02551u,P20FLGON);}
/* ALARM's call: TC ALARM (OCT 611 follows). */
static void alarm611(apollo_overlay_state *s){tc(s,02616u,ALARM);}
/* P34/P74E TC INTPRET. */
static void p34e_intpret(apollo_overlay_state *s){tc(s,02641u,INTPRET);}
ROUTINE(p34){ENTRY(02545u);tc(s,02546u,AVFLAGA);return EXEC_OK;}
ROUTINE(avflaga_ret){ENTRY(02546u);tc(s,02547u,02550u);p34a(s);return EXEC_OK;}
ROUTINE(p74){ENTRY(02547u);tc(s,02550u,AVFLAGP);return EXEC_OK;}
ROUTINE(r_p34a){ENTRY(02550u);p34a(s);return EXEC_OK;}
/* CAF V06N37; TC VNPOOH (where P20FLGON returns). */
ROUTINE(p20_ret){ENTRY(02551u);ai_caf(s,K_V06N37);tc(s,02553u,VNPOOH);return EXEC_OK;}
/* EXTEND; DCA 130DEG; DXCH CENTANG; CAF P30ZERO; TS NN; TC DISPLAYE (where
 * VNPOOH returns). */
ROUTINE(vnpooh_ret){static const agc_word k[2]={K_130DEG_HI,K_130DEG_LO};ENTRY(02553u);
 ai_dcaf(s,k);ai_dxch(s,CENTANG);ai_caf(s,K_P30ZERO);(void)ai_ts(s,NN);tc(s,02561u,DISPLAYE);return EXEC_OK;}
/* TC INTPRET (where DISPLAYE returns). */
ROUTINE(displaye_ret){ENTRY(02561u);tc(s,02562u,INTPRET);return EXEC_OK;}
/* After S33/34.1's EXIT: TC ALARM. */
ROUTINE(r_alarm){ENTRY(02615u);alarm611(s);return EXEC_OK;}
/* CAF V05N09; TC BANKCALL (CADR GOFLASH at Q) (where ALARM returns). */
ROUTINE(alarm_ret){ENTRY(02617u);ai_caf(s,K_V05N09);tc(s,02621u,BANKCALL);return EXEC_OK;}
/* GOFLASH's terminate, proceed and recycle returns: TC GOTOPOOH; TC
 * P34/P74A; TC -7. */
ROUTINE(flash_terminate){ENTRY(02622u);tc(s,02623u,GOTOPOOH);return EXEC_OK;}
ROUTINE(flash_proceed){ENTRY(02623u);tc(s,02624u,02550u);p34a(s);return EXEC_OK;}
ROUTINE(flash_recycle){ENTRY(02624u);tc(s,02625u,02615u);alarm611(s);return EXEC_OK;}
/* After SWCHCLR's EXIT: TC DISPLAYE; TC P34/P74E. */
ROUTINE(swchclr_exit){ENTRY(02633u);tc(s,02634u,DISPLAYE);return EXEC_OK;}
ROUTINE(displaye_ret2){ENTRY(02634u);tc(s,02635u,02640u);p34e_intpret(s);return EXEC_OK;}
/* After P34/P74D's EXIT: CAF V06N37; TC VNPOOH. */
ROUTINE(p34d_exit){ENTRY(02636u);ai_caf(s,K_V06N37);tc(s,02640u,VNPOOH);return EXEC_OK;}
ROUTINE(r_p34e){ENTRY(02640u);p34e_intpret(s);return EXEC_OK;}
/* After the EXIT at 2713: CAF V06N58; TC VNPOOH; TC INTPRET. */
ROUTINE(shiftr1_exit){ENTRY(02714u);ai_caf(s,K_V06N58);tc(s,02716u,VNPOOH);return EXEC_OK;}
ROUTINE(vnpooh_ret2){ENTRY(02716u);tc(s,02717u,INTPRET);return EXEC_OK;}
static const agc_routine_entry basic[]={
#if HAS_G1
 {LM_P34P74_B35(02545),p34},{LM_P34P74_B35(02546),avflaga_ret},{LM_P34P74_B35(02547),p74},{LM_P34P74_B35(02550),r_p34a},
 {LM_P34P74_B35(02551),p20_ret},{LM_P34P74_B35(02553),vnpooh_ret},{LM_P34P74_B35(02561),displaye_ret},
 {LM_P34P74_B35(02615),r_alarm},{LM_P34P74_B35(02617),alarm_ret},{LM_P34P74_B35(02622),flash_terminate},
 {LM_P34P74_B35(02623),flash_proceed},{LM_P34P74_B35(02624),flash_recycle},{LM_P34P74_B35(02633),swchclr_exit},
 {LM_P34P74_B35(02634),displaye_ret2},{LM_P34P74_B35(02636),p34d_exit},{LM_P34P74_B35(02640),r_p34e},
 {LM_P34P74_B35(02714),shiftr1_exit},{LM_P34P74_B35(02716),vnpooh_ret2},
#else
 {0,0}
#endif
};
const agc_routine_table lm_p34p74_basic_routines={basic,(uint16_t)(HAS_G1?sizeof basic/sizeof *basic:0u)};
