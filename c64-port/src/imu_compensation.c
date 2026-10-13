/* SPDX-License-Identifier: MIT */
/* Luminary099 and Comanche055 IMU_COMPENSATION_PACKAGE (bank 06) as native
 * routines of the control-transfer ABI; see include/imu_compensation.h and
 * docs/IMU-COMPENSATION.md. Each instruction of the source is one ai_*
 * register effect on the image, in source order; INDEX is the explicit
 * address offset (the loop indices are 4, 2 and the -0 that 1 + NEG1
 * leaves, which indexes as 0). */
#include "imu_compensation.h"
#include "agc_image_ops.h"
#include "p66_bias.h"
#include "cm_native_image.h"
#include "agc_image_w.h"
/* Erasable (EB-window addresses are the source's; EB selects the bank). */
#define TIME1 025u
#define FLAGWRD2 076u
#define FLAGWRD8 0104u
#define VBUF 0122u
#define BUF 0130u
#define TEM1 0141u
#define MPAC 0154u
#define MODE 0163u
#define NBDX 01460u
#define NBDY 01461u
#define NBDZ 01462u
#define ADIAX 01463u
#define ADIAY 01464u
#define ADIAZ 01465u
#define ADSRAX 01466u
#define ADSRAY 01467u
#define ADSRAZ 01470u
#define GCOMP 01471u
#define GCOMPSW 01477u
#define PIPABIAS 01452u
#define PIPASCF 01453u
/* Fixed constants (values checked against both ropes). */
#define K_BIT1 01u
#define K_BIT4 010u
#define K_BIT6 040u
#define K_BIT8 0200u
#define K_BIT9 0400u
#define K_BIT10 01000u
#define K_BIT12 04000u
#define K_HALF 020000u
#define K_POSMAX 037777u
#define K_NEG1 077776u   /* NEG1, NEGONE, COMPCHK */
#define K_PRIO21 021000u
#define K_PRIO31 031000u
#define K_LGCOMP 01471u  /* ECADR GCOMP */
/* Return points of TC MPAC +2, TC BUF +1 and TC Q: the TC stored there runs
 * from erasable (or from Q itself) and leaves Q one past it. */
#define RET_MPAC2 0157u
#define RET_BUF1 0132u
#define RET_Q 03u

typedef struct {
 agc_word pipadt,deltav,piptime_low;   /* 1/PIPADT, DELVX, PIPTIME(1) +1 */
 agc_word pipa_scale,irig_scale;       /* MP after PIPABIAS*1/PIPADT; GCOMPSUB shift */
 agc_word swreturn,bankcall,novac,endofjob;
} icp_mission;
static const icp_mission LM={01075u,0324u,01235u,K_BIT4,K_BIT9,LM_ICP_SWRETURN,LM_ICP_BANKCALL,LM_ICP_NOVAC,LM_ICP_ENDOFJOB};
static const icp_mission CM={01074u,01162u,01246u,K_BIT1,K_BIT12,CM_ICP_SWRETURN,CM_ICP_BANKCALL,CM_ICP_NOVAC,CM_ICP_ENDOFJOB};
/* Both io records keep interrupt_enabled as a 16-bit word. */
static void interrupts(const icp_mission *m,void *io,uint16_t on){
 if(m==&LM)((p66_bias_io *)io)->interrupt_enabled=on;else ((cm_native_io *)io)->interrupt_enabled=on;
}
/* INDEX K: the index word as the address offset (-0 indexes as 0). */
static unsigned ix(const apollo_overlay_state *s,unsigned k){agc_word v=ai_word(s,k);return v==077777u?0u:v;}
static unsigned ixa(const apollo_overlay_state *s){agc_word v=(agc_word)(((W[0]>>1)&040000u)|(W[0]&037777u));return v==077777u?0u:v;}
static void tcf(apollo_overlay_state *s,agc_word z){W[5]=z;}

/* GCOMPSUB: XCH MPAC; MP MPAC; DXCH VBUF; CA MPAC +1; MP MPAC; TS L;
 * CAF ZERO; DAS VBUF; CA VBUF; MP BIT9 (CM BIT12); INDEX BUF DAS GCOMP;
 * CA VBUF +1; MP BIT9; TS L; CAF ZERO; INDEX BUF DAS GCOMP; TC Q. */
