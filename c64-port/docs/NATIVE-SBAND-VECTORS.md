# Native S-band vector dependencies

`src/sband_vector.c` translates the original VXV and VPROJ bodies needed by
both antenna programs, including their DMPSUB, PREDOT/DOTSUB, VROUND and overflow
controls. Each explicit entry returns at the mission's pending DANZIG instruction,
with all canonical scratch, raw A/L, Q, bank registers and vector MODE effects
preserved. It does not decode or dispatch AGC opcodes. Neither S-band source file
nor the entire INTERPRETER is certified complete by this dependency milestone.

The image is the sole authoritative 2048-word erasable store. The owner excludes
producers through the entire call; there is no scheduling point within these
source arithmetic bodies. A/L retain raw sixteen-bit encodings. Q and the other
image words must be ordinary fifteen-bit values; ZERO is zero and EB/FB/BB are
coherent. Entry PC and MODE=-1 are checked. Invalid supported-domain inputs leave
all image words unchanged. SUPER and IRQ remain external and untouched.

Erasable arguments occupy six ordinary data words. ADDRWD is 0030..1772;
banked erasable storage resolves through EB, with physical first word>=0030.
Minor operands are read before the partial products and major operands after
the original MPTEMP/MPAC stores. This preserves live scratch aliases; taking a
snapshot of both factor words would fail those original cases. The constructor
exercises VBUF, BUF, MPTEMP, MPAC and banked storage aliases.

The additive `_fixed` entries accept a separate immutable six-word array bound
to the actual source ADDRWD..+5. Valid descriptors are2000..3772 or4000..7772;
a six-word argument cannot cross a bank boundary. The caller supplies the real
FB/SUPER-selected data and guarantees that the array does not alias mutable RAM.
The API never copies those words into authoritative erasable storage. The first
256 cases of each fixed group use the original HIUNITX/Y/Z constants and their
real selected bank. Subsequent cases inject six independently constructed
ordinary argument words into that passive fixed-data region in the host oracle;
original interpreter instruction and scalar-constant words remain unchanged.
The byte-original baseline rope and the arbitrary argument injection are recorded
separately, rather than calling a modified argument set the original full rope.

Both source assemblies retain the original production layouts: Luminary1f5326e0
and Comanche6e3150ae. The original source/assembled-word maps cover164 words per
mission and separately verify scalar constants and all three axis-vector data
arrays. The source-PC trace visits163 of those164 words. The no-final-carry
OVERFLOW `TC Q` exit is explicitly compiled and independently inspected, but this
cohort does not execute it; no exhaustive branch or state-product claim is made.
The target frontier excludes execution of DANZIG itself.

The full differential proof compares262144 complete 2048-word images plus both
external latches, and the complete ASAN/UBSAN replay matches every record. It
includes signed-zero and overflow corners, raw A/L, all EB choices, varied FB,
all eight external SUPER forms, both IRQ values, live erasable aliases, original
fixed axis constants and varied fixed arguments. These are broad independent
source samples, not an exhaustive enumeration of all mathematical input tuples.
The input-only host adapter links the original GPL engine; neither engine code
nor that adapter is linked into native target code.

The stock-ROM PAL C64 diagnostic compares384 frozen complete images word by
word using lossless expected deltas, with unmodified constructor words retained.
It needs29907 PRG bytes, loads through$7cd2 and ends BSS at$9cd2, below$c000.
Its mailbox is`a5800100`. This standalone two-image diagnostic establishes target
arithmetic and comparison; its allocation is not a whole-application resident
memory budget.

Reproduce from the original qualified layout fixtures:

```
python3 tools/sband_source.py
make -f tools/sband_vector.mk test sanitize freeze check-host
python3 tools/sband_vector_source.py
python3 tools/sband_vector_vice_test.py --cc /workspace/toolchains/llvm-mos/bin/mos-c64-clang --vice /workspace/toolchains/vice/usr/bin/x64sc --rom-dir /workspace/toolchains/roms/C64 --data-dir /workspace/toolchains/vice/usr/share/vice
```

Original orbital/display callees and antenna interpreter caller/return controls
remain the next S-band composition work. Existing frozen UNIT, MXV, inverse
trigonometric and scalar trigonometric modules supply the other math bodies;
this packet makes no claim that they are already composed into the antenna jobs.
