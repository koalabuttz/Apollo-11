/* SPDX-License-Identifier: MIT */
#include "overlay_v2.h"

uint8_t apollo_ov2_services_compatible(uint16_t module,uint16_t resident) {
    return (uint8_t)((module>>8)==(resident>>8) && (module&0xffu)<=(resident&0xffu));
}
/* A run's slots (job or task: run() refuses slot classes 2 and 3 first). */
static uint16_t run_slot_address(uint8_t slot) {
    return slot==APOLLO_OV2_SLOT_TASK ? APOLLO_OV2_TASK_ADDRESS : APOLLO_OV2_JOB_ADDRESS;
}
static uint16_t run_slot_bytes(uint8_t slot) {
    return slot==APOLLO_OV2_SLOT_TASK ? APOLLO_OV2_TASK_BYTES : APOLLO_OV2_JOB_BYTES;
}
uint16_t apollo_ov2_slot_address(uint8_t slot) {
    return slot==APOLLO_OV2_SLOT_TASK ? APOLLO_OV2_TASK_ADDRESS :
           slot==APOLLO_OV2_SLOT_FIXED ? APOLLO_OV2_FIXED_ADDRESS :
           slot==APOLLO_OV2_SLOT_PINNED ? APOLLO_OV2_PINNED_ADDRESS : APOLLO_OV2_JOB_ADDRESS;
}
uint16_t apollo_ov2_slot_bytes(uint8_t slot) {
    return slot==APOLLO_OV2_SLOT_TASK ? APOLLO_OV2_TASK_BYTES :
           slot==APOLLO_OV2_SLOT_FIXED ? APOLLO_OV2_FIXED_BYTES :
           slot==APOLLO_OV2_SLOT_PINNED ? APOLLO_OV2_PINNED_BYTES : APOLLO_OV2_JOB_BYTES;
}
void apollo_ov2_init(apollo_ov2_runtime *r,apollo_ov2_store store,uint8_t *job_slot,
    uint8_t *task_slot,uint8_t *header,apollo_overlay_state *state,uint8_t mission,
    uint16_t services,uint8_t verify) {
    uint8_t i;
    r->store=store; r->header=header; r->slot[APOLLO_OV2_SLOT_JOB]=job_slot; r->slot[APOLLO_OV2_SLOT_TASK]=task_slot;
    r->state=state; r->mission=mission; r->services=services; r->verify=verify;
    for(i=0;i<APOLLO_OV2_SLOTS;++i) { r->loaded[i]=0; r->selectors[i]=0; }
    for(i=0;i<APOLLO_OV2_CONTEXTS;++i) r->seams[i].depth=0;
    r->busy=0; r->fault=0;
}
static uint16_t le16(const uint8_t *bytes) {
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1]<<8));
}
/* Validate everything about a directory entry and its header before any byte
 * of the slot is written. */
