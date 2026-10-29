/* SPDX-License-Identifier: MIT */
#include "lm_fixed_compat.h"
#include "lm_fixed_compat_words.inc"
bool lm_fixed_compat_read(uint16_t address,agc_word *out){
 if(out==0||address>=40960u)return false;
 if(address>=36864u){*out=0;return true;}
#if !defined(LM_FIXED_COMPAT_GROUP) || LM_FIXED_COMPAT_GROUP == 0
 if(address<7168u){*out=lm_fixed_compat_group_0[address];return true;}
#endif
#if !defined(LM_FIXED_COMPAT_GROUP) || LM_FIXED_COMPAT_GROUP == 1
 if(address>=7168u&&address<14336u){*out=lm_fixed_compat_group_1[address-7168u];return true;}
#endif
#if !defined(LM_FIXED_COMPAT_GROUP) || LM_FIXED_COMPAT_GROUP == 2
 if(address>=14336u&&address<21504u){*out=lm_fixed_compat_group_2[address-14336u];return true;}
#endif
#if !defined(LM_FIXED_COMPAT_GROUP) || LM_FIXED_COMPAT_GROUP == 3
 if(address>=21504u&&address<28672u){*out=lm_fixed_compat_group_3[address-21504u];return true;}
#endif
#if !defined(LM_FIXED_COMPAT_GROUP) || LM_FIXED_COMPAT_GROUP == 4
 if(address>=28672u&&address<35840u){*out=lm_fixed_compat_group_4[address-28672u];return true;}
#endif
#if !defined(LM_FIXED_COMPAT_GROUP) || LM_FIXED_COMPAT_GROUP == 5
 if(address>=35840u){*out=lm_fixed_compat_group_5[address-35840u];return true;}
#endif
 return false;
}
