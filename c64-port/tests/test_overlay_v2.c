/* SPDX-License-Identifier: MIT
 * Host checks for overlay ABI 2 and the REU module store. The store image is
 * produced by tools/overlay_v2_pack.py --selftest; a register-level 17xx RAM
 * Expansion Controller model moves bytes between it and a 64 KB C64 RAM. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "overlay_reu.h"

#define LM 1u
#define SERVICES 0x0201u
#define GUARD 0x5au
#define RECORD (ram+0x0340u)   /* resident DMA record buffer */
#define HEADER (ram+0x0350u)   /* resident module header buffer */
static uint8_t ram[65536];
static uint8_t *reu,*pristine;
static unsigned long reu_size;
static uint8_t regs[16];
static unsigned dmas;
static int fail_dma_at=-1;

/* ---- 17xx controller model (fetch/stash/verify, immediate execute) ---- */
uint16_t apollo_reu_c64_address(const void *p) {
    const uint8_t *b=(const uint8_t *)p;
    assert(b>=ram && b<ram+sizeof ram);
    return (uint16_t)(b-ram);
}
uint8_t apollo_reu_peek(uint8_t reg) {
    uint8_t value=regs[reg];
    if(reg==APOLLO_REU_STATUS) regs[0]&=0x1fu;   /* reading clears IRQ/end/fault */
    return value;
}
void apollo_reu_poke(uint8_t reg,uint8_t value) {
    unsigned long i,length,c64,address;
    regs[reg]=value;
    if(reg!=APOLLO_REU_COMMAND || !(value & 0x80u)) return;
    assert(value & 0x10u);             /* port never uses the $FF00 trigger */
    c64=(unsigned long)regs[2] | ((unsigned long)regs[3]<<8);
    address=(unsigned long)regs[4] | ((unsigned long)regs[5]<<8) | ((unsigned long)regs[6]<<16);
    length=(unsigned long)regs[7] | ((unsigned long)regs[8]<<8);
    if(!length) length=65536ul;
    if(fail_dma_at>=0 && dmas==(unsigned)fail_dma_at) { ++dmas; regs[0]=0x20u; return; }
    ++dmas;
    for(i=0;i<length;++i) {
        uint8_t *c=&ram[(c64+i)&0xffffu],*r=&reu[(address+i)%reu_size];
        switch(value & 3u) {
        case 0: *r=*c; break;
        case 1: *c=*r; break;
        case 2: { uint8_t t=*c; *c=*r; *r=t; } break;
        default: if(*c!=*r) { regs[0]|=0x20u; i=length; } break;
        }
    }
    regs[0]|=0x40u;
}

/* ---- Simulated module bodies, dispatched on the loaded header ---- */
static apollo_ov2_runtime rt;
static apollo_reu_store store;
static int mode,nested;
static unsigned invocations;
static apollo_overlay_state *state=(apollo_overlay_state *)(ram+APOLLO_SHARED_ADDRESS);
static uint16_t header_id(const uint8_t *slot) { return (uint16_t)(slot[8] | (slot[9]<<8)); }
static uint16_t body(void *user,uint8_t *slot,apollo_ov2_frame *f,uint16_t selector) {
    uint16_t id=header_id(slot);
    (void)user; ++invocations;
    assert(f->state==state && f==&rt.frame);
    assert(slot==ram+(slot[6] ? APOLLO_OV2_TASK_ADDRESS : APOLLO_OV2_JOB_ADDRESS));
    if(nested) {
        uint16_t v=0; uint8_t a=0; apollo_ov2_cont c={0x0101,0},resume;
        assert(apollo_ov2_run(&rt,0,c,&v,&a,&resume)==APOLLO_OV2_BUSY);
    }
    switch(id) {
    case 0x0101:
        if(selector==0) { ++state->words[0300]; f->value=(uint16_t)(f->value+1u); return APOLLO_OV2_DONE; }
        if(selector==1) {
            f->next.module=0x0301; f->next.selector=0; f->ret.module=0x0101; f->ret.selector=2;
            return APOLLO_OV2_CALL;
        }
        if(selector==2) { f->value=(uint16_t)(f->value^0x2222u); return APOLLO_OV2_DONE; }
        f->hist_bank=024000; f->hist_loc=mode==7 ? 02177 : 02100;
        return APOLLO_OV2_JUMP_HISTORICAL;
    case 0x0301:
        if(selector==0) {
            f->value=(uint16_t)(f->value*2u);
            if(mode==3) { f->next.module=0x0301; f->next.selector=2; return 0x21u; }
            return APOLLO_OV2_RET;
        }
        if(selector==1) { f->next.module=0x0101; f->next.selector=1; return APOLLO_OV2_JUMP; }
        return APOLLO_OV2_RET;
    case 0x0102:
        assert(slot[0x3fff]==(uint8_t)((0x0102u*7u+0x4000u-25u)&0xffu));
        if(selector==0) { f->next.module=0x0101; f->next.selector=0; return APOLLO_OV2_JUMP; }
        f->next.module=0x0102; f->next.selector=1; f->ret.module=0x0102; f->ret.selector=1;
        return APOLLO_OV2_CALL;
    case 0x0f01:
        if(mode==1) return 5u;                       /* not a defined action */
        if(mode==2) return 0x100u;                   /* outside the action byte */
        f->next.module=0x0f01; f->next.selector=0; return APOLLO_OV2_JUMP;   /* runaway */
    }
    assert(0); return 0;
}

