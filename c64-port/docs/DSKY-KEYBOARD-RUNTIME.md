# Live native LM keyboard input and queued jobs

The C64 input adapter reads the actual KERNAL GETIN routine and maps PETSCII
characters to physical channel15 five-bit codes from the original Luminary
Pinball input table. Digits0..9, V(VERB), N(NOUN), R(error reset), K(key release),
plus, minus, Return(ENTER), C/Delete(CLEAR), and shifted PETSCII letter variants
are recognized. Empty or unmapped input leaves the channel word unchanged;
unmapped raw PETSCII is returned explicitly. The adapter does not call Executive
or mutate the flight image from a hardware IRQ.

The new resident `dsky_keyboard_mappings` entry is the actual queued
CHARIN `02077/060101` source address. Its arithmetic-first wrapper composes with
existing resident basic and numerical continuation tables, avoiding an
independent authoritative flight image. The callback imports/exports the one
canonical Executive image under restoring producer exclusion, invokes the
verified full VERB/NOUN CHARIN bodies, feeds only actual channel011 events into
DSKY peripheral latches, and returns genuine `EXEC_FINISH` after source
ENDOFJOB. Native Executive releases the real core and selects the next ready
job after the callback frame returns. Other key bodies stop explicitly at
CHARIN with an unsupported native fault and retain their admitted resources;
recognizing a hardware key does not claim that its Pinball body is translated.

The original idle NUDIRECT/SUPDXCHZ startup body is translated for admitted
core0 CHARIN. Its actual activity-lamp WOR output word is stored as LOC debris,
rather than assuming activity A=2 remains unchanged. Its TC L source sequence
executes the literal one-word TC in L and leaves Q=2. Source dispatch restores
the actual physical bank/SUPER and register effects before invoking CHARIN.

Independent original-assembly comparison covers 1024 complete native pipelines
with three checkpoints each: real KEYCOM/NOVAC admission at RESUME entry,
original NUDIRECT dispatch at CHARIN entry, and complete CHARIN at ENDOFJOB.
Every checkpoint checks all 2048 physical words plus SUPERBNK/IRQ/channel011.
Native and original 3072-row snapshot SHA256:
`1963f92ce42148a081b655becc32948971e9d5c93787b7e233f7d6a424c50ffe`.
The original KEYRUPT, Executive, and Pinball bodies remain unchanged; private
fixture assembly must produce the entire pinned rope byte for byte. The oracle
links no native kernels. The whole sanitizer stream equals the original.

Strict/sanitizer frozen tests execute 32 real admitted jobs and compare all96
complete source images before performing actual Executive job completion.
C64 executes these same jobs with actual KERNAL GETIN supplying V/N events.
It additionally checks 24 physical key variants, empty input, and unmapped input.
VICE PAL mailbox: `a500002003`, zero failures; PRG36726 bytes (loaded36724);
hardware/software stack sentinel extents22/722. No extra compiler zero-page
allocation. BSS ends below `C000`, below the `C000` stack reserve. Captured artifacts
use `build/dsky-keyboard-flow-*`; full native/reference/sanitizer streams are
also retained compressed. Standalone integration: `tools/dsky_keyboard_flow.mk`
provides all/test/sanitize/differential and C64 targets.

The pipeline explicitly selects original idle NUDIRECT semantic entry after the
proven KEYCOM RESUME-entry boundary. It does not claim interrupt lead-in or the
actual source RESUME register-restoration body as completed. The full IRQ pipeline below closes this gap for the actual idle ADVAN
continuation. All remaining Pinball key bodies, flight application
command handling, and actual CM Pinball source qualification remain required.

## Complete key interrupt, RESUME, and idle continuation

`dsky_key_irq` translates original fixed vector4024, complete KEYRUPT1/KEYCOM,
original RESUME register exchanges, and architecture return before the
interrupted source operation executes again. It restores Q/A/L/banks and IRQ,
clears the interrupt state, preserves BRUPT, and returns to the source address
ZRUPT-1. BRUPT is ordinary saved source data: no native fetch, decode, or
instruction interpreter exists. A native C continuation represents the
interrupted operation. Source resource exhaustion retains the actual BAILOUT
and interrupt state rather than pretending to restore a failed interrupt.

