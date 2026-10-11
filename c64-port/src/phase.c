/* SPDX-License-Identifier: MIT */
#include "phase.h"
enum phase_status phase_change(phase_state *s,const agc_word *words,uint8_t count,uint8_t two) {
 agc_word w, first=0, phase, oldbank=0, final_l=0;
 uint8_t i,k=0,group,firstgroup=0,variable,newpriority,newaddress;
 if(two>1 || count>5 || count<(uint8_t)(1+two) || s->q>07770 || (s->bbank&001770)!=0 || s->bbank>AGC_MASK ||
    s->superbank>AGC_MASK || (s->superbank&077617)!=0 || s->interrupt_enabled>1) return PHASE_OUT_OF_SCOPE;
 for(i=0;i<count;++i) if(words[i]>AGC_MASK) return PHASE_OUT_OF_SCOPE;
 if(two) { first=words[k++]; firstgroup=(uint8_t)(first&7);
  if(firstgroup==0 || firstgroup>6 || (first&014000)!=0) return PHASE_OUT_OF_SCOPE;
 }
 w=words[k++]; group=(uint8_t)(w&7);
 if(group==0 || group>6) return PHASE_OUT_OF_SCOPE;
 phase=(agc_word)((w&017770)>>3); variable=(uint8_t)((w&014000)!=0);
 newpriority=(uint8_t)(variable && ((w&010000)!=0 ? (w&04000)!=0 : (w&01000)!=0));
 newaddress=(uint8_t)(variable && (w&02000)!=0);
 if(count<(uint8_t)(k+newpriority+2*newaddress)) return PHASE_OUT_OF_SCOPE;
 s->q=(agc_word)(s->q+k);
 if(two) { s->scratch[PH_TEMPG2]=(agc_word)(firstgroup*2);
  s->scratch[PH_TEMPP2]=(agc_word)((first&017770)>>3);
  s->scratch[PH_TEMPSW2]=(agc_word)(first&040000);
 } else s->scratch[PH_TEMPSW2]=1;
 s->scratch[PH_TEMPG]=(agc_word)(group*2); s->scratch[PH_TEMPP]=phase;
 s->scratch[PH_TEMPSW]=(agc_word)(w&060000);
 --group;
 if(variable) {
  s->scratch[PH_TEMPPR]=newpriority ? words[k++] : s->records.priority[group];
  if(newpriority) ++s->q;
  if(newaddress) {
   s->scratch[PH_TEMPNM]=words[k++]; s->scratch[PH_TEMPBB]=words[k++]; s->q=(agc_word)(s->q+2);
  } else { s->scratch[PH_TEMPNM]=s->q;
   s->scratch[PH_TEMPBB]=(agc_word)(s->bbank|s->superbank);
  }
  s->records.priority[group]=s->scratch[PH_TEMPPR];
  oldbank=s->records.address[group].bank; final_l=oldbank;
  s->records.address[group].location=s->scratch[PH_TEMPNM];
  s->records.address[group].bank=s->scratch[PH_TEMPBB];
 }
 /* BELOW1 reads the first group after the second group store. Preserve
  * that order even if both group numbers agree, as the original does. */
 s->records.phase[group]=phase;
 if(two) { --firstgroup; final_l=s->records.phase[firstgroup]; s->records.phase[firstgroup]=s->scratch[PH_TEMPP2];
  s->records.complement[firstgroup]=(agc_word)(AGC_MASK^s->scratch[PH_TEMPP2]);
  s->scratch[PH_TEMPP2]=oldbank;
  if((first&040000)!=0) s->tbase[firstgroup]=(agc_word)(AGC_MASK^s->time[1]);
 }
 if((w&040000)!=0) s->tbase[group]=(agc_word)(AGC_MASK^s->time[1]);
 if((w&020000)!=0) { final_l=s->longbase[1]; s->longbase[0]=s->time[0]; s->longbase[1]=s->time[1]; }
 s->records.complement[group]=(agc_word)(AGC_MASK^phase);
 /* DTCB exchanges the caller return pair with Z/BBANK: next source
  * instruction 10,2312 and the working phase bank/erasable bank 10/E3. */
 s->a=02312; s->l=020003; s->scratch[PH_TEMPBBCN]=final_l;
 s->interrupt_enabled=1;
 return PHASE_OK;
}
