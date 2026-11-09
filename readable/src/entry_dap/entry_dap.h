/* SPDX-License-Identifier: MIT */
/* CM_ENTRY_DIGITAL_AUTOPILOT.agc, the CM's (Comanche055 pp. 1063-1092):
 * the entry DAP, which flies the command module through entry once the
 * service module is gone. The faithful C is c64-port/src/cm_entry_dap.c.
 *
 * Every 0.1 s READGYMB reads the gimbal angles and BODYRATE turns their
 * changes into the body rates P, Q and R; the attitude DAP (EXDAPIN) and
 * its jet calls (SETJTAG, CM/RCS) hold the commanded roll and damp pitch
 * and yaw, every 2 s the roll system (TIMETST, JETCALL) times the roll
 * jets, and CM/FDAI drives the needles. CM/DAPON (P61-P67) turns the
 * service module DAP off and starts it all; CM/DAPIC starts the 0.1 s
 * cycle.
 *
 * All basic code. The faithful port's code is in parts (13 of them), and
 * a run stops when control reaches a key of another part. Each function
 * here runs from a key; a TC or jump to another key leaves Z there, and
 * the binding (src/binding/cm_entry_dap_routines.c) goes on at a key of
 * the same part of the faithful port's, as its run does.
 *
 * Written in six parts (README.md):
 *   A: readgymb.c, READGYMB to ANGOVCOR and FLUSHJET (pp. 1063-1065, bank
 *     15 and fixed-fixed); dapon.c, CM/DAPON and CM/DAPIC (pp. 1066-1069,
 *     bank 20).
 *   B: bodyrate.c, RATEAVG and BODYRATE (pp. 1065 and 1070-1072);
 *     attrates.c, the flight path's turn, NOGAMDUT and ATTRATES (pp.
 *     1071-1074), bank 15.
 *   C: atm_dap.c, the yaw and pitch DAPs in the atmosphere and EXDAPIN
 *     (pp. 1074-1075); dead_zone.c, the dead zone tests (p. 1076), bank
 *     15.
 *   D: exdap.c, the extra atmospheric DAP, EXDAP to EXDAP2 (pp.
 *     1077-1079), bank 15.
 *   E: cm_rcs.c, SETJTAG and CM/RCS to OVRLINE1 (pp. 1081-1085), bank
 *     15.
 *   F: roll_jets.c, GETOFF, TIMETST, JETCALL and DZCALL (pp. 1085-1089);
 *     cm_fdai.c, DZCALL +3 to CM/END (pp. 1089-1091), bank 15. */
#ifndef READABLE_ENTRY_DAP_H
#define READABLE_ENTRY_DAP_H

#include "../agc/agc.h"

#if COMANCHE055
#include "../tvc/tvc.h" /* tc_to, tc_q */

/* ---- Part A: readgymb.c (bank 15, FLUSHJET in fixed-fixed) ---- */
void flushjet(void);                 /* FLUSHJET */
void readgymb(void);                 /* READGYMB */
void readgymb_after_flushjet(void);  /* READGYMB +012 */
void readgym1(void);                 /* READGYM1 */
void dobrate_q_ok(void);             /* DOBRATE? +4 */
void dobrate_q_first_pass(void);     /* DOBRATE? +5 */
void dobrate_q_noop(void);           /* DOBRATE? +6 */
void dobrate_q_standby(void);        /* DOBRATE? +7 */
void dobrate_q_group6(void);         /* DOBRATE? +010 */
void dobrate_q_after_phaschng(void); /* DOBRATE? +012 */
void dobrate1_after_waitlist(void);  /* DOBRATE1 +5 */
void dobrate1_taskover(void);        /* DOBRATE1 +7 */
void angovcor(void);                 /* ANGOVCOR */
void angovcor_return(void);          /* ANGOVCOR +5, part B's key */