4096 complete original checkpoints cover all32 physical FBANKs, all8 EBANKs,
all8 SUPER values, every admitted core/no-core bailout, signed-zero and raw
register-overflow corners. Every physical word and SUPER/IRQ/ISR metadata
matches original assembly. SHA256:
`4e48dfee836a46424bd98df938bba3c00a0f3555f3c6bd09670428d5e2c135a1`.
Full sanitizer stream equals the reference; strict/sanitizer/C64 frozen64 pass.
C64 mailbox `a500004002`; PRG17148 bytes; hardware/software stack26/698.

The resident hardware bridge now creates the original interrupt ZRUPT/BRUPT
frame, performs the complete native IRQ/RESUME, commits canonical Executive
words, and returns before calling a job. `dsky_keyboard_idle` implements the
actual idle ADVAN3214 +0 source branch through CAF TWO/NUDIRECT/SUPDXCHZ.
This path accepts arbitrary restored L, retaining its true BANKSET debris.
The registered CHARIN job then executes its original body and returns genuine
EXEC_FINISH; native Executive performs the actual lifecycle change.

An independent unchanged original oracle requests a **real source hardware key
interrupt** at ADVAN, executes vector4024 and RESUME, replays the original
interrupted source instruction, then runs original idle dispatch and CHARIN.
The native counterpart executes explicit compiled functions. 1024 pipelines
match at all3 original checkpoints (after RESUME, CHARIN entry, ENDOFJOB),
3072 complete2051-field states, SHA256:
`6435feca91228c46aae2f6faf0bd64517757e4b5cbfd91baeed72496bd64176e`.
There is no inserted semantic reset between IRQ and actual idle continuation.
Full sanitizer snapshots equal original snapshots byte for byte.

Native C64 runs16 actual GETIN→channel15→IRQ/RESUME→ADVAN→queued CHARIN jobs,
compares48 whole source images, and performs actual Executive completion.
The selected target inputs cover all8 retained EB and SUPER values, both keys,
all8 raw restored-L corners and four interrupted-Q classes. It also checks24
physical key variants and explicit empty/unmapped transport. Mailbox:
`a500001003`; PRG39137 bytes (loaded39135); hardware/software stack22/811;
BSS ends `B3C9`, below `C000`; no extra compiler zero-page allocation.
Diagnostic-only input values are regenerated deterministically during full-word
comparison, avoiding an additional4KB image. Strict/sanitizer frozen tests pass.
Captured evidence uses `build/dsky-key-irq-*` and
`build/dsky-keyboard-irq-flow-*`. Standalone fragments have portable host-only
`all` aliases, separate `test`/`sanitize`, and explicit `c64-*` targets.

The complete IRQ service supports wider original callers, but live application
continuation composition here proves the actual idle branch. General native
arithmetic/basic callbacks still require their explicit registered safe-point
continuation. Other Pinball key bodies and CM source qualification remain
separate required work.

## Current shared dispatcher qualification

After the shared native-dispatch source-stage/resolver extraction, the same
released keyboard bodies and frozen whole-source fixtures pass fresh strict
host/sanitizer and native C64 checks. Current bounded KEYCOM/dispatch pipeline:
mailbox `a500002003`, PRG37952 bytes (loaded37950), stack22/725. Current complete
hardware IRQ pipeline: mailbox `a500001003`, PRG40374 bytes (loaded40372),
stack22/814; BSS ends B8A6 below C000. Previous36726/39137-byte target images
and their actual logs are preserved under `build/before-packet-*`; their earlier
proof identities are not overwritten. Current logs use `*-current-*`; source
checkpoints and full oracle hashes remain unchanged. Exact shared source-file
identities are retained in `build/dsky-keyboard-current-dependencies.sha256`.

Final stable-core qualification (family guards and interpretive adoption):
Both default strict host and ASan/UBSan suites pass. C64 builds define only
`EXEC_NATIVE_FINISH_FAMILY=2`, the original complete/priority/sleep family:
qualified keyboard callbacks return FINISH; no removed action is invoked.
This preserves all32 bounded and16 complete hardware-IRQ source pipelines.
Bounded C64 mailbox `a500002003`, PRG34270 bytes (loaded34268), hardware/software
stack22/719 bytes. Complete IRQ mailbox `a500001003`, PRG36640 bytes
(loaded36638), stack22/808 bytes. Earlier default-all-families artifacts are
retained under `before-core-family-*`. Current exact source dependencies were
frozen before compilation in `build/dsky-keyboard-current-source.tar.gz`, with
the archived `tested-dependencies.sha256` and matching external manifest.
Every captured source identity still matches after both target executions.
