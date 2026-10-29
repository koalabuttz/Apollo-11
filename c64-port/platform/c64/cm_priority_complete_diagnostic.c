/* SPDX-License-Identifier: MIT */
#include "cm_priority_complete_binding.h"
#include "overlay_platform.h"
#include "cm_priority_complete_modules.h"
#define CM_PRIO_INDEX_ONLY 1
#include "cm_priority_complete_vectors.h"
static c64_overlay_io disk_io={8,0,0,0,0,0};static apollo_overlay_runtime loader;static uint16_t failures,checkpoints;
static apollo_overlay_state *const image=(apollo_overlay_state *)(uintptr_t)APOLLO_SHARED_ADDRESS;
static uint8_t enter(void){uint8_t p;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(uint8_t p){if((p&4u)==0)__asm__ volatile("cli":::"memory");else __asm__ volatile("sei":::"memory");}
static uint8_t call(uint8_t id,uint16_t selector,uint16_t *result){uint8_t token=enter();enum apollo_overlay_status status=apollo_overlay_call(&loader,id,selector,result);leave(token);return status==APOLLO_OVERLAY_OK;}
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;uint8_t token,p,port;uint16_t n,result;cm_prio_fixture *f=CM_PRIO_FIXTURE;
 apollo_overlay_init(&loader,cm_priority_complete_modules,3,c64_overlay_backend(&disk_io),(uint8_t *)(uintptr_t)APOLLO_OVERLAY_ADDRESS,image);mail[0]=0;port=*(volatile uint8_t*)1;token=enter();
 for(n=0;n<CM_PRIO_TARGET_CASES;++n){if(n&1u)leave((uint8_t)(token&~4u));else (void)enter();result=1;if(!call(248,n,&result)||result){++failures;break;}
  result=1;if(!call(212,0,&result)||result){++failures;break;}result=1;if(!call(249,n,&result)||result){++failures;break;}
  if(f->step!=1u)++failures;failures+=f->failures;++checkpoints;__asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)!=(n&1u?0u:4u)||*(volatile uint8_t*)1!=port)++failures;
 }
 leave(token);mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checkpoints;mail[4]=(uint8_t)(checkpoints>>8);mail[0]=0xa5;return failures!=0;
}
