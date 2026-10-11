# Native ENTER pass-zero noun admission

`dsky_enter_zero` / `cm_enter_zero` execute actual ENTPAS041,2035, with all
retained EB banks and SUPER100/120/140/160. They clear DECBRNCH, block new
numeric input with DSPCOUNT=-19, save the original complemented verb and
perform the real low-verb noun test. VERBREG0..99 and -0 are accepted.
NOUNREG0..99 or -0 is required only when the actual verb selects a noun lookup;
the high-verb path leaves unrelated noun words untouched.

The source DXCH-Z call composes the immutable, independently qualified
`apollo_noun_reader` for the actual mission, preserving exact saved return
pairs and all noun-table scratch. Mixed nouns reach actual MIXNOUN41,2221.
Unused normal nouns reach GODSPALM41,2351. Machine-address requests reach
REQADD41,2064. Positive ordinary noun addresses execute actual AD ONE /
SETNCADR stores and bank selection, reaching VERBFAN41,2133. Original -0 table
addresses increment NOUNCADR with correct register overflow behavior, execute
SETNADD and reach INTMCTBS+2 at41,2120. These are actual source continuations,
not fabricated job completion or substituted display bodies.

The canonical physical2048-word image remains authoritative; no duplicate noun
state or source opcode decoder exists. The shared noun-reader/data files are
unchanged. Caller owns restoring producer exclusion, including the native
reader/data payload lifetime. Unsupported input preserves image, external
SUPER/interrupt metadata and selected route.

Independent references rebuild each whole rope after restoring unchanged
Pinball, noun tables, T4, Interpreter, Executive, Display Interface and Fresh
Start sources. Only recorded CM indentation repair is allowed; rebuilt ropes
must equal the separately pinned fixtures.8192 full2051-field original,
native and ASan/UBSan checkpoints cover every noun0..99 and -0, all normal/
mixed/unused/machine-address branches, low/high/negative-zero verbs, all
EB/SUPER and raw L cases, unused invalid noun words on high-verb paths, and
NOUNCADR signed-zero/overflow edges. Full stream SHA256:
`c9472a1b1d05c358ee8601dd5a98eab5ad3c945e61a89048477b6caaf2bd656e`.

Sixty-four frozen complete original states run in strict host, sanitizer and
native C64 diagnostics. `tools/dsky_enter_zero.mk` supplies portable host
all/test/sanitize aliases and an explicit C64 target. Stock-ROM VICE checks
actual mailbox, RAM-image bound, zero-page allocation and hardware/software
stack sentinels. Mixed-noun data exchange, machine-address acquisition and
remaining actual verb application bodies still require their continuations;
this milestone does not certify the whole Pinball application.
