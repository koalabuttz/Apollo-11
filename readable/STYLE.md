# Readable C style guide

`readable/` is the study edition of the Apollo 11 flight software: the same
program as the faithful port in `c64-port/`, written so a reader can follow
it routine by routine. A reader should be able to put three things side by
side and walk across them:

1. the original listing (`Luminary099/*.agc`, `Comanche055/*.agc`),
2. the faithful C (`c64-port/src/`, one call per AGC instruction),
3. the readable C (`readable/src/`).

Everything below serves that walk. Where a rule here and readability
disagree, keep the rule and explain in a comment; where the listing's
names and C's habits disagree, the listing's names win.

## Language

- **C99**, as in `c64-port/`. Built with
  `-std=c99 -O2 -Wall -Wextra -Werror -pedantic`.
  (`-Wconversion` is off: AGC arithmetic is 15- and 16-bit work in
  `uint16_t`, and the casts it asks for would bury the logic.)
- **x86_64 host only.** The C64 stays the faithful port's target.
- **Formatting:** the port's `.clang-format` (LLVM base, 4-space indent,
  100 columns, function braces on their own line). `make format` in
  `readable/` applies it.
- No `goto`. No dynamic memory. No compiler extensions.
- Only standard headers and `c64-port/include` (for the program interface
  in `src/binding/`).

## Fidelity

A readable routine leaves **every word of memory, every register and every
I/O channel** as the AGC's routine does, including what the listing
calls *debris* (`DEBRIS -- CENTRALS A,Q,L`). That is what lets the
readable code run in the real flight program and be checked word for word
against the faithful port (gates 1 and 2) and against Virtual AGC running
the original rope (gate 3). The few deliberate departures, by either
port, are listed in one place: README.md, *Departures*. A new one goes
there too.

- When a value only survives as debris, set it where the listing does and
  say so (`/* A,L get NEWLOC's old words */`).
- Some outcomes cannot happen in a working program: a CCS on a word that is
  never +0, a branch onto a constant (`TC CCSHOLE`, `DEC 7` run as `TC 7`).
  Follow them anyway, as the AGC would, in as few lines as possible, with
  a comment that says never and what the AGC would do there
  (`/* (never: priorities are never +0) The CCS lands on NO.CORES ... */`).
  They are part of the program a reader is studying.
- **`not_modelled()`** is for the one kind of step a routine cannot
  follow: a step whose outcome the listing's lines do not say. An INDEX
  that would make an instruction the listing does not intend, a transfer
  through a computed address that is no place the routine goes, an index
  past its table's end, a channel the routine never uses. The call stops
  the routine and its binding refuses the state (README.md, *Departures*,
  lists these steps), and a comment at the call says what the AGC would
  do there. It is not for an outcome that never comes but that the
  listing's lines do say. Every outcome of a CCS is followed, whatever
  its word can hold: the words after the CCS are in the listing. (Where
  one is not, because the CCS's last words fall past the end of the
  bank's program, the call stands, and its comment says that the listing
  has no word there. It stands too where the word is not an instruction's
  line, and the comment says which: an interpretive word, or an address
  word the assembler makes, such as a CADR. A constant with a value, an
  OCT or a DEC, is not such a word: it runs as the instruction it is. And
  it stands in a helper that is one CCS at several places, since what the
  listing would run differs with the place, which the comment says. In
  each of these the comment also says that the outcome never comes and
  what rules it out: `never: the caller's MASK leaves no sign`. So does
  the comment of an outcome that is followed as far as such a call, as
  when a constant runs as a TC into the erasable memory.)
- Say what a thing is compared with. What a routine does is what the AGC
  does (`the AGC would run OCT 1104 as TC 1104`). What a test shows is
  that it matches the faithful port, or Virtual AGC in gate 3. Write
  "matches the faithful port", never "as the original".
- Three gates prove fidelity (see *Gates*). A change that fails any of them
  is not readable, it is wrong.

## Naming

Names match the original program wherever the original has one.

### AGC names

Every AGC symbol keeps its listing spelling, in upper case, and means in C
what it means in the listing:

| Listing | C | Meaning in C |
|---|---|---|
| erasable word `NEWJOB` | `NEWJOB` | the word itself (an lvalue) |
| erasable word used with offsets `BUF +1` | `BUF(1)` (and `BUF(0)`) | the word at that offset |
| core set word `PRIORITY` of core set k | `IN_CORE_SET(PRIORITY, k)` | INDEX k on the name |
| constant `EXECBANK` | `EXECBANK` | its value |
| code label `TASKOVER` | `TASKOVER` | its address (for `continue_at`, `tc_at`) |
| central registers `A`, `L`, `Q`, `Z` | `A`, `L`, `Q`, `Z` | the registers (lvalues) |
| channel `CHAN12`, `DSALMOUT`, `DNTM1` | `CHAN12`, `DSALMOUT`, `DNTM1` | its number (for `read_channel()`, `wor_channel()` and the rest) |
| bank registers `FBANK`, `EBANK`, `BBANK` | same | read; write with `set_FBANK()` etc. |

`ADDRESS_OF(NEWJOB)` is an erasable word's address, for INDEX and for the
instruction helpers.

A channel is written by its name, never by a bare number: the name the
listing uses at the line, or, where the listing writes the number, the
name ERASABLE ASSIGNMENTS gives that channel (`RAND 31` is
`rand_channel(CHAN31)`). A channel the listing never names keeps its
number, in octal, with a comment that says what it is.

C cannot spell every AGC name, so:

| Listing has | C uses | Example |
|---|---|---|
| `.` or `/` | `_` | `NO.CORES` → `NO_CORES`, `NEG1/2` → `NEG1_2` |
| a leading `-` | `MINUS_` | `-CCSPR` → `MINUS_CCSPR` |
| `-` inside a name | `_` | `TIG-30` → `TIG_30`, `tig_30()` |
| a trailing `-` | `_MINUS` | `WATLST0-` → `WATLST0_MINUS` |
| a leading digit | the digit spelled out | `2K` → `TWO_K` |
| `+` inside a name | `_PLUS_` | `Q+10000` → `Q_PLUS_10000`, `q_plus_10000()` |
| a leading `/` | `SLASH_` | `/NORM` → `SLASH_NORM` |
| a leading `*` | `STAR_` | `*SMNB*` → `STAR_SMNB_STAR` |
| a trailing `?` | `_Q` | `STABL?` → `STABL_Q`, `stabl_q()` |
| a leading `?` | `Q_` | `?GUIDSUB` → `Q_GUIDSUB`, `q_guidsub_plus1()` |
| `*` (the indexed forms) | `_STAR` | `DLOAD*` → `DLOAD_STAR`, `dodload_star()` |
| `*` inside a name | `_STAR_` | `AX*SR*T` → `AX_STAR_SR_STAR_T`, `ax_star_sr_star_t()` |
| `=` | `_EQ_` | `ACOS=0` → `ACOS_EQ_0` |
| `(` ... `)` | `_` and nothing | `LAT(SPL)` → `LAT_SPL` |
| `,` or `&` | `_` | `BITS4&6` → `BITS4_6` |
| a number with its base after it | the base first | `60DEC` → `DEC_60`, `77001OCT` → `OCT_77001` |
| a name that is its value | what the value is | `9,6,4` → `BITS_9_6_4`, `.5SEC` → `HALF_SEC` |
| a C keyword or library name | a trailing `_` | `GOTO` → `goto_()`, `ABS` → `abs_()` |

Inside the operand macros (`ADDRESS_OF`, `ADDRESS`, `ADDRESS_PLUS_ONE`,
`STORE` and the rest), which paste the name onto a prefix, a leading
digit stays as it is: `1/PIPADT` → `ADDRESS_OF(1_PIPADT)`, `4SEC(28)` →
`ADDRESS_PLUS_ONE(4SEC_28)`.

An erasable word named as a downlist or noun table operand, as its ECADR
(E-bank bits and address in the bank, the way a DNADR holds it), is
`E_<name>`: `E_R_OTHER` for `R-OTHER`, `E_MINUS_TPER` for `-TPER`.

Names come from the listing: a new AGC name goes in
`tools/listing_names.py`, which writes its address or value from the
assembled listings into `src/agc/<m>_names.inc`, and `src/agc/names.h`
gives it its C spelling. No address or constant is typed in by hand.

### Routines

- A routine's C function is its label in lower case: `NOVAC` → `novac()`,
  `T3RUPT2` → `t3rupt2()`.