/* ---- Part A: dapon.c (bank 20) ---- */
void cm_dapon(void);                /* CM/DAPON */
void cm_dapon_after_dapbit1(void);  /* CM/DAPON +4 */
void cm_dapon_after_dapbit2(void);  /* CM/DAPON +6 */
void cm_dapon_t5loc(void);          /* CM/DAPON +010 */
void cm_dapon_after_flushjet(void); /* CM/DAPON +015 */
void cm_dapon_plus4_tc(void);       /* CM/DAPON +020 */
void notyet(void);                  /* NOTYET */
void notyet_bankcall(void);         /* NOTYET +1 */
void notyet_plus4(void);            /* NOTYET +3, the listing's "+4" */
void notyet_alfacom(void);          /* NOTYET +020 */
void cm_dapic(void);                /* CM/DAPIC */
void cm_dap2c_jetem(void);          /* CM/DAP2C +5 */
void cm_dap2c_2cadr(void);          /* CM/DAP2C +020 */
void cm_dap2c_after_waitlist(void); /* CM/DAP2C +021 */
void cm_dap2c_beta(void);           /* CM/DAP2C +025 */
void cm_dap2c_beta_ts(void);        /* CM/DAP2C +026 */
void cm_dap2c_2phschng(void);       /* CM/DAP2C +031 */
void cm_dap2c_after_2phschng(void); /* CM/DAP2C +035 */

/* ---- Part B: bodyrate.c and attrates.c (bank 15) ---- */
void bodyrate(void);           /* BODYRATE */
void bodyrate_cosm(void);      /* BODYRATE +2 */
void bodyrate_sino(void);      /* BODYRATE +5 */
void bodyrate_coso(void);      /* BODYRATE +013 */
void bodyrate_qrel(void);      /* BODYRATE +032 */
void bodyrate_rrel(void);      /* BODYRATE +046 */
void bodyrate_sinm(void);      /* BODYRATE +051 */
void bodyrate_prel(void);      /* BODYRATE +065 */
void bodyrate_nogamdut(void);  /* BODYRATE +070 */
void bodyrate_gamdot(void);    /* BODYRATE +071 */
void bodyrate_sr_gamdot(void); /* BODYRATE +073 */
void bodyrate_trim_prel(void); /* BODYRATE +0100 */
void bodyrate_trim_qrel(void); /* BODYRATE +0103 */
void nogamdut(void);           /* NOGAMDUT */
void attrates(void);           /* ATTRATES */
void attrates_alfa(void);      /* ATTRATES +011 */
void attrates_calfa(void);     /* ATTRATES +013 */
void attrates_salfa(void);     /* ATTRATES +025 */
void attrates_sb_phidot(void); /* ATTRATES +037 */
void attrates_alfa_done(void); /* ATTRATES +043 */
void attrates_ak1(void);       /* ATTRATES +047 */
void attrates_rolltm(void);    /* ATTRATES +054 */

/* ---- Part C: atm_dap.c and dead_zone.c (bank 15) ---- */
void atm_cmdapmod(void);              /* ATTRATES +066 */
void atm_ak1(void);                   /* ATTRATES +067 */
void atm_yjetcode(void);              /* ATTRATES +076 */
void exdapin(void);                   /* EXDAPIN */
void exdapin_fdai(void);              /* EXDAPIN +7 */
void exdapin_fdair(void);             /* EXDAPIN +010 */
void three_d_dz(void);                /* 3DDZ */
void biasedz(void);                   /* BIASEDZ */
void biasedz_after_3ddz(void);        /* BIASEDZ +010 */
void two_d_sdz(void);                 /* 2D/SDZ (no key: only called) */
void jet_code(word table);            /* INDEX A; CAF table */
void index_a_ca(word base);           /* INDEX A; CA base */
void index_a_cs(word base);           /* INDEX A; CS base */
void index_ca(unsigned k, word base); /* INDEX k; CA base */

