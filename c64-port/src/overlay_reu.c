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
/* A page's REU address. The store opens only on a page boundary below
 * 16 MB (the REU's 24-bit address), so the page number fits 16 bits and the
 * add is a 16-bit one. */
static uint32_t page_address(const apollo_reu_store *s,uint16_t page) {
    return (uint32_t)(uint16_t)((uint16_t)(s->base >> 8) + page) << 8;
}
static uint8_t read_bytes(uint32_t reu,uint8_t *dest,uint16_t count) {
    return (uint8_t)((apollo_reu_transfer(APOLLO_REU_FETCH,dest,reu,count) &
        (APOLLO_REU_STATUS_END|APOLLO_REU_STATUS_FAULT))==APOLLO_REU_STATUS_END);
}
/* Recently resolved points, a record each, direct-mapped by
 * (loc, bank): the search reads its probes into the point's entry, so an
 * entry always holds a real record, and the next lookup of that point skips
 * the search. Open fills them with a key no record has (bank 0xFFFF). The
 * cache is valid only because resolver records are read-only once the
 * store is open: restart and GOJAM keep it, and only a reopen clears it.
 * 32 entries hold most of a flight phase's resolved points (CM entry
 * 17000-19000 cs: 80 points, 80% of lookups hit; 4 entries hit 29%). */
#ifndef APOLLO_REU_RESOLVE_CACHE
#define APOLLO_REU_RESOLVE_CACHE 32u
#endif
static uint8_t cache[APOLLO_REU_RESOLVE_CACHE * APOLLO_REU_RESOLVER_RECORD];
/* Where each bank's records start: fence[f] is the first record whose key
 * has bank bits (bank >> 10) of f or more. A flight key's bits are its FB
 * bank, 0-037 (the superbank flag 0100 is below them); any higher key goes
 * in the last group. Open finds them by search, so a cache miss searches
 * only its own bank's records. */
#define APOLLO_REU_FENCES 32u
static uint16_t fence[APOLLO_REU_FENCES + 2u];
/* The resolver's REU address, set when the store opens (one open store at a
 * time, as the cache and the fences). */
static uint32_t resolver_base;
uint8_t apollo_reu_store_open(apollo_reu_store *s,uint32_t base,uint8_t mission,
    uint16_t services,uint8_t *record) {
    uint8_t *h=record,i;
    if (base & 0xFF0000FFu) return 0;
    s->base=base; s->record=record;
    for (i = 0; i < APOLLO_REU_RESOLVE_CACHE; ++i)
        cache[i * APOLLO_REU_RESOLVER_RECORD] = cache[i * APOLLO_REU_RESOLVER_RECORD + 1u] = 0xFFu;
    if(!read_bytes(base,h,APOLLO_REU_INDEX_RECORD)) return 0;
    if(h[0]!='N' || h[1]!='R' || h[2]!='E' || h[3]!='U' || h[4]!=APOLLO_REU_FORMAT ||
       h[5]!=mission || le16(h+6)!=services) return 0;
    s->mission=mission; s->services=services;
    s->module_count=le16(h+8); s->index_page=le16(h+10); s->resolver_page=le16(h+12);
    s->resolver_count=le16(h+14);
    if(!read_bytes(base+APOLLO_REU_INDEX_RECORD,h,APOLLO_REU_INDEX_RECORD)) return 0;
    s->pages=le16(h);
    for(i=2;i<APOLLO_REU_INDEX_RECORD;++i) if(h[i]) return 0;
    /* Header page, index (the pages up to the resolver: blocks of 256
     * records = 16 pages, 1 to 16 blocks), then resolver, then modules:
     * every region must lie inside the image. */
    if (s->index_page < 1u || s->resolver_page <= s->index_page ||
        s->resolver_count >= 8192u || /* resolve(): offsets fit 16 bits */
        (uint16_t)(s->resolver_page - s->index_page) > 256u ||
        ((s->resolver_page - s->index_page) & 15u) ||
        (uint32_t)s->resolver_page * 256u +
                (uint32_t)s->resolver_count * APOLLO_REU_RESOLVER_RECORD >
            (uint32_t)s->pages * 256u)
        return 0;
    s->index_blocks=(uint8_t)((s->resolver_page-s->index_page)>>4);
    {
        uint32_t r = resolver_base = page_address(s, s->resolver_page);
        uint16_t low = 0;
        for (i = 0; i <= APOLLO_REU_FENCES; ++i) {
            uint16_t high = s->resolver_count;
            while (low < high) {
                uint16_t middle = (uint16_t)(low + (uint16_t)((high - low) >> 1));
                if (!read_bytes(r + (uint16_t)(middle * APOLLO_REU_RESOLVER_RECORD), h, 2u))
                    return 0;
                if ((uint8_t)(le16(h) >> 10) < i)
                    low = (uint16_t)(middle + 1u);
                else
                    high = middle;
            }
            fence[i] = low;
        }
        fence[APOLLO_REU_FENCES + 1u] = s->resolver_count;
    }
    return 1;
}
static uint8_t payload_inside(const apollo_reu_store *s,const apollo_ov2_dirent *d) {
    return d->page>=s->resolver_page &&
        (uint32_t)d->page*256u+d->bytes<=(uint32_t)s->pages*256u;
}
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
/* An index record's first 12 bytes are the dirent's fields in order,
 * little-endian, so a little-endian build reads them straight into it. */
