#!/usr/bin/python
import os
import sys
import re
import json
import mysql.connector
from collections import namedtuple

DB_HOST = "localhost"
DB_USER = "neat"
DB_PASS = "jjFj5S58p1LuFajCq6X5"
DB_DBASE = "neat"

db_conn = mysql.connector.connect(host=DB_HOST, user=DB_USER,
                                  password=DB_PASS,
                                  database=DB_DBASE)

def load_neatpmd(fname):
  ip_to_ifname = {}
  with open(fname) as f:
    for line in f:
      search_localip = re.search(r'\[local_ip\|(\d+[.]\d+[.]\d+[.]\d+)\]', line)
      if search_localip:
        local_ip = search_localip.group(1)
        search_ifname = re.search(r'\[interface\|(\w+)\]', line)
        if search_ifname:
          ifname = search_ifname.group(1)
          if local_ip in ip_to_ifname:
            if ip_to_ifname[local_ip] != ifname:
              print sched_dir, "ERROR", local_ip, ifname, ip_to_ifname
              exit(1)
          else:
            ip_to_ifname[local_ip] = ifname
  return ip_to_ifname


def load_metadata(fname, exp_id, sched_id, node_id):
  entries = []
  with open(fname) as f:
    for line in f:
      if line.startswith('{'):
        me = json.loads(line)
        entries.append({
          'exp_id': exp_id,
          'sched_id': sched_id,
          'node_id': node_id,
          'seq_no': me.get('SequenceNumber', None),
          'ts': me.get('Timestamp', None),
          'data_ver': me.get('DataVersion', None),
          'data_id': me.get('DataId', None),
          'iif_name': me.get('InternalInterface', None),
          'iccid': me.get('ICCID', None),
          'imsi': me.get('IMSI', None),
          'imei': me.get('IMEI', None),
          'op_name': me.get('Operator', None),
          'ip_addr': me.get('IPAddress', None),
          'if_name': me.get('InterfaceName', None),
          'imsimccmnc': me.get('IMSIMCCMNC', None),
          'nwmccmnc': me.get('NWMCCMNC', None),
          'lac': me.get('LAC', None),
          'cid': me.get('CID', None),
          'rssi': me.get('RSSI', None),
          'ecio': me.get('ECIO', None),
          'lte_rsrp': me.get('RSRP', None),
          'lte_rsrq': me.get('RSRQ', None),
          'lte_freq': me.get('Frequency', None),
          'lte_band': me.get('Band', None),
          'dev_mode': me.get('DeviceMode', None),
          'dev_submode': me.get('DeviceSubmode', None),
          'dev_state': me.get('DeviceState', None),
          'pci': me.get('PCI', None)
        })
  return entries

def gen_ifname_to_oper(metadata_entries):
  ifname_to_oper = {}
  for entry in metadata_entries:
    if entry['iif_name'] is not None and entry['imsimccmnc'] is not None:
      if entry['iif_name'] not in ifname_to_oper:
        ifname_to_oper[entry['iif_name']] = entry['imsimccmnc']
      elif ifname_to_oper[entry['iif_name']] != entry['imsimccmnc']:
        raise RuntimeError('Detected imsimccmnc change %s %s %s ' %
          (entry['sched_id'], entry['iif_name'], entry['imsimccmnc']))
  return ifname_to_oper

def load_tcp_ping(fname, dt, tm):
  entries = []
  dtm = "%s-%s-%s %s:%s:%s" % (dt[0:4], dt[4:6], dt[6:8], tm[0:2], tm[2:4], tm[4:6])
  with open(fname) as f:
    for line in f:
      fields = line.split('\t')
      entries.append({
        'run_tool': fields[0],
        'run_dtm':  dtm, 
        'iter_nr': int(fields[1]),
        'local_ip': fields[2],
        'remote_host': fields[3],
        'remote_port': int(fields[4]),
        'ping_type': fields[5],
        'rtt': float(fields[6])
      })
  return entries

def load_dwnl_test(fname, dt, tm):
  entries = []
  dtm = "%s-%s-%s %s:%s:%s" % (dt[0:4], dt[4:6], dt[6:8], tm[0:2], tm[2:4], tm[4:6])
  with open(fname) as f:
    for line in f:
      fields = line.split('\t')
      entries.append({
        'run_tool': fields[0],
        'run_dtm':  dtm, 
        'iter_nr': int(fields[1]),
        'local_ip': fields[2],
        'remote_host': fields[3],
        'remote_port': int(fields[4]),
        'dwnl_path': fields[5],
        'dwnl_size': int(fields[6]),
        'init_time': float(fields[7]),
        'total_time': float(fields[8])
      })
  return entries

tcp_ping = namedtuple('tcp_ping',
  ['exp_id', 'sched_id', 'node_id', 'run_tool', 'run_dtm',
   'iter_nr', 'local_ip', 'if_name', 'oper', 'remote_host',
   'remote_port', 'ping_type', 'rtt'])

