/* SPDX-License-Identifier: MIT */
#include "overlay_reu.h"

uint8_t apollo_reu_transfer(uint8_t command,void *c64,uint32_t reu,uint16_t length) {
    uint16_t address=apollo_reu_c64_address(c64);
    (void)apollo_reu_peek(APOLLO_REU_STATUS);   /* clear stale end/fault bits */
    apollo_reu_poke(APOLLO_REU_ADDR_CONTROL,0);
    apollo_reu_poke(APOLLO_REU_C64_LO,(uint8_t)address);
    apollo_reu_poke(APOLLO_REU_C64_HI,(uint8_t)(address>>8));
    apollo_reu_poke(APOLLO_REU_REU_LO,(uint8_t)reu);
    apollo_reu_poke(APOLLO_REU_REU_HI,(uint8_t)(reu>>8));
    apollo_reu_poke(APOLLO_REU_REU_BANK,(uint8_t)(reu>>16));
    apollo_reu_poke(APOLLO_REU_LEN_LO,(uint8_t)length);
    apollo_reu_poke(APOLLO_REU_LEN_HI,(uint8_t)(length>>8));
    apollo_reu_poke(APOLLO_REU_COMMAND,command);
    return apollo_reu_peek(APOLLO_REU_STATUS);
}
static uint16_t le16(const uint8_t *bytes) {
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1]<<8));
}
static uint32_t page_address(const apollo_reu_store *s,uint16_t page) {
    return s->base+((uint32_t)page<<8);
}
static uint8_t read_bytes(uint32_t reu,uint8_t *dest,uint16_t count) {
    return (uint8_t)((apollo_reu_transfer(APOLLO_REU_FETCH,dest,reu,count) &
        (APOLLO_REU_STATUS_END|APOLLO_REU_STATUS_FAULT))==APOLLO_REU_STATUS_END);
}
/* A resolver record for each of four point classes (two loc bits). The
 * search reads its probes into the point's entry, so an entry always holds
 * a real record, and the next lookup of that point skips the search. Open
 * fills them with a key no record has (bank 0xFFFF). The cache is valid
 * only because resolver records are read-only once the store is open:
 * restart and GOJAM keep it, and only a reopen clears it. */
