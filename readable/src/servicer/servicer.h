/* SPDX-License-Identifier: MIT */
/* SERVICER.agc (Luminary099 pp. 857-897): the average-G loop. Every two
 * seconds of a powered flight (or of a landing's last minutes) it reads
 * the accelerometers (the PIPAs) and the CDUs, updates the state vector
 * with the measured velocity change and the computed gravity, watches the
 * thrust, and hands over to the guidance through AVGEXIT. In the order of
 * the listing:
 *
 *   PREREAD, READACCS, AVEGOUT (readaccs.c, bank 37, pp. 857-859): the
 *     start, and the two-second task that reads the PIPAs and sets up the
 *     SERVICER job.
 *   SERVICER (servicer.c, bank 33, pp. 860-863): the job. The velocity
 *     change's magnitude, the mass, average G (AVERAGEG), the delta-V
 *     monitor, and the exit to the guidance (AVGEXIT).
 *   AVGEND, SERVIDLE, SERVEXIT (avgend.c, banks 22, 27 and 32, pp.
 *     863-866): the end of average G, and SERVICER kept running idle.
 *   NORMLIZE, COPYCYC (normlize.c, banks 23 and 33, pp. 867-868): the
 *     first pass, and the copy of the new state into the old.
 *   PIPASR, REREADAC, TMPTOSPT, LRHTASK (pipa_reader.c, banks 37, 33 and
 *     21, pp. 869-872): the PIPA reader and its restart, and the landing
 *     radar's altitude task.
 *   HIGATASK, MUNRETRN, R29?, NOR29NOW, ALTCHK (munretrn.c, bank 33, pp.
 *     873-879): the landing's part of each pass, the landing radar checks
 *     and the altitude and state for R10.
 *   CALCGRAV, CALCRVG, RVBOTH, MUNRVG, MUNGRAV (average_g.c, bank 33, pp.
 *     880-883): the average G integration.
 *   UPDATCHK, POSUPDAT, VELUPDAT, VUPDAT, VALTCHK, GNURVST, QUIKFAZ5
 *     (lr_update.c, bank 33, pp. 884-891): the landing radar's
 *     measurements checked and weighted into the state, and the restart
 *     point.
 *   LRVJOB, LRHJOB, RDGIMS, HIGATJOB, SETPOS (lr_jobs.c, banks 33 and 34,
 *     pp. 892-897): the landing radar's reading jobs and task, the move to
 *     position 2, and the beams' vectors.
 *
 * The faithful C is c64-port/src/lm_servicer.c.
 *
 * Each function runs from a word the program can resume at (an entry, a
 * subroutine's return, the word after an EXIT) until its code leaves it:
 * at a TC or TCF out of the file (Z the routine, Q the return), at a TC
 * to one of the file's own subroutines (Z that subroutine, Q the return,
 * which is a function of its own), at a TC INTPRET (run on to NEWOPS), or
 * at a transfer to an address the code computes (a TC Q, TC A, TC
 * LRADRET, SERVOUT's DXCH Z). The binding, src/binding/servicer_routines.c, runs on from there
 * while the address is the file's. */
#ifndef READABLE_SERVICER_H
#define READABLE_SERVICER_H

#include "../agc/agc.h"

#if LUMINARY099
#include "../agc/names.h"
#include "../interpreter/notation.h"

#ifndef COUNT
#define COUNT(array) (sizeof(array) / sizeof((array)[0]))
#endif

/* TC at, to a routine: Q gets the return and the routine runs next. */
static inline void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* TC Q: back to the word after the TC that called; TC A, to the address
 * in A; TC to an erasable word that holds a return (servicer.c). */
void return_to_q(void);
void return_to_a(void);
void tc_through_computed(unsigned address);

/* EXTEND; DCA of a two-word constant of the file. */
static inline void dca_constant(const word pair[2])
{
    set_L(pair[1]);
    set_A(pair[0]);
}

/* EXTEND; DCA NEG0; DXCH -PHASEn: restart group n off. DCA NEG0 reads
 * NEG0 and the word after it, ZERO: -PHASEn gets -0 and PHASEn +0. */
static inline void group_off(unsigned minus_phase)
{
    set_L(ZERO);
    set_A(NEG0);
    dxch(minus_phase);
}