/* ---- Part D: exdap.c (bank 15) ---- */
void exdap(void);                /* EXDAP */
void exdap_tcf(void);            /* EXDAP +3 */
void exdap_ad_c45lim(void);      /* EXDAP +4 */
void exdap_ts_a(void);           /* EXDAP +5 */
void exdap_p63_plus0(void);      /* EXDAP +013 */
void exdap_p63_negative(void);   /* EXDAP +014 */
void exdap_phaschng(void);       /* EXDAP +015 */
void exdap_after_phaschng(void); /* EXDAP +017 */
void exdap4(void);               /* EXDAP4 */
void exdap1(void);               /* EXDAP1 */
void exdap1_mode0(void);         /* EXDAP1 +5 */
void exdap1_after_biasedz(void); /* EXDAP1 +011 */
void exdap1_after_alfa_dz(void); /* EXDAP1 +022 */
void exdap2_p63flag(void);       /* EXDAP2 +3 */
void exdap2_paxerr1(void);       /* EXDAP2 +013 */
void exdap2_betadot(void);       /* EXDAP2 +022 */
void exdap2_after_biasedz(void); /* EXDAP2 +024 */
void exdap2_cs_l(void);          /* EXDAP2 +041 */
void nsec_constant(void);        /* EXDAP2 +043, NSEC */

/* ---- Part E: cm_rcs.c (bank 15) ---- */
void setjtag(void);            /* SETJTAG */
void setjtag_taskover(void);   /* SETJTAG +6 */
void cm_rcs(void);             /* CM/RCS */
void cm_rcs_vt_positive(void); /* GETLCX +012 */
void cm_rcs_vt_plus0(void);    /* GETLCX +013 */
void cm_rcs_vt_negative(void); /* GETLCX +014 */
void cm_rcs_shortest(void);    /* GETLCX +015 */
void cm_rcs_cs_pos1_2(void);   /* GETLCX +027 */
void cm_rcs_ad_lcx(void);      /* GETLCX +030 */
void compat(void);             /* COMPAT */
void compat_tc(void);          /* COMPAT +6 */
void compat_ca(void);          /* COMPAT +7 */
void reflect(void);            /* REFLECT */
void reflect_ts(void);         /* REFLECT +1 */
void reflect_ts_sr(void);      /* REFLECT +2 */
void reflect_jndx(void);       /* REFLECT +5 */
void dz1(void);                /* DZ1 */
void maxvtest_ad(void);        /* MAXVTEST +3 */
void maxvtim1(void);           /* MAXVTIM1 */
void ovrline(void);            /* OVRLINE */
void ovrline1(void);           /* OVRLINE1 */

/* ---- Part F: roll_jets.c and cm_fdai.c (bank 15) ---- */
void getoff(void);                 /* GETOFF */
void getoff_mp(void);              /* GETOFF +1 */
void getoff_ts(void);              /* GETOFF +3 */
void getoff_mp_ton1(void);         /* GETOFF +010 */
void toffovfl(void);               /* TOFFOVFL */
void timscal_ton1(void);           /* TIMSCAL +2 */
void ton2test(void);               /* TON2TEST */
void ton2test_t2bits(void);        /* TON2TEST +4 */
void ton2test_zero(void);          /* TON2TEST +013 */
void timetst3(void);               /* TIMETST3 */
void timetst4(void);               /* TIMETST4 */
void timetst5(void);               /* TIMETST5 */
void jetcall1(void);               /* JETCALL1 */
void jetcall3(void);               /* JETCALL3 */
void jetcall3_cs(void);            /* JETCALL3 +1 */
void jetcall3_toff(void);          /* JETCALL3 +2 */
void jetcall2(void);               /* JETCALL2 */
void jetactn(void);                /* JETACTN */
void rolldump(void);               /* ROLLDUMP */
void jetcall(void);                /* JETCALL */
void dzcall(void);                 /* DZCALL */
void dzcall_rolltm(void);          /* DZCALL +3 */
void dzcall_ts_a(void);            /* DZCALL +5 */
void dzcall_ad_l(void);            /* DZCALL +7 */
void dzcall_rollhold(void);        /* DZCALL +011 */
void dzcall1(void);                /* DZCALL1 */
void cm_fdai(void);                /* CM/FDAI */
void cm_fdai_mp(void);             /* CM/FDAI +1 */
void cm_fdai_ads(void);            /* CM/FDAI +5 */
void cm_fdair(void);               /* CM/FDAIR */
void cm_dumpr_after_needler(void); /* CM/DUMPR +6 */
void cmtmfile(void);               /* CMTMFILE */
void cmtmfile_ts(void);            /* CMTMFILE +4 */
void savendx(void);                /* SAVENDX */
void cm_end(void);                 /* CM/END */

#endif

#endif
