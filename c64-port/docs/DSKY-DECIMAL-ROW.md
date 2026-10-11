# Original decimal formatter row control

`dsky_decimal_row` translates the original LM and CM DSPDCEND bank call and its SWRETURN/row continuation. Explicit CALL selection starts at41,2566, saves BUF2={02570,062000}, corrects incoming A/L as the original exchanges do and enters actual DSPDECWD40,3226 (LM) or40,3133 (CM), Q04631/04675, retaining the selected EB and SUPER.

RETURN starts at actual SWRETURN with that saved BUF2 and executes its bank/exchange effects. DECOUNT1/2 is decremented and selects original DSPDCPUT41,2537, Q02575. DECOUNT0 selects actual erasable ENTRET entry0136, Q02573, before execution of the saved return instruction. No native opcode decoding, formatter body, saved ENTRET instruction, following NVSUBEND or Executive body runs here.

The canonical image alone owns RAM. Caller excludes producers and supplies ordinary RAM, coherent source banks/all EBs, selecting SUPER100/120/140/160, legal IRQ, row count0..2 and source saved return pair. Incoming A/L retain all16-bit patterns. Unsupported state is unchanged.

All8,192 complete original LM/CM states equal strict native and full ASAN/UBSAN streams, raw SHA256 `031302d804284b6d9700a31c30505a6383c13b0b5d0dfa383016c764d7f176dd`. Frozen checks compare128 full states plus30 unchanged invalid guards, including raw overflow registers, both signed zeros, all rows and banks. PAL C64 passes all128: mailbox `a500008000`, PRG6,610 bytes, hardware stack14 bytes including14 ROM bytes, software stack17 bytes and no additional zero-page allocation.

Exact sources, native dependencies and retained compressed original/target identities appear in `evidence/dsky-decimal-row-proof.json`. `tools/dsky_decimal_row.mk` provides all/snapshot, frozen tests, full-stream sanitizer and C64 builds. This qualifies the stated row-control bridges, not a whole Pinball or application closure.
