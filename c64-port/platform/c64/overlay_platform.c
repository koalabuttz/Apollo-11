/* SPDX-License-Identifier: MIT */
#include <cbm.h>
#include "overlay_platform.h"
static void restore(c64_overlay_io *io) {
    *(volatile uint8_t *)1=io->saved_port;
    __asm__ volatile("pha\nplp" : : "a"(io->saved_status) : "memory");
}
static uint8_t open_file(void *user,const char *name) {
    c64_overlay_io *io=(c64_overlay_io *)user;
    __asm__ volatile("php\npla" : "=a"(io->saved_status) : : "memory");
    io->saved_port=*(volatile uint8_t *)1;
    /* Keep RAM at $a000 visible, and expose KERNAL/I/O for stock serial I/O. */
    *(volatile uint8_t *)1=(uint8_t)((io->saved_port | 6u)&0xfeu);
    io->opened=0; io->eof=0; io->last_error=0;
    cbm_k_setnam(name); cbm_k_setlfs(2,io->device,2);
    io->last_error=cbm_k_open();
    if(io->last_error!=0) { cbm_k_close(2); restore(io); return 0; }
    io->opened=1;
    io->last_error=cbm_k_chkin(2);
    if(io->last_error!=0) { cbm_k_close(2); io->opened=0; restore(io); return 0; }
    return 1;
}
static int read_byte(void *user) {
    c64_overlay_io *io=(c64_overlay_io *)user;
    uint8_t value,status;
    if(io->eof) return -1;
    value=cbm_k_chrin(); status=cbm_k_readst();
    if((status & 0xbfu)!=0) { io->last_error=status; return -2; }
    if(status & 0x40u) io->eof=1;
    return value;
}
static void close_file(void *user) {
    c64_overlay_io *io=(c64_overlay_io *)user;
    cbm_k_clrch();
    if(io->opened) cbm_k_close(2);
    io->opened=0; restore(io);
}
static uint16_t invoke(void *user,uint8_t *slot,apollo_overlay_state *state,uint16_t selector) {
    apollo_overlay_entry entry=(apollo_overlay_entry)(uintptr_t)(slot+APOLLO_OVERLAY_HEADER_BYTES);
    uint8_t port=*(volatile uint8_t *)1,status; uint16_t result;
    (void)user;
    __asm__ volatile("php\npla" : "=a"(status) : : "memory");
    *(volatile uint8_t *)1=(uint8_t)((port | 6u)&0xfeu);
    result=entry(state,selector);
    *(volatile uint8_t *)1=port;
    __asm__ volatile("pha\nplp" : : "a"(status) : "memory");
    return result;
}
apollo_overlay_backend c64_overlay_backend(c64_overlay_io *io) {
    apollo_overlay_backend backend;
    backend.user=io; backend.open=open_file; backend.read=read_byte;
    backend.close=close_file; backend.invoke=invoke; return backend;
}
