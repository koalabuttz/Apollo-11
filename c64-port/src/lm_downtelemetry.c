/* SPDX-License-Identifier: MIT */
/* Luminary099 DOWN-TELEMETRY_PROGRAM: DODOWNTM, WOTEST, the DNTMGOTO continuation,
 * DNPHASE1/2, NEWLIST, NEXTINCL, SETWO, DODNCHAN, WOZERO, DODNPTR, SNAPLOOP,
 * SNAPAGN, FETCH2WD, DOSUBLST/NEXTINSL, DNTMEXIT, DNDUMPI, DNDUMP, DNDUMP1,
 * DNDUMP2 and SENDID. One agc_image_ops call per original instruction, under
 * labels L<address> (bank 05); channel instructions act on lm_dntm_io as
 * yaAGC's READ/WOR/WAND/WRITE do. An INDEX whose target word is computed at
 * run time (the downlist pointers, DNCHAN's READ) forms the instruction word
 * as the hardware does and must yield the CA or READ the source intends;
 * anything else refuses. Fixed words read as data come from
 * lm_dntm_data, compiled from the listing. The instruction sequence is
 * Comanche055's (src/cm_downtelemetry.c) apart from WOTEST/WO1. */
#include "lm_downtelemetry.h"
#include "agc_image_ops.h"
#include "lm_downtelemetry_data.inc"

#define W s->words
#define B5(z) (uint16_t)(LM_DNTM_BANK*1024u+((z)-02000u))
#define FB05 (LM_DNTM_BANK<<10)
#define L 1u
#define Q 2u
#define EBANK 3u
#define QRUPT 012u
#define BANKRUPT 016u
#define TIME1 025u
#define DNLSTCOD LM_DNTM_DNLSTCOD
#define DUMPCNT LM_DNTM_DUMPCNT
#define CTLIST 0334u
#define DNTMGOTO LM_DNTM_DNTMGOTO
#define DNECADR 0336u     /* = TMINDEX = DUMPLOC */
#define TMINDEX DNECADR
#define DUMPLOC DNECADR
#define SUBLIST 0337u
#define DNTMBUFF 0340u
/* Fixed words the program reads, by physical address (lm_dntm_data). */
#define K_BIT7 04745u
#define K_TWO 04752u
#define K_NEG0 04754u
#define K_ZERO 04755u
#define K_ERASID 05011u
#define K_LOW8 04357u
#define K_PRIO34 07722u
#define K_MINB12 07741u
#define K_NEGONE 07747u
#define K_7753 07753u      /* read by MINTIME2 executed as MASK 7753 */
#define K_4001 04001u      /* read by DNADRDCR executed as MASK 4001 */
#define K_7777 07777u      /* read by MINB1314 executed as CS 7777 */
#define K_LOWIDCOD B5(02065u)
#define K_DNTABLE 02441u
#define K_MINTIME2 B5(03525u)
#define K_DNADRDCR B5(03531u)
#define K_MINB1314 B5(03560u)
#define K_LDNPHAS2 B5(03640u)
#define K_LDNDUMP B5(03720u)
#define K_LDNDUMP1 B5(03721u)
/* Instruction words a run-time INDEX modifies (from the listing). */
#define I_CA0 030000u      /* 3544 CA 0 */
#define I_CADNTABLE 032441u
#define I_ZERO 000000u     /* 3612, 3635, 3667: 0 0 (= CA SSSS) */
#define I_READ_HI 044000u  /* 3574 INDEX 0 -4000 (executed as READ) */
#define I_READ_LO 043777u  /* 3600 INDEX 0 -4001 */
static const agc_word goto_words[LM_DNTM_GOTO_COUNT]={LM_DNTM_GOTO_WORDS};

static agc_word k(uint16_t physical){agc_word w=0;(void)agc_site_lookup(lm_dntm_data,LM_DNTM_DATA_COUNT,physical,&w);return w;}
/* A fixed word read only on a CCS branch with no producer: refuse when the
 * listing has no such word. */
