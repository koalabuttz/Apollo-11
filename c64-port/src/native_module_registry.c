/* SPDX-License-Identifier: MIT */
#include "native_module_registry.h"
typedef char lease_fits[(sizeof(native_registry_lease)<=NATIVE_REGISTRY_LEASE_BYTES)?1:-1];
#ifdef NATIVE_REGISTRY_FIXED_LEASE
#define LEASE(r) ((native_registry_lease *)(uintptr_t)NATIVE_REGISTRY_LEASE_ADDRESS)
#else
#define LEASE(r) ((r)->lease)
#endif
static void invalidate(native_registry_lease *out) {
 out->valid=0;out->module.id=0;out->module.filename=0;
 out->module.byte_count=0;out->module.crc16=0;out->filename[0]=0;
}
enum apollo_overlay_status native_registry_copy(const apollo_overlay_module *table,
 uint8_t count,native_registry_lease *out) {
 const apollo_overlay_module *found=0;uint8_t i,n;
 if(!out)return APOLLO_OVERLAY_BAD_MANIFEST;
 invalidate(out);
 if(!table||!count)return APOLLO_OVERLAY_BAD_MANIFEST;
 if(!out->requested||out->requested>255u)return APOLLO_OVERLAY_UNMAPPED;
 for(i=0;i<count;++i)if(table[i].id==out->requested){
  if(found)return APOLLO_OVERLAY_BAD_MANIFEST;
  found=&table[i];
 }
 if(!found)return APOLLO_OVERLAY_UNMAPPED;
 if(!found->filename||found->byte_count<=APOLLO_OVERLAY_HEADER_BYTES||
    found->byte_count>APOLLO_OVERLAY_BYTES)return APOLLO_OVERLAY_BAD_MANIFEST;
 for(n=0;n<NATIVE_REGISTRY_NAME_BYTES;++n){
  char ch=found->filename[n];out->filename[n]=ch;
  if(!ch)break;
 }
 if(!n||n==NATIVE_REGISTRY_NAME_BYTES){invalidate(out);return APOLLO_OVERLAY_BAD_MANIFEST;}
 out->module.id=found->id;out->module.byte_count=found->byte_count;
 out->module.crc16=found->crc16;out->module.filename=out->filename;
 out->valid=1;return APOLLO_OVERLAY_OK;
}
enum apollo_overlay_status native_registry_init(native_module_registry *r,
 apollo_overlay_runtime *loader,const apollo_overlay_module *boot,native_registry_lease *lease,
 const exec_exclusion *guard) {
 if(!r||!loader||!lease||!guard||!guard->enter||!guard->leave||!boot||!boot->id||
    !boot->filename||!boot->filename[0]||boot->byte_count<=APOLLO_OVERLAY_HEADER_BYTES||
    boot->byte_count>APOLLO_OVERLAY_BYTES||loader->busy)return APOLLO_OVERLAY_BAD_MANIFEST;
#ifdef NATIVE_REGISTRY_FIXED_LEASE
 if(lease!=(native_registry_lease *)(uintptr_t)NATIVE_REGISTRY_LEASE_ADDRESS)
  return APOLLO_OVERLAY_BAD_MANIFEST;
#endif
 r->loader=loader;r->bootstrap=boot;r->lease=lease;r->exclusion=guard;r->busy=0;
 invalidate(lease);lease->requested=0;return APOLLO_OVERLAY_OK;
}
static enum apollo_overlay_status lookup_locked(native_module_registry *r,uint16_t id) {
 apollo_overlay_runtime *loader=r->loader;uint16_t value=APOLLO_OVERLAY_BAD_MANIFEST;
 enum apollo_overlay_status status;
 invalidate(LEASE(r));LEASE(r)->requested=id;
 if(!id||id>255u||id==r->bootstrap->id)return APOLLO_OVERLAY_UNMAPPED;
 loader->modules=r->bootstrap;loader->count=1;
 status=apollo_overlay_call(loader,r->bootstrap->id,(uint16_t)(uintptr_t)LEASE(r),&value);
 if(status){invalidate(LEASE(r));return status;}
 if(value!=APOLLO_OVERLAY_OK){invalidate(LEASE(r));
  return value==APOLLO_OVERLAY_UNMAPPED?APOLLO_OVERLAY_UNMAPPED:APOLLO_OVERLAY_BAD_MANIFEST;}
 /* Bootstrap CRC qualifies the compiled copy routine; it bounds the name
  * and size before publishing valid. Recheck identity and resident ownership
  * here; the stage loader retains its independent size/header/CRC checks. */
 if(!LEASE(r)->valid||LEASE(r)->module.id!=id||
    LEASE(r)->module.filename!=LEASE(r)->filename){
  invalidate(LEASE(r));return APOLLO_OVERLAY_BAD_MANIFEST;}

 return APOLLO_OVERLAY_OK;
}
static enum apollo_overlay_status transaction(native_module_registry *r,uint16_t id,
 uint16_t selector,uint16_t *result,uint8_t invoke) {
 enum apollo_overlay_status status;uint8_t token;
 if(!r||(invoke&&!result))return APOLLO_OVERLAY_BAD_MANIFEST;
 if(r->busy||r->loader->busy)return APOLLO_OVERLAY_BUSY;
 r->busy=1;token=r->exclusion->enter(r->exclusion->user);
 status=lookup_locked(r,id);
 if(!status&&invoke){r->loader->modules=&LEASE(r)->module;r->loader->count=1;
  status=apollo_overlay_call(r->loader,(uint8_t)id,selector,result);
  if(status)invalidate(LEASE(r));
 }
 r->exclusion->leave(r->exclusion->user,token);r->busy=0;return status;
}
enum apollo_overlay_status native_registry_lookup(native_module_registry *r,uint16_t id) {
 return transaction(r,id,0,0,0);
}
enum apollo_overlay_status native_registry_call(native_module_registry *r,uint16_t id,
 uint16_t selector,uint16_t *result) {return transaction(r,id,selector,result,1);}