typedef char apollo_reu_dirent_is_record[sizeof(apollo_ov2_dirent) == 12u ? 1 : -1];
#define APOLLO_REU_DIRENT_DIRECT 1
#endif
static uint8_t lookup(void *user,uint16_t id,apollo_ov2_dirent *d) {
    apollo_reu_store *s=(apollo_reu_store *)user;
    if(!id || (uint8_t)(id>>8)>=s->index_blocks) return 0;
#ifdef APOLLO_REU_DIRENT_DIRECT
    if (!read_bytes(page_address(s, s->index_page) + (uint32_t)id * APOLLO_REU_INDEX_RECORD,
                    (uint8_t *)d, sizeof *d))
        return 0;
#else
    {
        const uint8_t *r = s->record;
        if (!read_bytes(page_address(s, s->index_page) + (uint32_t)id * APOLLO_REU_INDEX_RECORD,
                        s->record, APOLLO_REU_INDEX_RECORD))
            return 0;
        d->id = le16(r);
        d->page = le16(r + 2);
        d->bytes = le16(r + 4);
        d->crc16 = le16(r + 6);
        d->selectors = le16(r + 8);
        d->slot = r[10];
        d->flags = r[11];
    }
#endif
    if(!(d->flags & APOLLO_OV2_DIRENT_PRESENT) || d->id!=id) return 0;
#ifndef APOLLO_OV2_STORE_VERIFIED
    /* Payload must lie inside the image, after the resolver. A verified
     * store (APOLLO_OV2_STORE_VERIFIED) had this checked for every present
     * record when it opened (apollo_reu_store_verify). */
    if(!payload_inside(s,d)) return 0;
#endif
    return 1;
}
static uint8_t fetch(void *user,const apollo_ov2_dirent *d,uint16_t offset,uint8_t *dest,uint16_t count) {
    apollo_reu_store *s=(apollo_reu_store *)user;
    if(!count || (uint32_t)offset+count>d->bytes) return 0;
    return read_bytes(page_address(s,d->page)+offset,dest,count);
}
/* resolve's search state. Static: absolute addressing is smaller than the
 * soft stack. */
static uint16_t probe_low, probe_high;
/* The key searched for, most significant byte first: bank high, bank low,
 * loc high, loc low. A record holds it little-endian (bank, loc). */
