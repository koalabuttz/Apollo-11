/* SPDX-License-Identifier: MIT
 * ABI 2 entry: uint16_t apollo_ov2_main(apollo_ov2_frame *, uint16_t). */
.section .overlay_entry,"ax",@progbits
.globl apollo_ov2_jump
.type apollo_ov2_jump,@function
apollo_ov2_jump:
    jmp apollo_ov2_main
.size apollo_ov2_jump,.-apollo_ov2_jump
