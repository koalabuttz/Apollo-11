/* SPDX-License-Identifier: MIT */
#include "standard_lead.h"
static void bank(apollo_overlay_state *s,agc_word bb){s->words[6]=(agc_word)(bb&076007u);s->words[3]=(agc_word)((bb&7u)<<8);s->words[4]=(agc_word)(bb&076000u);}
static int context(const apollo_overlay_state *s,enum standard_lead_mission mission){return s&&(unsigned)mission<=1u&&s->words[7]==0&&s->words[3]<=03400u&&(s->words[3]&0377u)==0&&(s->words[4]&~076000u)==0&&s->words[6]==(agc_word)(s->words[4]|(s->words[3]>>8))&&s->words[0120]>=0401u&&s->words[0120]<=0661u&&(s->words[0120]-0401u)%054u==0;}
bool standard_lead_interp_call(apollo_overlay_state *s,enum standard_lead_mission mission){agc_word *w,ebuf,loc,callee,fb,bb,oldloc;
 if(!context(s,mission))return false;
 w=s->words;ebuf=mission==STANDARD_LEAD_LM?01167:0364;loc=mission==STANDARD_LEAD_LM?02624:02064;fb=mission==STANDARD_LEAD_LM?010000:026000;
 callee=w[ebuf];if(w[5]!=(mission==STANDARD_LEAD_LM?06037u:06006u)||w[2]!=loc||w[4]!=fb||callee<004000u||callee>AGC_MASK)return false;
 bb=w[6];oldloc=w[0164];w[2]=agc_extend(oldloc);w[w[0120]+052u]=(agc_word)(fb+loc+2u-02000u);w[0117]=callee;
 bank(s,(agc_word)((callee&076000u)|(bb&7u)));w[0165]=w[6];w[0164]=(agc_word)(02000u+(callee&01777u));w[0115]=(agc_word)(callee&040000u);w[0]=agc_extend(w[0115]);w[1]=agc_extend(agc_correct(w[1]));w[020]=050006;w[023]=0;w[5]=mission==STANDARD_LEAD_LM?06070:06037;return true;
}
bool standard_lead_interp_rvq(apollo_overlay_state *s,enum standard_lead_mission mission){agc_word *w,ret;
 if(!context(s,mission))return false;
 w=s->words;ret=w[w[0120]+052u];
 if(ret!=(mission==STANDARD_LEAD_LM?010626u:026066u)||w[5]!=(mission==STANDARD_LEAD_LM?06070u:06037u))return false;
 bank(s,(agc_word)((ret&076000u)|(w[0165]&7u)));w[0165]=w[6];w[0117]=ret;w[0164]=(agc_word)(02000u+(ret&01777u));w[0115]=(agc_word)(ret&040000u);w[0]=agc_extend(w[0115]);w[1]=agc_extend(agc_correct(w[1]));w[020]=7;w[023]=0;return true;
}
bool standard_lead_interp_exit(apollo_overlay_state *s,enum standard_lead_mission mission){agc_word *w,loc;
 if(!context(s,mission))return false;
 w=s->words;loc=mission==STANDARD_LEAD_LM?02626:02066;
 if(w[5]!=(mission==STANDARD_LEAD_LM?06070u:06037u)||w[0164]!=loc||(w[0165]&076000u)!=(mission==STANDARD_LEAD_LM?010000u:026000u))return false;
 bank(s,w[0165]);w[0]=agc_extend(w[0165]);w[1]=agc_extend(agc_correct(w[1]));w[020]=0;w[023]=0;w[2]=mission==STANDARD_LEAD_LM?06747:06716;w[5]=(agc_word)(loc+1u);return true;
}
bool standard_lead_interp_finish(apollo_overlay_state *s,enum standard_lead_mission mission){agc_word *w,oldloc;
 if(!context(s,mission))return false;
 w=s->words;if(w[5]!=(mission==STANDARD_LEAD_LM?06041u:06010u)||w[4]!=(mission==STANDARD_LEAD_LM?010000u:026000u)||agc_correct(w[1])>03777u)return false;
 oldloc=w[0164];w[0164]=agc_correct(w[1]);w[1]=agc_extend(oldloc);w[0165]=w[6];w[0115]=(agc_word)(w[6]&040000u);w[0]=agc_extend(w[0115]);w[023]=0;w[5]=mission==STANDARD_LEAD_LM?06070:06037;return true;
}
bool standard_lead_interp_control(apollo_overlay_state *s,p66_bias_io *io,enum standard_lead_mission mission,enum standard_lead_interp_boundary boundary){bool ok;
 if(!io||(io->superbank&~0160u)||io->interrupt_enabled>1u||(unsigned)boundary>3u)return false;
 if(boundary==STANDARD_LEAD_CALL)ok=standard_lead_interp_call(s,mission);
 else if(boundary==STANDARD_LEAD_RVQ)ok=standard_lead_interp_rvq(s,mission);
 else if(boundary==STANDARD_LEAD_EXIT)ok=standard_lead_interp_exit(s,mission);
 else ok=standard_lead_interp_finish(s,mission);
 if(ok&&boundary==STANDARD_LEAD_CALL)io->interrupt_enabled=1;
 return ok;
}
