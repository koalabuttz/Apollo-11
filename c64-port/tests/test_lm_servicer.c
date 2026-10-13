/* SPDX-License-Identifier: MIT */
/* lm-servicer: Luminary099 SERVICER as native routines, against digests
 * frozen from the original program; every routine, every reachable
 * instruction and order pair and decision outcome is taken; inputs no
 * original producer supplies are refused with the image unchanged. Built
 * with LMSV_COUNT_PATHS. */
#include <stdio.h>
#include <string.h>
#include "lm_servicer_cases.h"
#include "lm_servicer_vectors.h"
#define PATHS 1474u
#define E7(a) (07u*0400u+(a)-01400u)
unsigned lmsv_paths[PATHS];
static unsigned saved[PATHS];
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<200u)fprintf(stderr,"FAIL %s %o\n",what,n);}}
static apollo_overlay_state undo;
static agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
static unsigned reached[LMSV_ROUTINES];
/* Case n's image with `edit` applied must be refused, image unchanged. */
static void refused(uint16_t n,void (*edit)(apollo_overlay_state *,lm_sv_io *),const char *what){
 apollo_overlay_state s,before;lmsv_case c;lm_sv_io io;agc_routine call;
 lmsv_input(&s,&c,n);io.control.superbank=c.superbank;io.control.interrupt_enabled=c.interrupt_enabled;
 io.chan11=c.chan11;io.chan12=c.chan12;io.chan32=c.chan32;io.chan33=c.chan33;io.undo=&undo;
 edit(&s,&io);before=s;
 call=agc_transfer_resolve(&program,&s,c.superbank);
 check(call&&call(&program,&s,&io)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s),what,n);
}
static void fb10(apollo_overlay_state *s,lm_sv_io *io){(void)io;s->words[4]=020000u;s->words[6]=(agc_word)(020000u|(s->words[3]>>8));}
static void vac_bad(apollo_overlay_state *s,lm_sv_io *io){(void)io;s->words[0120]=0402u;}
static void edop(apollo_overlay_state *s,lm_sv_io *io){(void)io;s->words[023]=1u;}
static void newjob_neg(apollo_overlay_state *s,lm_sv_io *io){(void)io;s->words[067]=040000u;}
static void a_odd(apollo_overlay_state *s,lm_sv_io *io){(void)io;s->words[0]=3u;}
static void a_four(apollo_overlay_state *s,lm_sv_io *io){(void)io;s->words[0]=4u;}
static void a_eight(apollo_overlay_state *s,lm_sv_io *io){(void)io;s->words[0]=010u;}
static void vselect_bad(apollo_overlay_state *s,lm_sv_io *io){(void)io;s->words[0131]=5u;}
static void q_erasable(apollo_overlay_state *s,lm_sv_io *io){(void)io;s->words[0]=0;s->words[2]=01000u;}
static void q_dependent(apollo_overlay_state *s,lm_sv_io *io){(void)io;s->words[0]=0;s->words[2]=02333u;}
static void lradret_erasable(apollo_overlay_state *s,lm_sv_io *io){(void)io;s->words[E7(01650u)]=01000u;}
static void phase4_neg(apollo_overlay_state *s,lm_sv_io *io){(void)io;s->words[E7(01515u)]=0;s->words[0761]=040000u;}
static void stilbadv_m0(apollo_overlay_state *s,lm_sv_io *io){(void)io;s->words[E7(01675u)]=077777u;}
static void a_overflow(apollo_overlay_state *s,lm_sv_io *io){(void)io;s->words[0]=0100000u;}
static void no_undo(apollo_overlay_state *s,lm_sv_io *io){(void)s;io->undo=0;}
/* The first case of the routine at physical key k that the routine runs
 * to the file's exit unedited, so each refusal is the edit's. */
static uint16_t slot(uint16_t k){unsigned i,v;apollo_overlay_state t;
 for(i=0;i<LMSV_ROUTINES;++i)if(lmsv_routine_keys[i]==k)break;
 for(v=0;v<LMSV_STATES;++v)if(lmsv_run(&t,(uint16_t)(i+v*LMSV_ROUTINES),0)==LMSV_STOPPED)break;
 check(i<LMSV_ROUTINES&&v<LMSV_STATES,"refusal test case",k);
 return (uint16_t)(i+v*LMSV_ROUTINES);}
