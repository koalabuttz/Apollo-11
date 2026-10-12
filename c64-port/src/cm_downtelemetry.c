/* SPDX-License-Identifier: MIT */
/* Comanche055 DOWN-TELEMETRY_PROGRAM: DODOWNTM, the DNTMGOTO continuation,
 * DNPHASE1/2, NEWLIST, NEXTINCL, SETWO, DODNCHAN, WOZERO, DODNPTR, SNAPLOOP,
 * SNAPAGN, FETCH2WD, DOSUBLST/NEXTINSL, DNTMEXIT, DNDUMPI, DNDUMP, DNDUMP1,
 * DNDUMP2 and SENDID. One agc_image_ops call per original instruction, under
 * labels L<address> (bank 05); channel instructions act on cm_dntm_io as
 * yaAGC's READ/WOR/WAND/WRITE do. An INDEX whose target word is computed at
 * run time (the downlist pointers, DNCHAN's READ) forms the instruction word
 * as the hardware does and must yield the CA or READ the source intends;
 * anything else refuses. Fixed words read as data come from
 * cm_dntm_data, compiled from the listing. */
#include "cm_downtelemetry.h"
#include "agc_image_ops.h"
#include "cm_downtelemetry_data.inc"

#define W s->words
#define B5(z) (uint16_t)(CM_DNTM_BANK*1024u+((z)-02000u))
#define FB05 (CM_DNTM_BANK<<10)
#define L 1u
#define Q 2u
#define EBANK 3u
#define QRUPT 012u
#define BANKRUPT 016u
#define TIME1 025u
#define DNLSTCOD CM_DNTM_DNLSTCOD
#define DUMPCNT CM_DNTM_DUMPCNT
#define CTLIST 0334u
#define DNTMGOTO CM_DNTM_DNTMGOTO
#define DNECADR 0336u     /* = TMINDEX = DUMPLOC */
#define TMINDEX DNECADR
#define DUMPLOC DNECADR
#define SUBLIST 0337u
#define DNTMBUFF 0340u
/* Fixed words the program reads, by physical address (cm_dntm_data). */
#define K_BIT7 05011u
#define K_TWO 05016u
#define K_NEG0 05020u
#define K_ZERO 05021u
#define K_ERASID 05054u
#define K_LOW8 04373u
#define K_PRIO34 07671u
#define K_MINB12 07710u
#define K_NEGONE 07716u
#define K_7753 07753u      /* read by MINTIME2 executed as MASK 7753 */
#define K_4001 04001u      /* read by DNADRDCR executed as MASK 4001 */
#define K_7777 07777u      /* read by MINB1314 executed as CS 7777 */
#define K_LOWIDCOD B5(02000u)
#define K_DNTABLE 02335u
#define K_MINTIME2 B5(03540u)
#define K_DNADRDCR B5(03544u)
#define K_MINB1314 B5(03573u)
#define K_LDNPHAS2 B5(03653u)
#define K_LDNDUMP B5(03733u)
#define K_LDNDUMP1 B5(03734u)
/* Instruction words a run-time INDEX modifies (from the listing). */
#define I_CA0 030000u      /* 3557 CA 0 */
#define I_CADNTABLE 032335u
#define I_ZERO 000000u     /* 3625, 3650, 3702: 0 0 (= CA SSSS) */
#define I_READ_HI 044000u  /* 3607 INDEX 0 -4000 (executed as READ) */
#define I_READ_LO 043777u  /* 3613 INDEX 0 -4001 */
static const agc_word goto_words[CM_DNTM_GOTO_COUNT]={CM_DNTM_GOTO_WORDS};

static agc_word k(uint16_t physical){agc_word w=0;(void)agc_site_lookup(cm_dntm_data,CM_DNTM_DATA_COUNT,physical,&w);return w;}
/* A fixed word read only on a CCS branch with no producer: refuse when the
 * listing has no such word. */