/* The interpretive code. */
extern const struct interpretive_code GETABVAL_CODE, AVERAGEG_CODE, AVGEND_CODE, NORMLIZE_CODE,
    NORMLIZ1_CODE, MUNRETRN_CODE, NOR29NOW_CODE, CALCGRAV_CODE, CALCRVG_CODE, RVBOTH_CODE,
    RVBOTH_2_CODE, MUNRVG_CODE, MUNGRAV_CODE, POSUPDAT_CODE, POSUPDAT_2_CODE, POSUPDAT_3_CODE,
    NOREASON_CODE, NOREASON_2_CODE, VELUPDAT_CODE, VELUPDAT_2_CODE, WSTOR_CODE, SETPOS_CODE;

/* ---- readaccs.c: pp. 857-859 ---- */
void preread(void);                 /* PREREAD */
void preread_after_gnufaze5(void);  /* its TC GNUFAZE5's return */
void bibibias(void);                /* BIBIBIAS */
void bibibias_after_pipasr(void);   /* its TC PIPASR +3's return */
void bibibias_after_findvac(void);  /* (the job's 2CADR NORMLIZE) */
void goreadax(void);                /* GOREADAX */
void goreadax_after_gnutfaz5(void); /* its TC GNUTFAZ5's return */
void readaccs(void);                /* READACCS */
void pipsdone(void);                /* PIPSDONE: TC PIPASR's return */
void redo5_5(void);                 /* REDO5.5: TC GNUFAZE5's return */
void redo5_5_after_findvac(void);   /* (the job's 2CADR SERVICER) */
void makeaccs(void);                /* MAKEACCS */
void avegout(void);                 /* AVEGOUT */
void avegout_after_gnutfaz5(void);  /* its TC GNUTFAZ5's return */
void gnutfaz5(void);                /* GNUTFAZ5 */
void gnufaze5(void);                /* GNUFAZE5 */

/* ---- servicer.c: pp. 860-863 ---- */
void servicer(void);                /* SERVICER */
void servicer_after_phaschng(void); /* its OCT 16035's return */
void getabval(void);                /* GETABVAL: TC INTPRET */
void getabval_after_exit(void);     /* the word after its EXIT */
void massmon(void);                 /* MASSMON */
void moonspot(void);                /* MOONSPOT */
void moonspot_after_shortmp(void);  /* TC SHORTMP's return */
void moonspot_after_tmptospt(void); /* TC TMPTOSPT's return */
void moonspot_after_quictrig(void); /* (BANKCALL QUICTRIG) */
void moonspot_after_fleshpot(void); /* (BANKCALL FLESHPOT): TC INTPRET */
void goserv(void);                  /* GOSERV: AVERAGEG's EXIT */
void copycycl(void);                /* COPYCYCL: TC QUIKFAZ5's return */
void copycycl_after_copycyc(void);  /* TC COPYCYC's return */
void dvmon(void);                   /* DVMON */
void dvcntset(void);                /* DVCNTSET */
void usegts(void);                  /* USEGTS */
void nodvmon1(void);                /* NODVMON1 */
void nodvmon2(void);                /* NODVMON2 */
void lothrust(void);                /* LOTHRUST */
void lothrust_after_quikfaz5(void); /* its TC QUIKFAZ5's return */
void lothrust_after_phaschng(void); /* its OCT 00374's return */
void lothrust_after_novac(void);    /* (the job's 2CADR COMFAIL) */
void deccntr(void);                 /* DECCNTR */
void deccntr_after_quikfaz5(void);  /* its TC QUIKFAZ5's return */
void usejets(void);                 /* USEJETS: (IBNKCALL STOPRATE) */
void servout(void);                 /* SERVOUT */
void servout_after_1_accs(void);    /* (BANKCALL 1/ACCS) */
void servout_after_quikfaz5(void);  /* its TC QUIKFAZ5's return */

/* ---- avgend.c: pp. 863-866 ---- */
void avgend(void);                  /* AVGEND */
void avgend_after_upflag(void);     /* (ADRES DRIFTFLG) */
void avgend_after_pipfree(void);    /* (BANKCALL PIPFREE) */
void avgend_after_2phschng(void);   /* its OCT 20000's return: TC INTPRET */
void avertrn(void);                 /* AVERTRN: the EXIT's word */
void servidle(void);                /* SERVIDLE */
void servexit(void);                /* SERVEXIT */
void servexit_after_phaschng(void); /* SERVEXIT +2 */

/* ---- normlize.c: pp. 867-868 ---- */
void normlize(void);               /* NORMLIZE: TC INTPRET */
void ascspot_after_exit(void);     /* the word after ASCSPOT's EXIT */
void normliz2(void);               /* NORMLIZ2: NORMLIZ1's EXIT */
void normliz2_after_copycyc(void); /* TC COPYCYC +1's return */
void copycyc(void);                /* COPYCYC */
void copycyc_plus_1(void);         /* COPYCYC +1 (NORMLIZ2's TC) */