static void gcompsub(const icp_mission *m,apollo_overlay_state *s){
 ai_xch(s,MPAC);ai_mp(s,MPAC);ai_dxch(s,VBUF);
 ai_ca(s,MPAC+1u);ai_mp(s,MPAC);(void)ai_ts(s,1);ai_caf(s,0);ai_das(s,VBUF);
 ai_ca(s,VBUF);ai_mpf(s,m->irig_scale);ai_das(s,GCOMP+ix(s,BUF));
 ai_ca(s,VBUF+1u);ai_mpf(s,m->irig_scale);(void)ai_ts(s,1);ai_caf(s,0);ai_das(s,GCOMP+ix(s,BUF));
 ai_tc(s,RET_Q);
}
/* DRFTSUB2: CAF TWO; AD BUF; XCH BUF; INDEX A; CCS GCOMP; TCF +2 /
 * TC BUF +1 / MASK COMPCHK; CCS A; TS GCOMPSW; TC BUF +1. CCS leaves
 * A >= +0 and MASK keeps bit 15 clear, so CCS A is + or +0. */
static void drftsub2(apollo_overlay_state *s){uint8_t c;
 ai_caf(s,2u);ai_ad(s,BUF);ai_xch(s,BUF);
 c=ai_ccs(s,GCOMP+ixa(s));
 if(c==1u){ai_tc(s,RET_BUF1);return;}
 if(c!=3u)ai_maskf(s,K_NEG1);
 if(ai_ccs(s,0)==0u)(void)ai_ts(s,GCOMPSW);
 ai_tc(s,RET_BUF1);
}
/* DRIFTSUB: QXCH BUF +1; MP 1/PIPADT; LXCH MPAC +1; MP BIT4;
 * INDEX BUF DAS GCOMP; CA MPAC +1; MP BIT4; TS L; CAF ZERO;
 * INDEX BUF DAS GCOMP; DRFTSUB2. */
static void driftsub(const icp_mission *m,apollo_overlay_state *s){
 ai_qxch(s,BUF+1u);ai_mp(s,m->pipadt);ai_lxch(s,MPAC+1u);ai_mpf(s,K_BIT4);ai_das(s,GCOMP+ix(s,BUF));
 ai_ca(s,MPAC+1u);ai_mpf(s,K_BIT4);(void)ai_ts(s,1);ai_caf(s,0);ai_das(s,GCOMP+ix(s,BUF));
 drftsub2(s);
}
/* FBIASSUB: XCH Q; TS BUF +1; CA Q; MP MPAC; INDEX BUF DAS GCOMP; CA Q;
 * MP MPAC +1; TS L; CAF ZERO; INDEX BUF DAS GCOMP; TCF DRFTSUB2. */
static void fbiassub(apollo_overlay_state *s){
 ai_xch(s,2);(void)ai_ts(s,BUF+1u);ai_ca(s,2);ai_mp(s,MPAC);ai_das(s,GCOMP+ix(s,BUF));
 ai_ca(s,2);ai_mp(s,MPAC+1u);(void)ai_ts(s,1);ai_caf(s,0);ai_das(s,GCOMP+ix(s,BUF));
 drftsub2(s);
}
/* One acceleration pair of IRIGX/IRIGY/IRIGZ: DCS <pipa>; DXCH MPAC;
 * CA or CS <coefficient>; TC GCOMPSUB (Q = ret). */
