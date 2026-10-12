/* SPDX-License-Identifier: MIT */
/* lm-burn-baby: Luminary099 BURN_BABY_BURN as native routines, against
 * digests frozen from the original program; every routine, every reachable
 * instruction and order pair and decision outcome is taken; inputs no
 * original producer supplies are refused with the image unchanged. Built
 * with LMBB_COUNT_PATHS. */
#include <stdio.h>
#include <string.h>
#include "lm_burn_baby_cases.h"
#include "lm_burn_baby_vectors.h"
#define PATHS 01572u
unsigned lmbb_paths[PATHS];
static unsigned checks,failures;
static void check(int ok,const char *what,unsigned n){++checks;if(!ok){++failures;if(failures<20u)fprintf(stderr,"FAIL %s %u\n",what,n);}}
static apollo_overlay_state undo;
static agc_native_program program={0,0,0,0,0,0,AGC_TRANSFER_NONE,0,0,0,0,0};
static unsigned reached[LMBB_ROUTINES];
/* Case n's image with `edit` applied must be refused, image unchanged. */
static void refused(uint16_t n,void (*edit)(apollo_overlay_state *,lm_bb_io *),const char *what){
 apollo_overlay_state s,before;lmbb_case c;lm_bb_io io;agc_routine call;
 lmbb_input(&s,&c,n);io.control.superbank=c.superbank;io.control.interrupt_enabled=c.interrupt_enabled;
 io.chan11=c.chan11;io.chan14=c.chan14;io.chan30=c.chan30;io.undo=&undo;
 call=agc_transfer_resolve(&program,&s,c.superbank);
 edit(&s,&io);before=s;
 check(call&&call(&program,&s,&io)==EXEC_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s),what,n);
}
#define E7(a) (03400u+(a)-01400u)
static void fb10(apollo_overlay_state *s,lm_bb_io *io){(void)io;s->words[4]=020000u;s->words[6]=(agc_word)(020000u|(s->words[3]>>8));}
static void bankset10(apollo_overlay_state *s,lm_bb_io *io){(void)io;s->words[0165]=020000u;}
static void vac_bad(apollo_overlay_state *s,lm_bb_io *io){(void)io;s->words[0120]=0402u;}
static void edop(apollo_overlay_state *s,lm_bb_io *io){(void)io;s->words[023]=1u;}
static void newjob_neg(apollo_overlay_state *s,lm_bb_io *io){(void)io;s->words[067]=040000u;}
static void which_bad(apollo_overlay_state *s,lm_bb_io *io){(void)io;s->words[E7(01455u)]=02023u;}
static void phase5_neg(apollo_overlay_state *s,lm_bb_io *io){(void)io;s->words[0763]=070000u;}
static void phase3_neg(apollo_overlay_state *s,lm_bb_io *io){(void)io;s->words[0757]=077777u;}
static void dispdex_m0(apollo_overlay_state *s,lm_bb_io *io){(void)io;s->words[01163]=077777u;}
static void dispdex_bad(apollo_overlay_state *s,lm_bb_io *io){(void)io;s->words[01163]=077777u-5u;}
static void p40ret_bad(apollo_overlay_state *s,lm_bb_io *io){(void)io;s->words[01142]=03015u;}
static void q_erasable(apollo_overlay_state *s,lm_bb_io *io){(void)io;s->words[2]=01000u;}
static void q_not_site(apollo_overlay_state *s,lm_bb_io *io){(void)io;s->words[2]=02000u;}
static void a_overflow(apollo_overlay_state *s,lm_bb_io *io){(void)io;s->words[0]=0100000u;}
static void q_dependent(apollo_overlay_state *s,lm_bb_io *io){(void)io;s->words[2]=02327u;s->words[4]=074000u;s->words[6]=(agc_word)(074000u|(s->words[3]>>8));}
static void no_undo(apollo_overlay_state *s,lm_bb_io *io){(void)s;io->undo=0;}
/* The first case of the routine at physical key k. */
static uint16_t slot(uint16_t k){unsigned i;
 for(i=0;i<LMBB_ROUTINES;++i)if(lmbb_routine_keys[i]==k)return (uint16_t)i;
 return 0;}
