# Native ENTER pass selection and higher-pass admission

`dsky_enter` / `cm_enter` start at actual CHARIN with key034, or directly at
ENTER41,2002, for Luminary099 and Comanche055. CHARIN executes original lock/
key-release lamp setup and POSTJUMP into ENTER. Both paths clear CLPASS, set
actual ENDOFJOB in ENTRET, and select the original pass using signed CCS.

Nonnegative REQRET selects actual ENTPAS0 at41,2035. Negative REQRET, including
-0, tests the actual MMCHANG+1 request special case and low decimal mode bits.
Octal input permits any digit count. Decimal input with a positive count or
+0 reaches actual GODSPALM41,2351; negative count or -0 is accepted. The major
mode request skips that decimal count test as original. Accepted higher passes
complement REQRET, execute real FLASHOFF/channel11 output and reach physical
REQRET1013 with original Q2033, before the stored request executes.

These are real source transfer boundaries, not job completion. The stored
request, alarm, noun lookup and verb fanout remain separate continuations.
REQRET=-0 is supported rather than assumed illegal: accepting it produces
stored +0. Subsequent execution reaches source register A via TC0 and needs
explicit application closure; the current service stops before fetching that
request. No opcode fetch/decoder or fake callback is present in native code.

The authoritative state is the canonical2048-word image plus explicit SUPER,
IRQ and channel11 latch/output-event metadata. Bank40/41, SUPER100, all retained
EB0..7 and IRQ0/1 are qualified. Unsupported input preserves all image/IO and
route. Caller owns restoring producer exclusion.

The independent reference rebuilds unchanged original Pinball, T4, Interpreter,
Executive, Display Interface and Fresh Start sources in each program. CM
indentation repair is recorded; each whole rope must equal its pinned fixture.
8192 complete2053-field original/native/ASan/UBSan states compare every word
and output metadata, covering both entry paths, all EB, raw L, request/count
signed zeros, each decimal mode, occupied/empty CADRSTOR, and the major-mode
special case. Exact full-stream SHA256:
`02242eeda0147e7c57ece40e4793d2f4c2bacbf6a0a608b46a1683014f9dcdc7`.

The frozen64 complete original states run in strict host, sanitizer and native
C64 diagnostics. `tools/dsky_enter.mk` supplies portable host all/test/sanitize
aliases and an explicit C64 target. Stock-ROM VICE checks actual mailbox,
code/data RAM limits, zero-page budget and both stack sentinel regions.
This extends operator control coverage without certifying the remaining full
Pinball fanout.