static void accel(const icp_mission *m,apollo_overlay_state *s,unsigned pipa,unsigned coef,int negate,agc_word ret){
 ai_dcs(s,pipa);ai_dxch(s,MPAC);if(negate)ai_cs(s,coef);else ai_ca(s,coef);ai_tc(s,ret);gcompsub(m,s);
}
#define DELVY_OF(m) ((m)->deltav+2u)
#define DELVZ_OF(m) ((m)->deltav+4u)
/* Luminary IRIGX/IRIGY/IRIGZ: QXCH MPAC +2, two terms, TC MPAC +2. */
static void lm_irig(apollo_overlay_state *s,unsigned axis,agc_word at){const icp_mission *m=&LM;
 ai_qxch(s,MPAC+2u);
 if(axis==0u){accel(m,s,m->deltav,ADIAX,0,(agc_word)(at+7u));accel(m,s,DELVY_OF(m),ADSRAX,1,(agc_word)(at+014u));}
 else if(axis==1u){accel(m,s,DELVY_OF(m),ADIAY,0,(agc_word)(at+7u));accel(m,s,DELVZ_OF(m),ADSRAY,1,(agc_word)(at+014u));}
 else{accel(m,s,DELVY_OF(m),ADSRAZ,0,(agc_word)(at+7u));accel(m,s,DELVZ_OF(m),ADIAZ,0,(agc_word)(at+014u));}
 ai_tc(s,RET_MPAC2);
}
#define LM_IRIGX 03356u
#define LM_IRIGY 03373u
#define LM_IRIGZ 03410u
/* 1/GYRO: CAF FOUR; per axis (BUF 4, 2, -0) INDEX BUF CA GCOMP +1; MP BIT8;
 * INDEX BUF TS GCOMP +1; CAF ZERO; INDEX BUF XCH GCOMP; MP BIT8;
 * INDEX BUF DAS GCOMP; CCS BUF; AD NEG1; TCF 1/GYRO +1; then (-0) CAF LGCOMP;
 * TC BANKCALL with CADR IMUPULSE at Q. */
static enum exec_status gyro(const icp_mission *m,apollo_overlay_state *s,agc_word cadr_imupulse){uint8_t c;
 ai_caf(s,4u);
 do{(void)ai_ts(s,BUF);
  ai_ca(s,GCOMP+1u+ix(s,BUF));ai_mpf(s,K_BIT8);(void)ai_ts(s,GCOMP+1u+ix(s,BUF));
  ai_caf(s,0);ai_xch(s,GCOMP+ix(s,BUF));ai_mpf(s,K_BIT8);ai_das(s,GCOMP+ix(s,BUF));
  c=ai_ccs(s,BUF);if(c==0u)ai_adf(s,K_NEG1);
 }while(c<2u);
 /* BUF is 4, 2 or -0 here: CCS takes + then -0. */
 ai_caf(s,K_LGCOMP);ai_tc(s,cadr_imupulse);tcf(s,m->bankcall);
 return EXEC_OK;
}
/* GCOMP1 (no caller in either program): CAF FOUR; per axis INDEX BUF CA
 * GCOMP +1; MP BIT8; INDEX BUF LXCH GCOMP +1; CCS BUF; AD NEG1;
 * TCF GCOMP1 +1; then (-0) TCF ENDOFJOB. */
static enum exec_status gcomp1(const icp_mission *m,apollo_overlay_state *s){uint8_t c;
 ai_caf(s,4u);
 do{(void)ai_ts(s,BUF);
  ai_ca(s,GCOMP+1u+ix(s,BUF));ai_mpf(s,K_BIT8);ai_lxch(s,GCOMP+1u+ix(s,BUF));
  c=ai_ccs(s,BUF);if(c==0u)ai_adf(s,K_NEG1);
 }while(c<2u);
 tcf(s,m->endofjob);return EXEC_OK;
}
/* IRIG1: CA MODE; TS EBANK; TCF SWRETURN. */
static enum exec_status irig1(const icp_mission *m,apollo_overlay_state *s){ai_ca(s,MODE);(void)ai_ts(s,3);tcf(s,m->swreturn);return EXEC_OK;}
/* CAF LGCOMP; XCH EBANK; TS MODE: GCOMP's EBANK, the caller's saved. */
static void save_ebank(apollo_overlay_state *s){ai_caf(s,K_LGCOMP);ai_xch(s,3);(void)ai_ts(s,MODE);}
/* 1/PIPA1 loop over PIPAZ, PIPAY, PIPAX (BUF +2 = 4, 2, -0):
 * INDEX CA PIPASCF; MP DELVX; TS Q; CA L; MP BIT6; INDEX TS DELVX +1;
 * CA Q; MP BIT6; INDEX DAS DELVX; INDEX CS PIPABIAS; MP 1/PIPADT;
 * MP BIT4 (CM BIT1); INDEX DAS DELVX; CCS BUF +2; AD NEG1; TCF 1/PIPA1 +1.
 * Leaves A = +0 (the -0 branch of CCS). */
