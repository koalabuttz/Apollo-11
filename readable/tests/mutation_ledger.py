#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""The mutation ledger: the changes to the readable code that no test sees.

A mutant is one small change to a line of readable/src (an address moved by
one, a statement dropped, a test turned round). A test that is worth its
name fails on it. Every PR that wrote a readable file ran such changes and
named the ones its tests let pass, the survivors, each time with a reason.
tests/mutation_ledger.txt gathers them in one place, as they stand in
today's code: each entry is the change itself (the lines as they are, the
lines as changed), the routine it is in, a reason in brackets and one
sentence that says why no test can tell the two apart. A survivor that a
check or a case added since catches is kept too ([killed], with the check
or suite that catches it), so that it keeps being caught. A survivor with
no reason written yet is [owed]: the ledger says so in its counts.

    python3 tests/mutation_ledger.py            check the ledger against the tree
    python3 tests/mutation_ledger.py --write    write the ledger again in its order,
                                                with its counts
    python3 tests/mutation_ledger.py --run      make every change and run its tests
        [--jobs N] [--only PATH] [--kind KIND]  (those of one file; those of one reason;
        [--suite SUITE]                         those of the files one suite is made of)
    python3 tests/mutation_ledger.py --self-check

The check (`make all` runs it) holds each entry to the tree: its lines are
there, in the routine it names, and it has a reason of a known kind and a
why. It runs nothing. The run (`make mutation-ledger`, the nightly job)
makes each change in a copy of the tree under build/mutation, runs the
checks of `make all` that only read the source (tc-sites), builds the case
tests the changed file is part of and runs them: a survivor must build and
pass them all, and a [killed] entry must fail the check or suite it names.
The run fails on either side, so the ledger cannot keep a survivor that a
test now catches, nor a catch that a test has lost. A step that runs over
its time is neither, and fails the run. A catch by a suite is made again
alone before it counts (a survivor caught under load, or a [killed] entry's
catch, is held only if the suite fails again); nothing else is tried a
second time. build/mutation/report.txt has each result as it came, so a run
cut short says how far it got.

