# Build targets

Decided by David on 2026-10-13.

| Target | Basic code | Interpretive code | When |
| --- | --- | --- | --- |
| C64 (llvm-mos, 2 MB REU) | Native C | A native C port of INTERPRETER.agc runs the interpretive words from the rope image in the REU. Compiled routines stay on the C64 only for hot spots chosen by profiling. | Now (Interpreter lane) |
| Native x86_64 | Native C | Fully native: every interpretive routine is compiled to C by `tools/interp_compile.py`. A second build runs the same native interpreter core as the C64, for comparison. | Running now as a second track (`make x86-flight`, docs/X86-FLIGHT.md); it must not slow the C64 completion gate |

Why the C64 interprets: compiled interpretive code is about 50 B per AGC word, against 2 B in the rope. CONIC_SUBROUTINES alone compiles to 108.6 KB, while a C64 module slot is 16 KB and the REU store is 1.75 MB.

Keep `interp_compile.py` and the compiled routines maintained. They are the C64 interpreter's host oracle and the x86_64 build's code.
