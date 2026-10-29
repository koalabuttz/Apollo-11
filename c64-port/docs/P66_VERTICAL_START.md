# Canonical vertical-guidance start

The native VRTSTART continuation calls actual STOPRATE, retains BANKCALL/SWRETURN scratch, clears XOVINFLG/REDFLAG, selects VERTGUID and P66VERT through POSTJUMP, terminates phase3 and performs TWIDDLE100cs RODTASK admission. It returns at immediate RODCOMP or the original01203 bailout. Every source alias and physical bank/IRQ effect is retained in the canonical image.

All1,024 complete original-entry2,050-word states agree, including timer/interval branches, signed zeros, raw flags and scratch values. Snapshot SHA256 `eb1525d25e262e037aa2b5812173a972eff307e588d72cbdf08954fae55f0b11`. A private byte-identical historical-source assembly retains whole-rope identity. Unsupported entry state rejects unchanged; source table-exhaustion paths retain the original partial mutations and bailout.

Strict and sanitizer64 whole-image tests pass. The C64 checks every2,048 erasable word plus SUPER/IRQ for all64 states, mailbox `a500004002`, PRG24,963bytes, text14,290, rodata10,659, BSS8,197, static78bytes, additional zero page0, hardware/software stacks18/237bytes. The queued RODTASK later admits its job without self-rescheduling. Complete STARTP66 and the actual display continuation have separate requirements.
