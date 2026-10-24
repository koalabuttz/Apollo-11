# Build targets

Decided by David on 2026-10-13.

| Target | Basic code | Interpretive code | When |
| --- | --- | --- | --- |
| C64 (llvm-mos, 2 MB REU) | Native C | A native C port of INTERPRETER.agc runs the interpretive words from the rope image in the REU. Compiled routines stay on the C64 only for hot spots chosen by profiling. | Now (Interpreter lane) |
| Native x86_64 | Native C | Compiled: interpretive routines are compiled to C by `tools/interp_compile.py`, and the native interpreter core runs only the points no compiled table holds. A second build (`-core`) runs every interpretive point on the interpreter core, as on the C64. | Running now as a second track (`make x86-flight`, docs/X86-FLIGHT.md); it must not slow the C64 completion gate |

Why the C64 interprets: compiled interpretive code is about 50 B per AGC word, against 2 B in the rope. CONIC_SUBROUTINES alone compiles to 108.6 KB, while a C64 module slot is 16 KB and the REU store is 1.75 MB.

Keep `interp_compile.py` and the compiled routines maintained. They are the C64 interpreter's host oracle and the x86_64 build's code.
