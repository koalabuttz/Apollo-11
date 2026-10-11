# Native axis rotation

`p66_axis_rotate` implements AX*SR*T with the original raw DP multiply, scratch aliases and return-address rules. Both direction encodings (3 and 077774) retain the source operation order and TEM2/TEM3 trig aliases.

All 32,768 complete source cases match across 129 state words and two input fields (SHA256 `03d4e8b3c5d8419f039a08f32cef4211126723f55be2a640ab192696731e1a38`). Strict and all-state sanitizer comparisons pass; unsupported direction/return encodings reject without modifying state. The source qualification compares all 58 instruction/data rows after comment, SINSLOC spelling and explicit DEC B-14 transcription normalization.

The C64 proof compares all 128 frozen states, mailbox `a500008000`. PRG size is 45,655 bytes, image ends at $bb5c, additional zero-page usage is zero, and hardware/software stack extents are 23/570 bytes excluding the separate ROM frame. This verifies the bounded kernel; the live driver and whole-application coverage remain separate requirements.
