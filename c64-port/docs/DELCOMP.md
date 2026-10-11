# Original DELCOMP matrix construction

`delcomp_native` directly compiles DELCOMP in original Luminary099
`ATTITUDE_MANEUVER_ROUTINE.agc` lines828–920 and Comanche055 `ANGLFIND.agc`
lines443–535. Both byte-original files assemble without repairs into the same
original ropes used by the fixed-data/layout qualification. This milestone
qualifies DELCOMP, not the other routines in either containing file.

The source's sine/cosine, double square, scalar products, additions, shifts,
pushdown transactions, BOVB/SIGNMPAC saturation, nine matrix stores and RVQ
return are explicit native C calls and arithmetic. The source has no loop or
conditional matrix paths except overflow handling. Existing native polynomial
and arithmetic kernels are composed here and independently checked against
the original complete routine. No instruction decoder, stored instruction
execution, or GPL AGC engine is linked into native host or C64 targets. The
GPL engine is used only by the separate host source oracle.

The caller owns one authoritative 2048-word erasable image and excludes
producers until return. Numerical structs are transient views, and every
source scratch alias is committed to the same image. Hardware bank selectors
and interpreter metadata must describe the original DELCOMP entry; MODE is
DP+0, FIXLOC is one of the five original VAC bases, ordinary erasable words
are fifteen bits, and A/L/Q may contain raw sixteen-bit overflow. QPRET must
be a positive original high-half fixed continuation. The external continuation
body does not execute. Unsupported checked contexts return zero without
changing the image or latch object.

The source creates LM KEL or CM DEL from the angle in MPAC and COF. It leaves
three DP pushdown values at the selected VAC base and preserves every unowned
word. The source's observable BOVB bank-call return pair survives in BUF2 when
SIGNMPAC actually runs: LM02453/02000 or CM02500/02000. Positive-zero OVFIND
bypasses this call; negative zero reaches SIGNMPAC without replacing OVFIND.
Final RVQ decode leaves CYR7, and the preceding short SL2 write leaves editing
register SR0. Original CM polynomial return03572 differs from LM03573.

Run `python3 tools/delcomp_host_test.py` after the original fixed-data and
gimbal-source fixtures are available. The input-only constructors enumerate
10240 states: sixteen angles, sixteen raw COF patterns, four overflow flags,
five VAC bases and both missions, with all eight EBANK selectors, all eight
superbank latch forms and both source interrupt latch states. Inputs include both
signed zeros, independent DP signs, overflow-sized and ordinary normalized
axis directions, two actual fixed return banks and varying unowned words.
Every source/native/sanitized 2048-word image and both external latches agree
exactly. Separate negative tests check complete preservation for twenty-five
unsupported contexts.

`tools/freeze_delcomp.py` retains 320 independently derived target states as
lossless word streams. The actual C64 diagnostic compares all 2048 words;
the 52 potentially modified words use source expectations and the remaining
1996 use their constructed input values. The target reads every decoded word,
including both external latches; there is no expected-state hash comparison.
The diagnostic excludes real C64 IRQs and reveals RAM beneath BASIC ROM for
its larger proof fixture. Native execution begins from a stock-ROM PAL C64.

Target command:

```
python3 tools/delcomp_vice_test.py \
  --cc /workspace/toolchains/llvm-mos/bin/mos-c64-clang \
  --vice /workspace/toolchains/vice/usr/bin/x64sc \
  --rom-dir /workspace/toolchains/roms/C64 \
  --data-dir /workspace/toolchains/vice/usr/share/vice
```

Retained `evidence/delcomp-*` includes both full compressed canonical streams,
the source line/assembled-word audit, original assembly listings, host and
target logs, actual PRG/map and an exact SHA256 manifest. Full downstream
gimbal execution is a separate composed milestone; this packet does not claim
that either entire attitude file or complete application is translated.