#define KX(physical,w) do{if(!agc_site_lookup(lm_dntm_data,LM_DNTM_DATA_COUNT,(physical),&(w)))return EXEC_OUT_OF_SCOPE;}while(0)
static lm_dntm_io *ctx(const apollo_overlay_state *s,void *io){
 lm_dntm_io *x=io;
 if(s==0||x==0||x->control.interrupt_enabled>1u||(x->control.superbank&~0160u)||W[7]!=0||W[4]!=FB05)return 0;
 return x;
}
/* The instruction word an INDEX with value idx makes of `word` (yaAGC:
 * overflow-corrected 16-bit sum of the sign-extended words). */
static agc_word indexed(agc_word word,agc_word idx){
 return (agc_word)(agc_correct(agc_add_register(agc_extend(idx),agc_extend(word)))&077777u);
}
/* CA of a downlist word: the indexed word must be CA K with K a compiled
 * fixed word of bank 05 or fixed-fixed. */
static int ca_indexed(apollo_overlay_state *s,agc_word word,agc_word idx){
 agc_word ins=indexed(word,idx),address=(agc_word)(ins&07777u),w;uint16_t physical;
 if((ins>>12)!=3u||address<02000u)return 0;
 physical=address<04000u?B5(address):address;
 if(!agc_site_lookup(lm_dntm_data,LM_DNTM_DATA_COUNT,physical,&w))return 0;
 ai_caf(s,w);return 1;
}
static agc_word *channel(lm_dntm_io *x,agc_word c){
 switch(c){
 case 011u:return &x->channel11; case 012u:return &x->channel12;
 case 013u:return &x->channel13; case 014u:return &x->channel14;
 case 030u:return &x->channel30; case 031u:return &x->channel31;
 case 032u:return &x->channel32; case 033u:return &x->channel33;
 case 034u:return &x->channel34; case 035u:return &x->channel35;
 default:return 0;}
}
/* yaAGC CpuWriteIO for the channels this file writes (13, 34, 35). */
static void write_channel(lm_dntm_io *x,agc_word c,agc_word v){
 if(c==013u){if(v&004000u)x->trap31a=1;if(v&010000u)x->trap31b=1;if(v&020000u)x->trap32=1;v&=043777u;}
 *channel(x,c)=v;
}
/* EXTEND; WOR/WAND CHAN13. */
static void wor13(apollo_overlay_state *s,lm_dntm_io *x){agc_word v=(agc_word)(agc_correct(W[0])|x->channel13);write_channel(x,013u,v);W[0]=agc_extend(v);}
static void wand13(apollo_overlay_state *s,lm_dntm_io *x){agc_word v=(agc_word)(agc_correct(W[0])&x->channel13);write_channel(x,013u,v);W[0]=agc_extend(v);}
/* EXTEND; INDEX DNECADR; INDEX 0 -400n executed as READ: A = channel. */
static int read_indexed(apollo_overlay_state *s,lm_dntm_io *x,agc_word word){
 agc_word ins=indexed(word,ai_word(s,DNECADR)),*p;
 if((ins>>9)!=0u||(p=channel(x,(agc_word)(ins&0777u)))==0||(ins&0777u)>=034u)return 0;
 W[0]=agc_extend(*p);return 1;
}
#define TS(a) do{if(ai_ts(s,(a)))return EXEC_OUT_OF_SCOPE;}while(0)

/* DODOWNTM 05,3506 from the DOWNRUPT lead-in, through WOTEST, to TC DNTMGOTO:
 * the word-order bit is set only when it is clear. */
