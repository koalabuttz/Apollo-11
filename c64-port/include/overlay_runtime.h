/* SPDX-License-Identifier: MIT */
#ifndef OVERLAY_RUNTIME_H
#define OVERLAY_RUNTIME_H
#include <stdint.h>
#define APOLLO_OVERLAY_ABI 1u
#define APOLLO_SHARED_WORDS 2048u
#define APOLLO_SHARED_ADDRESS 0x7000u
#define APOLLO_OVERLAY_ADDRESS 0x8000u
#define APOLLO_OVERLAY_BYTES 0x4000u
#define APOLLO_OVERLAY_HEADER_BYTES 16u
#define APOLLO_OVERLAY_ENTRY_ADDRESS 0x8010u
/* Sole persistent erasable image. Source absolute erasable addresses index
 * words directly, including A/L's uncorrected 16-bit register representations.
 * Native structures may be transient views; commit them before returning to
 * the resident scheduler. No pointer into overlay code/data may persist. */
typedef struct { uint16_t words[APOLLO_SHARED_WORDS]; } apollo_overlay_state;
typedef uint16_t (*apollo_overlay_entry)(apollo_overlay_state *,uint16_t);
typedef struct {
    uint8_t id;
    const char *filename;
    uint16_t byte_count,crc16;
} apollo_overlay_module;
enum apollo_overlay_status {
    APOLLO_OVERLAY_OK=0, APOLLO_OVERLAY_BUSY=1,
    APOLLO_OVERLAY_UNMAPPED=2, APOLLO_OVERLAY_BAD_MANIFEST=3,
    APOLLO_OVERLAY_IO_ERROR=4, APOLLO_OVERLAY_SHORT_FILE=5,
    APOLLO_OVERLAY_LONG_FILE=6, APOLLO_OVERLAY_BAD_HEADER=7,
    APOLLO_OVERLAY_BAD_CHECKSUM=8
};
/* Backend read: byte 0..255, EOF -1, error -2. It must never write the slot
 * itself. open/close obey the resident platform's IRQ/ROM ownership contract.
 * invoke returns normally with the LLVM-MOS C ABI on the target. */
typedef struct {
    void *user;
    uint8_t (*open)(void *,const char *);
    int (*read)(void *);
    void (*close)(void *);
    uint16_t (*invoke)(void *,uint8_t *,apollo_overlay_state *,uint16_t);
} apollo_overlay_backend;
typedef struct {
    const apollo_overlay_module *modules;
    apollo_overlay_backend backend;
    uint8_t *slot;
    apollo_overlay_state *state;
    uint8_t count,loaded,busy;
} apollo_overlay_runtime;
uint16_t apollo_overlay_crc16(uint16_t crc,uint8_t byte);
/* Caller owns initialized distinct storage, source exclusion, module table
 * and backend lifetime. No loading/execution occurs during initialization. */
void apollo_overlay_init(apollo_overlay_runtime *,const apollo_overlay_module *,
    uint8_t,apollo_overlay_backend,uint8_t *,apollo_overlay_state *);
/* Explicit native entry selector; never an AGC instruction/opcode. Slot swaps
 * occur only before entry and after all previous overlay C frames returned.
 * All loader failures preserve persistent state and the caller's result. A
 * partial/failed image is invalidated and cannot be invoked. This is bounded
 * disk integrity checking, not authentication against hostile media. */
enum apollo_overlay_status apollo_overlay_call(apollo_overlay_runtime *,
    uint8_t module,uint16_t selector,uint16_t *result);
#endif