static void reset_ram(void) {
    unsigned i;
    memset(ram,GUARD,sizeof ram);
    for(i=0;i<APOLLO_SHARED_WORDS;++i) state->words[i]=(uint16_t)(i^0x7376u);
    memcpy(reu,pristine,reu_size);
    fail_dma_at=-1; mode=0; nested=0;
}
static void boot(uint8_t verify) {
    apollo_ov2_store backend;
    reset_ram();
    assert(apollo_reu_store_open(&store,0,LM,SERVICES,RECORD));
    backend=apollo_reu_store_backend(&store,body,0);
    apollo_ov2_init(&rt,backend,ram+APOLLO_OV2_JOB_ADDRESS,ram+APOLLO_OV2_TASK_ADDRESS,HEADER,state,LM,SERVICES,verify);
}
static enum apollo_ov2_status run(uint8_t context,uint16_t module,uint16_t selector,
    uint16_t *value,uint8_t *action,apollo_ov2_cont *resume) {
    apollo_ov2_cont c; c.module=module; c.selector=selector;
    return apollo_ov2_run(&rt,context,c,value,action,resume);
}
static void guards(void) {
    unsigned i;
    for(i=0xc000u;i<0xd000u;++i) assert(ram[i]==GUARD);
    for(i=0xfffau;i<0x10000u;++i) assert(ram[i]==GUARD);
    for(i=0x0000u;i<APOLLO_SHARED_ADDRESS;++i) assert(ram[i]==GUARD || (i>=0x0340u && i<0x0368u));
}
static void check_loaded(uint16_t id,uint16_t base,unsigned body_bytes,unsigned bss) {
    unsigned i; const uint8_t *s=ram+base;
    assert(header_id(s)==id);
    for(i=0;i<body_bytes;++i) assert(s[24+i]==(uint8_t)((id*7u+i)&0xffu));
    for(i=0;i<bss;++i) assert(s[24+body_bytes+i]==0);
}
static unsigned long reu_page(uint16_t id) {
    const uint8_t *r=reu+256u+id*16u; assert(r[0]==(uint8_t)id);
    return ((unsigned long)r[2] | ((unsigned long)r[3]<<8))*256ul;
}
static unsigned fixed_seen;
static uint8_t fixed_load(void *user,uint32_t reu_address,const apollo_ov2_dirent *d) {
    (void)user; ++fixed_seen;
    assert(d->id==0x0a02 && d->slot==APOLLO_OV2_SLOT_FIXED);
    return (uint8_t)((apollo_reu_transfer(APOLLO_REU_FETCH,ram+APOLLO_OV2_FIXED_ADDRESS,reu_address,d->bytes)&0x60u)==0x40u);
}
static void expect_fault(enum apollo_ov2_status expected,uint8_t context,uint16_t module,uint16_t selector) {
    uint16_t value=77; uint8_t action=99; apollo_ov2_cont resume={1,1};
    enum apollo_ov2_status got=run(context,module,selector,&value,&action,&resume);
    if(got!=expected) fprintf(stderr,"expected status %d, got %d\n",(int)expected,(int)got);
    assert(got==expected && rt.fault==(uint8_t)expected && !rt.busy);
    assert(run(0,0x0101,0,&value,&action,&resume)==APOLLO_OV2_FAULTED);
    apollo_ov2_restart(&rt);
    assert(!rt.fault && !rt.loaded[0] && !rt.loaded[1]);
}