def save_tcp_ping(entries):
  sql = """insert into tcp_ping
    (exp_id, sched_id, node_id, run_tool, run_dtm, iter_nr, local_ip,
    if_name, oper, remote_host, remote_port, ping_type, rtt)
    values (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
    """

  cursor = db_conn.cursor()

  for entry in entries:
    row = tcp_ping(**entry)
    cursor.execute(sql, row)

  db_conn.commit()
  cursor.close()

dwnl_test = namedtuple('dwnl_test',
  ['exp_id', 'sched_id', 'node_id', 'run_tool', 'run_dtm',
   'iter_nr', 'local_ip', 'if_name','oper', 'remote_host',
   'remote_port', 'dwnl_path', 'dwnl_size', 'init_time', 'total_time'])

def save_dwnl_test(entries):
  sql = """insert into dwnl_test
    (exp_id, sched_id, node_id, run_tool, run_dtm,
    iter_nr, local_ip, if_name, oper, remote_host,
     remote_port, dwnl_path, dwnl_size, init_time, total_time)
    values (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
    """

  cursor = db_conn.cursor()

  for entry in entries:
    row = dwnl_test(**entry)
    cursor.execute(sql, row)

  db_conn.commit()
  cursor.close()

metadata = namedtuple('metadata',
  ['exp_id', 'sched_id', 'node_id', 'seq_no', 'ts', 'data_ver',
   'data_id', 'iif_name', 'iccid', 'imsi', 'imei', 'op_name', 'ip_addr',
   'if_name', 'imsimccmnc', 'nwmccmnc', 'lac', 'cid',
   'rssi', 'ecio', 'lte_rsrp', 'lte_rsrq', 'lte_freq', 'lte_band',
   'dev_mode', 'dev_submode', 'dev_state', 'pci'])

def save_metadata(entries):
  sql = """insert into metadata
    (exp_id, sched_id, node_id, seq_no, ts, data_ver, data_id, iif_name,
    iccid, imsi, imei, op_name, ip_addr, if_name, imsimccmnc, nwmccmnc,
    lac, cid, rssi, ecio, lte_rsrp, lte_rsrq, lte_freq, lte_band,
    dev_mode, dev_submode, dev_state, pci) values ( 
    %s, %s, %s, %s, from_unixtime(%s), %s, %s, %s, %s, %s, %s, %s,
    %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
    """

  cursor = db_conn.cursor()

  for entry in entries:
    row = metadata(**entry)
    cursor.execute(sql, row)

  db_conn.commit()
  cursor.close()

def process_sched(exp_id, sched_id, node_id):
  tcp_ping_entries = []
  dwnl_test_entries = []
  metadata_entries = []
  ip_to_ifname = None
  ifname_to_oper = None

  for subdir, dirs, files in os.walk(str(sched_id)):
    for fname in files:
      fpath = os.path.join(subdir, fname)
      m = re.match(r'tcp-ping-(\d+)-(\d+).txt', fname)
      if m:
        entries = load_tcp_ping(os.path.join(subdir, fname), m.group(1), m.group(2))
        tcp_ping_entries.extend(entries)

      m = re.match(r'neat-tcp-ping-(\d+)-(\d+).txt', fname)
      if m:
        entries = load_tcp_ping(os.path.join(subdir, fname), m.group(1), m.group(2))
        tcp_ping_entries.extend(entries)

      m = re.match(r'dwnl-test-(\d+)-(\d+).txt', fname)
      if m:
        entries = load_dwnl_test(os.path.join(subdir, fname), m.group(1), m.group(2))
        dwnl_test_entries.extend(entries)

      m = re.match(r'neat-dwnl-test-(\d+)-(\d+).txt', fname)
      if m:
        entries = load_dwnl_test(os.path.join(subdir, fname), m.group(1), m.group(2))
        dwnl_test_entries.extend(entries)

      m = re.match(r'neatpmd-(\d+)-(\d+).txt', fname)
      if m:
        ip_to_ifname = load_neatpmd(os.path.join(subdir, fname))

      mobj = re.match(r'metadata-exporter-(\d+)-(\d+).txt', fname)
      if mobj:
        metadata_entries = load_metadata(fpath, exp_id, sched_id, node_id)
        ifname_to_oper = gen_ifname_to_oper(metadata_entries)


  for entry in tcp_ping_entries:
    entry['exp_id'] = exp_id
    entry['sched_id'] = sched_id
    entry['node_id'] = node_id
    entry['if_name'] = ip_to_ifname.get(entry['local_ip'], None)
    entry['oper'] = ifname_to_oper.get(entry['if_name'], None)

  for entry in dwnl_test_entries:
    entry['exp_id'] = exp_id
    entry['sched_id'] = sched_id
    entry['node_id'] = node_id
    entry['if_name'] = ip_to_ifname.get(entry['local_ip'], None)
    entry['oper'] = ifname_to_oper.get(entry['if_name'], None)
  
  save_tcp_ping(tcp_ping_entries)
  save_dwnl_test(dwnl_test_entries)
  save_metadata(metadata_entries)


with open(sys.argv[1]) as f:
  config = json.load(f)
  exp_id = config['id']
  for sched_id, sched_config in config['schedules'].iteritems():
    print exp_id, int(sched_id), sched_config['nodeid']
    process_sched(exp_id, int(sched_id), sched_config['nodeid'])

db_conn.disconnect()

