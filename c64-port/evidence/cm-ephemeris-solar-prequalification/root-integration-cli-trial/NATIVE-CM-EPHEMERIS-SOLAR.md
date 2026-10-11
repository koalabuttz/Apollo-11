# Native Comanche solar position

The native RES path computes the Sun position from original time, live OMEGAES,
RESO and VESO. It preserves the original PUSH/PDDL stack exchanges, COS/SIN,
two UNIT calculations, cross products, shifts, scalar multiplication, vector
addition and GOTO X2. All 14 local RES words are mapped against unchanged original
assembly. The RESA store and lunar continuation have separate coverage.

The original LSPOS callers return Q022557 (bank11,2556) and Q030231
(bank14,2230); both solar paths stop at true RESA NEWOPS/LOC02161. The public
SOLPOS entry has no original caller and is qualified with declared returnQ023377.
Other SOLPOS return frames are rejected before any image or metadata mutation.

65,536 complete original/native strict/full ASAN/UBSAN 2053-word snapshots match
SHA870592153dcedea29ee4d77f1ece872ebe13bcc43db5a8c2cff2d374dc32a916.
Every snapshot compares all2048 erasable words, original IRQ/SUPER and status.
The corpus covers all source VAC/bank contexts, both real LSPOS callers, declared
SOLPOS, zeros, signs, overflow arithmetic and real UNIT abort handling.

The combined diagnostic exceeds available target RAM. Five explicit compiled
computation stages carry only canonical image state. Twelve stock-ROM PAL C64
PRGs check320 full original stage snapshots (64 original inputs through each
stage), two immutable input guards and two null guards per diagnostic. Stage3
uses four16-state cohorts; the other stages use two32-state cohorts. Checkpoints
come from uninterrupted independently executed original code, with no substituted
expected state between source stages. There is no instruction decoder on target.
PRGs, maps, full-image input/output fixtures, mailbox and stack measurements are
retained with SHA identities. Host replay verifies every frozen stage under strict
and full sanitizer builds.

Development link failures, RAM rejection, provisional caller-index and unsupported
SOLPOS return-domain findings remain under the explicitly unqualified
`evidence/cm-ephemeris-solar-prequalification/` directory. Final guarded source,
fixtures and every target executable were rebuilt after correction; frozen older
arithmetic kernels were not changed. The independent peer review pins the final
source and documents the corrected caller guard.

This qualifies the solar source body and explicit native stages. Whole mission
execution, recurring hardware timing and the complete C64 application remain
separate acceptance requirements.
