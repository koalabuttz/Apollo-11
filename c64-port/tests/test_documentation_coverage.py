#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Prevent documentation preservation from bypassing native-code acceptance."""
import contextlib
import copy
import io
import json
from pathlib import Path
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
import port_coverage
class DocumentationCoverage(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.inventory=json.loads((ROOT/'docs/port-coverage.json').read_text())
    def audit_errors(self,inventory):
        with contextlib.redirect_stdout(io.StringIO()):
            errors,_=port_coverage.audit(inventory,False)
        return errors
    def test_reviewed_inventory(self):
        self.assertEqual(self.audit_errors(self.inventory),[])
    def test_executable_source_cannot_use_documentation_exception(self):
        forged=copy.deepcopy(self.inventory)
        source=next(e for e in forged['files'] if e['path']=='Comanche055/MYSUBS.agc')
        source['complete_native_coverage'].update(qualification_kind='documentation_preservation',algorithms_translated=0,emitted_words=0)
        self.assertTrue(any('Documentation-only certificate contains executable/data/declaration source' in e for e in self.audit_errors(forged)))
    def test_documentation_must_claim_no_runtime_emissions(self):
        forged=copy.deepcopy(self.inventory)
        source=next(e for e in forged['files'] if e['path']=='Comanche055/CONTRACT_AND_APPROVALS.agc')
        source['complete_native_coverage']['emitted_words']=1
        self.assertTrue(any('must claim zero algorithms and emitted words' in e for e in self.audit_errors(forged)))
if __name__=='__main__':unittest.main()
