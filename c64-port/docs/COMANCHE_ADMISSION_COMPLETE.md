# Comanche canonical admission and resource exits

`cm_admission_complete` translates actual NOVAC (`05134`), FINDVAC (`05147`),
SPVAC (`05160`) and SPVACIN (`05154`) through the genuine caller return or
BAILOUT (`07755`) entry. It operates on the sole physical 2,048-word image and
explicit I/O. It does not execute BAILOUT's later alarm/restart body or the
newly admitted application body.

Successful paths retain the source register, scratch, seven-core and five-VAC
effects already established by the native Executive. Resource exits now also
retain complete source CPU state: no VAC returns with Q=`02642`, no core with
Q=`02724`, A=0 and Executive FBANK. A VAC acquired before core exhaustion
remains reserved. Native `executive_alarm` is separate pending-continuation
metadata for the source `01201`/`01202` literal; it is not an erasable word or
an executed alarm body. SPVAC's actual XCH/AD NEG2/XCH entry adjusts the saved
caller Q before the common source return. SPVAC/SPVACIN require source caller
exclusion and NEWPRIO setup.

The supported domain uses positive upper priorities `01000..037000`,
ordinary compiled Q addresses, consistent physical banks and legal core/VAC
states. Source CCSHOLE states and priority=+0 core states remain excluded.
Unsupported inputs preserve the canonical image and explicit I/O. Inline job
pairs are compiled caller metadata; SPVAC entries use their actual A/L pair.
No AGC opcode decoder or runtime instruction-word dispatch is introduced.

The source corpus covers all eight first-free-core outcomes (including
exhaustion), all six first-free-VAC outcomes (including exhaustion), all four
entries and 64 priority/bank/background sets: 12,288 complete original states.
It compares raw A/L/Q, every physical word, SUPERBNK and IRQ. The retained
lossless stream SHA256 is
`d5791aa9a0204191d463cd192875e9fad306e0b2a4a1fc2911ba2467b0f73a33`.
Strict compilation and the complete ASAN/UBSAN stream pass. The source uses
the independently assembled original-layout rope `6e3150ae...`, the preserved
historical Executive, its documented indentation-only repairs and pinned
host-only VirtualAGC. No historical files are edited.

The C64 diagnostic uses 96 balanced representative contexts across all four
entries, resource corners and bank sets, with both resource failures included.
It uses actual disk-loaded compiled control module `210`,
source-input module `244` and complete-state checker `245`. Each stage commits
before its C frame returns and the next payload loads. The image remains
`$7000..$7fff`, metadata outside that image, and payloads fit `$8000..$bfff`.
Both incoming C64 interrupt states and ROM mapping restoration are checked.
The retained target report gives the exact cohort, mailbox, stack measurement,
complete samples and final direct whole-image comparison.

The earlier 192-case disk diagnostic exhausted its 600-second budget without
a completion mailbox. Its exact bytes, inputs and logs are preserved in
`evidence/cm-admission-complete-timeout-192`; it supplies no native pass/failure
verdict. The final cohort reduces disk traffic while the full host source and
sanitizer corpus remains unchanged.

Run `make -f Makefile -f tools/cm_admission_complete.mk
test-cm-admission-complete` and `sanitize-cm-admission-complete`. The source
target depends explicitly on `prepare-cm-original-rope` for a fresh checkout.
`cm_admission_complete_build.py` reproduces the actual payloads and disk;
`cm_admission_complete_vice_test.py` compares stock PAL C64 execution.
