# Source trigonometry and live CDU counts

The native kernels translate CDULOGIC, SIN, COS and CDUTRIGS using the original
Hastings double-precision coefficients and word arithmetic. No floating-point
approximation or precomputed flight trajectory is used. SIN/COS return the
historical half-amplitude result with signed-zero, partial-product, polynomial,
scratch and caller-return effects preserved.

CDULOGIC converts each 15-bit two's-complement CDU count into a double-precision
one's-complement revolution fraction. CDUTRIGS consumes stable X/Y/Z hardware
words and writes angle, sine and cosine arrays in original Y/Z/X order. Its
basic caller Q and bank metadata are explicit logical inputs; EXIT writes the
caller bank word into VBUF4. Unsupported MODE/Q/bank inputs reject unchanged.
Callers own scratch and exclude producers. TEM2/3 share storage with rotation;
BUF2, POLISH, CDUSPOT, SINCDU and COSCDU add 46 modeled bytes.

All 65,536 complete 130-word original-assembly states agree: all 32,768 CDU
encodings and 8,192 each SIN, COS and CDUTRIGS, including cardinal/raw corners,
random triples and all legal caller EBANK bits. This oracle uses the unchanged
pinned engine; these routines execute no DV. Snapshot SHA:
`7642493176a67ac961fd52ba0d6b74007998f1f63739428afd4538d6f57c0743`.
Strict host rejection checks and the complete ASan/UBSan corpus pass.

PAL C64 checks all 128 losslessly frozen complete states; mailbox
`a50000800003048224`. RAM ends at $bce3, extra zero page is zero, and measured
hardware/software stack extents are 37/541 bytes. Finite PAL cycle samples are
1,953 for CDULOGIC, 28,637 for SIN, 42,095 for COS, and 217,621/326,404 for
cardinal/random CDUTRIGS (about 331 ms for the random sample).

These kernels prove numerical stages. The P66 basic USPRCADR wrapper supplies
pending EXIT in EDOP, so its internal trig returns bypass Executive NEWJOB;
its real outer completed-pair scheduling boundaries are handled by the native
P66 continuation driver. General interpreted callers still need their own
source control bindings. `tools/trig.mk` supplies builds and checks; differential,
freezer and VICE tools reproduce the retained full-state evidence.
