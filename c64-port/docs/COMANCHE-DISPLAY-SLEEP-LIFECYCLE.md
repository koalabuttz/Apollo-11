# Comanche display sleep lifecycle composition

The original normal-display GOSLEEPS body now composes continuously with
canonical JOBWAKE, XCHTOEND/XCHSLEEP, and canonical JOBSLEEP through actual
basic DTCB, interpreted INTRSM, or idle ADVAN. Each helper commits the sole
shared image before the next source body; no state normalization, private
continuation, synthetic scheduling check or source instruction decoding is
introduced. The production APIs remain the separately qualified
`cm_makeplay_sleep_*`, `cm_basic_wake`, and `cm_basic_sleep` services.

The source corpus starts at actual bank10/03176 and executes512 continuous
inputs, observing four checkpoints each. All2,048 complete2,051-word states
match, including all EB/SUPER values, found/missing sleeping jobs, pending
priority bookkeeping, retained dormant contexts, and basic/interpreted/idle
selection. Strict and ASAN/UBSAN comparisons remain byte-identical. Source
stream SHA256:
`13205f47100ee40271ff6f8c116966ad73e02d275c6f4b282da4a0e5babbbada`.

The PAL C64 diagnostic replays64 complete original checkpoints across16 full
paths and compares every word plus external metadata against original delta
fixtures. Mailbox `a500004000`; PRG18800 bytes, hardware stack18 bytes excluding
14 ROM bytes, software stack737 bytes, no additional zero-page allocation.
Exact packet identities are in `evidence/cm-display-sleep-lifecycle-proof.json`.
Recipes are in `tools/cm_display_sleep_lifecycle.mk`. Source/native/sanitizer
cohorts stream directly to deterministic XZ; only the original reference is
copied into evidence. This composition stops at source dispatch and does not
execute the selected job body, astronaut response, or subsequent GOPLAY.
