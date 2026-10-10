# Next bounded milestone

1. Translate **basic-job CHANJOB context exchange and dispatch**. Preserve
   LOC/BANKSET, eight MPAC/MODE words, PUSHLOC/PRIORITY, FIXLOC and OVFIND behavior.
   Use native callable continuations behind an explicit historical-2CADR mapping.
   Document the C calling/continuation adaptation and retain the original scanning
   algorithm. Do not introduce a generic ready queue.
2. Extend the reference fixtures to sequential admission/completion/yield paths,
   all priority combinations needed for basic dispatch, and retained stale state.
   Compare original state before/after CHANJOB while excluding hardware return
   registers only with explicit justification. Test native continuation order on
   host and C64. Keep interpreter dispatch separately unimplemented.
3. Establish platform-owned interrupt exclusion boundaries before accepting
   interrupt-origin job requests. Measure safe points, stack bounds, and worst-case
   admission/completion costs, not only the easiest idle admission.

Acceptance for that milestone: two native basic jobs with different priorities
are actually executed, yielded and completed; resource/state exchanges and
observed order match controlled original executions, with fresh C64 evidence.

Subsequent work: JOBSLEEP/JOBWAKE and PRIOCHNG; WAITLIST task scheduling; CIA
timer/IRQ adaptation; BAILOUT1 and protected restart groups; needed interpreter
and multiword numerical semantics; then one bounded landing-guidance computation.
Real-time flight feasibility is a separate measurement milestone. Rust remains
an optional later component translation.

Open technical questions:

- Which native continuation representation most faithfully preserves basic
  Q/LOC yields and the distinct interpreter resume path without becoming an AGC
  emulator?
- Which job/bank identities must survive as historical metadata, and which can
  be mapped to native function identifiers?
- How should IRQ handlers protect LLVM-MOS imaginary registers, and how will
  INHINT/RELINT ordering map onto C64 IRQ exclusion?
- What is the worst-case cost of scans/context swaps with realistic interrupts?
- What protected phase/restart groups are indispensable for one selected flight
  routine, and how will abort side effects be verified independently?
- Which fixed-point/multiword operations does the first guidance slice actually
  require, and can its code/data be overlaid within stock C64 memory?
- Should the upstream assembly transcription's build errors be repaired in a
  separate provenance-preserving project? This branch deliberately leaves them.
