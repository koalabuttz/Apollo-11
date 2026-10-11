/* SPDX-License-Identifier: MIT */
#include "overlay_runtime.h"
uint16_t apollo_overlay_crc16(uint16_t crc,uint8_t byte) {
    uint8_t bit;
    crc=(uint16_t)(crc ^ ((uint16_t)byte<<8));
    for(bit=0;bit<8u;++bit)
        crc=(uint16_t)((crc & 0x8000u) ? ((uint16_t)(crc<<1)^0x1021u) : (uint16_t)(crc<<1));
    return crc;
}
void apollo_overlay_init(apollo_overlay_runtime *r,const apollo_overlay_module *m,
    uint8_t count,apollo_overlay_backend io,uint8_t *slot,apollo_overlay_state *state) {
    r->modules=m; r->count=count; r->backend=io; r->slot=slot; r->state=state;
    r->loaded=0; r->busy=0;
}
static uint16_t le16(const uint8_t *bytes) {
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1]<<8));
}
static enum apollo_overlay_status byte(apollo_overlay_runtime *r,uint8_t *out) {
    int value=r->backend.read(r->backend.user);
    if(value==-1) return APOLLO_OVERLAY_SHORT_FILE;
    if(value<0 || value>255) return APOLLO_OVERLAY_IO_ERROR;
    *out=(uint8_t)value; return APOLLO_OVERLAY_OK;
}
enum apollo_overlay_status apollo_overlay_call(apollo_overlay_runtime *r,
    uint8_t id,uint16_t selector,uint16_t *result) {
    const apollo_overlay_module *module=0;
    enum apollo_overlay_status status=APOLLO_OVERLAY_OK;
    uint8_t i,header[APOLLO_OVERLAY_HEADER_BYTES],address[2];
    uint16_t offset,crc=0xffffu,bss,bss_size;
    if(r->busy) return APOLLO_OVERLAY_BUSY;
    if(!id) return APOLLO_OVERLAY_UNMAPPED;
    for(i=0;i<r->count;++i) if(r->modules[i].id==id) {
        if(module) return APOLLO_OVERLAY_BAD_MANIFEST;
        module=&r->modules[i];
    }
    if(!module) return APOLLO_OVERLAY_UNMAPPED;
    if(!module->filename || module->byte_count<=APOLLO_OVERLAY_HEADER_BYTES ||
       module->byte_count>APOLLO_OVERLAY_BYTES || !result || !r->slot || !r->state ||
       !r->backend.open || !r->backend.read || !r->backend.close || !r->backend.invoke)
        return APOLLO_OVERLAY_BAD_MANIFEST;
    r->busy=1;
    if(r->loaded!=id) {
        int extra;
        r->loaded=0;
        if(!r->backend.open(r->backend.user,module->filename)) {
            status=APOLLO_OVERLAY_IO_ERROR; goto done;
        }
        for(i=0;i<2u;++i) { status=byte(r,&address[i]); if(status) goto close; }
        if(le16(address)!=APOLLO_OVERLAY_ADDRESS) { status=APOLLO_OVERLAY_BAD_HEADER; goto close; }
        for(i=0;i<APOLLO_OVERLAY_HEADER_BYTES;++i) {
            status=byte(r,&header[i]); if(status) goto close;
        }
        bss=le16(header+10); bss_size=le16(header+12);
        if(header[0]!='N' || header[1]!='A' || header[2]!='T' || header[3]!='V' ||
           header[4]!=APOLLO_OVERLAY_ABI || header[5]!=id ||
           header[6]!=APOLLO_OVERLAY_HEADER_BYTES || header[7]!=0 ||
           le16(header+8)!=APOLLO_OVERLAY_ENTRY_ADDRESS ||
           le16(header+14)!=module->byte_count ||
           bss<APOLLO_OVERLAY_ADDRESS+module->byte_count || bss>0xc000u ||
           bss_size>(uint16_t)(0xc000u-bss)) {
            status=APOLLO_OVERLAY_BAD_HEADER; goto close;
        }
        for(i=0;i<APOLLO_OVERLAY_HEADER_BYTES;++i) {
            r->slot[i]=header[i]; crc=apollo_overlay_crc16(crc,header[i]);
        }
        for(offset=APOLLO_OVERLAY_HEADER_BYTES;offset<module->byte_count;++offset) {
            uint8_t value;
            status=byte(r,&value); if(status) goto close;
            r->slot[offset]=value; crc=apollo_overlay_crc16(crc,value);
        }
        extra=r->backend.read(r->backend.user);
        if(extra>=0) status=APOLLO_OVERLAY_LONG_FILE;
        else if(extra!=-1) status=APOLLO_OVERLAY_IO_ERROR;
        else if(crc!=module->crc16) status=APOLLO_OVERLAY_BAD_CHECKSUM;
close:
        r->backend.close(r->backend.user);
        if(status) goto done;
        /* All overlay static storage is transient, reset on each successful
         * load. Initialized .data is part of the verified file payload. */
        offset=(uint16_t)(bss-APOLLO_OVERLAY_ADDRESS);
        while(bss_size) { r->slot[offset++]=0; --bss_size; }
        r->loaded=id;
    }
    *result=r->backend.invoke(r->backend.user,r->slot,r->state,selector);
done:
    r->busy=0; return status;
}
