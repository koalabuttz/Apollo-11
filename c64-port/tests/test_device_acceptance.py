#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""tools/device_acceptance.py on made-up host summaries and replay
results: what makes a window a match, a host scenario clean, an adapter
defined, and a role covered, and the doc's form (tools/acceptance_bundle.py
read_doc)."""
import hashlib
import json
import pathlib
import sys
import tempfile
import unittest
from unittest import mock

ROOT = pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import acceptance_bundle as ab  # noqa: E402
import device_acceptance as da  # noqa: E402
import flight_host_differential as fhd  # noqa: E402

HEAD = 'abc123def456'
FHR = da.FHR


def window(**kw):
    """A replay result that matches the host over a 120 cs window and
    touches channel 15 (a key) and drives channel 5 (a jet)."""
    r = dict(scenario='lm-a', mission='lm', snap_cs=100, requested_cs=120, ok=True, status=0, reu_bounds=dict(ok=True),
             replay_rows=4, replay_applied=4, replay_ended=True, replay_end_paired=True, replay_unpaired_count=0,
             replay_skipped=[], replay_differences=0, replay_matched_cs=[60, 120], commit=HEAD,
             replay_inputs=dict(counts={'2': 4}, erasable=[], pipas=[], channels=['15'], requests=0,
                                channel_set={'15': 0o21}, channel_clear={'15': 0o77756}, channel_changed={'15': 0o21}),
             replay_outputs=dict(channel_set={'channel 5': 0o20}, channel_changed={'channel 5': 0o20},
                                 nonzero=[], changed=[]),
             _evidence='`evidence/lm-acceptance-devices/lm-a-100.json`', _env={})
    r.update(kw)
    return r


def host_row(name='lm-a', **kw):
    r = dict(scenario=f'{name}-scaled-x1', mission='lm', exit=0, routine_mismatches=[], unregistered=[],
             native_routines=10, commit=HEAD)
    r.update(kw)
    return r


FLIGHT = [('lm-a', 'lm', 1000, '@100V37E', {'FLIGHT_ENGINE': '1'})]


def table(**over):
    """A key role and a jets role whose adapters are defined in the tree."""
    keys = da.role('keys', 'Keys', 'in', 'channel 15', [(FHR, 'key_due', 'keys')], [(da.DPC, 'replay_rows', 'rows')],
                   da.pressed('0-9A-Z'), da.either(da.channel(0o15, how='channel_set'), da.requests('KEY1')), [])
    jets = da.role('jets', 'Jets', 'out', 'channels 5, 6', [(FHR, 'rp_end', 'end')], [(da.DPC, 'replay_state', 'end')],
                   da.uses('FLIGHT_ENGINE'), da.either(da.moved(5), da.moved(6)), [])
    for k, v in over.items():
        (keys if k.startswith('keys_') else jets)[k.split('_', 1)[1]] = v
    return [keys, jets]


def assess(runs, host, tab=None):
    for r in runs:
        r['_why'] = da.not_matched(r, 120, HEAD)
    with mock.patch.object(fhd, 'FLIGHT', FLIGHT):
        return da.assess('lm', tab or table(), runs, host, HEAD)


class Match(unittest.TestCase):
    def test_baseline(self):
        self.assertEqual(da.not_matched(window(), 120, HEAD), [])

    def test_each_flag_breaks_it(self):
        for kw in [dict(ok=False), dict(status=5), dict(reu_bounds=dict(ok=False, dma_outside=4)), dict(reu_bounds=None),
                   dict(replay_ended=False), dict(replay_applied=3), dict(replay_end_paired=False),
                   dict(replay_end_paired=None), dict(replay_unpaired_count=1), dict(replay_skipped=['TIME5']),
                   dict(replay_differences=2), dict(replay_differences=None), dict(replay_matched_cs=[]),
                   dict(replay_matched_cs=[60]), dict(requested_cs=3, replay_matched_cs=[3]), dict(commit='0123456789ab'),
                   dict(commit=HEAD + ' (dirty)'), dict(error=['FAIL: no snapshot'])]:
            with self.subTest(kw=kw):
                self.assertNotEqual(da.not_matched(window(**kw), 120, HEAD), [])

    def test_not_a_program_window(self):
        self.assertIn("not one of the program's windows", da.not_matched(window(), None, HEAD))


class Host(unittest.TestCase):
    def rows(self, summary, env=None):
        with mock.patch.object(fhd, 'FLIGHT', FLIGHT):
            return da.host_rows('lm', summary, env, HEAD)

    def test_states(self):
        self.assertEqual(self.rows([host_row()])[0][1], 'clean')
        self.assertEqual(self.rows([])[0][1], 'MISSING')
        self.assertEqual(self.rows([host_row(commit='0123456789ab')])[0][1], 'STALE')
        self.assertEqual(self.rows([host_row(exit=1)])[0][1], 'NOT CLEAN')
        self.assertEqual(self.rows([host_row(routine_mismatches=[dict(point='01,2000')])])[0][1], 'NOT CLEAN')

    def test_unregistered_counted(self):
        self.assertIn('9 unregistered', self.rows([host_row(unregistered=[{}] * 9)])[0][2])

    def test_env_selects(self):
        self.assertEqual(self.rows([host_row()], da.uses('FLIGHT_RR')), [])
        self.assertEqual(len(self.rows([host_row()], da.uses('FLIGHT_ENGINE'))), 1)


class Adapters(unittest.TestCase):
    def check(self, text, symbol, suffix='.c'):
        with tempfile.TemporaryDirectory() as d:
            f = pathlib.Path(d) / f'x{suffix}'
            f.write_text(text)
            return da.adapter_ok(f, symbol)

    def test_definitions(self):
        self.assertTrue(self.check('static void lr_due(unsigned long t,\n  int x){', 'lr_due'))
        self.assertTrue(self.check('static __attribute__((noinline)) void radar(int *p)\n{', 'radar'))
        self.assertTrue(self.check('LOW_TEXT("dp_state") static void replay_state(uint16_t t)\n{', 'replay_state'))
        self.assertTrue(self.check('#define PRO $40\n', 'PRO', '.S'))
        self.assertTrue(self.check('rk_dsky_keys:\n  .byte 0\n', 'rk_dsky_keys', '.S'))

    def test_not_definitions(self):
        self.assertFalse(self.check('/* static void lr_due(int x){} */', 'lr_due'))
        self.assertFalse(self.check('// static void lr_due(int x){}', 'lr_due'))
        self.assertFalse(self.check('const char *s = "PROG";', 'PRO'))
        self.assertFalse(self.check('void f(void){ if(lr_due(1)){ } }', 'lr_due'))
        self.assertFalse(self.check('lr_due(1);', 'lr_due'))
        self.assertFalse(self.check('x: ; rk_dsky_keys:\n', 'rk_dsky_keys', '.S'))

    def test_every_role_defined_in_tree(self):
        for m in da.PROGRAM:
            for t in da.roles(m):
                for f, s, _ in t['host'] + t['c64']:
                    self.assertTrue(da.adapter_ok(f, s), f'{m} {t["key"]}: {f} {s}')


class Roles(unittest.TestCase):
    def status(self, runs, host=None, tab=None):
        out, bad = assess(runs, [host_row()] if host is None else host, tab)
        return {t['key']: s for t, s, *_ in out}, bad

    def test_baseline_covered(self):
        s, bad = self.status([window()])
        self.assertEqual(s, dict(keys='covered', jets='covered'))
        self.assertEqual(bad, [])

    def test_window_that_does_not_match(self):
        for kw in [dict(ok=False), dict(replay_end_paired=False), dict(replay_skipped=['TIME5']), dict(commit='x')]:
            with self.subTest(kw=kw):
                s, bad = self.status([window(**kw)])
                self.assertEqual(s, dict(keys='GAP', jets='GAP'))
                self.assertTrue(bad)

    def test_input_role_needs_its_rows(self):
        w = window(replay_inputs=dict(counts={}, erasable=[], pipas=[], channels=[], requests=0))
        s, bad = self.status([w])
        self.assertEqual(s['keys'], 'GAP')
        self.assertEqual(s['jets'], 'covered')

    def test_output_role_needs_its_output_to_move(self):
        w = window(replay_outputs=dict(channel_set={'channel 5': 0, 'channel 6': 0},
                                       channel_changed={'channel 5': 0, 'channel 6': 0}, nonzero=[], changed=[]))
        s, bad = self.status([w])
        self.assertEqual(s['jets'], 'GAP')
        self.assertEqual(s['keys'], 'covered')

    def test_a_static_output_is_not_driven(self):
        w = window(replay_outputs=dict(channel_set={'channel 5': 0o20}, channel_changed={'channel 5': 0},
                                       nonzero=['0055'], changed=[]))
        s, _ = self.status([w])
        self.assertEqual(s['jets'], 'GAP')
        self.assertFalse(da.words(0o55)(w))
        self.assertTrue(da.words(0o55)(window(replay_outputs=dict(changed=['0055']))))
        self.assertTrue(da.held(5, 0o20)(w))

    def test_host_gates(self):
        for host in [[], [host_row(commit='0123456789ab')], [host_row(exit=1)]]:
            with self.subTest(host=host):
                _, bad = self.status([window()], host)
                self.assertTrue(any('host scenario lm-a' in b for b in bad), bad)

    def test_no_host_scenario(self):
        _, bad = self.status([window()], tab=table(jets_env=da.uses('FLIGHT_RHC')))
        self.assertIn('jets: no host scenario drives it', bad)

    def test_no_host_adapter(self):
        _, bad = self.status([window()], tab=table(jets_host=[]))
        self.assertIn('jets: no host adapter', bad)

    def test_missing_symbol(self):
        _, bad = self.status([window()], tab=table(jets_host=[(FHR, 'rp_gone', 'end')]))
        self.assertTrue(any('defines no rp_gone' in b for b in bad), bad)

    def test_blocking_gap(self):
        s, bad = self.status([window()], tab=table(jets_gap='not linked', jets_blocking=True))
        self.assertEqual(s['jets'], 'GAP')
        self.assertIn('jets: not linked', bad)
        s, bad = self.status([window()], tab=table(jets_gap='a note'))
        self.assertEqual(s['jets'], 'covered, with open items')
        self.assertEqual(bad, [])

    def test_missing_make_target(self):
        _, bad = self.status([window()], tab=table(jets_units=['no-such-target']))
        self.assertIn('jets: no make target no-such-target', bad)


def outs(changed=(), held=None, moved=None):
    """A window whose host states changed the words changed, held the
    channel bits held ({channel: bits}) and changed the bits moved."""
    held, moved = held or {}, moved or {}
    return window(replay_outputs=dict(
        channel_set={f'channel {c:o}': b for c, b in {**held, **moved}.items()},
        channel_changed={f'channel {c:o}': b for c, b in moved.items()},
        nonzero=[f'{a:04o}' for a in changed], changed=[f'{a:04o}' for a in changed]))


def static(*words, held=None):
    """Words that sit non-zero and bits that are held, nothing changing."""
    r = outs(held=held)
    r['replay_outputs']['nonzero'] = [f'{a:04o}' for a in words]
    return r


class RealRoles(unittest.TestCase):
    """The predicates and flags of the real role table (roles())."""
    def role(self, m, key):
        return {t['key']: t for t in da.roles(m)}[key]

    def hits(self, m, key, r):
        return self.role(m, key)['window'](r)

    def test_jets_need_a_change(self):
        for m in da.PROGRAM:
            with self.subTest(m=m):
                self.assertFalse(self.hits(m, 'jets', static(held={5: 0o20, 6: 0o4000})))
                self.assertTrue(self.hits(m, 'jets', outs(moved={5: 0o20})))
                self.assertTrue(self.hits(m, 'jets', outs(moved={6: 0o4000})))
        self.assertTrue(self.hits('lm', 'jets', outs(moved={0o11: 0o20000})))
        self.assertTrue(self.hits('cm', 'jets', outs(moved={0o11: 0o10000})))

    def test_throttle_needs_a_change(self):
        self.assertFalse(self.hits('lm', 'thrust', static(0o55, held={0o14: 0o10})))
        self.assertTrue(self.hits('lm', 'thrust', outs(changed=[0o55])))
        self.assertTrue(self.hits('lm', 'thrust', outs(moved={0o14: 0o10})))

    def test_static_jets_and_throttle_are_gap(self):
        tab = [self.role('lm', 'jets'), self.role('lm', 'thrust')]
        out, bad = assess([static(0o55, held={5: 0o20, 6: 0o4000, 0o14: 0o10})], [host_row()], tab)
        self.assertEqual({t['key']: s for t, s, *_ in out}, dict(jets='GAP', thrust='GAP'))
        self.assertTrue(bad)

    def test_static_outputs_drive_no_output_role(self):
        """Every output channel bit set and every output word non-zero, none
        of them changing: no output role counts the window."""
        r = static(*range(0o42, 0o61), held={c: 0o77777 for c in (5, 6, 0o10, 0o11, 0o12, 0o13, 0o14, 0o34, 0o35)})
        for m in da.PROGRAM:
            for t in da.roles(m):
                if t['dir'] == 'out' and not t['elsewhere']:
                    with self.subTest(m=m, role=t['key']):
                        self.assertFalse(t['window'](r))

    def test_enable_must_hold_through_the_window(self):
        toggled = outs(changed=[0o53], moved={0o12: 0o200})
        self.assertFalse(self.hits('cm', 'tvc', toggled))
        self.assertFalse(self.hits('lm', 'landing-displays', toggled))
        self.assertFalse(self.hits('lm', 'rr-drive', toggled))
        self.assertFalse(self.hits('cm', 'optics-drive', toggled))

    def test_tvc_needs_its_enable(self):
        self.assertTrue(self.hits('cm', 'tvc', outs(changed=[0o53], held={0o12: 0o200})))
        self.assertFalse(self.hits('cm', 'tvc', outs(changed=[0o53])))
        self.assertFalse(self.hits('cm', 'tvc', static(0o53, held={0o12: 0o200})))

    def test_optics_drive_needs_tvc_off(self):
        self.assertTrue(self.hits('cm', 'optics-drive', outs(changed=[0o54])))
        self.assertTrue(self.hits('cm', 'optics-drive', outs(moved={0o14: 0o2000})))
        self.assertFalse(self.hits('cm', 'optics-drive', outs(changed=[0o54], held={0o12: 0o200})))

    def test_rr_drive_and_cross_pointers(self):
        self.assertTrue(self.hits('lm', 'rr-drive', outs(changed=[0o53])))
        self.assertFalse(self.hits('lm', 'rr-drive', outs(changed=[0o53], held={0o12: 0o200})))
        self.assertTrue(self.hits('lm', 'landing-displays', outs(changed=[0o53], held={0o12: 0o200})))
        self.assertFalse(self.hits('lm', 'landing-displays', outs(changed=[0o53])))
        self.assertTrue(self.hits('lm', 'landing-displays', outs(changed=[0o60])))
        self.assertFalse(self.hits('lm', 'landing-displays', static(0o60)))

    def test_channel_33_discretes(self):
        def ch33(bits):
            return window(replay_inputs=dict(channel_changed={'33': bits}))
        for m in da.PROGRAM:
            with self.subTest(m=m):
                for bit in (0o1000, 0o2000, 0o4000, 0o10000, 0o20000, 0o40000):
                    self.assertTrue(self.hits(m, 'discretes', ch33(bit)), oct(bit))
                self.assertFalse(self.hits(m, 'discretes', ch33(0o10)))

    def test_blocking_flags(self):
        blocking = dict(lm={'imu', 'rhc', 'iss', 'trim', 'gyro'}, cm={'imu', 'optics-drive', 'gyro'})
        for m in da.PROGRAM:
            with self.subTest(m=m):
                self.assertEqual({t['key'] for t in da.roles(m) if t['blocking']}, blocking[m])
                for t in da.roles(m):
                    if t['blocking']:
                        self.assertTrue(t['gap'], t['key'])


class Doc(unittest.TestCase):
    def write(self, runs, bad=None):
        out, failed = assess(runs, [host_row()])
        with tempfile.TemporaryDirectory() as d:
            root = pathlib.Path(d)
            ev = root / 'evidence/lm-acceptance-devices'
            ev.mkdir(parents=True)
            f = ev / 'lm-a-100.json'
            f.write_text('{}\n')
            digest = hashlib.sha256(f.read_bytes()).hexdigest()
            (ev / 'SHA256.json').write_text(json.dumps(dict(files={'lm-a-100.json': dict(sha256=digest)})))
            for r in runs:
                r['_evidence'] = f'`evidence/lm-acceptance-devices/lm-a-100.json` (sha256 `{digest[:16]}...`)'
            with mock.patch.object(da, 'ROOT', root):
                da.write_doc('lm', out, failed if bad is None else bad, runs, None, '0' * 64, HEAD)
            return ab.read_doc(root, 'lm', 'device_adapter_tests', 'tools/device_acceptance.py'), \
                (root / 'docs/acceptance/Luminary099-device_adapter_tests.md').read_text()

    def test_pass_doc_reads_clean(self):
        (commit, images, cites, problems), text = self.write([window()])
        self.assertEqual(problems, [])
        self.assertEqual(commit, HEAD)
        self.assertEqual(images, {'flight-store/lm.reu': '0' * 64})
        self.assertEqual(len(cites), 1)

    def test_fail_doc(self):
        (_, _, _, problems), text = self.write([window(ok=False)])
        self.assertEqual(problems, ['the doc says FAIL'])
        self.assertIn('## Failures', text)


class OffNominal(unittest.TestCase):
    """The Off-nominal inputs section (tools/device_offnominal.py)."""

    def results(self, ok=True):
        row = da.offnom.ROWS['lm'][0]
        checks = [dict(check='alarm', ok=ok, at_cs=100 if ok else None)]
        return row, [dict(row=row, host=dict(ok=ok, mismatches=0, bridged=0, checks=checks, evidence='`e`'))]

    def test_windows_are_window_tuples(self):
        for m in ('lm', 'cm'):
            for s, snap, cs, every in da.offnom.windows(m):
                self.assertTrue(s.startswith(f'{m}-'))
                self.assertGreater(cs, 0)
                self.assertGreater(every, 0)

    def test_host_failure_fails_the_doc(self):
        _, res = self.results(ok=False)
        lines, bad = da.offnom.section(res, [], da.first_diff)
        self.assertEqual(len(bad), 1)
        self.assertIn('not run', '\n'.join(lines))

    def test_window_match_and_difference(self):
        row, res = self.results()
        snap, cs = row['window']
        good = dict(window(scenario=row['scenario'], snap_cs=snap, requested_cs=cs), _why=[])
        lines, bad = da.offnom.section(res, [good], da.first_diff)
        self.assertEqual(bad, [])
        self.assertIn('matches the host at 60, 120 cs', '\n'.join(lines))
        diff = dict(good, _why=['differs'], replay_differences=2, replay_differ_at_cs=50,
                    replay_first=[dict(at='0035', host='00001', c64='00002')])
        lines, bad = da.offnom.section(res, [diff], da.first_diff)
        self.assertEqual(bad, [])  # a differing window is an open item, not a failure
        self.assertIn('differs: 2 words by 50 cs', '\n'.join(lines))


if __name__ == '__main__':
    unittest.main()