int main(int argc,char **argv) {
    FILE *file; uint16_t value; uint8_t action; apollo_ov2_cont resume;
    unsigned before,i; apollo_overlay_state saved;
    assert(argc==2);
    file=fopen(argv[1],"rb"); assert(file);
    fseek(file,0,SEEK_END); reu_size=(unsigned long)ftell(file); fseek(file,0,SEEK_SET);
    assert(reu_size==128ul*1024ul);
    reu=malloc(reu_size); pristine=malloc(reu_size); assert(reu && pristine);
    assert(fread(pristine,1,reu_size,file)==reu_size); fclose(file);

    assert(apollo_ov2_services_compatible(0x0200,0x0201) && apollo_ov2_services_compatible(0x0201,0x0201));
    assert(!apollo_ov2_services_compatible(0x0202,0x0201) && !apollo_ov2_services_compatible(0x0101,0x0201));

    /* Controller model: stash, fetch and verify round trip, 64 KB length. */
    reset_ram();
    memset(ram+0x9000,0xa7,32);
    assert((apollo_reu_transfer(APOLLO_REU_STASH,ram+0x9000,0x1ff00ul,32)&0x60u)==0x40u);
    assert((apollo_reu_transfer(APOLLO_REU_VERIFY,ram+0x9000,0x1ff00ul,32)&0x60u)==0x40u);
    ram[0x9005]=0;
    assert((apollo_reu_transfer(APOLLO_REU_VERIFY,ram+0x9000,0x1ff00ul,32)&0x60u)==0x60u);
    assert((apollo_reu_transfer(APOLLO_REU_FETCH,ram+0x9000,0x1ff00ul,32)&0x60u)==0x40u && ram[0x9005]==0xa7);
    assert((apollo_reu_transfer(APOLLO_REU_FETCH,ram,0,0)&0x60u)==0x40u && memcmp(ram,reu,65536)==0);

    /* Store open: magic, mission and service-table version gate the image. */
    reset_ram();
    assert(apollo_reu_store_open(&store,0,LM,SERVICES,RECORD));
    assert(store.module_count==5 && store.resolver_count==3 && store.index_page==1);
    assert(!apollo_reu_store_open(&store,0,2,SERVICES,RECORD));
    assert(!apollo_reu_store_open(&store,0,LM,SERVICES+1u,RECORD));
    reu[0]='X'; assert(!apollo_reu_store_open(&store,0,LM,SERVICES,RECORD));
    reset_ram(); reu[16]=(uint8_t)(reu[16]-1u);      /* image shorter than its regions */
    assert(apollo_reu_store_open(&store,0,LM,SERVICES,RECORD)); /* still inside: resolver/modules */
    reset_ram(); reu[16]=1; reu[17]=0;
    assert(!apollo_reu_store_open(&store,0,LM,SERVICES,RECORD));

    /* Boot verification over the packed image, then each corruption class. */
    boot(0); assert(apollo_reu_store_verify(&store,ram+0xc000)==APOLLO_OV2_OK);
    memset(ram+0xc000,GUARD,256);
    reu[reu_page(0x0101)+100]^=1u;
    assert(apollo_reu_store_verify(&store,ram+0xc000)==APOLLO_OV2_BAD_CHECKSUM);
    boot(0); reu[reu_page(0x0301)+7]=2;                  /* header mission */
    assert(apollo_reu_store_verify(&store,ram+0xc000)==APOLLO_OV2_BAD_HEADER);
    boot(0); reu[256u+0x0200u*16u+3u]=1;                 /* stray absent record */
    assert(apollo_reu_store_verify(&store,ram+0xc000)==APOLLO_OV2_BAD_DIRECTORY);
    boot(0); reu[256u+0x0101u*16u+10u]=7;                /* unknown slot class */
    assert(apollo_reu_store_verify(&store,ram+0xc000)==APOLLO_OV2_BAD_DIRECTORY);
    boot(0); {                                            /* resolver order */
        unsigned long r=(unsigned long)store.resolver_page*256ul;
        uint8_t t[8]; memcpy(t,reu+r,8); memcpy(reu+r,reu+r+8,8); memcpy(reu+r+8,t,8);
        assert(apollo_reu_store_verify(&store,ram+0xc000)==APOLLO_OV2_BAD_DIRECTORY);
    }
    boot(0); reu[(unsigned long)store.resolver_page*256ul+6u]=9;   /* selector out of range */
    assert(apollo_reu_store_verify(&store,ram+0xc000)==APOLLO_OV2_UNRESOLVED);
    boot(0); {                                            /* record naming the fixed extension */
        unsigned long r=(unsigned long)store.resolver_page*256ul;
        reu[r+4u]=0x02; reu[r+5u]=0x0a; reu[r+6u]=0; reu[r+7u]=0;
        assert(apollo_reu_store_verify(&store,ram+0xc000)==APOLLO_OV2_UNRESOLVED);
    }

    /* Slot class 2: boot lists the fixed extension for loading at $d000;
     * a run never loads it. */
    boot(0); fixed_seen=0;
    assert(apollo_reu_store_fixed(&store,fixed_load,0) && fixed_seen==1);
    check_loaded(0x0a02,APOLLO_OV2_FIXED_ADDRESS,100,0); guards();
    assert(apollo_ov2_slot_address(APOLLO_OV2_SLOT_FIXED)==0xd000u && apollo_ov2_slot_bytes(APOLLO_OV2_SLOT_FIXED)==0x1000u);
    boot(1); expect_fault(APOLLO_OV2_BAD_DIRECTORY,0,0x0a02,0);

    /* Load, cache and swap the job slot; BSS is zeroed, payload exact. */
    boot(1); value=41;
    assert(run(0,0x0101,0,&value,&action,&resume)==APOLLO_OV2_OK && action==APOLLO_OV2_DONE && value==42);
    check_loaded(0x0101,0x8000,300,16); assert(state->words[0300]==(0300^0x7376u)+1u);
    before=dmas; ram[0x8000u+24u+300u]=0x33;   /* BSS write survives a cached call */
    assert(run(0,0x0101,0,&value,&action,&resume)==APOLLO_OV2_OK && value==43);
    assert(dmas==before && ram[0x8000u+24u+300u]==0x33);     /* no DMA at all */
    assert(run(0,0x0102,0,&value,&action,&resume)==APOLLO_OV2_OK && value==44);  /* swap J twice */
    check_loaded(0x0101,0x8000,300,16); guards();

    /* A full 16 KB job module reaches $bfff exactly. */
    boot(1); value=0; mode=0;
    {
        apollo_ov2_cont c={0x0102,0};
        assert(apollo_ov2_run(&rt,1,c,&value,&action,&resume)==APOLLO_OV2_OK);
    }
    assert(value==1); guards();

    /* CALL into the task slot and RET back keeps the job slot cached. */
    boot(1); value=5;
    assert(run(2,0x0101,1,&value,&action,&resume)==APOLLO_OV2_OK && action==APOLLO_OV2_DONE);
    assert(value==(uint16_t)((5u*2u)^0x2222u) && rt.seams[2].depth==0);
    check_loaded(0x0101,0x8000,300,16); check_loaded(0x0301,0xe000,200,8);
    assert(rt.loaded[0]==0x0101 && rt.loaded[1]==0x0301); guards();

    /* Kernel action while a seam is pending; CHANJOB moves the seam with its
     * job; the resumed body returns through it. */
    boot(1); value=3; mode=3;
    assert(run(3,0x0101,1,&value,&action,&resume)==APOLLO_OV2_OK);
    assert(action==0x21u && value==6 && resume.module==0x0301 && resume.selector==2);
    assert(rt.seams[3].depth==1);
    apollo_ov2_seam_exchange(&rt,3,5);
    assert(rt.seams[3].depth==0 && rt.seams[5].depth==1);
    mode=0;
    assert(apollo_ov2_run(&rt,5,resume,&value,&action,&resume)==APOLLO_OV2_OK);
    assert(action==APOLLO_OV2_DONE && value==(6u^0x2222u) && rt.seams[5].depth==0);
    apollo_ov2_seam_exchange(&rt,5,APOLLO_OV2_TASK_CONTEXT);   /* task context never exchanges */
    apollo_ov2_seam_release(&rt,5);

    /* Historical continuation: resolver hit and miss. */
    boot(1); value=4;
    assert(run(0,0x0101,3,&value,&action,&resume)==APOLLO_OV2_OK);
    assert(action==APOLLO_OV2_DONE && value==(8u^0x2222u) && rt.seams[0].depth==0);
    boot(1); mode=7; expect_fault(APOLLO_OV2_UNRESOLVED,0,0x0101,3);

    /* Faults latch until restart and never corrupt the canonical image. */
    boot(1); saved=*state;
    expect_fault(APOLLO_OV2_SEAM_OVERFLOW,4,0x0102,1);
    expect_fault(APOLLO_OV2_SEAM_UNDERFLOW,4,0x0301,2);
    mode=1; expect_fault(APOLLO_OV2_BAD_ACTION,0,0x0f01,0);
    mode=2; expect_fault(APOLLO_OV2_BAD_ACTION,0,0x0f01,0);
    mode=0; expect_fault(APOLLO_OV2_RUNAWAY,0,0x0f01,0);
    expect_fault(APOLLO_OV2_BAD_SELECTOR,0,0x0101,4);
    expect_fault(APOLLO_OV2_UNMAPPED,0,0x0103,0);
    expect_fault(APOLLO_OV2_UNMAPPED,0,0,0);
    assert(memcmp(state,&saved,sizeof saved)==0); guards();
    value=0; assert(run(APOLLO_OV2_CONTEXTS,0x0101,0,&value,&action,&resume)==APOLLO_OV2_BAD_CONTEXT);

    /* Nested runs are refused while a body is live. */
    boot(1); nested=1; value=0;
    assert(run(0,0x0101,0,&value,&action,&resume)==APOLLO_OV2_OK && value==1);

    /* Header rejection happens before any slot byte changes. */
    boot(1); value=0;
    assert(run(0,0x0101,0,&value,&action,&resume)==APOLLO_OV2_OK);
    reu[reu_page(0x0102)+7]=2;
    {
        uint8_t copy[0x4000]; memcpy(copy,ram+0x8000,sizeof copy);
        expect_fault(APOLLO_OV2_BAD_HEADER,0,0x0102,0);
        assert(memcmp(copy,ram+0x8000,sizeof copy)==0);
    }
    /* Payload corruption: CRC catches it when per-load verify is on, the slot
     * is invalidated; header mismatch between the two reads is an I/O error. */
    boot(1); reu[reu_page(0x0101)+200]^=0x80u;
    expect_fault(APOLLO_OV2_BAD_CHECKSUM,0,0x0101,0);
    boot(0); reu[reu_page(0x0101)+200]^=0x80u;
    value=0; assert(run(0,0x0101,0,&value,&action,&resume)==APOLLO_OV2_OK);
    /* DMA failure at each step of a cold load. */
    for(i=0;i<3u;++i) {
        boot(1); fail_dma_at=(int)(dmas+i);
        expect_fault(i==0 ? APOLLO_OV2_UNMAPPED : APOLLO_OV2_IO_ERROR,0,0x0101,0);
        assert(!rt.loaded[0]);
    }
    /* Directory record that names another module. */
    boot(1); reu[256u+0x0101u*16u]=0x02;
    expect_fault(APOLLO_OV2_UNMAPPED,0,0x0101,0);

    /* Restart drops seams and caches but keeps canonical words. */
    boot(1); mode=3; value=1;
    assert(run(6,0x0101,1,&value,&action,&resume)==APOLLO_OV2_OK && rt.seams[6].depth==1);
    saved=*state; apollo_ov2_restart(&rt);
    assert(rt.seams[6].depth==0 && !rt.loaded[0] && !rt.loaded[1] && memcmp(state,&saved,sizeof saved)==0);

    printf("overlay v2: %u invocations, %u DMA transfers, all checks passed\n",invocations,dmas);
    free(reu); free(pristine);
    return 0;
}
