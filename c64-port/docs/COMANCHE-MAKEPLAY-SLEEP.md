# Comanche normal display sleep continuation

`cm_makeplay_sleep_enter`, `cm_makeplay_sleep_return`, and
`cm_makeplay_sleep_resume` translate the original normal-display GOSLEEPS,
XCHTOEND/XCHSLEEP continuation, and PLAYJUM1 bodies. They operate on the sole
canonical image and explicit external CM metadata; callers own producer
exclusion. Unsupported boundaries preserve the complete image and io.

The enter body starts at bank10/03176 with COPINDEX2, sets normal wait bit01000,
retains the CM UPENT2 mask in ITEMP1 and its flag-word index4 in L, and enters
actual JOBWAKE05201 with the original normal wake CADR021037. The return body
accepts source LOCCTR-2 or a seven-core displacement0..72, replaces a found
sleeping continuation with ENDOFJOB05217, and enters actual JOBSLEEP entry **05175**. PLAYJUM1 bank10/03037 restores COPINDEX2 and enters GOPLAY03312.
This package covers these bounded bodies; canonical wake/sleep services and
the subsequent Pinball consumer execute separately.

All 6,144 complete 2,051-word states agree with the unchanged original assembly,
covering all EB/SUPER/IRQ values, both signs and zeros in caller L, all legal
wake displacements including0/-2, arbitrary ordinary caller scratch, and both
initial wait-bit values. Strict checks freeze64 complete states and reject39
invalid boundaries without mutation. All source states remain byte-identical
under ASAN/UBSAN. The uncompressed source stream SHA256 is
`fa1b35a32a571822a1adc6372a494b009c57e575b28a60bd146a6dfa4ff27f0d`.

The PAL C64 VICE diagnostic passes all64 frozen complete states with mailbox
`a500004000`; PRG5141 bytes, measured hardware stack14 bytes excluding14 ROM
bytes, software stack73 bytes, and no additional zero-page allocation.
`evidence/cm-makeplay-sleep-proof.json` records exact source/artifact hashes.
Only the reference XZ archive is copied into evidence; complete native and
sanitizer output identities are retained in the proof metadata and logs.
The differential and sanitizer runners stream compressed cohorts directly.
Recipes are in `tools/cm_makeplay_sleep.mk`. Native code uses no emulator,
instruction decoder, floating point, or heap.
