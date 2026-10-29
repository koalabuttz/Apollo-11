#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Audit source-exact sphere/caller stages; neither orbital callee is claimed."""
import json,pathlib
from fixed_data_generate import symbols,fixed_address,physical_erasable
R=pathlib.Path(__file__).resolve().parents[1]
def main():
 result={};audits=json.loads((R/'build/sband-source/audit.json').read_text())
 for mission,name in [('LM','Luminary099'),('CM','Comanche055')]:
  sy=symbols((R/'build/sband-source'/name/'assembly.lst').read_text());a=next(x for x in audits if x['source_path'].startswith(name+'/'))
  ranges=[(0o3616,0o3627),(0o3636,0o3642)]if mission=='LM'else[(0o3662,0o3673),(0o3677,0o3704)]
  selected=[r for r in a['source_line_word_map']if r['words'] and any(low<=int(r['assembled_address'].split(',')[-1],8)<high for low,high in ranges)]
  assert sy['X2']=='0000047'and sy['RATT']=='0000000'and sy['TAT']=='0000014'
  if mission=='LM':assert physical_erasable(sy['RLM'])==0o2204 and sy['UNIT']=='00,3023'and sy['LUNPOS']=='15,3664'
  else:assert sy['RCM']=='0000002'and sy['MXV']=='7303'and sy['LUNPOS']=='26,2115'and sy['REFSMMAT']=='E3,1735'
  result[mission]=dict(source_path=a['source_path'],source_sha256=a['source_sha256'],rope_sha256=a['rope_sha256'],source_edits=0,native_source='src/sband_orbit.c',containing_file_status='partial',scope='Original returned-conic SLOAD/BHIZ sphere selection. Earth reaches actual LM UNIT or CM MXV before helper executes; Moon saves RATT and loads TAT before actual LUNPOS interpreted entry. LUNPOS body and its following vector math are not executed.',source_line_word_map=selected,original_bindings={k:sy[k]for k in ['SBANDANT','X2','RATT','TAT','RLM'if mission=='LM'else 'RCM','LUNPOS','UNIT','MXV','REFSMMAT','CONV4'if mission=='LM'else 'EISOI','NEWOPS','DANZIG']},sphere_domain='All32768 ordinary X2 words occur in actual factory, with both zero representations taking Earth and all nonzero representations taking Moon. Other source contexts varied, not exhaustively Cartesian.')
 (R/'build/sband-orbit-source-map.json').write_text(json.dumps(result,indent=2)+'\n');print('PASS original sphere/caller word maps',[(m,sum(len(r['words'])for r in x['source_line_word_map']))for m,x in result.items()])
if __name__=='__main__':main()