What it does not hold: that these are all the survivors. They are the
ones the PRs' own mutation runs found, where those runs could be made
again (the ledger's head says which could not). A change nobody made is
not here.
"""
import os
import re
import shutil
import signal
import subprocess
import sys
import threading

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
LEDGER = os.path.join(HERE, 'mutation_ledger.txt')
WORK = os.path.join(ROOT, 'build', 'mutation')
# The checks of `make all` that read the source and run nothing: every change to src/ is held to them
# too, before its case tests.
STATIC = ('tc-sites',)

# Where the ledger's changes come from: the head of the ledger says it, so that a reader knows what
# was looked for and what was not.
SOURCE = '''
Where these come from. The PRs that wrote readable/ made such changes by
script and named in their texts the ones their tests let pass. 282 PR
texts were read; 122 name survivors. 216 of the scripts were kept, and
they give 7,280 exact changes that still apply to this tree. The logs of
132 of the scripts were kept too: they name 1,216 of those changes as
survivors (read widely: a log that names a line stands for every change
its script made to the line). Each of the 1,216 was made again on this
tree. 556 are caught by a case test now and are not here, 17 no longer
build, 7 leave a test running past its time limit, and the other 636 are
the entries below, written as whole-line changes (two come to the same
one): the ones a check or a case added since catches as [killed], the
rest as survivors.

The other 84 scripts have no log, so every change of theirs that still
applies, 1,484 of them, was made on this tree and run: 111 are caught by
tc-sites, 1,305 by a case test, 17 no longer build, 1 leaves a test running
past its time limit, 16 are changes to the tests and not to the code, and
34 are entries below (one of them a case added with this ledger now
catches: [killed]).

Not here: any change no script made.
'''

HEAD = '''\
# The changes to the readable code that no test sees, and why.
# tests/mutation_ledger.py holds this file to the tree (`make all`) and
# makes each change and runs its tests (`make mutation-ledger`).
#
# An entry is one change: the file and the routine (or table) it is in, a
# reason in brackets, the lines as they are (-) and as changed (+), and why
# no test can tell the two apart. "(2)" after the routine's name is the
# second place those lines stand in it; "-" for the routine is a line
# outside any.
#
%(source)s
#
# %(counts)s
#
# A reason in brackets is one of these:
'''

CLASSES = {
    'rewritten': '''What the change touches is written again before anything reads it, on
    every path from the changed line: the return address a TC leaves in Q,
    which the next TC sets again; A or L, loaded again by the next
    instruction; the interrupt latch, which a later INHINT or RELINT (or
    INTPRET's own RELINT) sets before the pass ends, and a case test's pass
    takes no interrupt in between. The why names the later write.''',
    'same': '''The two texts are one thing under two names: two orders with one code,
    two names of one word or one bit, two branches that run the same code.
    The why says which.''',
    'unreached': '''The changed line differs from the line as it is only in a state that
    no pass brings there, and the why says what rules the state out: an
    overflow that a sign-extended word cannot make, a -0 that a MASK cannot
    leave, a bound that an earlier line of the same routine already holds.''',
    'repeated': '''The changed line is a check that another check repeats before anything
    is run or written: most are a binding's, repeated by the interpreter's
    own check at its entry, by bind_machine's, or by the same binding's
    earlier line. Dropping one of the two changes no outcome. The why names
    the other check.''',
    'restore': '''The changed line puts a binding's state back after a refusal, and no
    pass of this file refuses after it has written what the line restores
    (a refusal comes before the first write, or the routines write none of
    it). The line is the binding's promise that a refusal leaves the state
    as it found it, kept for the day a routine refuses later.''',
    'refusal': '''The changed line refuses a state the flight program never makes (a word
    the AGC cannot hold, an index outside its table), and no case makes one
    up to see the refusal. The readable code refuses there because the
    faithful port does. The why says what state.''',
    'owed': '''No test tells the two apart, and no reason is written yet: the why says
    what the change touches and what was not checked. Each is owed a case
    that catches it or a reason of one of the kinds above.''',
    'killed': '''Not a survivor any more: a check or a case added since catches it. Kept
    so that the run holds the catch: the entry names the check or suite that
    must fail (tc-sites is tools/tc_sites.py, every TC's place held to the
    listing both ways).''',
}
ORDER = ['rewritten', 'same', 'unreached', 'repeated', 'restore', 'refusal', 'owed', 'killed']


class Entry:
    def __init__(self, path, item, nth, klass, suite, old, new, why, line):
        self.path, self.item, self.nth, self.klass, self.suite = path, item, nth, klass, suite
        self.old, self.new, self.why, self.line = old, new, why, line

    def key(self):
        return (self.path, self.item, self.nth, tuple(self.old), tuple(self.new))

    def title(self):
        nth = ' (%d)' % self.nth if self.nth > 1 else ''
        klass = self.klass + (': ' + self.suite if self.suite else '')
        return '%s: %s%s [%s]' % (self.path, self.item, nth, klass)


def item_name(decl):
    """The name a top-level declaration gives: a function's, a table's."""
    decl = decl.split('{')[0]
    paren, equals = decl.find('('), decl.find('=')
    if paren >= 0 and (equals < 0 or paren < equals):
        found = re.findall(r'[A-Za-z_]\w*', decl[:paren])
    else:
        found = re.findall(r'[A-Za-z_]\w*', re.split(r'[\[=;]', decl)[0])
    return found[-1] if found else '-'


def items(lines):
    """The file's top-level stretches that have a name: (name, first line, last line). A stretch
    begins at a line that starts with a letter and ends where its braces close, or at its `;` if it
    opens none."""
    out = []
    i = 0
    while i < len(lines):
        if re.match(r'[A-Za-z_]', lines[i]):
            j, depth, opened = i, 0, False
            while j < len(lines):
                code = re.sub(r'/\*.*?\*/|"(?:[^"\\]|\\.)*"|\'(?:[^\'\\]|\\.)\'', '', lines[j])
                if not code.lstrip().startswith(('*', '/*')):
                    depth += code.count('{') - code.count('}')
                    opened = opened or '{' in code
                    if (opened and depth <= 0) or (not opened and code.rstrip().endswith(';')):
                        break
                j += 1
            j = min(j, len(lines) - 1)
            out.append((item_name(' '.join(lines[i:j + 1])), i, j))
            i = j + 1
        else:
            i += 1
    return out


def places(entry, lines):
    """Every line of the file at which the entry's lines stand in the routine it names."""
    spans = [(0, len(lines) - 1)] if entry.item == '-' else \
        [(first, last) for name, first, last in items(lines) if name == entry.item]
    out = []
    n = len(entry.old)
    for first, last in spans:
        for at in range(first, last - n + 2):
            if lines[at:at + n] == entry.old:
                out.append(at)
    return out


def changed(entry, text):
    """The file's text with the entry's change made, or None if its lines are not there."""
    lines = text.split('\n')
    at = places(entry, lines)
    if len(at) < entry.nth:
        return None
    start = at[entry.nth - 1]
    return '\n'.join(lines[:start] + entry.new + lines[start + len(entry.old):])


TITLE = re.compile(r'^(\S+): (\S+)(?: \((\d+)\))? \[([a-z]+)(?:: ([a-z0-9-]+))?\]$')


def read_ledger(text):
    """The ledger's entries and what is wrong with its form."""
    entries, wrong = [], []
    entry = None
    for number, line in enumerate(text.split('\n'), 1):
        if line.startswith('#') or (entry is None and (not line.strip() or line.startswith(('[', '    ')))):
            continue
        if entry is None:
            m = TITLE.match(line)
            if not m:
                wrong.append('line %d: not an entry\'s first line: %s' % (number, line[:60]))
                continue
            entry = Entry(m.group(1), m.group(2), int(m.group(3) or 1), m.group(4), m.group(5), [], [], [], number)
            entries.append(entry)
        elif line == '-' or line.startswith('- '):
            entry.old.append(line[2:])
        elif line == '+' or line.startswith('+ '):
            entry.new.append(line[2:])
        elif line.startswith('  ') and line.strip():
            entry.why.append(line.strip())
        elif not line.strip():
            entry = None
        else:
            wrong.append('line %d: neither a changed line nor a why: %s' % (number, line[:60]))
    return entries, wrong


def targets():
    """The checks and suites the Makefile names, a [killed] entry's among them: the static checks
    and the case suites (the -cases targets), not any other target (all, clean)."""
    with open(os.path.join(ROOT, 'Makefile')) as f:
        names = set(re.findall(r'^([A-Za-z0-9][\w.-]*):(?!=)', f.read(), re.M))
    return {n for n in names if n in STATIC or n.endswith('-cases')}


def judge(entries, read=None, known=None):
    """What is wrong with the entries against the tree."""
    wrong = []
    seen = {}
    texts = {}
    known = targets() if known is None else known
    for e in entries:
        where = 'line %d (%s)' % (e.line, e.title())
        if e.klass not in CLASSES:
            wrong.append('%s: no such reason as [%s]' % (where, e.klass))
        if (e.klass == 'killed') != bool(e.suite):
            wrong.append('%s: a [killed] entry names its suite, and no other does' % where)
        if e.suite and e.suite not in known:
            wrong.append('%s: the Makefile has no check or suite %s' % (where, e.suite))
        if not e.old:
            wrong.append('%s: no line to change' % where)
            continue
        if e.old == e.new:
            wrong.append('%s: the change changes nothing' % where)
        if not ' '.join(e.why).strip() or ' '.join(e.why).strip().lower() in ('(no reason)', 'todo'):
            wrong.append('%s: no why' % where)
        if e.key() in seen:
            wrong.append('%s: the same change as line %d' % (where, seen[e.key()]))
        seen[e.key()] = e.line
        if e.path not in texts:
            try:
                texts[e.path] = (read or default_read)(e.path)
            except OSError:
                texts[e.path] = None
        if texts[e.path] is None:
            wrong.append('%s: no such file' % where)
        elif not (e.path.startswith('src/') or e.path.startswith('tests/')):
            wrong.append('%s: not a file of src/ or tests/' % where)
        elif changed(e, texts[e.path]) is None:
            wrong.append('%s: its lines are not in %s%s' % (where, e.item, ' %d times' % e.nth if e.nth > 1 else ''))
    return wrong


def default_read(path):
    with open(os.path.join(ROOT, path)) as f:
        return f.read()


def counts(entries):
    by = {}
    for e in entries:
        by[e.klass] = by.get(e.klass, 0) + 1
    files = len({e.path for e in entries})
    alive = sum(n for k, n in by.items() if k != 'killed')
    text = '%d survivors in %d files' % (alive, len({e.path for e in entries if e.klass != "killed"}))
    text += (': ' + ', '.join('%d %s' % (by[k], k) for k in ORDER if k in by and k != 'killed') if alive else '') + '.'
    if by.get('killed'):
        text += ' %d more that a check or a case added since catches (killed).' % by['killed']
    del files
    return text


def wrap(text, width, first, rest):
    out, line = [], first
    for word in text.split():
        if len(line) + len(word) + 1 > width and line.strip():
            out.append(line.rstrip())
            line = rest
        line += word + ' '
    out.append(line.rstrip())
    return out


def render(entries, read=None):
    """The ledger's text: its head with the counts, then the entries by file and place."""
    read = read or default_read
    head = HEAD % {'counts': '\n# '.join(wrap(counts(entries), 74, '', '')),
                   'source': '\n'.join(('# ' + line).rstrip() for line in SOURCE.strip('\n').split('\n'))}
    lines = head.rstrip('\n').split('\n')
    lines.append('')
    for k in ORDER:
        lines.append('[%s] %s' % (k, CLASSES[k]))
    order = {}
    for e in entries:
        try:
            at = places(e, read(e.path).split('\n'))
        except OSError:
            at = []
        order[id(e)] = (e.path, at[e.nth - 1] if len(at) >= e.nth else 10 ** 9, e.new)
    for e in sorted(entries, key=lambda e: order[id(e)]):
        lines.append('')
        lines.append(e.title())
        lines.extend(('- ' + s).rstrip() for s in e.old)
        lines.extend(('+ ' + s).rstrip() for s in e.new)
        lines.extend(wrap(' '.join(e.why), 78, '  ', '  '))
    return '\n'.join(lines) + '\n'


# The run.

def graph():
    """What each make target is made of, from make's own database."""
    db = subprocess.run(['make', '-pn', 'all'], cwd=ROOT, capture_output=True, text=True).stdout
    deps = {}
    for line in db[db.index('# Files'):].split('\n'):
        if not line or line[0] in '#\t ' or '%' in line.split(':')[0]:
            continue
        m = re.match(r'^([^:=]+):(?!=)\s*(.*)$', line)
        if m:
            for target in m.group(1).split():
                deps.setdefault(target, set()).update(m.group(2).replace('|', ' ').split())
    return deps


def includers(header, read_all=None):
    """The .c files that include a header, itself or through another header."""
    texts = (read_all or sources)()
    want, out, grew = {header}, set(), True
    while grew:
        grew = False
        for path, body in texts.items():
            for h in list(want):
                if path not in want and path not in out and \
                        re.search(r'#include\s+"(?:[^"]*/)?%s"' % re.escape(os.path.basename(h)), body):
                    (want if path.endswith('.h') else out).add(path)
                    grew = True
    return out


def sources():
    out = {}
    for top in ('src', 'tests'):
        for folder, _, names in os.walk(os.path.join(ROOT, top)):
            for name in names:
                if name.endswith(('.c', '.h', '.inc')):
                    path = os.path.join(folder, name)
                    with open(path, errors='replace') as f:
                        out[os.path.relpath(path, ROOT)] = f.read()
    return out


def suites_of(deps, paths):
    """The case suites of `make all` each file is part of. (The Makefile makes every object depend on
    every header, so a header's are those of the .c files that include it.)"""
    memo = {}

    def closure(target):
        if target not in memo:
            memo[target] = set()
            out = set()
            for d in deps.get(target, ()):
                out.add(d)
                out |= closure(d)
            memo[target] = out
        return memo[target]

    sys.setrecursionlimit(100000)
    suites = sorted(t for t in deps.get('all', ()) if t.endswith('-cases'))
    out = {}
    for p in paths:
        files = includers(p) if p.endswith('.h') else {p}
        out[p] = [t for t in suites if files & closure(t)]
    return out


def sh(command, cwd, seconds):
    p = subprocess.Popen(command, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
                         start_new_session=True)
    try:
        out, _ = p.communicate(timeout=seconds)
        return p.returncode, out
    except subprocess.TimeoutExpired:
        os.killpg(p.pid, signal.SIGKILL)
        p.communicate()
        return None, 'ran over %d s' % seconds


class Both:
    """Two files written as one: the run's lines go to the terminal and to the report."""

    def __init__(self, *files):
        self.files = files

    def write(self, text):
        for f in self.files:
            f.write(text)

    def flush(self):
        for f in self.files:
            f.flush()


def copy_of_tree(number):
    """A copy of readable/ (without its build) beside links to the tree's other directories, so
    that the Makefile's paths out of readable/ hold."""
    base = os.path.join(WORK, 'w%d' % number)
    view = os.path.join(base, 'readable')
    if os.path.isdir(view):
        shutil.rmtree(view)
    os.makedirs(view)
    for name in os.listdir(ROOT):
        if name in ('build', '__pycache__') or name.startswith('.'):
            continue
        source = os.path.join(ROOT, name)
        if os.path.isdir(source):
            shutil.copytree(source, os.path.join(view, name), ignore=shutil.ignore_patterns('__pycache__'))
        else:
            shutil.copy2(source, os.path.join(view, name))
    up = os.path.dirname(ROOT)
    for name in os.listdir(up):
        link = os.path.join(base, name)
        if name != os.path.basename(ROOT) and not name.startswith('.git') and not os.path.lexists(link):
            os.symlink(os.path.join(up, name), link)
    return view


def try_one(view, entry, suites, deps, locks=None):
    """Make the change in the copy, run the checks that read the source, build what the file is part of
    and run it: ('build', text), ('killed', suite, text), ('survived',), or ('time', what, text) where a
    step ran over its time, which is neither a catch nor a pass. A suite runs in one copy at a time
    (locks): some write their results beside the port's build, which the copies share."""
    path = os.path.join(view, entry.path)
    good = default_read(entry.path)
    with open(path, 'w') as f:
        f.write(changed(entry, good))
    try:
        for check in STATIC:
            if check in suites:
                code, out = sh(['make', '-s', check], view, 600)
                if code is None:
                    return ('time', check, out)
                if code != 0:
                    return ('killed', check, out[-400:])
        suites = [t for t in suites if t not in STATIC]
        programs = sorted({d for t in suites for d in deps.get(t, ()) if d.startswith('build/')})
        if programs:
            code, out = sh(['make', '-s', '-k'] + programs, view, 3600)
            if code is None:
                return ('time', 'the build', out)
            if code != 0:
                return ('build', out[-400:])
        for suite in suites:
            with (locks[suite] if locks is not None else threading.Lock()):
                code, out = sh(['make', '-s', suite], view, 3600)
            if code is None:
                return ('time', suite, out)
            if code != 0:
                return ('killed', suite, out[-400:])
        return ('survived',)
    finally:
        with open(path, 'w') as f:
            f.write(good)


def verdict(entry, result):
    """None if the result is what the entry says, else what is wrong."""
    if result[0] == 'build':
        return 'does not build: ' + result[1].strip().split('\n')[-1][:200]
    if result[0] == 'time':
        return '%s %s: neither a catch nor a pass' % (result[1], result[2])
    if entry.klass == 'killed':
        if result[0] == 'survived':
            return 'no suite catches it any more'
        if result[1] != entry.suite:
            return 'caught by %s, not by %s' % (result[1], entry.suite)
        return None
    if result[0] == 'killed':
        return 'caught by %s (a survivor no more: make it [killed: %s], or drop it): %s' % (
            result[1], result[1], result[2].strip().split('\n')[-1][:160])
    return None


def run(entries, jobs, out=sys.stdout):
    """Every entry made and run: what is wrong. Each result is written to out as it comes, so that a
    run cut short still says how far it came."""
    deps = graph()
    suites = suites_of(deps, sorted({e.path for e in entries}))
    for path in suites:
        if path.startswith('src/'):
            suites[path] = list(STATIC) + suites[path]
    todo = list(entries)
    results = {}
    lock = threading.Lock()
    locks = {suite: threading.Lock() for of_path in suites.values() for suite in of_path}

    def worker(number):
        view = copy_of_tree(number)
        while True:
            with lock:
                if not todo:
                    return
                entry = todo.pop(0)
            mine = suites[entry.path]
            if entry.klass == 'killed':
                mine = [entry.suite] if entry.suite in mine else []
            result = try_one(view, entry, mine, deps, locks) if mine \
                else ('build', 'no case suite is made of ' + entry.path)
            with lock:
                results[id(entry)] = result
                out.write('%s %s\n' % (result[0], entry.title()))
                out.flush()

    threads = [threading.Thread(target=worker, args=(n,)) for n in range(max(1, min(jobs, len(todo))))]
    for t in threads:
        t.start()
    for t in threads:
        t.join()
    # A suite that times a loop by the clock can fail under the load of the others, so a change of a
    # survivor that a suite caught is tried once more, alone, and counts as caught only if it is
    # caught again; and a [killed] entry's catch by a suite (not a static check) must be made alone
    # too. A result that did not come from a catch (a build that fails, a step over its time, a
    # change that survived where a catch is held) is not tried again: a retry that could turn it
    # round would hide a flaky catch behind a pass.
    wrong = []
    view = os.path.join(WORK, 'w0', 'readable')
    for entry in entries:
        first = results[id(entry)]
        if first[0] == 'killed' and first[1] not in STATIC:
            again = try_one(view, entry, [first[1]], deps)
            out.write('alone: %s %s\n' % (again[0], entry.title()))
            out.flush()
            if again[0] != 'killed':
                first = again if entry.klass != 'killed' else \
                    ('time', first[1], 'caught under load, not alone (a flaky catch)')
            results[id(entry)] = first
        said = verdict(entry, results[id(entry)])
        if said:
            wrong.append('line %d (%s): %s' % (entry.line, entry.title(), said))
    return wrong


# The self-check: made-up ledgers and a made-up file, each wrong one must be found.

def self_check():
    source = '''\
static void first(void)
{
    tc_at(HERE + 1u);
    inhint();
    tc_at(HERE + 1u);
}

static const word TABLE_CODE[] = {
    ORDERS(DLOAD, SL1),
};

static const word one_line[] = {1u, 2u};

void second(int n)
{
    if (n) {
        tc_at(HERE + 1u);
    }
}
'''
    def read(path):
        if path != 'src/made/up.c':
            raise OSError(path)
        return source

    def ledger(*blocks):
        return '# head\n\n[rewritten] x\n    y\n\n' + '\n\n'.join(blocks) + '\n'

    def block(title, old, new=None, why='Why.'):
        lines = ['src/made/' + title, '-     ' + old]
        if new is not None:
            lines.append('+     ' + new)
        return '\n'.join(lines + (['  ' + why] if why else []))

    good = block('up.c: first (2) [rewritten]', 'tc_at(HERE + 1u);', 'tc_at(HERE + 2u);', 'Q only.')
    cases = [
        ('a good entry', ledger(good), None),
        ('a table entry', ledger(block('up.c: TABLE_CODE [same]', 'ORDERS(DLOAD, SL1),', 'ORDERS(DLOAD, SR1),')), None),
        ('a killed entry', ledger(block('up.c: second [killed: made-cases]', 'if (n) {', 'if (!n) {')), None),
        ('an owed entry', ledger(block('up.c: second [owed]', 'if (n) {', 'if (1) {', 'Not checked.')), None),
        ('a routine after a one-line table', ledger(block('up.c: one_line [same]', 'if (n) {', 'if (1) {')),
         'are not in'),
        ('a dropped line', ledger(block('up.c: first [rewritten]', 'inhint();')), None),
        ('lines not in the routine', ledger(block('up.c: second [rewritten]', 'inhint();', ';')), 'are not in'),
        ('too few places', ledger(block('up.c: first (3) [rewritten]', 'tc_at(HERE + 1u);', ';')), 'are not in'),
        ('no such routine', ledger(block('up.c: third [rewritten]', 'inhint();', ';')), 'are not in'),
        ('no why', ledger(block('up.c: first [rewritten]', 'inhint();', ';', '')), 'no why'),
        ('an unknown reason', ledger(block('up.c: first [equivalent]', 'inhint();', ';')), 'no such reason'),
        ('killed without a suite', ledger(block('up.c: first [killed]', 'inhint();', ';')), 'names its suite'),
        ('a suite the Makefile has not', ledger(block('up.c: second [killed: made-cases]', 'if (n) {', 'if (!n) {')),
         'has no check or suite'),
        ('a target that is no suite', ledger(block('up.c: second [killed: clean]', 'if (n) {', 'if (!n) {')),
         'has no check or suite'),
        ('a suite on a survivor', None, 'names its suite'),
        ('a change of nothing', ledger(block('up.c: first [rewritten]', 'inhint();', 'inhint();')), 'changes nothing'),
        ('the same change twice', ledger(good, good), 'the same change'),
        ('no such file', ledger(block('down.c: first [rewritten]', 'inhint();', ';')), 'no such file'),
        ('a file outside the tree', None, 'not a file of'),
        ('a line that is no entry', ledger('what is this'), 'not an entry'),
        ('a stray line', ledger(block('up.c: first [rewritten]', 'inhint();', None, '') + '\nstray'), 'neither'),
    ]
    failed = 0
    line = ['    inhint();']
    for name, text, want in cases:
        if text is None and want == 'names its suite':
            entries, wrong = [Entry('src/made/up.c', 'first', 1, 'rewritten', 'x-cases', line, [], ['Why.'], 1)], []
        elif text is None:
            entries = [Entry('../made/up.c', 'first', 1, 'rewritten', None, line, [], ['Why.'], 1)]
            wrong = judge(entries, lambda path: source, set())
            entries = []
        else:
            entries, wrong = read_ledger(text)
        wrong = wrong + judge(entries, read, {'made-cases'} if name != 'a suite the Makefile has not' else set())
        ok = (not wrong) if want is None else any(want in w for w in wrong)
        if not ok:
            failed += 1
            print('self-check: %s: %s' % (name, wrong or 'nothing found wrong'))
    # The change itself, and the order of the written ledger.
    entries, _ = read_ledger(ledger(good))
    made = changed(entries[0], source).split('\n')
    if [made[2], made[4], made[16]] != ['    tc_at(HERE + 1u);', '    tc_at(HERE + 2u);', '        tc_at(HERE + 1u);']:
        failed += 1
        print('self-check: the change is made at the wrong place')
    two, _ = read_ledger(ledger(block('up.c: second [unreached]', 'if (n) {', 'if (1) {'), good))
    text = render(two, read)
    again, wrong = read_ledger(text)
    if wrong or [e.item for e in again] != ['first', 'second'] or render(again, read) != text:
        failed += 1
        print('self-check: the written ledger is not read back as it was written')
    if verdict(two[0], ('killed', 'x-cases', 'failures')) is None or verdict(two[0], ('survived',)) is not None \
            or verdict(two[0], ('build', 'error')) is None or verdict(two[0], ('time', 'x-cases', 'ran over')) is None:
        failed += 1
        print('self-check: a survivor\'s verdict is wrong')
    dead = Entry('src/made/up.c', 'first', 1, 'killed', 'x-cases', ['    inhint();'], [], ['Why.'], 1)
    if verdict(dead, ('survived',)) is None or verdict(dead, ('killed', 'y-cases', '')) is None \
            or verdict(dead, ('killed', 'x-cases', '')) is not None \
            or verdict(dead, ('time', 'x-cases', 'ran over')) is None:
        failed += 1
        print('self-check: a killed entry\'s verdict is wrong')
    names = targets()
    if 'all' in names or 'clean' in names or 'tc-sites' not in names or 'pinball-cases' not in names:
        failed += 1
        print('self-check: a [killed] entry may name a check or a suite, and no other target')
    print('mutation_ledger self-check: %d parts, %s' % (len(cases) + 5, 'ok' if not failed else '%d FAILED' % failed))
    return 1 if failed else 0


def main():
    args = sys.argv[1:]
    if '--self-check' in args:
        return self_check()
    with open(LEDGER) as f:
        text = f.read()
    entries, wrong = read_ledger(text)
    wrong += judge(entries)
    if '--write' in args and not wrong:
        with open(LEDGER, 'w') as f:
            f.write(render(entries))
        print('mutation_ledger: wrote %d entries' % len(entries))
        return 0
    if not wrong and render(entries) != text:
        wrong.append('the file is not as --write writes it (its counts, its order or its head)')
    if '--run' in args and not wrong:
        if '--only' in args:
            only = args[args.index('--only') + 1]
            entries = [e for e in entries if e.path == only]
        if '--kind' in args:
            kind = args[args.index('--kind') + 1]
            entries = [e for e in entries if e.klass == kind]
        if '--suite' in args:
            suite = args[args.index('--suite') + 1]
            part_of = suites_of(graph(), sorted({e.path for e in entries}))
            entries = [e for e in entries if suite in part_of[e.path]]
        jobs = int(args[args.index('--jobs') + 1]) if '--jobs' in args else (os.cpu_count() or 1)
        os.makedirs(WORK, exist_ok=True)
        with open(os.path.join(WORK, 'report.txt'), 'w') as report:
            wrong = run(entries, jobs, Both(sys.stdout, report))
            report.write(''.join('wrong: %s\n' % w for w in wrong) or 'nothing wrong\n')
        what = 'made and run'
    else:
        what = 'held to the tree'
    for w in wrong:
        print('mutation_ledger: ' + w)
    print('mutation_ledger: %d entries %s (%s): %s' %
          (len(entries), what, counts(entries), 'FAILED' if wrong else 'ok'))
    return 1 if wrong else 0


if __name__ == '__main__':
    sys.exit(main())
