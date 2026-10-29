/* SPDX-License-Identifier: MIT */
#include "cm_fixed_compat.h"
#include "cm_fixed_compat_words.inc"
bool cm_fixed_compat_read(uint16_t address,agc_word *out){
 if(out==0||address>=40960u)return false;
 if(address>=36864u){*out=0;return true;}
#if !defined(CM_FIXED_COMPAT_GROUP) || CM_FIXED_COMPAT_GROUP == 0
 if(address<7168u){*out=cm_fixed_compat_group_0[address];return true;}
#endif
#if !defined(CM_FIXED_COMPAT_GROUP) || CM_FIXED_COMPAT_GROUP == 1
 if(address>=7168u&&address<14336u){*out=cm_fixed_compat_group_1[address-7168u];return true;}
#endif
#if !defined(CM_FIXED_COMPAT_GROUP) || CM_FIXED_COMPAT_GROUP == 2
 if(address>=14336u&&address<21504u){*out=cm_fixed_compat_group_2[address-14336u];return true;}
#endif
#if !defined(CM_FIXED_COMPAT_GROUP) || CM_FIXED_COMPAT_GROUP == 3
 if(address>=21504u&&address<28672u){*out=cm_fixed_compat_group_3[address-21504u];return true;}
#endif
#if !defined(CM_FIXED_COMPAT_GROUP) || CM_FIXED_COMPAT_GROUP == 4
 if(address>=28672u&&address<35840u){*out=cm_fixed_compat_group_4[address-28672u];return true;}
#endif
#if !defined(CM_FIXED_COMPAT_GROUP) || CM_FIXED_COMPAT_GROUP == 5
 if(address>=35840u){*out=cm_fixed_compat_group_5[address-35840u];return true;}
#endif
 return false;
}