static void pipa_loop(const icp_mission *m,apollo_overlay_state *s){uint8_t c;unsigned k;
 ai_caf(s,4u);
 do{(void)ai_ts(s,BUF+2u);k=ix(s,BUF+2u);
  ai_ca(s,PIPASCF+k);ai_mp(s,m->deltav+k);(void)ai_ts(s,2);
  ai_ca(s,1);ai_mpf(s,K_BIT6);(void)ai_ts(s,m->deltav+1u+k);
  ai_ca(s,2);ai_mpf(s,K_BIT6);ai_das(s,m->deltav+k);
  ai_cs(s,PIPABIAS+k);ai_mp(s,m->pipadt);ai_mpf(s,m->pipa_scale);ai_das(s,m->deltav+k);
  c=ai_ccs(s,BUF+2u);if(c==0u)ai_adf(s,K_NEG1);
 }while(c<2u);
}
/* End of IRIGCOMP: CCS GCOMPSW; TCF +2; TCF IRIG1; CA PRIO21;
 * TC NOVAC (2CADR 1/GYRO at Q); the -0 branch enters TC NOVAC with A +0. */
static enum exec_status gyro_job(const icp_mission *m,apollo_overlay_state *s,agc_word cadr_gyro){
 uint8_t c=ai_ccs(s,GCOMPSW);
 if(c==1u)return irig1(m,s);
 if(c!=3u)ai_caf(s,K_PRIO21);
 ai_tc(s,cadr_gyro);tcf(s,m->novac);return EXEC_OK;
}

/* ---- Luminary099 ---- */
/* 06,3263 1/PIPA (BANKCALL from SERVICER, P51-P53, IMU_PERFORMANCE_TESTS_4). */
static enum exec_status lm_1pipa(const agc_native_program *p,apollo_overlay_state *s,void *io){const icp_mission *m=&LM;uint8_t c;(void)p;(void)io;
 save_ebank(s);
 c=ai_ccs(s,GCOMPSW);
 if(c==2u)return irig1(m,s);
 pipa_loop(m,s);
 /* IRIGCOMP */
 (void)ai_ts(s,GCOMPSW);(void)ai_ts(s,BUF);
 ai_tc(s,03333u);lm_irig(s,0,LM_IRIGX);ai_cs(s,NBDX);ai_tc(s,03335u);driftsub(m,s);
 ai_tc(s,03336u);lm_irig(s,1,LM_IRIGY);ai_cs(s,NBDY);ai_tc(s,03340u);driftsub(m,s);
 ai_tc(s,03341u);lm_irig(s,2,LM_IRIGZ);ai_ca(s,NBDZ);ai_tc(s,03343u);driftsub(m,s);
 return gyro_job(m,s,03350u);
}
/* 06,3352 after NOVAC: RELINT; IRIG1. */
static enum exec_status lm_novac_return(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;interrupts(&LM,io,1);return irig1(&LM,s);}
/* 06,3507 1/GYRO (the job NOVAC admits; also TCF from NBDONLY/LASTBIAS). */
static enum exec_status lm_1gyro(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;return gyro(&LM,s,03534u);}
/* 06,3535 after IMUPULSE: TC BANKCALL with CADR IMUSTALL at Q. */
static enum exec_status lm_imupulse_return(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;ai_tc(s,03536u);tcf(s,LM.bankcall);return EXEC_OK;}
/* 06,3537 after IMUSTALL: TCF ENDOFJOB. */
static enum exec_status lm_imustall_return(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;tcf(s,LM.endofjob);return EXEC_OK;}
static enum exec_status lm_gcomp1(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;return gcomp1(&LM,s);}
/* NBD2 through the end of NBDONLY (Luminary): AD HALF; AD HALF; XCH L;
 * XCH L; NBD3 MP BIT10; DXCH VBUF +2; CA ZERO; TS GCOMPSW; TS BUF; then per
 * axis CCS TEM1 / TC IRIGx, the DELTAT pair into MPAC (DCA, DCS, DCS
 * VBUF +2) and TC FBIASSUB with -NBDX, +NBDY, -NBDZ; CCS GCOMPSW;
 * TCF 1/GYRO; TCF ENDOFJOB. TEM1 is +0 or BIT8. */
