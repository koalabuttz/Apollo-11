/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "overlay_runtime.h"
static apollo_overlay_state state,before;
static struct { uint8_t low[16],slot[APOLLO_OVERLAY_BYTES],high[16]; } storage;
static uint8_t file_a[32],file_b[32];
static apollo_overlay_module modules[2];
static apollo_overlay_runtime runtime;
typedef struct {
    const uint8_t *file;
    unsigned position,length,opens,closes,invocations;
    int error_at;
    uint8_t missing,nested;
} mock_io;
static mock_io io;
static uint8_t open_file(void *user,const char *name) {
    mock_io *m=(mock_io *)user;
    ++m->opens; m->position=0;
    m->file=name[0]=='A' ? file_a : file_b;
    return (uint8_t)!m->missing;
}
static int read_byte(void *user) {
    mock_io *m=(mock_io *)user;
    if(m->error_at>=0 && m->position==(unsigned)m->error_at) return -2;
    if(m->position==m->length) return -1;
    return m->file[m->position++];
}
static void close_file(void *user) { ++((mock_io *)user)->closes; }
static uint16_t invoke(void *user,uint8_t *slot,apollo_overlay_state *image,uint16_t selector) {
    mock_io *m=(mock_io *)user;
    assert(slot==storage.slot && image==&state);
    ++m->invocations;
    if(m->nested) {
        uint16_t result=456;
        assert(apollo_overlay_call(&runtime,2,selector,&result)==APOLLO_OVERLAY_BUSY);
        assert(result==456);
    }
    ++state.words[0300];
    return (uint16_t)(selector^0x9876u);
}
static uint16_t checksum(const uint8_t *bytes,unsigned length) {
    uint16_t crc=0xffff; unsigned i;
    for(i=0;i<length;++i) crc=apollo_overlay_crc16(crc,bytes[i]);
    return crc;
}
static void prepare(void) {
    unsigned i; apollo_overlay_backend backend={&io,open_file,read_byte,close_file,invoke};
    static const uint8_t first[22]={0,128,'N','A','T','V',1,1,16,0,16,128,20,128,2,0,20,0,234,234,234,234};
    memset(&io,0,sizeof io); io.length=22; io.error_at=-1;
    memcpy(file_a,first,sizeof first); memcpy(file_b,first,sizeof first); file_b[7]=2;
    modules[0].id=1; modules[0].filename="A"; modules[0].byte_count=20; modules[0].crc16=checksum(file_a+2,20);
    modules[1].id=2; modules[1].filename="B"; modules[1].byte_count=20; modules[1].crc16=checksum(file_b+2,20);
    memset(&storage,0x5a,sizeof storage);
    for(i=0;i<APOLLO_SHARED_WORDS;++i) state.words[i]=(uint16_t)(i^0x7376u);
    apollo_overlay_init(&runtime,modules,2,backend,storage.slot,&state);
}
static void guards(void) {
    unsigned i; for(i=0;i<16;++i) assert(storage.low[i]==0x5a && storage.high[i]==0x5a);
}
static void reject(enum apollo_overlay_status expected) {
    uint16_t result=123; unsigned calls=io.invocations;
    enum apollo_overlay_status actual;
    before=state;
    actual=apollo_overlay_call(&runtime,1,42,&result);
    if(actual!=expected) fprintf(stderr,"overlay rejection expected %u, got %u\n",(unsigned)expected,(unsigned)actual);
    assert(actual==expected);
    assert(result==123 && io.invocations==calls && !runtime.loaded && !runtime.busy);
    assert(memcmp(&state,&before,sizeof state)==0); guards();
}
int main(void) {
    uint16_t result; unsigned i;
    assert(checksum((const uint8_t *)"123456789",9)==0x29b1);
    prepare(); io.nested=1;
    assert(apollo_overlay_call(&runtime,1,42,&result)==APOLLO_OVERLAY_OK && result==(42^0x9876));
    assert(io.opens==1 && io.closes==1 && storage.slot[20]==0 && storage.slot[21]==0);
    assert(apollo_overlay_call(&runtime,1,43,&result)==APOLLO_OVERLAY_OK && io.opens==1);
    assert(apollo_overlay_call(&runtime,2,44,&result)==APOLLO_OVERLAY_OK && io.opens==2);
    storage.slot[20]=85;
    assert(apollo_overlay_call(&runtime,1,45,&result)==APOLLO_OVERLAY_OK && io.opens==3 && storage.slot[20]==0);
    assert(io.invocations==4); guards();
    prepare(); io.missing=1; reject(APOLLO_OVERLAY_IO_ERROR); assert(io.closes==0);
    for(i=0;i<22;++i) { prepare(); io.length=i; reject(APOLLO_OVERLAY_SHORT_FILE); }
    for(i=0;i<22;++i) { prepare(); io.error_at=(int)i; reject(APOLLO_OVERLAY_IO_ERROR); }
    prepare(); io.error_at=22; reject(APOLLO_OVERLAY_IO_ERROR);
    prepare(); io.length=23; reject(APOLLO_OVERLAY_LONG_FILE);
    prepare(); file_a[20]^=1; reject(APOLLO_OVERLAY_BAD_CHECKSUM);
    /* Wrong PRG load address, identity, ABI, entry, length and BSS ranges. */
    for(i=0;i<14;++i) {
        static const uint8_t fields[14]={0,1,2,3,4,5,6,7,8,9,10,11,16,17};
        prepare(); file_a[fields[i]]^=1; reject(APOLLO_OVERLAY_BAD_HEADER);
    }
    prepare(); file_a[12]=0; reject(APOLLO_OVERLAY_BAD_HEADER);
    prepare(); file_a[13]=0xff; reject(APOLLO_OVERLAY_BAD_HEADER);
    prepare(); file_a[14]=0xff; file_a[15]=0xff; reject(APOLLO_OVERLAY_BAD_HEADER);
    prepare(); modules[0].byte_count=APOLLO_OVERLAY_BYTES+1u; reject(APOLLO_OVERLAY_BAD_MANIFEST);
    prepare(); modules[1].id=1; reject(APOLLO_OVERLAY_BAD_MANIFEST);
    prepare(); result=789; before=state;
    assert(apollo_overlay_call(&runtime,99,0,&result)==APOLLO_OVERLAY_UNMAPPED);
    assert(result==789 && memcmp(&state,&before,sizeof state)==0 && io.opens==0);
    puts("PASS: bounded native overlay loader; exact header/CRC; short/long/I/O failures preserve all2048words; cache, BSS reload, nested-call rejection and slot guards");
    return 0;
}