- An entry the listing names by an offset from a label is the label plus
  the offset: `CHANJOB -2` → `chanjob_minus2()`, `DUMMYJOB +2` →
  `dummyjob_plus2()`.
- An offset is a number of the listing's, and the listing counts in octal
  unless it writes a D: `CG +14` is the word 014 after CG, `LST2 +16D`
  the word 16 after LST2. An offset the listing does not write (the
  address of a word some lines below a label) is counted the same way,
  in octal with its leading 0 (`POODOO + 014u`). A function's name takes
  the digits as they are written in its header, which says where that is.
- When one C function covers several labels (a loop through `NOVAC3` and
  `NEXTCORE`), it takes the first label's name and its comment lists the
  others.
- A routine the original reaches only as a continuation of another (the
  C64 port's tables key it separately) gets a name saying what it follows:
  `svct3_after_novac()`.

### Everything else

Names that are ours, not the AGC's (the machine model, helpers), are
`lower_snake_case` and say what they do: `sign_extend()`,
`exchange_core_set_pair()`. Types are `word` (a 15-bit AGC word), `reg16`
(a 16-bit register) and `enum sign` (the four outcomes of CCS). Macros that
are not AGC names are `UPPER_SNAKE_CASE`.

### Missions

One source serves both programs. The mission is chosen at build time with
`LUMINARY099=1` (LM) or `COMANCHE055=1` (CM), and where the two listings
differ the code says so in place, in `#if LUMINARY099` / `#else` blocks,
so a reader sees both versions together.

## Comments and listing references

- **File header:** the `.agc` file, its pages in each mission's listing, and
  the faithful C it corresponds to.
- **Function header:** the first line names the routine(s) and the listing
  pages, LM first:

  ```c
  /* NOVAC -- LM p. 1103, CM p. 1208.
   * Start a job that needs no VAC area (a basic job): ... */
  ```

  Pages are the `# Page NNNN` markers of the transcribed listings. A routine
  from another file names it: `INTRSM -- INTERPRETER.agc, LM p. 1002, CM
  p. 1107.`

  The page is the page of the word the function stands at, which is the
  place its key names in the tables of places. A function that begins some
  words into a routine names that word and its page (`CKSTALL +7 -- LM
  p. 208`), and one keyed at a jump says so (`EXBRAK +5 -- p. 812, after
  the EXIT (TCF STEER?)`). A file of one mission writes `p. 812` alone. A
  function that covers several pages gives them all (`pp. 1332-1333`). A
  helper that is an idiom of the listing written once, at no one place, has
  no page.

  `make style-check` holds every header to this (tools/header_pages.py):
  it reads each word's page from the assembled listing and fails on a
  header without the page of its place, one that names another place than
  its key, or an offset in decimal without a D. It also compares the page
  of every label in the assembled transcription with this repository's
  marks. They agree but for page 531 of Comanche, which
  `Comanche055/STABLE_ORBIT.agc` has no mark for: PREC/TT and INTRPVP are
  on page 531.
- Then say **what it does, in plain words**, before how.
- Quote the listing's own comments in double quotes
  (`"negative LOC shows job = interpretive"`).
- Where the C follows an AGC idiom, show the idiom once, as the listing has
  it (the four-word `CCS ... TCF` tables, `TS` skipping on overflow), and
  say how the C maps it.
- Comments are short sentences, no markers like `TODO` in merged code.

## Structure

- The machine is global, as the AGC's memory was: `agc` (`src/agc/agc.h`),
  pointed at the flight program's state by `src/binding/`. Routines take
  only what the listing passes inline (the 2CADR after `TC NOVAC`).
- A routine **leaves** by setting Z: `continue_at(address)`, `dtcb(at)`, or a
  call to the routine the listing goes on to. Nothing is returned.
- A `TCF` to another routine is a call to it, as the last statement. A
  branch back is a loop. `continue_at(LABEL)` is for a routine of another
  file (the binding goes on there), and for a place of this one where
  the faithful port ends a step: the flight program charges each step its
  machine time and takes interrupts between steps, so the readable C
  ends its steps where the faithful port does. Such a place is a
  function named for it, so the jump can be followed by name:
  `continue_at(GENTRAN + 1u)` goes on at `gentran_plus1()`. (An outcome
  that never happens may end at its place without one. Its comment says
  never, and why.)
- **CCS** is `switch (ccs(word))` with its four cases in the listing's order
  (`POSITIVE`, `IS_PLUS_ZERO`, `NEGATIVE`, `IS_MINUS_ZERO`), each falling
  through with a `/* fall through */` comment where the listing does. All
  four are named: the listing has a word for each, so no `default:` stands
  for two of them.
- Register effects that carry state from one step to the next use the
  instruction helpers (`dxch()`, `xch()`, `ts()`), named as the instruction.
  Effects that do not are plain C on named words.
- **Interpretive code** is written in the listing's notation, not
  translated: a table of words built with the macros of
  `src/interpreter/notation.h`, one line of the listing per entry, with
  its labels and comments (`ORDERS(SET, BOF)`, `SWITCH(SET, VINTFLAG)`,
  `ADDRESS(DOINT)`), run by the readable interpreter as the AGC's
  interpreter ran the listing's words. A gate checks the words against the
  rope.
- A helper that more than one file needs is defined once: `COUNT()` and
  `tc_to()` in `src/agc/agc.h`, the interpretive notation in
  `src/interpreter/notation.h` (with `BANK_OF()` and `IN_BANK()`, a
  label's bank and its address there). A file does not carry its own
  copy. (The tables of `src/assembly`, which do not use the machine, have
  `COUNT()` from their own header.)
- One subsystem per directory under `src/`, named after the listing's log
  section: `EXECUTIVE.agc` → `src/executive/`. `src/agc/` is the machine
  model; `src/binding/` connects the routines to the flight program and
  holds nothing of the AGC's.

## Gates

Every change to `readable/` must keep all three. README.md, *Gates*,
describes each in full, and `.github/workflows/readable-ci.yml` runs them.

1. **The case tests** (`make -j4 all`, about half an hour). The Makefile's
   `all` is the list:
   - `names-check`: the listing names (`src/agc/<m>_names.inc`) are the
     assembled listings' (`tools/listing_names.py`);
   - `coverage`: which listing files are readable C
     (`tools/readable_coverage.py`);
   - `code-extents`: every table of interpretive code begins and ends
     where the listing's code or constants do (`tools/code_extents.py`);
   - `x86-link-check` and `chan13-sites`: two checks the x86 gates rest
     on, each tried on what it must fail;
   - `style-check`: the rules of this guide that a script can count
     (`tools/style_check.py`: the order of a CCS's cases, a `default:`
     among them, a CCS outcome refused, a channel by number, a jump to a
     label plus an offset, a helper defined twice, a decimal offset, a
     long line, `goto`, the wording). `tests/style_owed.txt` lists what each directory of `src/`
     still owes of the whole-tree review (issue #1334); a new break
     anywhere fails, and the list only gets shorter. `tools/ccs_helpers.py`
     holds every call of a helper that is one CCS at several places to
     the helper's premise, a mask whose sign bit is clear: the C compiler
     checks a static assert of each mask. What a script cannot
     count (names, comments, whether the C says what the listing does) a
     person reads;
   - the `*-cases` targets, over a hundred, one or more for each listing
     file (`kernel-cases`, `interbank-cases`, ... `assembly-cases`). Each
     is of one of two kinds, or both. The first runs the faithful port's
     own test of the file (its frozen cases, each matching the original
     program's yaAGC digest) on the readable routines. The second runs
     the readable routines and the faithful ones side by side on the
     same random machine states, at every key: the same result, memory
     and channels. A subsystem without a corpus of its own gets a test
     of the second kind.
2. **The x86 exact scenario suite** (`make x86-scenarios-quick` on PRs,
   `make x86-scenarios` for all 86 rows): the x86_64 flight program built
   from the readable routines, with none of the faithful ones linked
   beside them, flies each scenario against the harness, which runs the
   faithful port, and must end with the same erasable memory, word for
   word (words 061-03777 at the row's end). A readable PR may not lower
   the exact count.
3. **The x86 program against Virtual AGC** (`make x86-lockstep-quick` on
   PRs, `make x86-lockstep` for all 117 rows): after every routine the
   program runs, its erasable memory, channels and interrupt state must
   agree with yaAGC's, run on the original rope from where the routine
   started. Each row's counts are held to the run of record
   (`tests/x86_lockstep_record.txt`).

## The faithful port

`c64-port/` is the frozen reference. A readable change does not edit it,
except to fix a bug the readable work finds there, in its own commit, with
a scenario or case that shows the bug.