#define KX(physical,w) do{if(!agc_site_lookup(cm_dntm_data,CM_DNTM_DATA_COUNT,(physical),&(w)))return EXEC_OUT_OF_SCOPE;}while(0)
static cm_dntm_io *ctx(const apollo_overlay_state *s,void *io){
 cm_dntm_io *x=io;
 if(s==0||x==0||x->io.interrupt_enabled>1u||(x->io.superbank&~0160u)||W[7]!=0||W[4]!=FB05)return 0;
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
 if(!agc_site_lookup(cm_dntm_data,CM_DNTM_DATA_COUNT,physical,&w))return 0;
 ai_caf(s,w);return 1;
}
static agc_word *channel(cm_dntm_io *x,agc_word c){
 switch(c){
 case 011u:return &x->io.channel11; case 012u:return &x->io.channel12;
 case 013u:return &x->io.channel13; case 014u:return &x->io.channel14;
 case 030u:return &x->channel30; case 031u:return &x->channel31;
 case 032u:return &x->channel32; case 033u:return &x->channel33;
 case 034u:return &x->channel34; case 035u:return &x->channel35;
 default:return 0;}
}
/* yaAGC CpuWriteIO for the channels this file writes (13, 34, 35). */
static void write_channel(cm_dntm_io *x,agc_word c,agc_word v){
 if(c==013u){if(v&004000u)x->io.trap31a=1;if(v&010000u)x->io.trap31b=1;if(v&020000u)x->io.trap32=1;v&=043777u;}
 *channel(x,c)=v;
}
/* EXTEND; WOR/WAND CHAN13. */
static void wor13(apollo_overlay_state *s,cm_dntm_io *x){agc_word v=(agc_word)(agc_correct(W[0])|x->io.channel13);write_channel(x,013u,v);W[0]=agc_extend(v);}
static void wand13(apollo_overlay_state *s,cm_dntm_io *x){agc_word v=(agc_word)(agc_correct(W[0])&x->io.channel13);write_channel(x,013u,v);W[0]=agc_extend(v);}
/* EXTEND; INDEX DNECADR; INDEX 0 -400n executed as READ: A = channel. */
static int read_indexed(apollo_overlay_state *s,cm_dntm_io *x,agc_word word){
 agc_word ins=indexed(word,ai_word(s,DNECADR)),*p;
 if((ins>>9)!=0u||(p=channel(x,(agc_word)(ins&0777u)))==0||(ins&0777u)>=034u)return 0;
 W[0]=agc_extend(*p);return 1;
}
#define TS(a) do{if(ai_ts(s,(a)))return EXEC_OUT_OF_SCOPE;}while(0)

/* DODOWNTM 05,3521 from the DOWNRUPT lead-in, to TC DNTMGOTO. */
static enum exec_status dodowntm(const agc_native_program *p,apollo_overlay_state *s,void *io){
 cm_dntm_io *x=ctx(s,io);(void)p;
 if(!x)return EXEC_OUT_OF_SCOPE;
 TS(BANKRUPT);                       /* 3521 */
 ai_qxch(s,QRUPT);                   /* 3522-3523 */
 ai_caf(s,k(K_BIT7));wor13(s,x);     /* 3524-3526 */
 ai_tc(s,03530u);W[5]=DNTMGOTO;      /* 3527 TC DNTMGOTO */
 return EXEC_OK;
}
/* DNTMGOTO: the stored word executed as TC (Q = DNTMGOTO+1). */
static enum exec_status dntmgoto(const agc_native_program *p,apollo_overlay_state *s,void *io){
 cm_dntm_io *x=ctx(s,io);agc_word w;unsigned i;(void)p;
 if(!x)return EXEC_OUT_OF_SCOPE;
 w=ai_word(s,DNTMGOTO);
 for(i=0;i<CM_DNTM_GOTO_COUNT&&goto_words[i]!=w;++i){}
 if(i==CM_DNTM_GOTO_COUNT)return EXEC_OUT_OF_SCOPE;
 ai_tc(s,(agc_word)(DNTMGOTO+1u));W[5]=w;
 return EXEC_OK;
}