/* ---- pipa_reader.c: pp. 869-872 ---- */
void pipasr(void);        /* PIPASR */
void pipasr_plus_3(void); /* PIPASR +3 */
void repip1(void);        /* REPIP1 */
void repip3(void);        /* REPIP3 */
void dodelvz(void);       /* DODELVZ */
void repip4(void);        /* REPIP4 */
void rereadac(void);      /* REREADAC */
void chktemx(void);       /* CHKTEMX */
void tmptospt(void);      /* TMPTOSPT */
void lrhtask(void);       /* LRHTASK */
void grp2off(void);       /* GRP2OFF: (the job's 2CADR LRHJOB) */

/* ---- munretrn.c: pp. 873-879 ---- */
void higatask(void);                      /* HIGATASK */
void higatask_after_findvac(void);        /* (the job's 2CADR HIGATJOB) */
void munretrn_after_exit(void);           /* the word after MUNRETRN's EXIT */
void hitest(void);                        /* HITEST */
void pos2chk(void);                       /* POS2CHK */
void higatchk(void);                      /* HIGATCHK */
void pos1chk(void);                       /* POS1CHK */
void lrposalm(void);                      /* LRPOSALM */
void contserv(void);                      /* CONTSERV: TC ALARM's return */
void copycyc1(void);                      /* COPYCYC1: (IBNKCALL R12LITES) */
void r29_q(void);                         /* R29?: TC QUIKFAZ5's return */
void r29nodes(void);                      /* R29NODES */
void nor29now(void);                      /* NOR29NOW: TC INTPRET */
void copycyc2_after_exit(void);           /* the word after COPYCYC2's EXIT */
void altchk(void);                        /* ALTCHK */
void altchk_after_branch(void);           /* TC BRANCH +4's return, positive */
void altchk_after_branch_zero(void);      /* its return on +0 */
void altchk_after_branch_negative(void);  /* its return, negative */
void xorchk(void);                        /* XORCHK */
void xorchk_after_altchk(void);           /* (BANKCALL ALTCHK), above */
void xorchk_below(void);                  /* its return a word on, below */
void xorchk_after_xovinflg(void);         /* (ADRES XOVINFLG) */
void xorchk_after_xorflg(void);           /* (ADRES XORFLG) */
void thirty_five_kchk(void);              /* 35KCHK */
void thirty_five_kchk_after_altchk(void); /* (BANKCALL ALTCHK), above */
void thirty_five_kchk_below(void);        /* its return a word on, below */
void thirty_five_kchk_after_readlr(void); /* (ADRES READLR) */

/* ---- average_g.c: pp. 880-883 ---- */
void rvboth_after_exit(void);            /* the word after RVBOTH's EXIT */
void rvboth_after_quikfaz5(void);        /* its TC QUIKFAZ5's return: TC INTPRET */
void rvboth_after_second_exit(void);     /* the word after the second EXIT */
void rvboth_after_second_quikfaz5(void); /* its TC QUIKFAZ5's return: TC INTPRET */