static uint8_t cache[4][APOLLO_REU_RESOLVER_RECORD];
uint8_t apollo_reu_store_open(apollo_reu_store *s,uint32_t base,uint8_t mission,
    uint16_t services,uint8_t *record) {
    uint8_t *h=record,i;
    s->base=base; s->record=record;
    for (i = 0; i < 4; ++i) cache[i][0] = cache[i][1] = 0xFFu;
    if(!read_bytes(base,h,APOLLO_REU_INDEX_RECORD)) return 0;
    if(h[0]!='N' || h[1]!='R' || h[2]!='E' || h[3]!='U' || h[4]!=APOLLO_REU_FORMAT ||
       h[5]!=mission || le16(h+6)!=services) return 0;
    s->mission=mission; s->services=services;
    s->module_count=le16(h+8); s->index_page=le16(h+10); s->resolver_page=le16(h+12);
    s->resolver_count=le16(h+14);
    if(!read_bytes(base+APOLLO_REU_INDEX_RECORD,h,APOLLO_REU_INDEX_RECORD)) return 0;
    s->pages=le16(h);
    for(i=2;i<APOLLO_REU_INDEX_RECORD;++i) if(h[i]) return 0;
    /* Header page, index (4096 records = 256 pages), then resolver, then
     * modules: every region must lie inside the image. */
    if(s->index_page<1u || (uint32_t)s->index_page+256u>s->pages ||
       s->resolver_page<s->index_page+256u ||
       (uint32_t)s->resolver_page*256u+(uint32_t)s->resolver_count*APOLLO_REU_RESOLVER_RECORD>
       (uint32_t)s->pages*256u)
        return 0;
    return 1;
}
static uint8_t lookup(void *user,uint16_t id,apollo_ov2_dirent *d) {
    apollo_reu_store *s=(apollo_reu_store *)user;
    const uint8_t *r=s->record;
    if(!id || id>=APOLLO_REU_INDEX_ENTRIES) return 0;
    if(!read_bytes(page_address(s,s->index_page)+(uint32_t)id*APOLLO_REU_INDEX_RECORD,
        s->record,APOLLO_REU_INDEX_RECORD)) return 0;
    d->id=le16(r); d->page=le16(r+2); d->bytes=le16(r+4); d->crc16=le16(r+6);
    d->selectors=le16(r+8); d->slot=r[10]; d->flags=r[11];
    if(!(d->flags & APOLLO_OV2_DIRENT_PRESENT) || d->id!=id) return 0;
    /* Payload must lie inside the image, after the resolver. */
    if(d->page<s->resolver_page || (uint32_t)d->page*256u+d->bytes>(uint32_t)s->pages*256u)
        return 0;
    return 1;
}
static uint8_t fetch(void *user,const apollo_ov2_dirent *d,uint16_t offset,uint8_t *dest,uint16_t count) {
    apollo_reu_store *s=(apollo_reu_store *)user;
    if(!count || (uint32_t)offset+count>d->bytes) return 0;
    return read_bytes(page_address(s,d->page)+offset,dest,count);
}
static uint8_t resolve(void *user,uint16_t bank,uint16_t loc,apollo_ov2_cont *out) {
    apollo_reu_store *s=(apollo_reu_store *)user;
    uint16_t low=0,high=s->resolver_count;
    uint32_t key=((uint32_t)bank<<16)|loc;
    uint8_t *e = cache[(uint8_t)(loc ^ (loc >> 2)) & 3u];
    uint32_t probe = ((uint32_t)le16(e) << 16) | le16(e + 2);
    while (probe != key) {
        uint16_t middle=(uint16_t)(low+(uint16_t)((high-low)>>1));
        if (low >= high || !read_bytes(page_address(s, s->resolver_page) +
                                           (uint32_t)middle * APOLLO_REU_RESOLVER_RECORD,
                                       e, APOLLO_REU_RESOLVER_RECORD))
            return 0;
        probe = ((uint32_t)le16(e) << 16) | le16(e + 2);
        if(probe<key) low=(uint16_t)(middle+1u); else high=middle;
    }
    out->module = le16(e + 4);
    out->selector = le16(e + 6);
    return 1;
}
static uint16_t invoke(void *user,uint8_t *slot,apollo_ov2_frame *frame,uint16_t selector) {
    apollo_reu_store *s=(apollo_reu_store *)user;
    return s->invoke(s->invoke_user,slot,frame,selector);
}
uint8_t apollo_reu_store_fixed(apollo_reu_store *s,
    uint8_t (*load)(void *,uint32_t,const apollo_ov2_dirent *),void *user) {
    uint16_t id;
    for(id=1;id<APOLLO_REU_INDEX_ENTRIES;++id) {
        apollo_ov2_dirent d;
        if(lookup(s,id,&d) && d.slot>=APOLLO_OV2_SLOT_FIXED &&
           !load(user,page_address(s,d.page),&d)) return 0;
    }
    return 1;
}
apollo_ov2_store apollo_reu_store_backend(apollo_reu_store *s,
    uint16_t (*call)(void *,uint8_t *,apollo_ov2_frame *,uint16_t),void *invoke_user) {
    apollo_ov2_store store;
    s->invoke=call; s->invoke_user=invoke_user;
    store.user=s; store.lookup=lookup; store.fetch=fetch; store.resolve=resolve; store.invoke=invoke;
    return store;
}
enum apollo_ov2_status apollo_reu_store_verify(apollo_reu_store *s,uint8_t *buffer) {
    uint16_t id,count=0,i;
    uint32_t previous=0;
    for(id=1;id<APOLLO_REU_INDEX_ENTRIES;++id) {
        apollo_ov2_dirent d;
        uint16_t offset,crc=0xffffu;
        if(!read_bytes(page_address(s,s->index_page)+(uint32_t)id*APOLLO_REU_INDEX_RECORD,
            s->record,APOLLO_REU_INDEX_RECORD)) return APOLLO_OV2_IO_ERROR;
        if(!(s->record[11] & APOLLO_OV2_DIRENT_PRESENT)) {
            for(i=0;i<APOLLO_REU_INDEX_RECORD;++i) if(s->record[i]) return APOLLO_OV2_BAD_DIRECTORY;
            continue;
        }
        if(!lookup(s,id,&d) || d.slot>APOLLO_OV2_SLOT_PINNED_TASK || d.selectors==0 ||
           APOLLO_OV2_LANE(id)==0 || d.bytes<=APOLLO_OV2_HEADER_BYTES ||
           d.bytes>apollo_ov2_slot_bytes(d.slot) || d.flags!=APOLLO_OV2_DIRENT_PRESENT ||
           s->record[12] || s->record[13] || s->record[14] || s->record[15])
            return APOLLO_OV2_BAD_DIRECTORY;
        for(offset=0;offset<d.bytes;) {
            uint16_t chunk=(uint16_t)(d.bytes-offset);
            if(chunk>256u) chunk=256u;
            if(!fetch(s,&d,offset,buffer,chunk)) return APOLLO_OV2_IO_ERROR;
            if(offset==0) {
                /* Same header rules the runtime applies at load time. */
                uint16_t base=apollo_ov2_slot_address(d.slot);
                if(buffer[0]!='N' || buffer[1]!='A' || buffer[2]!='T' || buffer[3]!='V' ||
                   buffer[APOLLO_OV2_H_ABI]!=APOLLO_OV2_ABI ||
                   buffer[APOLLO_OV2_H_SLOT]!=d.slot || buffer[APOLLO_OV2_H_MISSION]!=s->mission ||
                   le16(buffer+APOLLO_OV2_H_ID)!=id ||
                   le16(buffer+APOLLO_OV2_H_ENTRY)!=(uint16_t)(base+APOLLO_OV2_HEADER_BYTES) ||
                   le16(buffer+APOLLO_OV2_H_LOAD_BYTES)!=d.bytes ||
                   !apollo_ov2_services_compatible(le16(buffer+APOLLO_OV2_H_SERVICES),s->services) ||
                   le16(buffer+APOLLO_OV2_H_SELECTORS)!=d.selectors)
                    return APOLLO_OV2_BAD_HEADER;
            }
            for(i=0;i<chunk;++i) crc=apollo_overlay_crc16(crc,buffer[i]);
            offset=(uint16_t)(offset+chunk);
        }
        if(crc!=d.crc16) return APOLLO_OV2_BAD_CHECKSUM;
        ++count;
    }
    if(count!=s->module_count) return APOLLO_OV2_BAD_DIRECTORY;
    for(i=0;i<s->resolver_count;++i) {
        apollo_ov2_dirent d;
        uint32_t key;
        uint16_t module,selector;
        if(!read_bytes(page_address(s,s->resolver_page)+(uint32_t)i*APOLLO_REU_RESOLVER_RECORD,
            s->record,APOLLO_REU_RESOLVER_RECORD)) return APOLLO_OV2_IO_ERROR;
        key=((uint32_t)le16(s->record)<<16)|le16(s->record+2);
        module=le16(s->record+4); selector=le16(s->record+6);
        if(i && key<=previous) return APOLLO_OV2_BAD_DIRECTORY;
        previous=key;
        if(!lookup(s,module,&d) || selector>=d.selectors || d.slot>=APOLLO_OV2_SLOTS)
            return APOLLO_OV2_UNRESOLVED;
    }
    return APOLLO_OV2_OK;
}
