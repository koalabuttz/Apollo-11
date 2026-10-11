# Complete original gimbal calculations

`gimbal_complete` composes all native execution and data in both original
`GIMBAL_LOCK_AVOIDANCE.agc` files. LM enters through NOGIMLOC or WCALC; CM
enters through WCALC. Both leave the interpreter at the original SETGO target
NEWANGL+1, before that external continuation body executes. LOCSKIRT is the
original compile-time entry alias. All 30 LM and 45 CM noncomment source lines
are mapped, including 21/25 executable words, placement/alias declarations
and 10/30 immutable data words. This is a whole-file milestone, not a complete
application milestone.

The frozen entry and DELCOMP modules remain unchanged. The new composition
loads the selected ARATE pair, builds the complete incremental matrix through
DELCOMP, forms BRATE from COF, multiplies AM by ANGLTIME, divides by the actual
selected rate and shifts/stores TM. CM also applies QUADROT and constructs
BIASTEMP using BIASCALE. The source sets CALCMAN2 (STATE word 2, value 2),
restores its interrupt latch and branches through the exact original fixed
address. All register, scratch, pushdown and alias effects are retained:
Q holds the signextended previous STATE word; final A is raw octal 140000, L octal 010000,
CYR octal 020016, SR 0 and the source switch buffers are 2.

One 2048-word canonical erasable image is authoritative. Numerical context
structs are transient views. Entry checks and producer exclusion are the
same as `gimbal_entry`: original NEWOPS/LOC and bank 22, all eight consistent
erasable bank selectors, all five VAC/FIXLOC bases, ordinary 15-bit words,
raw 16-bit A/L/Q and original four selected rate indices 0/2/4/6. The eight
superbank latch forms 0/020/040/060 and 100/120/140/160 and either input source
interrupt latch are supported. Unsupported checked entries preserve the
entire image and latch object. The final source continuation is explicit;
this function does not decode or execute stored AGC instructions.

The CM matrix operation is the actual MXV arithmetic and source exchanges
with the fixed original QUADROT matrix. It uses verified native unrounded
DOT arithmetic and retains all TP and VBUF/BUF2 debris. The source calls
DOTSUB directly, so the native composition restores vector MODE and the
major accumulation in A before the following DXCH exchanges. QUADROT's
entries have magnitude at most 0.1, and the four selected CM ARATE values
at most 0.088889; their bounded products cannot trigger DOT's overflow return.
This proof does not qualify arbitrary caller matrices or VXM. The separate
generic matrix milestone owns that broader domain.

The independent source fixture overlays byte-original gimbal, attitude,
interpreter and RTB/RT8 source on the qualified original declaration/data
baseline. LM helpers require no normalization. CM INTERPRETER requires the
same 36 explicitly recorded indentation-only pseudo-label repairs as the
CM powered-flight qualification. No numerical source words change, and both
complete ropes remain byte-identical to the original-layout baseline:
LM `1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e`;
CM `6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`.
The original CM ALTVAR 01357 and R53FLBIT BIT8 allocation remain intact.
The host-only GPL engine is never linked into native targets.

Run `python3 tools/gimbal_complete_host_test.py`. Inputs only constructors
cover 10240 full states, both missions, sixteen COF patterns, sixteen raw AM
pairs, four OVFIND flags, five VAC bases, all eight EBANK selectors, all four
selected rates, all eight superbank forms, both input source interrupt latches,
LM's two entry forms, signed zeros, independent DP signs and overflow-sized
raw vectors. Source/native/ASAN/UBSAN states compare all 2048 words plus both
external latches. Twenty-five unsupported-entry probes check preservation.
The frozen generator also verifies 1969 unowned words in every source case.

Two actual C64 diagnostics each execute 80 independent frozen states, all
five VAC bases, all rates and the complete sixteen raw angle patterns;
together they cover both programs and read all 40 original file constants.
Every changed word is decoded losslessly and compared individually; all
unowned words compare against their input values. To save a second 4096-byte
image, the target reconstructs the input-only pattern and saves its eight
COF/AM exceptions. The generator checks this reconstruction against all
10240 constructor snapshots. The diagnostic excludes C64 IRQ producers and
reveals RAM beneath BASIC ROM before accessing its proof buffers.

Target command:

```
python3 tools/gimbal_complete_vice_test.py \
  --cc /workspace/toolchains/llvm-mos/bin/mos-c64-clang \
  --vice /workspace/toolchains/vice/usr/bin/x64sc \
  --rom-dir /workspace/toolchains/roms/C64 \
  --data-dir /workspace/toolchains/vice/usr/share/vice
```

`evidence/gimbal-complete-*` retains full compressed states, original source
listings, helper normalization patches and maps, host/target logs, actual PRGs
and linker maps, source location observations and exact SHA256 identities.
The diagnostics prove native C64 execution within their memory bounds; a
complete resident application and arithmetic overlay composition remain
separate application integration work.

This API requires excluded producers for the complete call. It composes the
original operation-pair boundaries with NEWJOB 0 and does not expose live
Executive yields between them. Whole-call arithmetic/control qualification
therefore does not assert live application scheduling qualification.
