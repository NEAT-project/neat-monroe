#!/usr/bin/python
import os
import sys
import re
import json
import mysql.connector
from datetime import datetime
from collections import namedtuple

DB_HOST = "localhost"
DB_USER = "neat"
DB_PASS = "jjFj5S58p1LuFajCq6X5"
DB_DBASE = "neat"

def get_matching_files(sched_id, name):
  for subdir, dirs, files in os.walk(str(sched_id)):
    for fname in files:
      pattern = name + r'-(\d+)-(\d+).txt'
      mobj = re.match(pattern, fname)
      if mobj:
        dt = mobj.group(1)
        tm = mobj.group(2)
        d = datetime(int(dt[0:4]), int(dt[4:6]), int(dt[6:8]),
                     int(tm[0:2]), int(tm[2:4]), int(tm[4:6]), 0)
        dtm = (d - datetime(1970, 1, 1)).total_seconds()
        yield os.path.join(subdir, fname), dtm

def process_neatpmd(sched_id):
  ip_to_ifname = {}
  for fpath, dtm in get_matching_files(sched_id, 'neatpmd'):
    with open(fpath) as f:
      for line in f:

        search_localip = re.search(r'\[local_ip\|(\d+[.]\d+[.]\d+[.]\d+)\]', line)
        if not search_localip:
          continue
        local_ip = search_localip.group(1)

        search_ifname = re.search(r'\[interface\|(\w+)\]', line)
        if not search_ifname:
          continue
        ifname = search_ifname.group(1)

        if local_ip not in ip_to_ifname:
          ip_to_ifname[local_ip] = ifname
        elif ip_to_ifname[local_ip] != ifname:
          raise RuntimeError('Detected local_ip change %s %s %s ' %
            (sched_id, local_ip, ifname))
  return ip_to_ifname

def process_metadata(db_conn, exp_id, sched_id, node_id):
  ifname_to_oper = {}

  metadata = namedtuple('metadata',
    ['exp_id', 'sched_id', 'node_id', 'seq_no', 'ts', 'span', 'data_ver',
     'data_id', 'iif_name', 'iccid', 'imsi', 'imei', 'op_name', 'ip_addr',
     'if_name', 'imsimccmnc', 'nwmccmnc', 'lac', 'cid',
     'rssi', 'ecio', 'lte_rsrp', 'lte_rsrq', 'lte_freq', 'lte_band',
     'dev_mode', 'dev_submode', 'dev_state', 'pci'])

  sql = """insert into metadata
    (exp_id, sched_id, node_id, seq_no, ts, span, data_ver, data_id, iif_name,
    iccid, imsi, imei, op_name, ip_addr, if_name, imsimccmnc, nwmccmnc,
    lac, cid, rssi, ecio, lte_rsrp, lte_rsrq, lte_freq, lte_band,
    dev_mode, dev_submode, dev_state, pci) values ( 
    %s, %s, %s, %s, from_unixtime(%s), %s, %s, %s, %s, %s, %s, %s, %s,
    %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)
    """

  rows = []
  prev_ts = {}
  for fpath, dtm in get_matching_files(sched_id, 'metadata-exporter'):
    with open(fpath) as f:
      for line in f:
        if not line.startswith('{'):
          continue

        me = json.loads(line)

        iif_name=me['InternalInterface'] # me.get('InternalInterface', None),
        ts=me['Timestamp'] #me.get('Timestamp', None)

        span = ts - prev_ts[iif_name] if iif_name in prev_ts else 0.0
        prev_ts[iif_name] = ts

        row = metadata(
          exp_id=exp_id,
          sched_id=sched_id,
          node_id=node_id,
          seq_no=me.get('SequenceNumber', None),
          ts=ts,
          span=span,
          data_ver=me.get('DataVersion', None),
          data_id=me.get('DataId', None),
          iif_name=iif_name,
          iccid=me.get('ICCID', None),
          imsi=me.get('IMSI', None),
          imei=me.get('IMEI', None),
          op_name=me.get('Operator', None),
          ip_addr=me.get('IPAddress', None),
          if_name=me.get('InterfaceName', None),
          imsimccmnc=me.get('IMSIMCCMNC', None),
          nwmccmnc= me.get('NWMCCMNC', None),
          lac= me.get('LAC', None),
          cid=me.get('CID', None),
          rssi=me.get('RSSI', None),
          ecio=me.get('ECIO', None),
          lte_rsrp=me.get('RSRP', None),
          lte_rsrq=me.get('RSRQ', None),
          lte_freq=me.get('Frequency', None),
          lte_band=me.get('Band', None),
          dev_mode=me.get('DeviceMode', None),
          dev_submode=me.get('DeviceSubmode', None),
          dev_state=me.get('DeviceState', None),
          pci=me.get('PCI', None)
        )

        iif_name = getattr(row,'iif_name')
        imsimccmnc = getattr(row, 'imsimccmnc')
        if iif_name is not None and imsimccmnc is not None:
          if iif_name not in ifname_to_oper:
            ifname_to_oper[iif_name] = imsimccmnc
          elif ifname_to_oper[iif_name] != imsimccmnc:
            raise RuntimeError('Detected imsimccmnc change %s %s %s ' %
              (sched_id, iif_name, imsimccmnc))

        rows.append(row)

  cursor = db_conn.cursor()
  cursor.executemany(sql, rows)
  cursor.close()

  return rows, ifname_to_oper