static enum apollo_ov2_status check_header(const apollo_ov2_runtime *r,
    const apollo_ov2_dirent *d,const uint8_t *h) {
    uint16_t base=run_slot_address(d->slot),capacity=run_slot_bytes(d->slot);
    uint16_t bss=le16(h+APOLLO_OV2_H_BSS),bss_bytes=le16(h+APOLLO_OV2_H_BSS_BYTES);
    uint32_t end=(uint32_t)base+capacity;
    if(h[0]!='N' || h[1]!='A' || h[2]!='T' || h[3]!='V' ||
       h[APOLLO_OV2_H_ABI]!=APOLLO_OV2_ABI || h[APOLLO_OV2_H_SIZE]!=APOLLO_OV2_HEADER_BYTES ||
       h[APOLLO_OV2_H_SLOT]!=d->slot || h[APOLLO_OV2_H_MISSION]!=r->mission ||
       le16(h+APOLLO_OV2_H_ID)!=d->id ||
       le16(h+APOLLO_OV2_H_ENTRY)!=(uint16_t)(base+APOLLO_OV2_HEADER_BYTES) ||
       le16(h+APOLLO_OV2_H_LOAD_BYTES)!=d->bytes ||
       !apollo_ov2_services_compatible(le16(h+APOLLO_OV2_H_SERVICES),r->services) ||
       le16(h+APOLLO_OV2_H_SELECTORS)!=d->selectors || d->selectors==0 ||
       (uint32_t)bss<(uint32_t)base+d->bytes || (uint32_t)bss>end ||
       (uint32_t)bss_bytes>end-bss)
        return APOLLO_OV2_BAD_HEADER;
    return APOLLO_OV2_OK;
}
static enum apollo_ov2_status load(apollo_ov2_runtime *r,const apollo_ov2_dirent *d) {
    uint8_t *slot=r->slot[d->slot],*header=r->header;   /* the caller refused slot classes 2 and 3 */
    uint16_t i,bss_offset,bss_bytes,crc=0xffffu;
    enum apollo_ov2_status status;
    if(!(d->flags & APOLLO_OV2_DIRENT_PRESENT) || d->slot>=APOLLO_OV2_SLOTS || !slot ||
       d->bytes<=APOLLO_OV2_HEADER_BYTES || d->bytes>run_slot_bytes(d->slot))
        return APOLLO_OV2_BAD_DIRECTORY;
    if(!r->store.fetch(r->store.user,d,0,header,APOLLO_OV2_HEADER_BYTES)) return APOLLO_OV2_IO_ERROR;
    status=check_header(r,d,header);
    if(status) return status;
    /* From here the slot is being overwritten: it is invalid until done. */
    r->loaded[d->slot]=0;
    if(!r->store.fetch(r->store.user,d,0,slot,d->bytes)) return APOLLO_OV2_IO_ERROR;
    for(i=0;i<APOLLO_OV2_HEADER_BYTES;++i) if(slot[i]!=header[i]) return APOLLO_OV2_IO_ERROR;
    if(r->verify) {
        for(i=0;i<d->bytes;++i) crc=apollo_overlay_crc16(crc,slot[i]);
        if(crc!=d->crc16) return APOLLO_OV2_BAD_CHECKSUM;
    }
    bss_offset=(uint16_t)(le16(header+APOLLO_OV2_H_BSS)-run_slot_address(d->slot));
    bss_bytes=le16(header+APOLLO_OV2_H_BSS_BYTES);
    while(bss_bytes) { slot[bss_offset++]=0; --bss_bytes; }
    r->loaded[d->slot]=d->id; r->selectors[d->slot]=d->selectors;
    return APOLLO_OV2_OK;
}
static enum apollo_ov2_status fail(apollo_ov2_runtime *r,enum apollo_ov2_status status) {
    r->fault=(uint8_t)status; r->busy=0; return status;
}
enum apollo_ov2_status apollo_ov2_run(apollo_ov2_runtime *r,uint8_t context,
    apollo_ov2_cont start,uint16_t *value,uint8_t *action,apollo_ov2_cont *resume) {
    apollo_ov2_cont at=start;
    apollo_ov2_seams *seams;
    uint8_t chain=0;
    if(r->busy) return APOLLO_OV2_BUSY;
    if(r->fault) return APOLLO_OV2_FAULTED;
    if(context>=APOLLO_OV2_CONTEXTS) return APOLLO_OV2_BAD_CONTEXT;
    if(!value || !action || !resume || !r->state || !r->header || !r->store.lookup || !r->store.fetch ||
       !r->store.resolve || !r->store.invoke)
        return APOLLO_OV2_BAD_DIRECTORY;
    seams=&r->seams[context];
    r->busy=1;
    r->frame.value=*value;
    for(;;) {
        apollo_ov2_dirent d;
        enum apollo_ov2_status status;
        uint16_t result;
        uint8_t slot;
        if(++chain>APOLLO_OV2_CHAIN_LIMIT) return fail(r,APOLLO_OV2_RUNAWAY);
        if(!at.module) return fail(r,APOLLO_OV2_UNMAPPED);
        /* A cached module needs no directory read. */
        if(r->loaded[APOLLO_OV2_SLOT_JOB]==at.module) slot=APOLLO_OV2_SLOT_JOB;
        else if(r->loaded[APOLLO_OV2_SLOT_TASK]==at.module) slot=APOLLO_OV2_SLOT_TASK;
        else {
            if(!r->store.lookup(r->store.user,at.module,&d) || d.id!=at.module)
                return fail(r,APOLLO_OV2_UNMAPPED);
            if(d.slot>=APOLLO_OV2_SLOTS) return fail(r,APOLLO_OV2_BAD_DIRECTORY);
            status=load(r,&d);
            if(status) return fail(r,status);
            slot=d.slot;
        }
        if(at.selector>=r->selectors[slot]) return fail(r,APOLLO_OV2_BAD_SELECTOR);
        r->frame.state=r->state; r->frame.context=context; r->frame.reserved=0;
        r->frame.next.module=0; r->frame.next.selector=0;
        r->frame.ret.module=0; r->frame.ret.selector=0;
        result=r->store.invoke(r->store.user,r->slot[slot],&r->frame,at.selector);
        switch(result) {
        case APOLLO_OV2_DONE:
            *value=r->frame.value; *action=APOLLO_OV2_DONE;
            resume->module=0; resume->selector=0; r->busy=0; return APOLLO_OV2_OK;
        case APOLLO_OV2_JUMP:
            at=r->frame.next; break;
        case APOLLO_OV2_CALL:
            if(seams->depth>=APOLLO_OV2_SEAM_DEPTH) return fail(r,APOLLO_OV2_SEAM_OVERFLOW);
            seams->stack[seams->depth++]=r->frame.ret; at=r->frame.next; break;
        case APOLLO_OV2_RET:
            if(!seams->depth) return fail(r,APOLLO_OV2_SEAM_UNDERFLOW);
            at=seams->stack[--seams->depth]; break;
        case APOLLO_OV2_JUMP_HISTORICAL:
            if(!r->store.resolve(r->store.user,r->frame.hist_bank,r->frame.hist_loc,&at))
                return fail(r,APOLLO_OV2_UNRESOLVED);
            break;
        default:
            if(result<APOLLO_OV2_KERNEL_ACTION || result>0xffu) return fail(r,APOLLO_OV2_BAD_ACTION);
            *value=r->frame.value; *action=(uint8_t)result; *resume=r->frame.next;
            r->busy=0; return APOLLO_OV2_OK;
        }
    }
}
void apollo_ov2_seam_exchange(apollo_ov2_runtime *r,uint8_t a,uint8_t b) {
    apollo_ov2_seams t;
    if(a>=APOLLO_OV2_JOB_CONTEXTS || b>=APOLLO_OV2_JOB_CONTEXTS || a==b) return;
    t=r->seams[a]; r->seams[a]=r->seams[b]; r->seams[b]=t;
}
void apollo_ov2_seam_release(apollo_ov2_runtime *r,uint8_t context) {
    if(context<APOLLO_OV2_CONTEXTS) r->seams[context].depth=0;
}
void apollo_ov2_restart(apollo_ov2_runtime *r) {
    uint8_t i;
    for(i=0;i<APOLLO_OV2_CONTEXTS;++i) r->seams[i].depth=0;
    for(i=0;i<APOLLO_OV2_SLOTS;++i) { r->loaded[i]=0; r->selectors[i]=0; }
    r->busy=0; r->fault=0;
}