static enum exec_status lm_nbd2(apollo_overlay_state *s){const icp_mission *m=&LM;uint8_t c;
 ai_adf(s,K_HALF);ai_adf(s,K_HALF);ai_xch(s,1);ai_xch(s,1);
 ai_mpf(s,K_BIT10);ai_dxch(s,VBUF+2u);ai_caf(s,0);(void)ai_ts(s,GCOMPSW);(void)ai_ts(s,BUF);
 if(ai_ccs(s,TEM1)==0u){ai_tc(s,03616u);lm_irig(s,0,LM_IRIGX);}
 ai_dca(s,VBUF+2u);ai_dxch(s,MPAC);ai_cs(s,NBDX);ai_tc(s,03623u);fbiassub(s);
 if(ai_ccs(s,TEM1)==0u){ai_tc(s,03625u);lm_irig(s,1,LM_IRIGY);}
 ai_dcs(s,VBUF+2u);ai_dxch(s,MPAC);ai_ca(s,NBDY);ai_tc(s,03632u);fbiassub(s);
 if(ai_ccs(s,TEM1)==0u){ai_tc(s,03634u);lm_irig(s,2,LM_IRIGZ);}
 ai_dcs(s,VBUF+2u);ai_dxch(s,MPAC);ai_cs(s,NBDZ);ai_tc(s,03641u);fbiassub(s);
 /* GCOMPSW is +0 or positive here (DRFTSUB2 stores only CCS results). */
 c=ai_ccs(s,GCOMPSW);
 if(c==0u)return gyro(m,s,03534u);
 tcf(s,m->endofjob);return EXEC_OK;
}
/* 06,3555 NBDONLY (job from WAITLIST CKIMUSE): CCS GCOMPSW; TCF +3; TCF +2;
 * TCF ENDOFJOB; INHINT; CCS FLAGWRD2; TCF ENDOFJOB; TCF ENDOFJOB; TCF +1;
 * CA FLAGWRD8; MASK BIT8; TS TEM1; EXTEND; BZF +3; TC IBNKCALL
 * (CADR PIPASR +3 at Q); then the continuation at 06,3575. */
static enum exec_status lm_pipasr_return(const agc_native_program *,apollo_overlay_state *,void *);
static enum exec_status lm_nbdonly(const agc_native_program *p,apollo_overlay_state *s,void *io){uint8_t c;
 c=ai_ccs(s,GCOMPSW);
 if(c==2u){tcf(s,LM.endofjob);return EXEC_OK;}
 interrupts(&LM,io,0);
 c=ai_ccs(s,FLAGWRD2);
 if(c<2u){tcf(s,LM.endofjob);return EXEC_OK;}
 ai_ca(s,FLAGWRD8);ai_maskf(s,K_BIT8);(void)ai_ts(s,TEM1);
 if(ai_bzf(s))return lm_pipasr_return(p,s,io);
 ai_tc(s,03574u);tcf(s,LM_ICP_IBNKCALL);return EXEC_OK;
}
/* 06,3575: CA TIME1; XCH 1/PIPADT; RELINT; COM; AD 1/PIPADT; NBD2. */
static enum exec_status lm_pipasr_return(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;
 if(W[TEM1]!=0u&&W[TEM1]!=K_BIT8)return EXEC_OUT_OF_SCOPE;
 ai_ca(s,TIME1);ai_xch(s,LM.pipadt);interrupts(&LM,io,1);ai_com(s);ai_ad(s,LM.pipadt);
 return lm_nbd2(s);
}
/* 06,3663 LASTBIAS (job from SERVICER PREREAD): TC BANKCALL, CADR PIPUSE1. */
static enum exec_status lm_lastbias(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;ai_tc(s,03664u);tcf(s,LM.bankcall);return EXEC_OK;}
/* 06,3665: CCS GCOMPSW; TCF +3; TCF +2; TCF ENDOFJOB; CA FLAGWRD8;
 * MASK SURFFBIT; TS TEM1; CAF PRIO31; XCH 1/PIPADT; COM; AD PIPTIME +1;
 * TCF NBD2. */
static enum exec_status lm_pipuse1_return(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;
 if(ai_ccs(s,GCOMPSW)==2u){tcf(s,LM.endofjob);return EXEC_OK;}
 ai_ca(s,FLAGWRD8);ai_maskf(s,K_BIT8);(void)ai_ts(s,TEM1);
 ai_caf(s,K_PRIO31);ai_xch(s,LM.pipadt);ai_com(s);ai_ad(s,LM.piptime_low);
 return lm_nbd2(s);
}
/* 06,3701 GCOMPZER (BANKCALL from P51-P53): save EBANK; zero GCOMPSW and
 * GCOMP..GCOMP +5; IRIG1. */
