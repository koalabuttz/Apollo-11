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
                   da.uses('FLIGHT_ENGINE'), da.either(da.driven(5), da.driven(6)), [])
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


if __name__ == '__main__':
    unittest.main()