static enum exec_status dodowntm(const agc_native_program *p,apollo_overlay_state *s,void *io){
 lm_dntm_io *x=ctx(s,io);uint8_t c;(void)p;
 if(!x)return EXEC_OUT_OF_SCOPE;
 TS(BANKRUPT);                       /* 3506 */
 ai_qxch(s,QRUPT);                   /* 3507-3510; 3511 TCF WOTEST */
 ai_caf(s,k(K_BIT7));                /* WOTEST 3763 */
 W[0]=agc_extend((agc_word)(agc_correct(W[0])&x->channel13)); /* 3764-3765 EXTEND; RAND CHAN13 */
 c=ai_ccs(s,0u);                     /* 3766 */
 if(c==0u){ai_tc(s,03770u);W[5]=DNTMGOTO;return EXEC_OK;} /* 3767 TC DNTMGOTO */
 if(c==3u)return EXEC_OUT_OF_SCOPE;  /* RAND of BIT7 is never -0 */
 if(c==1u)ai_caf(s,k(K_BIT7));       /* 3770; 3771 TCF WO1 */
 wor13(s,x);                         /* WO1 3512-3513 */
 ai_tc(s,03515u);W[5]=DNTMGOTO;      /* 3514 TC DNTMGOTO */
 return EXEC_OK;
}
/* DNTMGOTO: the stored word executed as TC (Q = DNTMGOTO+1). */
static enum exec_status dntmgoto(const agc_native_program *p,apollo_overlay_state *s,void *io){
 lm_dntm_io *x=ctx(s,io);agc_word w;unsigned i;(void)p;
 if(!x)return EXEC_OUT_OF_SCOPE;
 w=ai_word(s,DNTMGOTO);
 for(i=0;i<LM_DNTM_GOTO_COUNT&&goto_words[i]!=w;++i){}
 if(i==LM_DNTM_GOTO_COUNT)return EXEC_OUT_OF_SCOPE;
 ai_tc(s,(agc_word)(DNTMGOTO+1u));W[5]=w;
 return EXEC_OK;
}

