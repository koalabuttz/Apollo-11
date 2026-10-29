#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Summarize diagnostic source checkpoints without changing target state."""
import argparse,json,re
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('monitor',type=Path);p.add_argument('--output',type=Path);a=p.parse_args()
text=a.monitor.read_text();events=[]
for block in text.split('#2 (Trace store c001)')[1:]:
 fields=re.search(r'>C:c000\s+([0-9a-f]{2}) ([0-9a-f]{2}) ([0-9a-f]{2})',block)
 cpu=re.search(r'\.C:([0-9a-f]{4}).*SP:([0-9a-f]{2}).*\s([0-9]+)\s*\n',block)
 lease=re.search(r'>C:c600\s+((?:[0-9a-f]{2}[ \t]+){16})',block)
 if not(fields and cpu):continue
 item={'input':int(fields[1],16),'completed_source_checkpoints':int(fields[2],16),'source_failures':int(fields[3],16),'pc':int(cpu[1],16),'hardware_sp':int(cpu[2],16),'cycles':int(cpu[3])}
 if lease:
  words=bytes.fromhex(lease[1]);item['descriptor_id']=words[0];item['filename_pointer']=int.from_bytes(words[1:3],'little');item['loaded_size']=int.from_bytes(words[3:5],'little');item['crc16']=int.from_bytes(words[5:7],'little');item['filename_prefix']=words[7:].split(b'\0')[0].decode('ascii',errors='replace')
 events.append(item)
assert events and all(e['source_failures']==0 for e in events)
per_case={str(n):max(e['completed_source_checkpoints']for e in events if e['input']==n)for n in sorted({e['input']for e in events})}
report={'diagnostic_only':True,'monitor':str(a.monitor),'event_count':len(events),'per_input_completed_source_checkpoints':per_case,'total_complete_checkpoints':sum(per_case.values()),'source_failures':0,'first_event':events[0],'last_event':events[-1],'events':events}
out=json.dumps(report,indent=2)+'\n'
if a.output:a.output.write_text(out)
else:print(out,end='')
