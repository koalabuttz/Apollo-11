# Compiled P66 driver and canonical erasable binding

The bounded live driver executes the original RODCOMP preparation and the
interpretive prefix through STOVL24D/R. Its callbacks perform the translated
arithmetic, then return through the native Executive trampoline. The current
milestone stops with the job and VAC retained before UNIT; it never substitutes
ENDJOB or declares the remaining landing program complete.

| Native continuation | Kind | Arithmetic completed | Next continuation |
| --- | --- | --- | --- |
| 03275 | basic | preparation, before TC INTPRET03353 | ENTER03354 |
| 03354 | interpretive | VXSC/PDDL THISTPIP, LOC03356 | YIELD03357 |
| 03357 | interpretive | DSU PIPTIME/STORE30D, LOC03361 | YIELD03362 |
| 03362 | interpretive | DDV4SEC(28)/PDVL GDT/2, LOC03364 | YIELD03365 |
| 03365 | interpretive | velocity update/STOVL24D/R, LOC03373 | YIELD03374 |
| 03374 | interpretive | bounded observation before UNIT | scope fault |

Every interpretive yield has source EDOP=+0 with the operation pair consumed.
The source RODTASK's admission has complete BBCON064067, including SUPERBNK060;
the table also accepts the physical pair064007. Neither is a C function address.
Extension modules can replace the last mapping and continue at subsequent
independently established source boundaries.

## Ownership and storage

`apollo_overlay_state.words[2048]` is the sole authoritative flight erasable
image. Absolute source addresses index it directly. Each numeric entry gathers
ordinary transient preparation/scaling/elapsed/divide/VAC objects, calls the
verified kernels, and commits them before returning its C frame. Inputs at
PIPTIME, GDT/2, VBIAS, V and R remain stable under caller producer exclusion.
A/L use their uncorrected16-bit representations. No independent persistent
numerical context, heap, opcode decoder or saved C stack exists.

Owned numerical words are the117-word velocity fixture set: rate/sensor/time
state, preparation scratch, MPAC/MODE, MPTEMP/OVFIND, FIXLOC/PUSHLOC,43 VAC
payload words, elapsed/divide scratch and the four stable input vectors/pairs.
Compiled final operand addresses are retained explicitly: ADDRWD01761 after
PDDL THISTPIP; FIXLOC+30 decimal after STORE30D; ADDRWD01636 after PDVL GDT/2;
and ADDRWD01520 after R. The gravity boundary still has physical EBANK2/
BBANK064002 before DANZIG restores BANKSET; this source debris is preserved.
Final EDOP and hardware-zero scratch are+0.

Executive import/export copies actual core/VAC arrays plus its source scalar
metadata. Its alarm field is native boundary metadata and is preserved on
import. A caller that mutates the transient Executive through admission, wake
or another service must export it before the next driver call. Each driver
call re-gathers its view; it exports at the arithmetic boundary and again after
the scheduler action. Runtime metadata and C function pointers are native
objects; they do not occupy source LOC or BANKSET.

The driver requires restoring producer-exclusion hooks. They protect the whole
numeric operation, including an overlay load/call, and the canonical-view
transfers. Executive exchanges occur after callback return under the runtime's
own guard. The platform restores the caller's IRQ state, including a masked
caller; this is the declared native adaptation of source INHINT/RELINT, rather
than an emulated interrupt-enable register.

The arithmetic observer sees A/L immediately before the corresponding source
control boundary. Executive does not save these registers per job. Their image
values are arithmetic observations, not a claim that native scheduler routines
emulate every transient AGC CPU register. Q, arbitrary interpreter dispatch
scratch and every unowned erasable word are outside this driver's equivalence
claim. The442-word mixed checkpoint adds all96 core words,220 VAC words,
FIXLOC/OVFIND and ADDRWD/EDOP/INTBIT15/ZERO/EBANK/FBANK/BBANK to117 numerical
words, intentionally retaining overlapping aliases in both views.

## Returning overlays and target layout

`P66_NATIVE_ARITH_ONLY` with `P66_NATIVE_ONLY_GROUP=N` produces one bounded
numeric module. `P66_NATIVE_DISPATCH_ONLY` produces the resident bridge with an
explicit overlay-invoker hook. A missing invoker fails closed. The invoker must
return normally before a subsequent module may replace the slot. Source groups
can later be split into internal returned selectors without scheduling until
the original pair-consumed boundary.

The target proof uses production layout: resident below$7000, canonical image
$7000..$7fff, one overlay slot$8000..$bfff, software stack$c800..$cfff. The
reserved scratch allocation is fixture metadata$c000..$c03f, mailbox$c080,
transient Executive view$c100..$c3ff and native scheduler metadata$c400..$c7ff.
Compile-time bounds enforce both view and metadata leases. None becomes a
second authoritative source image. Test input/checker modules use IDs250/251,
leaving numeric selectors1..5 and future continuation IDs available.

The proof input/checker code is streamed as separate test overlays so source
constructors and frozen expectations do not inflate production resident size.
Checker modules read the flight image and update only diagnostic metadata.
They return before any next arithmetic module loads. The five numeric modules
have no BSS and are independently linked native C, each below16KB.

## Verification

`tools/p66_native_differential.py deps/fixture` compares1,155 full-prefix inputs
and5,775 complete117-word arithmetic observations with unchanged original
assembly, using the independently pinned hardware-DV reference. An additional
16 two-P66-job sequences admit an already prepared higher-priority context at
the first pair-consumed yield. Its real preparation and four arithmetic groups
execute while the first job's MPAC/MODE/PUSH/OVFIND encoding and owned VAC remain
saved. All112 complete442-word snapshots match the original CHANG2 exchange
and subsequent arithmetic, with both SUPERBNK forms.

The bounded-stop handler rejects continued execution without releasing the
running core/VAC. Host frozen-state tests check160 original fingerprints plus
invalid selectors, MODE and VAC ownership. The target D64 proof runs actual
numeric overlays under the resident mixed Executive. The bounded default
uses three source cases spanning both SUPERBNK forms and three VAC bases,
checking30 numerical117-word and42 mixed442-word original fingerprints with
both incoming enabled and masked IRQ states and restoring$01. Eight proof
cases are available with `--cases 8`; source
expectations remain separate from production resident code.
