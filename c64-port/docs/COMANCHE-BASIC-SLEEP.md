# Canonical Comanche JOBSLEEP

`cm_basic_sleep` translates actual JOBSLEEP05175 through JOBSLP1, the complete
seven-core EJSCAN, context exchange, and basic DTCB, interpreted INTRSM, or idle
ADVAN03231 continuation. It commits the sole canonical image and external
SUPER/IRQ/channel11 metadata and reports the actual selected transfer. The
selected job body is not executed by this bounded service. Producer exclusion
belongs to the caller.

The source retains the sleeping job's core and VAC, stores its wake CADR,
complements its priority and saves the original EB/SUPER bits. Native context
exchange reuses the verified CM Executive service through a transient view;
negative saved LOC advances through the actual interpreted epilogue to
NEWOPS06037. EJSCAN priority1 cannot win the source empty scan; both this case
and exact BUF/Q debris are retained. Idle keeps the source L debris and clears
channel11's activity bit. Unsupported inputs preserve image, io and transfer.
The header records the ordinary caller-word, raw register, bank and priority
domains.

All16,384 complete2,051-word original assembly outputs agree, covering basic
and interpreted selection, all seven cores, ties and priority1/2 edges, idle,
five VAC bases, arbitrary ordinary caller scratch, raw L/Q patterns, and all
EB/FB/SUPER/IRQ values. Strict checks freeze64 complete states, independently
check each returned transfer against the source CPU continuation, and reject23
invalid boundaries without mutation. All16,384 outputs remain byte-identical
under ASAN/UBSAN. Uncompressed source SHA256:
`c2e5493b3678d4a52f34fc36d0bba252a40fffe485455544b0ff6a7422913fde`.

The PAL C64 VICE diagnostic passes all64 full states and transfers with mailbox
`a500004000`. PRG16652 bytes; measured hardware stack18 bytes excluding14 ROM
bytes, software stack728 bytes, and no additional zero-page allocation.
`evidence/cm-sleep-proof.json` records exact source/artifact identities.
Only the original reference XZ archive is copied into evidence; native and
sanitizer whole-stream identities are preserved in metadata and logs. Recipes
are in `tools/cm_sleep.mk`. Native code contains no AGC decoder, emulator,
floating point or heap.
