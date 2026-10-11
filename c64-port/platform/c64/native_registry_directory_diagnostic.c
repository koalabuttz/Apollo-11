/* SPDX-License-Identifier: MIT */
#include "native_module_registry.h"
#include "native_registry_bootstrap.h"
#include "overlay_platform.h"
static apollo_overlay_runtime loader;static native_module_registry registry;
static native_registry_lease *const lease=(native_registry_lease *)(uintptr_t)0xc600;
static apollo_overlay_state *const image=(apollo_overlay_state *)(uintptr_t)0x7000;
static c64_overlay_io io={8,0,0,0,0,0};
static uint8_t enter(void*u){uint8_t p;(void)u;__asm__ volatile("php\nsei\npla":"=a"(p)::"memory");return p;}
static void leave(void*u,uint8_t p){(void)u;if(p&4u)__asm__ volatile("sei":::"memory");else __asm__ volatile("cli":::"memory");}
static const exec_exclusion guard={enter,leave,0};
static uint16_t failures,checked;
static void directory(void){uint16_t id,crc=0xffffu,count=0;uint8_t n;
 for(id=0;id<257u;++id){enum apollo_overlay_status s=native_registry_lookup(&registry,id);
  if(s==APOLLO_OVERLAY_UNMAPPED){if(lease->valid||lease->module.id||lease->module.filename)++failures;continue;}
  if(s!=APOLLO_OVERLAY_OK){++failures;continue;}++count;++checked;
  if(lease->module.id!=id||lease->module.filename!=lease->filename)++failures;
  crc=apollo_overlay_crc16(crc,(uint8_t)id);
  crc=apollo_overlay_crc16(crc,(uint8_t)lease->module.byte_count);
  crc=apollo_overlay_crc16(crc,(uint8_t)(lease->module.byte_count>>8));
  crc=apollo_overlay_crc16(crc,(uint8_t)lease->module.crc16);
  crc=apollo_overlay_crc16(crc,(uint8_t)(lease->module.crc16>>8));
  for(n=0;n<NATIVE_REGISTRY_NAME_BYTES;++n){crc=apollo_overlay_crc16(crc,(uint8_t)lease->filename[n]);if(!lease->filename[n])break;}
 }
 if(count!=NATIVE_REGISTRY_EXPECTED_COUNT||crc!=NATIVE_REGISTRY_EXPECTED_CRC)++failures;
}
int main(void){uint16_t n,value=0xbeef;uint8_t token,p,port=*(volatile uint8_t*)1;
 volatile uint8_t*mail=(volatile uint8_t*)0xc080;apollo_overlay_module bad=native_registry_bootstrap;mail[0]=0;
 apollo_overlay_init(&loader,0,0,c64_overlay_backend(&io),(uint8_t*)(uintptr_t)0x8000,image);
 if(native_registry_init(&registry,&loader,&native_registry_bootstrap,lease,&guard))++failures;
 for(n=0;n<2048u;++n)image->words[n]=(uint16_t)(n^0xaaaa);
 directory();token=enter(0);directory();leave(0,token);
 if(native_registry_lookup(&registry,1)||lease->module.id!=1)++failures;
 if(native_registry_call(&registry,0,0,&value)!=APOLLO_OVERLAY_UNMAPPED||value!=0xbeef||lease->valid)++failures;
 if(native_registry_call(&registry,1,0,0)!=APOLLO_OVERLAY_BAD_MANIFEST)++failures;
 if(native_registry_lookup(0,1)!=APOLLO_OVERLAY_BAD_MANIFEST)++failures;
 bad.crc16^=1;registry.bootstrap=&bad;loader.loaded=0;
 if(native_registry_call(&registry,1,0,&value)!=APOLLO_OVERLAY_BAD_CHECKSUM||value!=0xbeef||lease->valid)++failures;
 bad=native_registry_bootstrap;bad.filename="MISSING";loader.loaded=0;
 if(native_registry_lookup(&registry,1)!=APOLLO_OVERLAY_IO_ERROR||lease->valid)++failures;
 registry.bootstrap=&native_registry_bootstrap;
 if(native_registry_lookup(&registry,1)||lease->module.id!=1)++failures;
 for(n=0;n<2048u;++n)if(image->words[n]!=(uint16_t)(n^0xaaaa))++failures;
 __asm__ volatile("php\npla":"=a"(p)::"memory");if((p&4u)!=(token&4u)||*(volatile uint8_t*)1!=port)++failures;
 mail[1]=(uint8_t)failures;mail[2]=(uint8_t)(failures>>8);mail[3]=(uint8_t)checked;mail[4]=(uint8_t)(checked>>8);mail[5]=2;mail[0]=0xa5;return failures!=0;
}