static enum exec_status body(apollo_overlay_state *s,cm_dntm_io *x,agc_word entry){
 uint8_t c;agc_word ret,w;
 switch(entry){
 case CM_DNTM_DNPHASE1:goto L3530; case CM_DNTM_DNPHASE2:goto L3536;
 case CM_DNTM_DNDUMPI:goto L3722; case CM_DNTM_DNDUMPI3:goto L3725;
 case CM_DNTM_DNDUMP:goto L3735; case CM_DNTM_DNDUMP1:goto L3750;
 default:return EXEC_OUT_OF_SCOPE;}
L3530: /* DNPHASE1 */
 ai_caf(s,k(K_NEGONE));TS(SUBLIST);TS(DNECADR);
 ai_caf(s,k(K_LDNPHAS2));TS(DNTMGOTO);
 goto L3551;
L3536: /* DNPHASE2 */
 c=ai_ccs(s,DNECADR);
 if(c==0u){ai_tc(s,03540u);goto L3666;}     /* 3537 DODNADR TC FETCH2WD */
 if(c==1u){KX(K_7753,w);ai_maskf(s,w);}       /* 3540 MINTIME2 as MASK 7753; 3541 TCF +1 */
 c=ai_ccs(s,SUBLIST);                        /* 3542 */
 if(c==0u)goto L3701;                        /* 3543 TCF NEXTINSL */
 if(c==1u){KX(K_4001,w);ai_maskf(s,w);}       /* 3544 DNADRDCR as MASK 4001 */
 if(c<3u)ai_ca(s,CTLIST);                    /* 3545 CHKLIST */
 if(ai_bzmf(s))goto L3551;                   /* 3546-3547 */
 goto L3556;                                 /* 3550 */
L3551: /* NEWLIST */
 if(!ca_indexed(s,I_CADNTABLE,ai_word(s,DNLSTCOD)))return EXEC_OUT_OF_SCOPE;
 TS(CTLIST);ai_cs(s,DNLSTCOD);
 goto L3771;                                 /* TCF SENDID +3 */
L3556: /* NEXTINCL */
 if(!ca_indexed(s,I_CA0,ai_word(s,CTLIST)))return EXEC_OUT_OF_SCOPE;
 c=ai_ccs(s,0u);                             /* 3560 */
 if(c==0u)ai_incr(s,CTLIST);                 /* 3561; 3562 TCF +4 */
 else if(c>=2u){
  if(c==2u)ai_xch(s,CTLIST);                 /* 3563 */
  ai_com(s);ai_xch(s,CTLIST);                /* 3564-3565 */
 }
 ai_incr(s,0u);                              /* 3566 */
 TS(DNECADR);ai_adf(s,k(K_MINTIME2));        /* 3567-3570 */
 c=ai_ccs(s,0u);                             /* 3571 */
 if(c==1u){KX(K_7777,w);ai_csf(s,w);}         /* 3573 MINB1314 as CS 7777 */
 if(c<3u)goto L3576;                         /* 3572/3574 TCF SETWO +1 */
 ai_tc(s,03576u);goto L3620;                 /* 3575 SETWO TC WOZERO */
L3576:
 ai_ca(s,DNECADR);
L3577:
 ai_adf(s,k(K_MINB1314));
 if(ai_bzmf(s))goto L3666;                   /* BZMF FETCH2WD */
 ai_adf(s,k(K_MINB12));
 if(ai_bzmf(s))goto L3624;                   /* BZMF DODNPTR */
 /* DODNCHAN 3605: TC 6 (EXTEND); INDEX DNECADR; INDEX 0 -4000 (READ) */
 if(!read_indexed(s,x,I_READ_HI))return EXEC_OUT_OF_SCOPE;
 TS(L);
 if(!read_indexed(s,x,I_READ_LO))return EXEC_OUT_OF_SCOPE; /* 3611-3613 */
 TS(DNECADR);ai_caf(s,k(K_NEGONE));ai_xch(s,DNECADR);
 goto L3714;
L3620: /* WOZERO */
 ai_csf(s,k(K_BIT7));wand13(s,x);
 ret=ai_word(s,Q);ai_tc(s,03624u);           /* 3623 TC Q */
 if(ret==03576u)goto L3576;
 if(ret==03773u)goto L3773;
 return EXEC_OUT_OF_SCOPE;
L3624: /* DODNPTR */
 if(!ca_indexed(s,I_ZERO,ai_word(s,DNECADR)))return EXEC_OUT_OF_SCOPE;
 c=ai_ccs(s,0u);                             /* 3626 */
 if(c==0u){ai_ca(s,DNECADR);goto L3700;}     /* 3627-3630 */
 if(c==1u)goto L3700;                        /* 3630 TCF DOSUBLST */
 if(c==2u)ai_xch(s,DNECADR);                 /* 3631 */
 TS(SUBLIST);ai_caf(s,k(K_ZERO));ai_xch(s,TMINDEX); /* 3632-3634 */
L3635: /* SNAPLOOP */
 TS(EBANK);ai_maskf(s,k(K_LOW8));
 ai_dca(s,01401u+agc_correct(W[0]));         /* EXTEND; INDEX A; DCA 1401 */
 ai_dxch(s,DNTMBUFF+ai_word(s,TMINDEX));     /* INDEX TMINDEX; DXCH DNTMBUFF */
 ai_incr(s,TMINDEX);ai_incr(s,TMINDEX);
 ai_incr(s,SUBLIST);                         /* SNAPAGN */
 if(!ca_indexed(s,I_ZERO,ai_word(s,SUBLIST)))return EXEC_OUT_OF_SCOPE;
 c=ai_ccs(s,0u);                             /* 3651 */
 if(c==0u)goto L3635;
 if(c==1u)return EXEC_OUT_OF_SCOPE;          /* 3653 LDNPHAS2 as TC DNPHASE2: no producer */
 if(c==2u)TS(SUBLIST);                       /* 3654 */
 ai_caf(s,k(K_NEGONE));TS(DNECADR);ai_xch(s,SUBLIST); /* 3655-3657 */
 TS(EBANK);ai_maskf(s,k(K_LOW8));
 ai_dca(s,01401u+agc_correct(W[0]));         /* 3662-3664 */
 goto L3714;                                 /* SNAPEND */
L3666: /* FETCH2WD */
 ai_ca(s,DNECADR);TS(EBANK);ai_maskf(s,k(K_LOW8));TS(L);
 ai_caf(s,k(K_DNADRDCR));ai_ads(s,DNECADR);
 ai_dca(s,01400u+agc_correct(W[1]));         /* EXTEND; INDEX L; DCA 1400 */
 goto L3714;
L3700: /* DOSUBLST */
 TS(SUBLIST);
L3701: /* NEXTINSL */
 if(!ca_indexed(s,I_ZERO,ai_word(s,SUBLIST)))return EXEC_OUT_OF_SCOPE;
 c=ai_ccs(s,0u);                             /* 3703 */
 if(c==0u)ai_incr(s,SUBLIST);                /* 3704; 3705 TCF +4 */
 else if(c>=2u){
  if(c==2u)TS(SUBLIST);                      /* 3706 */
  ai_caf(s,k(K_NEGONE));ai_xch(s,SUBLIST);   /* 3707-3710 */
 }
 ai_incr(s,0u);TS(DNECADR);                  /* 3711-3712 */
 goto L3577;                                 /* TCF SETWO +2 */
L3714: /* DNTMEXIT */
 write_channel(x,034u,agc_correct(W[0]));    /* EXTEND; WRITE DNTM1 */
 ai_ca(s,L);
 write_channel(x,035u,agc_correct(W[0]));    /* EXTEND; WRITE DNTM2 */
 W[5]=CM_DNTM_RESUME;                        /* TMRESUME TCF RESUME */
 return EXEC_OK;
L3722: /* DNDUMPI */
 ai_caf(s,k(K_ZERO));TS(DUMPLOC);
L3724:
 ai_tc(s,03725u);goto L3766;                 /* TC SENDID */
L3725:
 ai_caf(s,k(K_LDNDUMP1));TS(DNTMGOTO);
 ai_ca(s,TIME1);ai_xch(s,L);ai_ca(s,DUMPLOC);
 goto L3714;
L3735: /* DNDUMP */
 ai_caf(s,k(K_TWO));ai_ads(s,DUMPLOC);ai_maskf(s,k(K_LOW8));
 c=ai_ccs(s,0u);                             /* 3740 */
 if(c==0u)goto L3752;                        /* 3741 TCF DNDUMP2 */
 if(c==1u)ai_ca(s,DUMPLOC);                  /* 3742 */
 if(c<3u)ai_mask(s,DUMPCNT);                 /* 3743 */
 ai_maskf(s,k(K_PRIO34));                    /* 3744 */
 c=ai_ccs(s,0u);                             /* 3745 */
 if(c==0u)goto L3530;                        /* 3746 TCF DNPHASE1 */
 if(c==1u)goto L3724;                        /* 3747 TCF DNDUMPI +2 */
 if(c==3u)goto L3751;
L3750: /* DNDUMP1 */
 ai_caf(s,k(K_LDNDUMP));
L3751:
 TS(DNTMGOTO);
L3752: /* DNDUMP2 */
 ai_ca(s,DUMPLOC);TS(EBANK);ai_maskf(s,k(K_LOW8));TS(Q);
 ai_caf(s,k(K_NEG0));TS(L);
 W[5]=03762u;ai_mask(s,01401u+ai_word(s,Q)); /* INDEX Q; MASK 1401 (Z reads 3762) */
 ai_xch(s,L);
 W[5]=03765u;ai_mask(s,01400u+ai_word(s,Q)); /* INDEX Q; MASK 1400 */
 goto L3714;
L3766: /* SENDID */
 ai_qxch(s,DNTMGOTO);ai_caf(s,k(K_ERASID));
L3771:
 TS(L);ai_tc(s,03773u);goto L3620;           /* TC WOZERO */
L3773:
 ai_caf(s,k(K_LOWIDCOD));ai_xch(s,L);
 goto L3714;
}
#define ENTRY(name,at) static enum exec_status name(const agc_native_program *p,apollo_overlay_state *s,void *io){\
 cm_dntm_io *x=ctx(s,io);(void)p;return x?body(s,x,at):EXEC_OUT_OF_SCOPE;}
ENTRY(dnphase1,CM_DNTM_DNPHASE1)
ENTRY(dnphase2,CM_DNTM_DNPHASE2)
ENTRY(dndumpi,CM_DNTM_DNDUMPI)
ENTRY(dndumpi3,CM_DNTM_DNDUMPI3)
ENTRY(dndump,CM_DNTM_DNDUMP)
ENTRY(dndump1,CM_DNTM_DNDUMP1)

static const agc_routine_entry entries[CM_DNTM_ROUTINES]={
 {B5(CM_DNTM_DODOWNTM),dodowntm},{B5(CM_DNTM_DNPHASE1),dnphase1},{B5(CM_DNTM_DNPHASE2),dnphase2},
 {B5(CM_DNTM_DNDUMPI),dndumpi},{B5(CM_DNTM_DNDUMPI3),dndumpi3},{B5(CM_DNTM_DNDUMP),dndump},
 {B5(CM_DNTM_DNDUMP1),dndump1},{AGC_TRANSFER_ERASABLE_BASE+CM_DNTM_DNTMGOTO,dntmgoto}};
const agc_routine_table cm_downtelemetry_routines={entries,CM_DNTM_ROUTINES};