static enum exec_status lm_gcompzer(const agc_native_program *p,apollo_overlay_state *s,void *io){unsigned i;(void)p;(void)io;
 save_ebank(s);ai_caf(s,0);(void)ai_ts(s,GCOMPSW);for(i=0;i<6u;++i)(void)ai_ts(s,GCOMP+i);
 return irig1(&LM,s);
}
#define B6(z) (uint16_t)(6u*1024u+((z)-02000u))
static const agc_routine_entry lm_entries[11]={
 {B6(LM_ICP_1PIPA),lm_1pipa},{B6(LM_ICP_NOVAC_RETURN),lm_novac_return},{B6(LM_ICP_1GYRO),lm_1gyro},
 {B6(LM_ICP_IMUPULSE_RETURN),lm_imupulse_return},{B6(LM_ICP_IMUSTALL_RETURN),lm_imustall_return},
 {B6(LM_ICP_GCOMP1),lm_gcomp1},{B6(LM_ICP_NBDONLY),lm_nbdonly},{B6(LM_ICP_PIPASR_RETURN),lm_pipasr_return},
 {B6(LM_ICP_LASTBIAS),lm_lastbias},{B6(LM_ICP_PIPUSE1_RETURN),lm_pipuse1_return},{B6(LM_ICP_GCOMPZER),lm_gcompzer}};
const agc_routine_table lm_imu_compensation_routines={lm_entries,11};

/* ---- Comanche055 ---- */
/* IRIGX/IRIGY/IRIGZ fall through each other in Comanche: two acceleration
 * terms, then CS/CA NBDx; TC DRIFTSUB. */
static void cm_irig(apollo_overlay_state *s){const icp_mission *m=&CM;
 accel(m,s,m->deltav,ADIAX,0,03340u);accel(m,s,DELVY_OF(m),ADSRAX,1,03345u);ai_cs(s,NBDX);ai_tc(s,03347u);driftsub(m,s);
 accel(m,s,DELVY_OF(m),ADIAY,0,03354u);accel(m,s,DELVZ_OF(m),ADSRAY,1,03361u);ai_cs(s,NBDY);ai_tc(s,03363u);driftsub(m,s);
 accel(m,s,DELVY_OF(m),ADSRAZ,0,03370u);accel(m,s,DELVZ_OF(m),ADIAZ,0,03375u);ai_ca(s,NBDZ);ai_tc(s,03377u);driftsub(m,s);
}
/* 06,3262 1/PIPA (BANKCALL from SERVICER207 and IMU_CALIBRATION_AND_ALIGNMENT):
 * save EBANK; CCS GCOMPSW (negative: IRIG1); INHINT; 1/PIPA1 loop; RELINT;
 * IRIGCOMP TS GCOMPSW; TS BUF; IRIGX..IRIGZ; CCS GCOMPSW ... TC NOVAC. */
static enum exec_status cm_1pipa(const agc_native_program *p,apollo_overlay_state *s,void *io){const icp_mission *m=&CM;uint8_t c;(void)p;
 save_ebank(s);
 c=ai_ccs(s,GCOMPSW);
 if(c==2u)return irig1(m,s);
 /* The -0 branch (06,3271) is the INHINT itself. */
 interrupts(m,io,0);
 pipa_loop(m,s);
 interrupts(m,io,1);
 (void)ai_ts(s,GCOMPSW);(void)ai_ts(s,BUF);
 cm_irig(s);
 return gyro_job(m,s,03404u);
}
static enum exec_status cm_novac_return(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;interrupts(&CM,io,1);return irig1(&CM,s);}
static enum exec_status cm_1gyro(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;return gyro(&CM,s,03521u);}
static enum exec_status cm_imupulse_return(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;ai_tc(s,03523u);tcf(s,CM.bankcall);return EXEC_OK;}
static enum exec_status cm_imustall_return(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;tcf(s,CM.endofjob);return EXEC_OK;}
static enum exec_status cm_gcomp1(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;return gcomp1(&CM,s);}
/* NBD2 (Comanche): CCS A; AD ONE; TCF NBD3; TCF +2; TCF ENDOFJOB; COM;
 * AD POSMAX; NBD3 MP BIT10; DXCH VBUF; DCA VBUF; DXCH MPAC; CAF ZERO;
 * TS GCOMPSW; TS BUF; CS NBDX; TC FBIASSUB; DCS VBUF; DXCH MPAC; CA NBDY;
 * TC FBIASSUB; DCS VBUF; DXCH MPAC; CS NBDZ; TC FBIASSUB; CCS GCOMPSW;
 * TCF 1/GYRO; TCF ENDOFJOB. */
