---
id: legacy-065-correct-dsky-positive-zero-blanking-and-nout-increment-o
title: Correct DSKY positive-zero blanking and NOUT increment overflow
seq: 65
---

Original 2BLANK skips INCR NOUT for positive zero, and original INCR overflow-corrects register addition at037777. Generic Pinball and private P66 mode display now preserve both source behaviors. New independent full-image corpora cover4,096 Pinball and1,024 mode cases, with complete sanitizer streams and128 actualC64 source states passing. Prior published source corpus identities rerun unchanged; earlier constructors did not establish these edges. Source arithmetic and historical files remain unchanged outside the native corrections. Current dependent basic diagnostic also passes its original source images, task admission and real screen output after the fixes. See `docs/DSKY-ZERO-CORRECTIONS.md`.