/* ---- lr_update.c: pp. 884-891 ---- */
void updatchk(void);                       /* UPDATCHK */
void posupdat(void);                       /* POSUPDAT: TC INTPRET */
void posupdat_after_exit(void);            /* the word after its EXIT */
void posupdat_after_shortmp(void);         /* +3: TC SHORTMP's return, TC INTPRET */
void posupdat_after_second_exit(void);     /* the word after the second EXIT */
void posupdat_after_third_exit(void);      /* the word after the third EXIT */
void posupdat_after_branch(void);          /* TC BRANCH's return, positive */
void posupdat_after_branch_zero(void);     /* its return on +0 */
void posupdat_after_branch_negative(void); /* its return, negative */
void noreason(void);                       /* NOREASON: (ADRES HFLSHFLG) */
void noreason_after_exit(void);            /* the word after its EXIT */
void noreason_after_alsignag(void);        /* TC ALSIGNAG's return */
void noreason_after_shortmp2(void);        /* TC SHORTMP2's return: TC INTPRET */
void noreason_after_second_exit(void);     /* the word after the second EXIT */
void noreason_after_quikfaz5(void);        /* its TC QUIKFAZ5's return */
void rupdated(void);                       /* RUPDATED */
void vmeaschk(void);                       /* VMEASCHK: TC GNURVST's return */
void vmeaschk_after_quikfaz5(void);        /* its TC QUIKFAZ5's return */
void velupdat(void);                       /* VELUPDAT */
void velupdat_after_quictrig(void);        /* (BANKCALL QUICTRIG): TC INTPRET */
void velupdat_after_exit(void);            /* the word after its EXIT: TC INTPRET */
void velupdat_after_second_exit(void);     /* the word after the second EXIT */
void velupdat_after_branch(void);          /* TC BRANCH's return, positive */
void velupdat_after_branch_zero(void);     /* its return on +0 */
void velupdat_after_branch_negative(void); /* its return, negative */
void velupdat_after_vflshflg(void);        /* (ADRES VFLSHFLG) */
void velupdat_after_vxinh(void);           /* (ADRES VXINH) */
void vupdat(void);                         /* VUPDAT */
void usevf(void);                          /* USEVF */
void wstor(void);                          /* WSTOR: TC INTPRET */
void wstor_after_exit(void);               /* the word after its EXIT */
void wstor_after_quikfaz5(void);           /* its TC QUIKFAZ5's return */
void vupdated(void);                       /* VUPDATED */
void valtchk(void);                        /* VALTCHK: TC GNURVST's return */
void valtchk_after_quikfaz5(void);         /* its TC QUIKFAZ5's return */
void readv(void);                          /* READV: (ADRES READVEL) */
void readv_after_novac(void);              /* (the job's 2CADR LRVJOB) */
void gnurvst(void);                        /* GNURVST */
void quikfaz5(void);                       /* QUIKFAZ5 */
void hfail(void);                          /* HFAIL */
void norlite(void);                        /* NORLITE: (ADRES HFLSHFLG) */
void vfail(void);                          /* VFAIL */
void nolite(void);                         /* NOLITE: (ADRES VFLSHFLG) */
void nolite_after_vxinh(void);             /* (ADRES VXINH) */

/* ---- lr_jobs.c: pp. 892-897 ---- */
void lrvjob(void);                    /* LRVJOB */
void lrvjob_after_waitlist(void);     /* (the task's 2CADR RDGIMS) */
void lrvjob_after_lrvel(void);        /* (BANKCALL LRVEL) */
void lrvjob_bad_exit(void);           /* (BANKCALL RADSTALL), bad */
void lrvjob_good_exit(void);          /* its return a word on, good */
void endlrv(void);                    /* ENDLRV */
void vbad(void);                      /* VBAD */
void vstilbad(void);                  /* VSTILBAD */
void lrhjob(void);                    /* LRHJOB */
void lrhjob_after_lralt(void);        /* (BANKCALL LRALT) */
void lrhjob_bad_exit(void);           /* (BANKCALL RADSTALL), bad */
void lrhjob_good_exit(void);          /* its return a word on, good */
void endlrh(void);                    /* ENDLRH */
void hbad(void);                      /* HBAD */
void hbad_after_rngscflg(void);       /* (ADRES RNGSCFLG) */
void hbad_reset(void);                /* HSTILBAD -1 */
void hstilbad(void);                  /* HSTILBAD */
void rdgims(void);                    /* RDGIMS */
void higatjob(void);                  /* HIGATJOB */
void higatjob_after_lrpos2(void);     /* (BANKCALL LRPOS2) */
void higatjob_bad_exit(void);         /* (BANKCALL RADSTALL), bad */
void posgood(void);                   /* POSGOOD: its return a word on, good */
void posgood_after_priochng(void);    /* TC PRIOCHNG's return */
void posgood_after_setpos2(void);     /* TC SETPOS2's return */
void posgood_after_nolrread(void);    /* (ADRES NOLRREAD) */
void posalarm(void);                  /* POSALARM */
void posalarm_terminate(void);        /* (BANKCALL PRIOLARM), terminate */
void posalarm_proceed(void);          /* its proceed return */
void posalarm_enter(void);            /* its enter return */
void posalarm_immediate_return(void); /* its immediate return */
void posalarm_retry(void);            /* POSALARM +7 */
void setpos1(void);                   /* SETPOS1 */
void setpos1_after_makecadr(void);    /* TC MAKECADR's return */
void setpos1_after_setpos(void);      /* TC SETPOS's return */
void setpos2(void);                   /* SETPOS2 */
void setpos(void);                    /* SETPOS: TC INTPRET */
void setpos_after_exit(void);         /* the word after its EXIT */

#endif

#endif