#define K36(a) (uint16_t)(036u*1024u+((a)-02000u))
#define K27(a) (uint16_t)(027u*1024u+((a)-02000u))
int main(void){apollo_overlay_state s;unsigned n,i,stopped=0,refusals=0;
 /* Paths no admitted input reaches: the tables' fifth words (TCF
  * TASKOVER, 02026, 02042, 02065, 02102, 02116), which no INDEX WHICH in
  * Luminary099 selects; the negative and -0 outcomes of a CCS of a MASK
  * result (P42IGN BR 23-24, IGNITE BR 40-41, P40A/P BR 48-49), refused
  * if ever taken; ABVAL's alarm (BR 2), which needs SQRT of a negative
  * sum of squares. */
 static const unsigned unreachable[]={04u,020u,043u,060u,074u,01502u,01527u,01530u,01550u,01551u,01560u,01561u};
 /* Every instruction address (bank 36 a - 02022; bank 35 01200 + a -
  * 03747; bank 17 01240 + a - 02000; bank 27 01300 + a - 02200;
  * fixed-fixed 01350 + a - 06027), order pair (01400 + LOC - 02152) and
  * decision outcome (01500 + n) of the file. */
 static const unsigned paths[]={
 01u,02u,03u,04u,05u,011u,012u,013u,015u,016u,017u,020u,
 021u,025u,026u,027u,030u,031u,035u,036u,040u,041u,042u,043u,
 044u,050u,051u,052u,053u,055u,056u,057u,060u,061u,065u,066u,
 067u,071u,072u,073u,074u,075u,076u,077u,0100u,0101u,0102u,0103u,
 0104u,0106u,0107u,0110u,0111u,0113u,0114u,0115u,0116u,0117u,0121u,0122u,
 0123u,0124u,0125u,0127u,0160u,0161u,0162u,0163u,0164u,0165u,0166u,0167u,
 0170u,0171u,0172u,0173u,0174u,0175u,0176u,0201u,0203u,0205u,0206u,0207u,
 0210u,0215u,0216u,0217u,0221u,0223u,0224u,0225u,0226u,0227u,0230u,0231u,
 0232u,0234u,0235u,0240u,0241u,0243u,0244u,0245u,0250u,0251u,0252u,0254u,
 0255u,0257u,0260u,0261u,0262u,0263u,0264u,0265u,0266u,0270u,0271u,0272u,
 0273u,0274u,0275u,0276u,0277u,0300u,0301u,0302u,0303u,0304u,0305u,0306u,
 0307u,0310u,0311u,0312u,0313u,0314u,0315u,0316u,0317u,0320u,0321u,0322u,
 0324u,0325u,0327u,0330u,0331u,0332u,0333u,0334u,0336u,0340u,0342u,0343u,
 0344u,0345u,0346u,0347u,0352u,0354u,0355u,0356u,0360u,0361u,0362u,0363u,
 0364u,0366u,0367u,0370u,0373u,0377u,0400u,0401u,0402u,0403u,0404u,0405u,
 0406u,0407u,0410u,0411u,0412u,0413u,0414u,0415u,0416u,0417u,0420u,0421u,
 0422u,0423u,0424u,0425u,0426u,0427u,0430u,0431u,0432u,0433u,0434u,0435u,
 0436u,0437u,0440u,0441u,0442u,0443u,0444u,0445u,0446u,0447u,0450u,0451u,
 0452u,0453u,0454u,0455u,0456u,0457u,0460u,0461u,0462u,0463u,0464u,0465u,
 0466u,0467u,0472u,0476u,0477u,0500u,0501u,0502u,0503u,0504u,0505u,0506u,
 0507u,0510u,0511u,0512u,0513u,0514u,0515u,0516u,0517u,0520u,0521u,0522u,
 0523u,0524u,0525u,0526u,0530u,0532u,0534u,0536u,0540u,0541u,0542u,0543u,
 0544u,0545u,0547u,0551u,0553u,0554u,0555u,0556u,0557u,0561u,0562u,0563u,
 0565u,0566u,0567u,0570u,0571u,0572u,0573u,0574u,0576u,0600u,0601u,0603u,
 0604u,0606u,0610u,0612u,0614u,0616u,0617u,0620u,0622u,0623u,0624u,0625u,
 0626u,0627u,0630u,0631u,0632u,0633u,0634u,0635u,0636u,0637u,0640u,0641u,
 0642u,0643u,0644u,0645u,0646u,0647u,0650u,0651u,0652u,0653u,0654u,0655u,
 0656u,0657u,0660u,0661u,0662u,0663u,0664u,0665u,0667u,0673u,0674u,0675u,
 0676u,0677u,0700u,0701u,0702u,0703u,0706u,0710u,0711u,0712u,0713u,0714u,
 0715u,0716u,0717u,0720u,0721u,0722u,0723u,0724u,0725u,0726u,0727u,0730u,
 0731u,0732u,0733u,0734u,0735u,0736u,0740u,0741u,0742u,0743u,0744u,0746u,
 0747u,0750u,0751u,0752u,0753u,0755u,0756u,0757u,0760u,0761u,0763u,0764u,
 0765u,0766u,0770u,0771u,0772u,0773u,0774u,0775u,0776u,0777u,01001u,01003u,
 01004u,01005u,01006u,01010u,01011u,01012u,01013u,01016u,01017u,01021u,01022u,01023u,
 01024u,01025u,01026u,01031u,01032u,01034u,01035u,01037u,01040u,01041u,01044u,01046u,
 01047u,01051u,01052u,01057u,01060u,01061u,01062u,01063u,01064u,01065u,01066u,01070u,
 01071u,01072u,01073u,01074u,01075u,01076u,01077u,01101u,01102u,01104u,01105u,01106u,
 01107u,01110u,01200u,01201u,01202u,01204u,01205u,01206u,01207u,01210u,01211u,01212u,
 01213u,01214u,01215u,01216u,01217u,01220u,01222u,01223u,01224u,01225u,01240u,01241u,
 01244u,01246u,01247u,01250u,01251u,01252u,01254u,01255u,01256u,01257u,01260u,01261u,
 01262u,01264u,01300u,01301u,01302u,01303u,01304u,01305u,01306u,01307u,01310u,01311u,
 01312u,01313u,01314u,01316u,01317u,01320u,01321u,01322u,01323u,01324u,01325u,01326u,
 01327u,01330u,01331u,01332u,01334u,01335u,01336u,01337u,01340u,01341u,01342u,01344u,
 01345u,01350u,01351u,01352u,01353u,01354u,01355u,01356u,01400u,01403u,01405u,01411u,
 01414u,01415u,01417u,01421u,01423u,01425u,01426u,01461u,01463u,01464u,01500u,01501u,
 01502u,01503u,01504u,01505u,01506u,01507u,01510u,01511u,01512u,01513u,01514u,01515u,
 01516u,01517u,01520u,01521u,01522u,01523u,01524u,01525u,01526u,01527u,01530u,01531u,
 01532u,01533u,01534u,01535u,01536u,01537u,01540u,01541u,01542u,01543u,01544u,01545u,
 01546u,01547u,01550u,01551u,01552u,01553u,01554u,01555u,01556u,01557u,01560u,01561u,
 01562u,01563u,01564u,01565u,01566u,01567u,01570u,01571u};
 {static agc_routine_table basic[1],interp[1];basic[0]=lm_burn_baby_routines;interp[0]=lm_burn_baby_interpretive;
  program.basic=basic;program.basic_count=1;program.interpretive=interp;program.interpretive_count=1;
  program.interpreter_fetch=(uint16_t)(2048u+06070u-04000u);program.loc=0164;program.bankset=0165;}
 memset(lmbb_paths,0,sizeof lmbb_paths);
 for(n=0;n<LMBB_CASES;++n){unsigned kind;
  check(lmbb_digest(&s,(uint16_t)n)==lmbb_expected[n],"digest",n);
  kind=lmbb_run(&s,(uint16_t)n,0);
  if(kind==LMBB_STOPPED){++stopped;++reached[n%LMBB_ROUTINES];}
  if(kind==LMBB_REFUSED)++refusals;
  check(kind!=3u,"every case leaves the file or is refused",n);}
 for(i=0;i<LMBB_ROUTINES;++i)check(reached[i]>0u,"routine reached",i);
 for(i=0;i<sizeof paths/sizeof paths[0];++i){unsigned j,skip=0;
  for(j=0;j<sizeof unreachable/sizeof unreachable[0];++j)skip|=unreachable[j]==paths[i];
  if(!skip)check(lmbb_paths[paths[i]]>0u,"path taken",paths[i]);}
 /* Inputs no producer supplies. */
 refused(slot(K36(02126u)),fb10,"BURNBABY under FB 10");
 refused(slot(K36(02157u)),bankset10,"36,2157 with BANKSET bank 10");
 refused(slot(K36(02163u)),vac_bad,"36,2163 with a work area off the VAC areas");
 refused(slot(K36(02175u)),edop,"36,2175 with EDOP set");
 refused(slot(K36(02163u)),newjob_neg,"36,2163 with NEWJOB negative");
 refused(slot(K36(02143u)),which_bad,"B*RNB*B* with WHICH not a table");
 refused(slot(K36(02342u)),phase5_neg,"REDO4.2 with PHASE5 negative");
 refused(slot(K36(02366u)),phase3_neg,"P40SJUNK with PHASE3 -0");
 refused(slot(K36(02737u)),dispdex_m0,"CLOKJOB with DISPDEX -0");
 refused(slot(K36(02737u)),dispdex_bad,"CLOKJOB with DISPDEX -5");
 refused(slot(K36(03025u)),p40ret_bad,"NULLCLOK's TC P40/RET with an unknown word");
 refused(slot(K36(02653u)),q_erasable,"NOULLAGE's TC Q to erasable");
 refused(slot(06027u),q_not_site,"KILLTASK with a CADR not in the site table");
 refused(slot(K27(02200u)),a_overflow,"KILLTSK2 with overflow in A");
 refused(slot(K36(02657u)),q_dependent,"ONULLAGE's TC Q onto an indexed word");
 refused(slot(K36(02126u)),no_undo,"BURNBABY without an undo image");
 printf("lm-burn-baby: %u cases left the file, %u refusals; routines",stopped,refusals);
 for(i=0;i<LMBB_ROUTINES;++i)printf(" %u",reached[i]);
 printf("; %u checks, %u failures\n",checks,failures);
 return failures!=0u;
}