static uint8_t key[4];
static const uint8_t key_at[4] = {1u, 0u, 3u, 2u};
/* 0 when the record at e has the key, else 1 when its key is below, 2 above. */
static uint8_t key_order(const uint8_t *e)
{
    uint8_t i;
    for (i = 0; i < 4u; ++i) {
        uint8_t b = e[key_at[i]];
        if (b != key[i]) return b < key[i] ? 1u : 2u;
    }
    return 0;
}
static uint8_t resolve(void *user,uint16_t bank,uint16_t loc,apollo_ov2_cont *out) {
    uint8_t *e = cache + (uint8_t)(((uint8_t)loc ^ (uint8_t)(loc >> 4) ^ (uint8_t)(bank >> 10)) &
                                   (APOLLO_REU_RESOLVE_CACHE - 1u)) *
                             APOLLO_REU_RESOLVER_RECORD;
    (void)user;
    key[0] = (uint8_t)(bank >> 8);
    key[1] = (uint8_t)bank;
    key[2] = (uint8_t)(loc >> 8);
    key[3] = (uint8_t)loc;
    if (key_order(e)) {
        /* Records are sorted by (bank, loc), 8 bytes each, fewer than 8192,
         * so a record's offset in the resolver fits 16 bits. */
        uint8_t f = key[0];
        const uint16_t *g = fence + (f & 0x80u ? APOLLO_REU_FENCES : (uint8_t)(f >> 2));
        probe_low = g[0];
        probe_high = g[1];
        for (;;) {
            uint16_t middle = (uint16_t)(probe_low + (uint16_t)((probe_high - probe_low) >> 1));
            uint8_t o;
            if (probe_low >= probe_high ||
                !read_bytes(resolver_base + (uint16_t)(middle * APOLLO_REU_RESOLVER_RECORD), e,
                            APOLLO_REU_RESOLVER_RECORD))
                return 0;
            o = key_order(e);
            if (!o) break;
            if (o == 1u)
                probe_low = (uint16_t)(middle + 1u);
            else
                probe_high = middle;
        }
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
    for(id=1;id<(uint16_t)(s->index_blocks<<8);++id) {
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
    /* The CRC a byte at a time from a table (the high bytes at buffer+256,
     * the low at buffer+512): the same CRC as apollo_overlay_crc16, two
     * lookups a byte instead of eight shifts. The whole store passes through
     * here at boot, so this is most of the boot's time. */
    uint8_t *crc_high = buffer + 256, *crc_low = buffer + 512;
    /* The CRC at buffer+768: in memory at a fixed
     * address (the resident's buffer is), not on the soft stack, where
     * every byte of the loop below reloads them (about 60 cycles a byte
     * instead of 47: a minute of the C64's boot). */
    volatile uint8_t *crc_hi = buffer + 768, *crc_lo = buffer + 769;
    for (i = 0; i < 256u; ++i) {
        uint16_t t = apollo_overlay_crc16(0, (uint8_t)i);
        crc_high[i] = (uint8_t)(t >> 8);
        crc_low[i] = (uint8_t)t;
    }
    for(id=1;id<(uint16_t)(s->index_blocks<<8);++id) {
        apollo_ov2_dirent d;
        uint16_t offset;
        if(!read_bytes(page_address(s,s->index_page)+(uint32_t)id*APOLLO_REU_INDEX_RECORD,
            s->record,APOLLO_REU_INDEX_RECORD)) return APOLLO_OV2_IO_ERROR;
        if(!(s->record[11] & APOLLO_OV2_DIRENT_PRESENT)) {
            for(i=0;i<APOLLO_REU_INDEX_RECORD;++i) if(s->record[i]) return APOLLO_OV2_BAD_DIRECTORY;
            continue;
        }
        if(!lookup(s,id,&d) || !payload_inside(s,&d) ||
           d.slot>APOLLO_OV2_SLOT_PINNED_TASK || d.selectors==0 ||
           APOLLO_OV2_LANE(id)==0 || d.bytes<=APOLLO_OV2_HEADER_BYTES ||
           d.bytes>apollo_ov2_slot_bytes(d.slot) || d.flags!=APOLLO_OV2_DIRENT_PRESENT ||
           s->record[12] || s->record[13] || s->record[14] || s->record[15])
            return APOLLO_OV2_BAD_DIRECTORY;
        *crc_hi = *crc_lo = 0xffu;
        for(offset=0;offset<d.bytes;) {
            uint16_t chunk=(uint16_t)(d.bytes-offset);
            if(chunk>256u) chunk=256u;
            if(!fetch(s,&d,offset,buffer,chunk)) return APOLLO_OV2_IO_ERROR;
            if(offset==0) {
                /* Same header rules the runtime applies at load time, the
                 * BSS range inside the slot included: with
                 * APOLLO_OV2_STORE_VERIFIED a load checks only the id, and
                 * zeroes this range. */
                uint16_t base=apollo_ov2_slot_address(d.slot);
                uint32_t end = (uint32_t)base + apollo_ov2_slot_bytes(d.slot);
                uint16_t bss = le16(buffer + APOLLO_OV2_H_BSS);
                if (buffer[0] != 'N' || buffer[1] != 'A' || buffer[2] != 'T' || buffer[3] != 'V' ||
                    buffer[APOLLO_OV2_H_ABI] != APOLLO_OV2_ABI ||
                    buffer[APOLLO_OV2_H_SIZE] != APOLLO_OV2_HEADER_BYTES ||
                    (uint32_t)bss < (uint32_t)base + d.bytes || (uint32_t)bss > end ||
                    (uint32_t)le16(buffer + APOLLO_OV2_H_BSS_BYTES) > end - bss ||
                    buffer[APOLLO_OV2_H_SLOT] != d.slot ||
                    buffer[APOLLO_OV2_H_MISSION] != s->mission ||
                    le16(buffer + APOLLO_OV2_H_ID) != id ||
                    le16(buffer + APOLLO_OV2_H_ENTRY) !=
                        (uint16_t)(base + APOLLO_OV2_HEADER_BYTES) ||
                    le16(buffer + APOLLO_OV2_H_LOAD_BYTES) != d.bytes ||
                    !apollo_ov2_services_compatible(le16(buffer + APOLLO_OV2_H_SERVICES),
                                                    s->services) ||
                    le16(buffer + APOLLO_OV2_H_SELECTORS) != d.selectors)
                    return APOLLO_OV2_BAD_HEADER;
            }
            {
                /* chunk is 1 to 256: an 8-bit count (0 for 256) */
                uint8_t j = 0;
                const uint8_t end = (uint8_t)chunk;
                do {
                    uint8_t x = (uint8_t)(*crc_hi ^ buffer[j]);
                    *crc_hi = (uint8_t)(*crc_lo ^ crc_high[x]);
                    *crc_lo = crc_low[x];
                } while (++j != end);
            }
            offset=(uint16_t)(offset+chunk);
        }
        if ((uint16_t)((uint16_t)*crc_hi << 8 | *crc_lo) != d.crc16) return APOLLO_OV2_BAD_CHECKSUM;
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