def find_metadata_ts(metadata_table, iface, dtm):
  m_ts = None
  for row in metadata_table:
    ts = getattr(row, 'ts')
    iif_name = getattr(row, 'iif_name')
    
    if iif_name != iface:
      continue

    if ts > dtm:
      break
    else:
      m_ts = ts

  return m_ts

def process_tcp_ping(db_conn, exp_id, sched_id, node_id, ip_to_ifname, ifname_to_oper, metadata_table):

  tcp_ping = namedtuple('tcp_ping',
    ['exp_id', 'sched_id', 'node_id', 'run_tool', 'run_dtm',
     'iter_nr', 'local_ip', 'if_name', 'oper', 'remote_host',
     'remote_port', 'ping_type', 'rtt', 'm_ts'])

  sql = """insert into tcp_ping
    (exp_id, sched_id, node_id, run_tool, run_dtm, iter_nr, local_ip,
    if_name, oper, remote_host, remote_port, ping_type, rtt, m_ts)
    values (%s, %s, %s, %s, from_unixtime(%s), %s, %s, %s, %s, %s,
    %s, %s, %s, from_unixtime(%s))
    """

  rows = []
  for fpath, dtm in get_matching_files(sched_id, '(?:neat-)?tcp-ping'):
    with open(fpath) as f:
      for line in f:
        fields = line.split('\t')

        local_ip = fields[2]
        if_name = ip_to_ifname.get(local_ip, None)
        oper = ifname_to_oper.get(if_name, None)

        row = tcp_ping(
          exp_id=exp_id,
          sched_id=sched_id,
          node_id = node_id,
          run_tool=fields[0],
          run_dtm=dtm, 
          iter_nr=int(fields[1]),
          local_ip=local_ip,
          if_name=if_name,
          oper=oper,
          remote_host=fields[3],
          remote_port=int(fields[4]),
          ping_type=fields[5],
          rtt=float(fields[6]),
          m_ts=find_metadata_ts(metadata_table, if_name, dtm)
        )

        rows.append(row)

  cursor = db_conn.cursor()
  cursor.executemany(sql, rows)
  cursor.close()

def process_dwnl_test(db_conn, exp_id, sched_id, node_id, ip_to_ifname, ifname_to_oper, metadata_table):

  dwnl_test = namedtuple('dwnl_test',
    ['exp_id', 'sched_id', 'node_id', 'run_tool', 'run_dtm', 'iter_nr',
     'local_ip', 'if_name','oper', 'remote_host', 'remote_port',
     'dwnl_path', 'dwnl_size', 'init_time','total_time', 'm_ts'])

  sql = """insert into dwnl_test
    (exp_id, sched_id, node_id, run_tool, run_dtm,
    iter_nr, local_ip, if_name, oper, remote_host, remote_port,
    dwnl_path, dwnl_size, init_time, total_time, m_ts)
    values (%s, %s, %s, %s, from_unixtime(%s), %s, %s, %s, %s,
    %s, %s, %s, %s, %s, %s, from_unixtime(%s))
    """

  rows = []
  for fpath, dtm in get_matching_files(sched_id, '(?:neat-)?dwnl-test'):
    with open(fpath) as f:
      for line in f:
        fields = line.split('\t')

        local_ip = fields[2]
        if_name = ip_to_ifname.get(local_ip, None)
        oper = ifname_to_oper.get(if_name, None)

        row = dwnl_test(
          exp_id=exp_id,
          sched_id=sched_id,
          node_id = node_id,
          run_tool=fields[0],
          run_dtm=dtm,
          iter_nr=int(fields[1]),
          local_ip=local_ip,
          if_name=if_name,
          oper=oper,
          remote_host=fields[3],
          remote_port=int(fields[4]),
          dwnl_path=fields[5],
          dwnl_size=int(fields[6]),
          init_time=float(fields[7]),
          total_time=float(fields[8]),
          m_ts=find_metadata_ts(metadata_table, if_name, dtm)
        )

        rows.append(row)

  cursor = db_conn.cursor()
  cursor.executemany(sql, rows)
  cursor.close()

def main():
  db_conn = mysql.connector.connect(host=DB_HOST, user=DB_USER,
                                    password=DB_PASS,
                                    database=DB_DBASE)
  db_conn.start_transaction()

  with open(sys.argv[1]) as f:
    config = json.load(f)
    exp_id = config['id']
    for sched_id_str, sched_config in config['schedules'].iteritems():
      sched_id = int(sched_id_str)
      node_id = sched_config['nodeid'] 
      print exp_id, int(sched_id), sched_config['nodeid']

      ip_to_ifname = process_neatpmd(sched_id)
      metadata_table, ifname_to_oper = process_metadata(db_conn, exp_id, sched_id, node_id)
      process_tcp_ping(db_conn, exp_id, sched_id, node_id, ip_to_ifname, ifname_to_oper, metadata_table)
      process_dwnl_test(db_conn, exp_id, sched_id, node_id, ip_to_ifname, ifname_to_oper, metadata_table)
     
  db_conn.commit()
  db_conn.disconnect()

if __name__ == "__main__":
    main()

