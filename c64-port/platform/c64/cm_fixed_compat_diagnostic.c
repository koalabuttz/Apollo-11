/* SPDX-License-Identifier: MIT */
#include "cm_fixed_compat_binding.h"
#include "overlay_platform.h"
#include "cm_fixed_compat_modules.h"
#define CM_FIXED_COMPAT_INDEX_ONLY 1
#include "cm_fixed_compat_vectors.h"
static c64_overlay_io disk_io={8,0,0,0,0,0};static apollo_overlay_runtime loader;static uint16_t failures,checkpoints,runs;static uint8_t data_ok;
static apollo_overlay_state *const image=(apollo_overlay_state *)(uintptr_t)APOLLO_SHARED_ADDRESS;
static uint8_t enter(void){uint8_t p;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(uint8_t p){if((p&4u)==0)__asm__ volatile("cli":::"memory");else __asm__ volatile("sei":::"memory");}
static uint8_t call(uint8_t id,uint16_t selector,uint16_t *result){uint8_t token=enter();enum apollo_overlay_status status=apollo_overlay_call(&loader,id,selector,result);leave(token);return status==APOLLO_OVERLAY_OK;}
static uint32_t fold(uint32_t h,uint16_t v){h=(h^(v&255u))*16777619u;return (h^(v>>8))*16777619u;}
static void read_all(void){uint16_t address,word,i;uint32_t h=2166136261u;
 for(i=0;i<2048u;++i)image->words[i]=0;
 for(address=0;address<40960u;++address){if(!call((uint16_t)(200u+address/7168u),address,&word)||word>AGC_MASK){++failures;return;}h=fold(h,word);}
 if(h!=CM_FIXED_COMPAT_ALL_DATA_HASH)++failures;else data_ok=1;
 for(i=0;i<2048u;++i)if(image->words[i])++failures;
}
static void replay(void){uint16_t n,phase,result,address;cm_fixed_fixture *f=CM_FIXED_FIXTURE;
 for(n=0;n<CM_FIXED_TARGET_CASES;++n){result=1;if(!call(240,n,&result)||result){++failures;return;}
  for(phase=0;phase<2u;++phase){f->phase=phase;f->status=0;result=1;
   if(!phase){if(!call(206,(uint16_t)(n>=CM_FIXED_TARGET_CASES/2u),&result)||result){++failures;return;}}
   else{if(cm_supdacal_address(image,CM_FIXED_IO,&address)!=EXEC_OK||!call((uint16_t)(200u+address/7168u),address,&result)||result>AGC_MASK){++failures;return;}*CM_FIXED_FETCH=result;result=1;if(!call(206,2,&result)||result){++failures;return;}}
   result=1;if(!call(241,phase,&result)||result){++failures;return;}++checkpoints;
  }
  if(f->step!=2u)++failures;failures+=f->failures;++runs;
 }
}
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;uint8_t token,p,port;
 apollo_overlay_init(&loader,cm_fixed_compat_modules,9,c64_overlay_backend(&disk_io),(uint8_t *)(uintptr_t)APOLLO_OVERLAY_ADDRESS,image);
 mail[0]=0;port=*(volatile uint8_t*)1;token=enter();read_all();leave((uint8_t)(token&~4u));replay();__asm__ volatile("php\npla":"=a"(p)::"memory");if(p&4u)++failures;
 (void)enter();replay();__asm__ volatile("php\npla":"=a"(p)::"memory");if(!(p&4u))++failures;leave(token);__asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)!=(token&4u)||*(volatile uint8_t*)1!=port)++failures;
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checkpoints;mail[4]=(uint8_t)(checkpoints>>8);mail[5]=(uint8_t)runs;mail[6]=data_ok;mail[0]=0xa5;return failures!=0;
}
