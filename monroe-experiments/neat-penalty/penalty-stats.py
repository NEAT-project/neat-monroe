#!/usr/bin/python
import sys
import json

fname = sys.argv[1]

#{ dtm
#  cpu
#  mem { total avail }
#  app { name, cpu, pss
#  metadata-exporter
#  neatpmd
#  neat-proxy }

entries = []

first_line = True
with open(sys.argv[1]) as f:
  for line in f:
    if first_line:
      first_line = False
      continue

    fields = line.split('\t')

    entry = {
      'ts': str(fields[0]),
      'cpu': float(fields[1]),
      'mem': (float(fields[2]) - float(fields[3])) / (1024 * 1024)
    }

    if fields[4] == '(no-app)':
      entry[fields[4]] = None

    if fields[4] in ['tcp-ping', 'neat-tcp-ping', 'dwnl-test', 'neat-dwnl-test']:
      entry[fields[4]] = {
        'cpu': float(fields[5]) / 4 if fields[5] != 'None' else None,
        'pss': float(fields[6]) / (1024 * 1024)
    }

    if fields[7] == 'neat-metadata-exporter':
      entry[fields[7]] = {'cpu': float(fields[8]) / 4, 'pss': float(fields[9]) / (1024 * 1024)}
  
    if fields[10] == 'neatpmd':
      entry[fields[10]] = {'cpu': float(fields[11]) / 4, 'pss': float(fields[12]) / (1024 * 1024)}

    if fields[13] == 'neat-proxy':
      entry[fields[13]] = {'cpu': float(fields[14]) / 4, 'pss': float(fields[15]) / (1024 * 1024)}

    entries.append(entry)

#print entries

def calc_avr(stats, key, value):
  if value is not None:
    if key not in stats:
      stats[key] = {'avr': value, 'cnt': 1}
    else:
      avr = stats[key]['avr']
      cnt = stats[key]['cnt']
      avr = (avr * cnt + value) / (cnt + 1)
      cnt += 1
      stats[key]['avr'] = avr
      stats[key]['cnt'] = cnt

stats = {
  'idle': {},
  'non-neat': {},
  'neat': {},
  'proxy': {}
}

for i in range(1,len(entries)-1):

  # 1) Idle (no test running)
  if ('(no-app)' in entries[i] and 'neat-metadata-exporter' not in entries[i] and
      'neatpmd' not in entries[i] and 'neat-proxy' not in entries[i]):
    calc_avr(stats['idle'], 'cpu', entries[i]['cpu'])
    calc_avr(stats['idle'], 'mem', entries[i]['mem'])

  # 2) non-NEAT
  if (('tcp-ping' in entries[i] or 'dwnl-test' in entries[i]) and
      'neat-proxy' not in entries[i]):
    calc_avr(stats['non-neat'], 'cpu', entries[i]['cpu'])
    calc_avr(stats['non-neat'], 'mem', entries[i]['mem'])
    if 'tcp-ping' in entries[i]:
      calc_avr(stats['non-neat'], 'tcp-ping-cpu', entries[i]['tcp-ping']['cpu'])
      calc_avr(stats['non-neat'], 'tcp-ping-mem', entries[i]['tcp-ping']['pss'])
    if 'dwnl-test' in entries[i]:
      calc_avr(stats['non-neat'], 'dwnl-test-cpu', entries[i]['dwnl-test']['cpu'])
      calc_avr(stats['non-neat'], 'dwnl-test-mem', entries[i]['dwnl-test']['pss'])

  # 3) NEAT
  if 'neat-tcp-ping' in entries[i] or 'neat-dwnl-test' in entries[i]:
    calc_avr(stats['neat'], 'cpu', entries[i]['cpu'])
    calc_avr(stats['neat'], 'mem', entries[i]['mem'])
    if 'neat-tcp-ping' in entries[i]:
      calc_avr(stats['neat'], 'neat-tcp-ping-cpu', entries[i]['neat-tcp-ping']['cpu'])
      calc_avr(stats['neat'], 'neat-tcp-ping-mem', entries[i]['neat-tcp-ping']['pss'])
    if 'neat-dwnl-test' in entries[i]:
      calc_avr(stats['neat'], 'neat-dwnl-test-cpu', entries[i]['neat-dwnl-test']['cpu'])
      calc_avr(stats['neat'], 'neat-dwnl-test-mem', entries[i]['neat-dwnl-test']['pss'])
    if 'neat-metadata-exporter' in entries[i]:
      calc_avr(stats['neat'], 'neat-metadata-exporter-cpu', entries[i]['neat-metadata-exporter']['cpu'])
      calc_avr(stats['neat'], 'neat-metadata-exporter-mem', entries[i]['neat-metadata-exporter']['pss'])
    if 'neatpmd' in entries[i]:
      calc_avr(stats['neat'], 'neatpmd-cpu', entries[i]['neatpmd']['cpu'])
      calc_avr(stats['neat'], 'neatpmd-mem', entries[i]['neatpmd']['pss'])

  # 4) proxy
  if (('tcp-ping' in entries[i] or 'dwnl-test' in entries[i]) and
      'neat-proxy' in entries[i]):
    calc_avr(stats['proxy'], 'cpu', entries[i]['cpu'])
    calc_avr(stats['proxy'], 'mem', entries[i]['mem'])
    if 'tcp-ping' in entries[i]:
      calc_avr(stats['proxy'], 'tcp-ping-cpu', entries[i]['tcp-ping']['cpu'])
      calc_avr(stats['proxy'], 'tcp-ping-mem', entries[i]['tcp-ping']['pss'])
    if 'dwnl-test' in entries[i]:
      calc_avr(stats['proxy'], 'dwnl-test-cpu', entries[i]['dwnl-test']['cpu'])
      calc_avr(stats['proxy'], 'dwnl-test-mem', entries[i]['dwnl-test']['pss'])
    if 'neat-metadata-exporter' in entries[i]:
      calc_avr(stats['proxy'], 'neat-metadata-exporter-cpu', entries[i]['neat-metadata-exporter']['cpu'])
      calc_avr(stats['proxy'], 'neat-metadata-exporter-mem', entries[i]['neat-metadata-exporter']['pss'])
    if 'neatpmd' in entries[i]:
      calc_avr(stats['proxy'], 'neatpmd-cpu', entries[i]['neatpmd']['cpu'])
      calc_avr(stats['proxy'], 'neatpmd-mem', entries[i]['neatpmd']['pss'])
    if 'neat-proxy' in entries[i]:
      calc_avr(stats['proxy'], 'neat-proxy-cpu', entries[i]['neat-proxy']['cpu'])
      calc_avr(stats['proxy'], 'neat-proxy-mem', entries[i]['neat-proxy']['pss'])

#print json.dumps(stats)

for test, test_stats in stats.iteritems():
  for key, key_stats in test_stats.iteritems():
    print("%s\t%s\t%d\t%f" % (test, key, key_stats['cnt'], key_stats['avr']))
  
