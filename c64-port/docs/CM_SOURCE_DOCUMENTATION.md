Two original CM source files require documentation preservation rather than executable translation:

- `Comanche055/CONTRACT_AND_APPROVALS.agc` retains the original project contract, submission and approval notices.
- `Comanche055/LUNAR_LANDMARK_SELECTION_FOR_CM.agc` retains page936 and the original end-of-section comment.

Every line in both files is a full-line comment or blank/whitespace. There are no instructions, constants, declarations, entry points or runtime effects. The original bytes are preserved in the corresponding `evidence/documentation-source-*.agc.original` artifacts. `evidence/documentation-source-audit.json` maps every source line to retained documentation and records each original SHA256.

`tools/documentation_source_audit.py` independently checks this lexical domain without stripping trailing text after executable source, then copies both byte-original files into an isolated original-layout CM fixture and assembles the complete rope afresh. Its output is byte-identical to the baseline original-layout rope (`6e3150ae9b43535237e10f565205d1f65e6d60cafbe8935be598e03e9810b061`). This proves that preserving these source comments introduces no code/data effect; it does not qualify any instruction in other source files.

The two certificate recommendations explicitly count zero translated algorithms and zero emitted code/data words. No C no-op, runtime placeholder, hardware proof or additional target payload is appropriate for documentation-only content. Parent inventory merging should preserve that distinction from whole-file algorithm translation.

Reproduce after preparing the original-layout fixed-data fixture:

```sh
python3 tools/documentation_source_audit.py
```
