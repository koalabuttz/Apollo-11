/* SPDX-License-Identifier: MIT
 * Fixed STRTP66A source sequence, Luminary099 guidance lines149..173.
 * No generic operation decoder: all inputs, stores and continuation metadata
 * below are explicit bindings of this single original initialization path. */
#include "p66_bias.h"
#include "guidance_product.h"
enum p66_bias_status p66_bias_start(apollo_overlay_state *image,p66_bias_io *io) {
 p66_preparation s={0};p66_scale_scratch scale;agc_dp factor={0,01217};
 uint16_t *w=image->words,base=w[0120],last_low=w[03760];uint8_t i;
 static const uint8_t positions[]={0,3,5};
 if(w[6]!=062007 || w[4]!=062000 || w[3]!=03400 || w[067]!=0 ||
    base<0401 || base>0661 || (base-0401u)%054u!=0 || w[0166]!=base ||
    w[base-1u]!=0 || (w[0167]&0777u)!=base || w[0167]<01000 || w[0167]>=040000 ||
    io->superbank>060 || (io->superbank&017u)!=0 || io->interrupt_enabled>1)
  return P66_BIAS_OUT_OF_SCOPE;
 /* Three SLOADs clear low/third words; PUSH retains Z/Y scalar pairs in the
  * owned VAC. VDEF pops those two pairs into vector component positions. */
 w[base]=w[01456];w[base+1u]=0;w[base+2u]=w[01454];w[base+3u]=0;
 for(i=0;i<3;++i){s.mpac[positions[i]]=w[01452+2u*i];s.mpac[positions[i]+1u]=0;}
 s.mpac[2]=0;s.mpac[7]=077776;scale.mptemp=w[0135];scale.ovfind=w[0121];
 (void)p66_vector_multiply(&s,&scale,&factor);
 for(i=0;i<3;++i){w[02630+2u*i]=s.mpac[positions[i]];w[02631+2u*i]=s.mpac[positions[i]+1u];}
 w[0135]=scale.mptemp;w[0121]=scale.ovfind;
 /* SET RODFLAG (FLAGWRD1 bit12). These are the unchanged SET handler's
  * mask/index scratch values; POLISH retains the inline flag descriptor. */
 w[075]=(uint16_t)(w[075]|04000u);w[0130]=1;w[0131]=04000;w[0117]=00463;
 /* STOVL VBIAS/TEMX; VCOMP; STOVL OLDPIPAX/ZEROVECS. The six-word
  * OLDPIPAX store aliases the first three DELVROD words; subsequent STODL
  * DELVROD clears all six, so final old PIPA retains the first three. */
 for(i=0;i<6;++i)w[03763+i]=agc_negate(w[01254+i]);
 for(i=0;i<6;++i)w[03766+i]=0;
 /* STODL RODSCAL1 writes two words at3756/3757; STORE LASTTPIP then
  * overwrites3757. Preserve this real source alias rather than two structs. */
 w[03756]=w[02537];w[03757]=w[02540];w[03757]=w[01234];w[03760]=w[01235];
 w[0154]=w[01234];w[0155]=w[01235];
 for(i=0156;i<=0163;++i)w[i]=0;
 w[0164]=02531;w[0165]=062007;w[0166]=base;
 /* EXIT restores BANKSET; basic CAF/TS clears the weight history. */
 w[03620]=0;w[03610]=0;w[03611]=0;
 w[0]=0;w[1]=agc_extend(last_low);w[2]=06747;w[5]=02536;w[7]=0;
 w[020]=0;w[023]=0;w[0115]=040000;w[0116]=01757;
 io->interrupt_enabled=1;
 return P66_BIAS_OK;
}