static enum exec_status cm_nbd2(apollo_overlay_state *s){const icp_mission *m=&CM;uint8_t c;
 /* + : AD ONE; TCF NBD3. +0: TCF NBD3. - : TCF +2; COM; AD POSMAX.
  * -0: TCF ENDOFJOB. */
 c=ai_ccs(s,0);
 if(c==0u)ai_adf(s,K_BIT1);
 else if(c==2u){ai_com(s);ai_adf(s,K_POSMAX);}
 else if(c==3u){tcf(s,m->endofjob);return EXEC_OK;}
 ai_mpf(s,K_BIT10);ai_dxch(s,VBUF);ai_dca(s,VBUF);ai_dxch(s,MPAC);
 ai_caf(s,0);(void)ai_ts(s,GCOMPSW);(void)ai_ts(s,BUF);
 ai_cs(s,NBDX);ai_tc(s,03602u);fbiassub(s);
 ai_dcs(s,VBUF);ai_dxch(s,MPAC);ai_ca(s,NBDY);ai_tc(s,03607u);fbiassub(s);
 ai_dcs(s,VBUF);ai_dxch(s,MPAC);ai_cs(s,NBDZ);ai_tc(s,03614u);fbiassub(s);
 c=ai_ccs(s,GCOMPSW);
 if(c==0u)return gyro(m,s,03521u);
 tcf(s,m->endofjob);return EXEC_OK;
}
/* 06,3542 NBDONLY: CCS GCOMPSW; ...; INHINT; CCS FLAGWRD2; ...; CA TIME1;
 * XCH 1/PIPADT; RELINT; COM; AD 1/PIPADT; NBD2. */
static enum exec_status cm_nbdonly(const agc_native_program *p,apollo_overlay_state *s,void *io){uint8_t c;(void)p;
 c=ai_ccs(s,GCOMPSW);
 if(c==2u){tcf(s,CM.endofjob);return EXEC_OK;}
 interrupts(&CM,io,0);
 c=ai_ccs(s,FLAGWRD2);
 if(c<2u){tcf(s,CM.endofjob);return EXEC_OK;}
 ai_ca(s,TIME1);ai_xch(s,CM.pipadt);interrupts(&CM,io,1);ai_com(s);ai_ad(s,CM.pipadt);
 return cm_nbd2(s);
}
static enum exec_status cm_lastbias(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;ai_tc(s,03637u);tcf(s,CM.bankcall);return EXEC_OK;}
/* 06,3640: CCS GCOMPSW; TCF +3; TCF +2; TCF ENDOFJOB; CAF PRIO31;
 * XCH 1/PIPADT; COM; AD PIPTIME1 +1; TCF NBD2. */
static enum exec_status cm_pipuse_return(const agc_native_program *p,apollo_overlay_state *s,void *io){(void)p;(void)io;
 if(ai_ccs(s,GCOMPSW)==2u){tcf(s,CM.endofjob);return EXEC_OK;}
 ai_caf(s,K_PRIO31);ai_xch(s,CM.pipadt);ai_com(s);ai_ad(s,CM.piptime_low);
 return cm_nbd2(s);
}
static const agc_routine_entry cm_entries[9]={
 {B6(CM_ICP_1PIPA),cm_1pipa},{B6(CM_ICP_NOVAC_RETURN),cm_novac_return},{B6(CM_ICP_1GYRO),cm_1gyro},
 {B6(CM_ICP_IMUPULSE_RETURN),cm_imupulse_return},{B6(CM_ICP_IMUSTALL_RETURN),cm_imustall_return},
 {B6(CM_ICP_GCOMP1),cm_gcomp1},{B6(CM_ICP_NBDONLY),cm_nbdonly},{B6(CM_ICP_LASTBIAS),cm_lastbias},
 {B6(CM_ICP_PIPUSE_RETURN),cm_pipuse_return}};
const agc_routine_table cm_imu_compensation_routines={cm_entries,9};