#define K(b,a) (uint16_t)((0##b)*1024u+((a)-02000u))
int main(void){apollo_overlay_state s;unsigned n,i,stopped=0,refusals=0;
 /* Paths no admitted input reaches (02400 + n): ABVAL's, ABS's and UNIT's
  * alarm 1302 (BR 2-4), SQRTSUB's abort on a negative sum of squares;
  * READACCS's CCS A +0 (BR 9), since TIME5 - 037771 is never +0 in ones'
  * complement; the negative and -0 outcomes of a CCS of a MASK result
  * (BR 24-25, 28-29, 32-33, 38-39, 52-53, 104-105, 108-109, 112-113,
  * 116-117, 126-127, 134-135, 142-143, 152-153), of ITEMP1 (BR 56-57) and
  * of VSELECT (BR 166-167, 171, 179), counts the file stores as 0 to 20
  * and 0 to 2; LOTHRUST's negative PHASE4 (BR 48) and the -0 of STILBADV
  * and STILBADH (BR 175, 183), refused, since their CCS would run the
  * PHASCHNG operand or a word after EXTEND. */
 static const unsigned unreachable[]={02402u,02403u,02404u,02411u,02430u,02431u,02434u,02435u,02440u,02441u,
  02446u,02447u,02460u,02464u,02465u,02470u,02471u,02474u,02550u,02551u,02554u,02555u,02560u,02561u,02564u,02565u,
  02576u,02577u,02606u,02607u,02616u,02617u,02630u,02631u,02646u,02647u,02653u,02657u,02663u,02667u};
 /* Every instruction address and order pair (bank 33 a - 02206; bank 37
  * 01600 + a - 03410; bank 22 02100 + a - 03661; bank 27 02140 + a - 03321;
  * bank 32 02200 + a - 03770; bank 23 02210 + a - 02461; bank 21 02240 +
  * a - 02130; bank 34 02270 + a - 03716) and decision outcome (02400 + n)
  * of the file. */
 static const unsigned paths[]={
 00u,05u,06u,07u,011u,012u,014u,015u,016u,017u,020u,021u,
 022u,023u,024u,025u,026u,027u,030u,031u,032u,033u,034u,035u,
 036u,037u,040u,041u,042u,043u,044u,045u,046u,047u,050u,051u,
 052u,053u,055u,056u,060u,061u,065u,066u,067u,070u,071u,072u,
 073u,074u,075u,076u,077u,0100u,0101u,0102u,0103u,0104u,0105u,0106u,
 0107u,0110u,0111u,0112u,0113u,0114u,0115u,0116u,0117u,0120u,0121u,0122u,
 0123u,0124u,0125u,0126u,0127u,0130u,0131u,0132u,0133u,0134u,0135u,0136u,
 0137u,0140u,0141u,0142u,0143u,0144u,0145u,0146u,0147u,0150u,0151u,0153u,
 0154u,0157u,0160u,0161u,0162u,0163u,0164u,0165u,0167u,0170u,0171u,0172u,
 0173u,0175u,0176u,0177u,0200u,0201u,0202u,0203u,0204u,0205u,0207u,0211u,
 0212u,0213u,0214u,0215u,0216u,0217u,0220u,0221u,0222u,0223u,0224u,0225u,
 0226u,0227u,0230u,0232u,0233u,0234u,0235u,0236u,0237u,0240u,0241u,0242u,
 0243u,0244u,0245u,0246u,0247u,0252u,0253u,0254u,0255u,0256u,0257u,0260u,
 0261u,0262u,0263u,0264u,0265u,0266u,0267u,0270u,0271u,0272u,0273u,0274u,
 0275u,0276u,0277u,0300u,0301u,0302u,0303u,0304u,0305u,0306u,0307u,0310u,
 0311u,0312u,0313u,0314u,0315u,0316u,0317u,0320u,0321u,0322u,0323u,0324u,
 0325u,0326u,0327u,0330u,0332u,0333u,0334u,0335u,0336u,0340u,0341u,0342u,
 0343u,0344u,0345u,0346u,0347u,0350u,0351u,0352u,0353u,0354u,0355u,0356u,
 0357u,0360u,0361u,0362u,0363u,0365u,0367u,0370u,0371u,0374u,0376u,0400u,
 0402u,0404u,0406u,0410u,0412u,0414u,0415u,0416u,0421u,0422u,0423u,0424u,
 0425u,0426u,0427u,0430u,0431u,0432u,0433u,0434u,0435u,0436u,0437u,0440u,
 0441u,0442u,0443u,0444u,0445u,0446u,0447u,0450u,0451u,0452u,0453u,0454u,
 0455u,0456u,0457u,0460u,0461u,0462u,0463u,0464u,0465u,0466u,0467u,0470u,
 0502u,0503u,0505u,0506u,0510u,0512u,0513u,0514u,0516u,0517u,0521u,0522u,
 0523u,0524u,0527u,0531u,0534u,0535u,0537u,0542u,0543u,0545u,0547u,0552u,
 0554u,0556u,0557u,0562u,0565u,0567u,0570u,0573u,0575u,0576u,0577u,0601u,
 0604u,0606u,0611u,0613u,0614u,0616u,0624u,0626u,0631u,0633u,0635u,0637u,
 0641u,0643u,0644u,0645u,0646u,0650u,0652u,0654u,0655u,0656u,0657u,0660u,
 0663u,0665u,0667u,0672u,0673u,0675u,0677u,0700u,0702u,0703u,0704u,0706u,
 0710u,0712u,0714u,0716u,0720u,0722u,0723u,0725u,0730u,0733u,0734u,0747u,
 0750u,0751u,0752u,0753u,0754u,0755u,0756u,0757u,0760u,0761u,0762u,0765u,
 0767u,0772u,0774u,0775u,0776u,0777u,01000u,01001u,01002u,01003u,01006u,01010u,
 01013u,01014u,01015u,01016u,01017u,01020u,01021u,01022u,01024u,01026u,01027u,01030u,
 01031u,01032u,01033u,01035u,01036u,01037u,01040u,01041u,01042u,01044u,01045u,01046u,
 01047u,01050u,01051u,01052u,01053u,01054u,01055u,01056u,01057u,01060u,01061u,01062u,
 01063u,01064u,01065u,01066u,01067u,01070u,01071u,01074u,01076u,01077u,01100u,01101u,
 01102u,01103u,01104u,01105u,01106u,01107u,01110u,01111u,01112u,01113u,01114u,01115u,
 01116u,01117u,01120u,01121u,01122u,01123u,01124u,01125u,01126u,01130u,01131u,01132u,
 01133u,01136u,01141u,01143u,01144u,01145u,01146u,01147u,01150u,01151u,01152u,01153u,
 01154u,01155u,01156u,01157u,01160u,01161u,01162u,01165u,01170u,01172u,01174u,01176u,
 01177u,01201u,01202u,01204u,01205u,01207u,01210u,01211u,01212u,01213u,01215u,01216u,
 01217u,01220u,01221u,01223u,01224u,01225u,01226u,01227u,01230u,01231u,01232u,01233u,
 01234u,01235u,01236u,01237u,01240u,01241u,01242u,01243u,01244u,01245u,01246u,01247u,
 01250u,01251u,01252u,01253u,01254u,01255u,01256u,01257u,01260u,01261u,01262u,01263u,
 01264u,01265u,01266u,01267u,01270u,01271u,01272u,01273u,01274u,01275u,01276u,01277u,
 01301u,01302u,01303u,01304u,01305u,01306u,01307u,01310u,01311u,01312u,01313u,01314u,
 01315u,01316u,01317u,01321u,01322u,01325u,01326u,01327u,01330u,01331u,01332u,01333u,
 01334u,01335u,01336u,01337u,01340u,01341u,01342u,01343u,01344u,01345u,01346u,01347u,
 01350u,01351u,01352u,01353u,01354u,01355u,01356u,01357u,01360u,01361u,01362u,01364u,
 01365u,01366u,01367u,01370u,01371u,01372u,01373u,01374u,01375u,01376u,01377u,01401u,
 01402u,01403u,01404u,01405u,01407u,01410u,01411u,01414u,01415u,01416u,01417u,01420u,
 01422u,01424u,01425u,01426u,01427u,01430u,01431u,01432u,01433u,01434u,01435u,01436u,
 01437u,01440u,01441u,01442u,01443u,01444u,01445u,01446u,01447u,01450u,01451u,01452u,
 01453u,01454u,01455u,01456u,01457u,01460u,01461u,01462u,01464u,01466u,01467u,01470u,
 01471u,01472u,01474u,01475u,01476u,01500u,01501u,01502u,01503u,01504u,01505u,01506u,
 01507u,01510u,01511u,01513u,01514u,01515u,01516u,01517u,01520u,01521u,01522u,01523u,
 01524u,01525u,01526u,01527u,01530u,01531u,01532u,01533u,01534u,01535u,01536u,01537u,
 01540u,01541u,01542u,01543u,01544u,01545u,01546u,01547u,01550u,01553u,01555u,01557u,
 01560u,01562u,01564u,01566u,01567u,01570u,01600u,01601u,01602u,01603u,01606u,01607u,
 01610u,01611u,01612u,01613u,01614u,01615u,01616u,01617u,01620u,01621u,01622u,01625u,
 01626u,01627u,01630u,01631u,01632u,01633u,01634u,01635u,01636u,01637u,01640u,01641u,
 01642u,01643u,01644u,01645u,01646u,01651u,01652u,01653u,01654u,01655u,01656u,01657u,
 01660u,01661u,01662u,01663u,01664u,01665u,01666u,01667u,01670u,01671u,01672u,01673u,
 01674u,01675u,01676u,01677u,01700u,01701u,01702u,01705u,01706u,01707u,01710u,01711u,
 01712u,01713u,01714u,01715u,01716u,01717u,01720u,01721u,01722u,01723u,01724u,01731u,
 01732u,01733u,01734u,01735u,01736u,01737u,01740u,01741u,01742u,01743u,01744u,01745u,
 01746u,01747u,01750u,01751u,01752u,01753u,01754u,01755u,01756u,01757u,01760u,01761u,
 01762u,01763u,01764u,01765u,01766u,01767u,01770u,01771u,01772u,01773u,01774u,01775u,
 01776u,01777u,02000u,02001u,02002u,02003u,02004u,02005u,02006u,02007u,02010u,02011u,
 02012u,02013u,02014u,02015u,02016u,02017u,02020u,02021u,02022u,02023u,02024u,02025u,
 02026u,02027u,02030u,02031u,02032u,02033u,02034u,02035u,02036u,02037u,02040u,02041u,
 02042u,02043u,02100u,02101u,02102u,02104u,02106u,02107u,02110u,02111u,02115u,02116u,
 02121u,02124u,02126u,02127u,02140u,02141u,02142u,02143u,02144u,02145u,02146u,02147u,
 02150u,02151u,02152u,02153u,02154u,02155u,02156u,02157u,02160u,02161u,02162u,02163u,
 02164u,02165u,02166u,02167u,02170u,02171u,02172u,02173u,02174u,02200u,02202u,02210u,
 02211u,02215u,02217u,02221u,02223u,02225u,02227u,02231u,02232u,02233u,02234u,02235u,
 02236u,02240u,02241u,02242u,02243u,02244u,02245u,02246u,02247u,02250u,02251u,02252u,
 02253u,02254u,02255u,02260u,02261u,02262u,02263u,02270u,02272u,02274u,02275u,02276u,
 02277u,02300u,02301u,02302u,02303u,02304u,02305u,02306u,02307u,02310u,02311u,02312u,
 02313u,02314u,02315u,02316u,02317u,02320u,02321u,02322u,02323u,02325u,02326u,02327u,
 02330u,02331u,02332u,02333u,02334u,02335u,02336u,02337u,02340u,02341u,02342u,02343u,
 02344u,02345u,02346u,02400u,02401u,02402u,02403u,02404u,02410u,02411u,02412u,02413u,
 02414u,02415u,02416u,02417u,02420u,02421u,02422u,02423u,02424u,02425u,02426u,02427u,
 02430u,02431u,02432u,02433u,02434u,02435u,02436u,02437u,02440u,02441u,02442u,02443u,
 02444u,02445u,02446u,02447u,02450u,02451u,02452u,02453u,02454u,02455u,02456u,02457u,
 02460u,02461u,02462u,02463u,02464u,02465u,02466u,02467u,02470u,02471u,02472u,02473u,
 02474u,02475u,02476u,02477u,02500u,02501u,02502u,02503u,02504u,02505u,02506u,02507u,
 02510u,02511u,02512u,02513u,02514u,02515u,02516u,02517u,02520u,02521u,02522u,02523u,
 02524u,02525u,02526u,02527u,02530u,02531u,02532u,02533u,02534u,02535u,02536u,02537u,
 02540u,02541u,02542u,02543u,02544u,02545u,02546u,02547u,02550u,02551u,02552u,02553u,
 02554u,02555u,02556u,02557u,02560u,02561u,02562u,02563u,02564u,02565u,02566u,02567u,
 02570u,02571u,02572u,02573u,02574u,02575u,02576u,02577u,02600u,02601u,02602u,02603u,
 02604u,02605u,02606u,02607u,02610u,02611u,02612u,02613u,02614u,02615u,02616u,02617u,
 02620u,02621u,02622u,02623u,02624u,02625u,02626u,02627u,02630u,02631u,02632u,02633u,
 02634u,02635u,02636u,02637u,02640u,02641u,02642u,02643u,02644u,02645u,02646u,02647u,
 02650u,02651u,02652u,02653u,02654u,02655u,02656u,02657u,02660u,02661u,02662u,02663u,
 02664u,02665u,02666u,02667u,02670u,02671u,02672u,02673u,02674u,02675u,02676u,02677u,
 02700u,02701u};

 {static agc_routine_table basic[1],interp[1];basic[0]=lm_servicer_routines;interp[0]=lm_servicer_interpretive;
  program.basic=basic;program.basic_count=1;program.interpretive=interp;program.interpretive_count=1;
  program.interpreter_fetch=(uint16_t)(2048u+06070u-04000u);program.loc=0164;program.bankset=0165;}
 memset(lmsv_paths,0,sizeof lmsv_paths);
 /* Paths count only in the runs that are compared (not refused). */
 for(n=0;n<LMSV_CASES;++n){unsigned kind;
  memcpy(saved,lmsv_paths,sizeof saved);
  check(lmsv_digest(&s,(uint16_t)n)==lmsv_expected[n],"digest",n);
  memcpy(lmsv_paths,saved,sizeof saved);
  kind=lmsv_run(&s,(uint16_t)n,0);
  if(kind==LMSV_REFUSED)memcpy(lmsv_paths,saved,sizeof saved);
  if(kind==LMSV_STOPPED){++stopped;++reached[n%LMSV_ROUTINES];}
  if(kind==LMSV_REFUSED)++refusals;
  check(kind!=3u,"every case leaves the file or is refused",n);}
 for(i=0;i<LMSV_ROUTINES;++i)check(reached[i]>0u,"routine reached",i);
 for(i=0;i<sizeof paths/sizeof paths[0];++i){unsigned j,skip=0;
  for(j=0;j<sizeof unreachable/sizeof unreachable[0];++j)skip|=unreachable[j]==paths[i];
  if(!skip)check(lmsv_paths[paths[i]]>0u,"path taken",paths[i]);}
 /* Inputs no producer supplies. */
 refused(slot(K(33,02730u)),fb10,"CALCGRAV under FB 10");
 refused(slot(K(33,02730u)),vac_bad,"CALCGRAV with a work area off the VAC areas");
 refused(slot(K(33,02730u)),edop,"CALCGRAV with EDOP set");
 refused(slot(K(33,02730u)),newjob_neg,"CALCGRAV with NEWJOB negative");
 refused(slot(K(33,02664u)),a_odd,"ALTCHK with A 3");
 refused(slot(K(33,02664u)),a_eight,"ALTCHK with A 8");
 refused(slot(K(33,03315u)),vac_bad,"VELUPDAT with FIXLOC off the VAC areas");
 refused(slot(K(33,03466u)),vselect_bad,"33,3466 with VSELECT* 5");
 refused(slot(K(33,03534u)),a_odd,"GNURVST with A 3");
 refused(slot(K(33,03740u)),a_four,"SETPOS with A 4");
 refused(slot(K(33,03534u)),q_erasable,"GNURVST's TC Q to erasable");
 refused(slot(K(33,03534u)),q_dependent,"GNURVST's TC Q onto a word after EXTEND");
 refused(slot(K(33,03774u)),lradret_erasable,"TC LRADRET to erasable");
 refused(slot(K(33,02352u)),phase4_neg,"LOTHRUST with PHASE4 negative");
 refused(slot(K(33,03633u)),stilbadv_m0,"33,3633 with STILBADV -0");
 refused(slot(K(33,03534u)),a_overflow,"GNURVST with overflow in A");
 refused(slot(K(37,03410u)),no_undo,"SERVICER without an undo image");
 printf("lm-servicer: %u cases left the file, %u refusals; routines",stopped,refusals);
 for(i=0;i<LMSV_ROUTINES;++i)printf(" %u",reached[i]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
