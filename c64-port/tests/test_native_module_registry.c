/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "native_module_registry.h"
#include "native_registry_manifest.h"
typedef struct {
 native_module_registry registry;native_registry_lease lease;
 apollo_overlay_runtime loader;apollo_overlay_state image;
 uint8_t slot[APOLLO_OVERLAY_BYTES],file[APOLLO_OVERLAY_BYTES+2];
 unsigned at,length,opens,calls,enters,leaves,depth,in_frame;
 uint8_t enabled,opened_id,fail_open,bad_crc,bad_return,synthetic_crc,stage_error;
} fixture;
static uint8_t enter(void *u){fixture*f=u;uint8_t p=f->enabled;
 assert(!f->depth);f->depth=1;f->enabled=0;++f->enters;return p;}
static void leave(void*u,uint8_t p){fixture*f=u;assert(f->depth&&f->in_frame==0);
 f->depth=0;f->enabled=p;++f->leaves;}
static uint8_t open_file(void*u,const char*name){fixture*f=u;unsigned i,n=0;uint8_t id=148;
 assert(f->depth&&!f->enabled&&!f->in_frame);++f->opens;
 if(f->fail_open)return 0;
 if(strcmp(name,"MINFO")){id=0;for(i=0;i<111;++i)if(!strcmp(name,qualified_modules[i].filename)){id=qualified_modules[i].id;n=qualified_modules[i].byte_count;}}
 assert(id);if(id==148)n=32;
 memset(f->file,0,n+2);f->file[1]=0x80;memcpy(f->file+2,"NATV",4);
 f->file[6]=1;f->file[7]=id;f->file[8]=16;f->file[10]=0x10;f->file[11]=0x80;
 f->file[12]=(uint8_t)(0x8000+n);f->file[13]=(uint8_t)((0x8000+n)>>8);
 f->file[16]=(uint8_t)n;f->file[17]=(uint8_t)(n>>8);
 if(f->bad_crc)f->file[20]=1;
 f->at=0;f->length=n+2;f->opened_id=id;
 if(id!=148&&f->stage_error==1)--f->length;
 if(id!=148&&f->stage_error==2){f->file[f->length++]=0;}
 if(id!=148&&f->stage_error==3)f->file[7]^=1;
 return 1;
}
static int read_file(void*u){fixture*f=u;assert(f->depth);return f->at<f->length?f->file[f->at++]:-1;}
static void close_file(void*u){fixture*f=u;assert(f->depth);}
static uint16_t fake_crc(uint8_t,unsigned);
static uint16_t invoke(void*u,uint8_t*slot,apollo_overlay_state*image,uint16_t selector){fixture*f=u;
 assert(f->depth&&!f->in_frame&&slot==f->slot&&image==&f->image);f->in_frame=1;++f->calls;
 assert(native_registry_lookup(&f->registry,2)==APOLLO_OVERLAY_BUSY);
 if(slot[5]==148){enum apollo_overlay_status s;
  s=native_registry_copy(qualified_modules,111,&f->lease);
  if(!s&&f->synthetic_crc)f->lease.module.crc16=fake_crc(f->lease.module.id,f->lease.module.byte_count);
  if(f->bad_return==1)f->lease.module.filename=(const char*)slot;
  if(f->bad_return==2)memset(f->lease.filename,'X',sizeof(f->lease.filename));
  if(f->bad_return==3)f->lease.module.id=0;
  f->in_frame=0;return s;
 }
 f->in_frame=0;return (uint16_t)(selector^0x1234);
}
static uint16_t fake_crc(uint8_t id,unsigned n){fixture f;uint16_t crc=0xffff;unsigned i;
 memset(&f,0,sizeof(f));f.depth=1;
 if(id==148)open_file(&f,"MINFO");else{unsigned j;for(j=0;j<111;++j)if(qualified_modules[j].id==id)open_file(&f,qualified_modules[j].filename);}
 assert(f.length==n+2);for(i=2;i<f.length;++i)crc=apollo_overlay_crc16(crc,f.file[i]);return crc;
}
static fixture f;
int main(void){exec_exclusion guard={enter,leave,&f};apollo_overlay_module boot={148,"MINFO",32,0};
 apollo_overlay_backend backend={&f,open_file,read_file,close_file,invoke};
 unsigned i,n=0;uint16_t result=0xbeef;apollo_overlay_state before;
 memset(&f,0,sizeof(f));f.enabled=1;boot.crc16=fake_crc(148,32);
 apollo_overlay_init(&f.loader,0,0,backend,f.slot,&f.image);
 assert(native_registry_init(&f.registry,&f.loader,&boot,&f.lease,&guard)==APOLLO_OVERLAY_OK);
 for(i=0;i<2048;++i)f.image.words[i]=(uint16_t)(i^0xaaaa);
 before=f.image;
 for(i=0;i<257;++i){enum apollo_overlay_status s=native_registry_lookup(&f.registry,(uint16_t)i);
  unsigned j;const apollo_overlay_module*m=0;for(j=0;j<111;++j)if(qualified_modules[j].id==i)m=&qualified_modules[j];
  if(!m){assert(s==APOLLO_OVERLAY_UNMAPPED&&!f.lease.valid);continue;}++n;
  assert(s==APOLLO_OVERLAY_OK&&f.lease.valid&&f.lease.module.filename==f.lease.filename);
  assert(f.lease.module.id==m->id&&f.lease.module.byte_count==m->byte_count&&f.lease.module.crc16==m->crc16&&!strcmp(f.lease.filename,m->filename));
 }
 assert(n==111&&!memcmp(&before,&f.image,sizeof(before))&&f.enabled);
 /* Real load/checksum error preserves output and invalidates stale record. */
 assert(native_registry_call(&f.registry,1,7,&result)==APOLLO_OVERLAY_BAD_CHECKSUM&&result==0xbeef&&!f.lease.valid);
 /* Supply a separately qualified native descriptor to exercise successful
  * metadata-return -> stage-load ordering, not the original stage byte CRC. */
 {unsigned guards=f.enters;f.synthetic_crc=1;
  assert(native_registry_call(&f.registry,1,7,&result)==APOLLO_OVERLAY_OK&&result==(7^0x1234));
  assert(f.enters==guards+1);f.synthetic_crc=0;
 }
 {static const enum apollo_overlay_status expected[]={APOLLO_OVERLAY_SHORT_FILE,APOLLO_OVERLAY_LONG_FILE,APOLLO_OVERLAY_BAD_HEADER};
  for(i=0;i<3;++i){result=0xbeef;f.stage_error=(uint8_t)(i+1u);
   assert(native_registry_call(&f.registry,2,7,&result)==expected[i]&&result==0xbeef&&!f.lease.valid);}
  f.stage_error=0;
 }
 for(i=1;i<=3;i+=2){f.bad_return=(uint8_t)i;assert(native_registry_lookup(&f.registry,2)==APOLLO_OVERLAY_BAD_MANIFEST&&!f.lease.valid);}f.bad_return=0;
 f.loader.loaded=0;f.bad_crc=1;assert(native_registry_lookup(&f.registry,2)==APOLLO_OVERLAY_BAD_CHECKSUM&&!f.lease.valid);f.bad_crc=0;
 f.fail_open=1;assert(native_registry_lookup(&f.registry,2)==APOLLO_OVERLAY_IO_ERROR);f.fail_open=0;
 f.enabled=0;assert(native_registry_lookup(&f.registry,2)==APOLLO_OVERLAY_OK&&!f.enabled);
 f.registry.busy=1;assert(native_registry_lookup(&f.registry,2)==APOLLO_OVERLAY_BUSY);f.registry.busy=0;
 assert(native_registry_call(&f.registry,2,0,0)==APOLLO_OVERLAY_BAD_MANIFEST);
 assert(native_registry_lookup(0,2)==APOLLO_OVERLAY_BAD_MANIFEST);
 assert(native_registry_init(0,&f.loader,&boot,&f.lease,&guard)==APOLLO_OVERLAY_BAD_MANIFEST);
 {apollo_overlay_module m[2]={{1,"",32,0},{1,"DUP",32,0}};f.lease.requested=1;
  assert(native_registry_copy(m,2,&f.lease)==APOLLO_OVERLAY_BAD_MANIFEST);
  assert(native_registry_copy(m,1,&f.lease)==APOLLO_OVERLAY_BAD_MANIFEST);
  m[0].filename="12345678901234567";assert(native_registry_copy(m,1,&f.lease)==APOLLO_OVERLAY_BAD_MANIFEST);
  m[0].filename="1234567890123456";assert(native_registry_copy(m,1,&f.lease)==APOLLO_OVERLAY_OK);
  assert(native_registry_copy(0,1,&f.lease)==APOLLO_OVERLAY_BAD_MANIFEST);
  assert(native_registry_copy(m,1,0)==APOLLO_OVERLAY_BAD_MANIFEST);
 }
 assert(f.enters==f.leaves&&!f.depth&&!f.in_frame&&!memcmp(&before,&f.image,sizeof(before)));
 puts("PASS: all111 frozen descriptors; bounded resident copies, unknown/stale/error/IRQ-state seams and unchanged2048-word source image");return 0;
}