static enum exec_status body(apollo_overlay_state *s,lm_dntm_io *x,agc_word entry){
 uint8_t c;agc_word ret,w;
 switch(entry){
 case LM_DNTM_DNPHASE1:goto L3515; case LM_DNTM_DNPHASE2:goto L3523;
 case LM_DNTM_DNDUMPI:goto L3707; case LM_DNTM_DNDUMPI3:goto L3712;
 case LM_DNTM_DNDUMP:goto L3722; case LM_DNTM_DNDUMP1:goto L3735;
 default:return EXEC_OUT_OF_SCOPE;}
L3515: /* DNPHASE1 */
 ai_caf(s,k(K_NEGONE));TS(SUBLIST);TS(DNECADR);
 ai_caf(s,k(K_LDNPHAS2));TS(DNTMGOTO);
 goto L3536;
L3523: /* DNPHASE2 */
 c=ai_ccs(s,DNECADR);
 if(c==0u){ai_tc(s,03525u);goto L3653;}     /* 3524 DODNADR TC FETCH2WD */
 if(c==1u){KX(K_7753,w);ai_maskf(s,w);}       /* 3525 MINTIME2 as MASK 7753; 3526 TCF +1 */
 c=ai_ccs(s,SUBLIST);                        /* 3527 */
 if(c==0u)goto L3666;                        /* 3530 TCF NEXTINSL */
 if(c==1u){KX(K_4001,w);ai_maskf(s,w);}       /* 3531 DNADRDCR as MASK 4001 */
 if(c<3u)ai_ca(s,CTLIST);                    /* 3532 CHKLIST */
 if(ai_bzmf(s))goto L3536;                   /* 3533-3534 */
 goto L3543;                                 /* 3535 */
L3536: /* NEWLIST */
 if(!ca_indexed(s,I_CADNTABLE,ai_word(s,DNLSTCOD)))return EXEC_OUT_OF_SCOPE;
 TS(CTLIST);ai_cs(s,DNLSTCOD);
 goto L3756;                                 /* TCF SENDID +3 */
L3543: /* NEXTINCL */
 if(!ca_indexed(s,I_CA0,ai_word(s,CTLIST)))return EXEC_OUT_OF_SCOPE;
 c=ai_ccs(s,0u);                             /* 3545 */
 if(c==0u)ai_incr(s,CTLIST);                 /* 3546; 3547 TCF +4 */
 else if(c>=2u){
  if(c==2u)ai_xch(s,CTLIST);                 /* 3550 */
  ai_com(s);ai_xch(s,CTLIST);                /* 3551-3552 */
 }
 ai_incr(s,0u);                              /* 3553 */
 TS(DNECADR);ai_adf(s,k(K_MINTIME2));        /* 3554-3555 */
 c=ai_ccs(s,0u);                             /* 3556 */
 if(c==1u){KX(K_7777,w);ai_csf(s,w);}         /* 3560 MINB1314 as CS 7777 */
 if(c<3u)goto L3563;                         /* 3557/3561 TCF SETWO +1 */
 ai_tc(s,03563u);goto L3605;                 /* 3562 SETWO TC WOZERO */
L3563:
 ai_ca(s,DNECADR);
L3564:
 ai_adf(s,k(K_MINB1314));
 if(ai_bzmf(s))goto L3653;                   /* BZMF FETCH2WD */
 ai_adf(s,k(K_MINB12));
 if(ai_bzmf(s))goto L3611;                   /* BZMF DODNPTR */
 /* DODNCHAN 3572: TC 6 (EXTEND); INDEX DNECADR; INDEX 0 -4000 (READ) */
 if(!read_indexed(s,x,I_READ_HI))return EXEC_OUT_OF_SCOPE;
 TS(L);
 if(!read_indexed(s,x,I_READ_LO))return EXEC_OUT_OF_SCOPE; /* 3576-3600 */
 TS(DNECADR);ai_caf(s,k(K_NEGONE));ai_xch(s,DNECADR);
 goto L3701;
L3605: /* WOZERO */
 ai_csf(s,k(K_BIT7));wand13(s,x);
 ret=ai_word(s,Q);ai_tc(s,03611u);           /* 3610 TC Q */
 if(ret==03563u)goto L3563;
 if(ret==03760u)goto L3760;
 return EXEC_OUT_OF_SCOPE;
L3611: /* DODNPTR */
 if(!ca_indexed(s,I_ZERO,ai_word(s,DNECADR)))return EXEC_OUT_OF_SCOPE;
 c=ai_ccs(s,0u);                             /* 3613 */
 if(c==0u){ai_ca(s,DNECADR);goto L3665;}     /* 3614-3615 */
 if(c==1u)goto L3665;                        /* 3615 TCF DOSUBLST */
 if(c==2u)ai_xch(s,DNECADR);                 /* 3616 */
 TS(SUBLIST);ai_caf(s,k(K_ZERO));ai_xch(s,TMINDEX); /* 3617-3621 */
L3622: /* SNAPLOOP */
 TS(EBANK);ai_maskf(s,k(K_LOW8));
 ai_dca(s,01401u+agc_correct(W[0]));         /* EXTEND; INDEX A; DCA 1401 */
 ai_dxch(s,DNTMBUFF+ai_word(s,TMINDEX));     /* INDEX TMINDEX; DXCH DNTMBUFF */
 ai_incr(s,TMINDEX);ai_incr(s,TMINDEX);
 ai_incr(s,SUBLIST);                         /* SNAPAGN */
 if(!ca_indexed(s,I_ZERO,ai_word(s,SUBLIST)))return EXEC_OUT_OF_SCOPE;
 c=ai_ccs(s,0u);                             /* 3636 */
 if(c==0u)goto L3622;
 if(c==1u)return EXEC_OUT_OF_SCOPE;          /* 3640 LDNPHAS2 as TC DNPHASE2: no producer */
 if(c==2u)TS(SUBLIST);                       /* 3641 */
 ai_caf(s,k(K_NEGONE));TS(DNECADR);ai_xch(s,SUBLIST); /* 3642-3644 */
 TS(EBANK);ai_maskf(s,k(K_LOW8));
 ai_dca(s,01401u+agc_correct(W[0]));         /* 3647-3651 */
 goto L3701;                                 /* SNAPEND */
L3653: /* FETCH2WD */
 ai_ca(s,DNECADR);TS(EBANK);ai_maskf(s,k(K_LOW8));TS(L);
 ai_caf(s,k(K_DNADRDCR));ai_ads(s,DNECADR);
 ai_dca(s,01400u+agc_correct(W[1]));         /* EXTEND; INDEX L; DCA 1400 */
 goto L3701;
L3665: /* DOSUBLST */
 TS(SUBLIST);
L3666: /* NEXTINSL */
 if(!ca_indexed(s,I_ZERO,ai_word(s,SUBLIST)))return EXEC_OUT_OF_SCOPE;
 c=ai_ccs(s,0u);                             /* 3670 */
 if(c==0u)ai_incr(s,SUBLIST);                /* 3671; 3672 TCF +4 */
 else if(c>=2u){
  if(c==2u)TS(SUBLIST);                      /* 3673 */
  ai_caf(s,k(K_NEGONE));ai_xch(s,SUBLIST);   /* 3674-3675 */
 }
 ai_incr(s,0u);TS(DNECADR);                  /* 3676-3677 */
 goto L3564;                                 /* TCF SETWO +2 */
L3701: /* DNTMEXIT */
 write_channel(x,034u,agc_correct(W[0]));    /* EXTEND; WRITE DNTM1 */
 ai_ca(s,L);
 write_channel(x,035u,agc_correct(W[0]));    /* EXTEND; WRITE DNTM2 */
 W[5]=LM_DNTM_RESUME;                        /* TMRESUME TCF RESUME */
 return EXEC_OK;
L3707: /* DNDUMPI */
 ai_caf(s,k(K_ZERO));TS(DUMPLOC);
L3711:
 ai_tc(s,03712u);goto L3753;                 /* TC SENDID */
L3712:
 ai_caf(s,k(K_LDNDUMP1));TS(DNTMGOTO);
 ai_ca(s,TIME1);ai_xch(s,L);ai_ca(s,DUMPLOC);
 goto L3701;
L3722: /* DNDUMP */
 ai_caf(s,k(K_TWO));ai_ads(s,DUMPLOC);ai_maskf(s,k(K_LOW8));
 c=ai_ccs(s,0u);                             /* 3725 */
 if(c==0u)goto L3737;                        /* 3726 TCF DNDUMP2 */
 if(c==1u)ai_ca(s,DUMPLOC);                  /* 3727 */
 if(c<3u)ai_mask(s,DUMPCNT);                 /* 3730 */
 ai_maskf(s,k(K_PRIO34));                    /* 3731 */
 c=ai_ccs(s,0u);                             /* 3732 */
 if(c==0u)goto L3515;                        /* 3733 TCF DNPHASE1 */
 if(c==1u)goto L3711;                        /* 3734 TCF DNDUMPI +2 */
 if(c==3u)goto L3736;
L3735: /* DNDUMP1 */
 ai_caf(s,k(K_LDNDUMP));
L3736:
 TS(DNTMGOTO);
L3737: /* DNDUMP2 */
 ai_ca(s,DUMPLOC);TS(EBANK);ai_maskf(s,k(K_LOW8));TS(Q);
 ai_caf(s,k(K_NEG0));TS(L);
 W[5]=03747u;ai_mask(s,01401u+ai_word(s,Q)); /* INDEX Q; MASK 1401 (Z reads 3747) */
 ai_xch(s,L);
 W[5]=03752u;ai_mask(s,01400u+ai_word(s,Q)); /* INDEX Q; MASK 1400 */
 goto L3701;
L3753: /* SENDID */
 ai_qxch(s,DNTMGOTO);ai_caf(s,k(K_ERASID));
L3756:
 TS(L);ai_tc(s,03760u);goto L3605;           /* TC WOZERO */
L3760:
 ai_caf(s,k(K_LOWIDCOD));ai_xch(s,L);
 goto L3701;
}
#define ENTRY(name,at) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io){\
 lm_dntm_io *x=ctx(s,io);(void)p;return x?body(s,x,at):EXEC_OUT_OF_SCOPE;}
ENTRY(dnphase1,LM_DNTM_DNPHASE1)
ENTRY(dnphase2,LM_DNTM_DNPHASE2)
ENTRY(dndumpi,LM_DNTM_DNDUMPI)
ENTRY(dndumpi3,LM_DNTM_DNDUMPI3)
ENTRY(dndump,LM_DNTM_DNDUMP)
ENTRY(dndump1,LM_DNTM_DNDUMP1)

static const agc_routine_entry entries[LM_DNTM_ROUTINES]={
 {B5(LM_DNTM_DODOWNTM),dodowntm},{B5(LM_DNTM_DNPHASE1),dnphase1},{B5(LM_DNTM_DNPHASE2),dnphase2},
 {B5(LM_DNTM_DNDUMPI),dndumpi},{B5(LM_DNTM_DNDUMPI3),dndumpi3},{B5(LM_DNTM_DNDUMP),dndump},
 {B5(LM_DNTM_DNDUMP1),dndump1},{AGC_TRANSFER_ERASABLE_BASE+LM_DNTM_DNTMGOTO,dntmgoto}};
const agc_routine_table lm_downtelemetry_routines={entries,LM_DNTM_ROUTINES};
