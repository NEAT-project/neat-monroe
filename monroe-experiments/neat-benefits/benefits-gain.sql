-- 1) RTT gain

-- a) Overall RTT gain 

select
  count(*) as cnt_all,
  sum(if(t.run_tool = 'tcp-ping', 1, null)) as cnt,
  truncate(avg(if(t.run_tool = 'tcp-ping', t.rtt, null)), 6) as 'rtt [sec]',
  sum(if(t.run_tool = 'neat-tcp-ping', 1, null)) as cnt_neat,
  truncate(avg(if(t.run_tool = 'neat-tcp-ping', t.rtt, null)), 6) as 'rtt_neat [sec]',
  truncate(100 * sum(if(t.run_tool = 'neat-tcp-ping', 1, -1)) / sum(if(t.run_tool = 'tcp-ping', 1, 0)), 2) as 'cnt_gain [%]',
  truncate(100.0 * (avg(if(t.run_tool = 'neat-tcp-ping', t.rtt, null)) - avg(if(t.run_tool = 'tcp-ping', t.rtt, null))) / avg(if(t.run_tool = 'tcp-ping', t.rtt, null)), 2) as 'rtt_gain [%]'
from
  tcp_ping t
where
  t.exp_id in (26555, 26557, 26561, 26558, 26559, 26573) and
  t.ping_type = 'ECHO'
;

-- b) RTT gain per experiment

select
  t.exp_id as exp_id,
  count(*) as cnt_all,
  sum(if(t.run_tool = 'tcp-ping', 1, null)) as cnt,
  truncate(avg(if(t.run_tool = 'tcp-ping', t.rtt, null)), 6) as 'rtt [sec]',
  sum(if(t.run_tool = 'neat-tcp-ping', 1, null)) as cnt_neat,
  truncate(avg(if(t.run_tool = 'neat-tcp-ping', t.rtt, null)), 6) as 'rtt_neat [sec]',
  truncate(100 * sum(if(t.run_tool = 'neat-tcp-ping', 1, -1)) / sum(if(t.run_tool = 'tcp-ping', 1, 0)), 2) as 'cnt_gain [%]',
  truncate(100.0 * (avg(if(t.run_tool = 'neat-tcp-ping', t.rtt, null)) - avg(if(t.run_tool = 'tcp-ping', t.rtt, null))) / avg(if(t.run_tool = 'tcp-ping', t.rtt, null)), 2) as 'rtt_gain [%]'
from
  tcp_ping t
where
  t.exp_id in (26555, 26557, 26561, 26558, 26559, 26573) and
  t.ping_type = 'ECHO'
group by
  t.exp_id
;

-- c) RTT gain per node

select
  group_concat(distinct t.exp_id) as exp_id,
  sched_id as sched_id,
  group_concat(distinct t.node_id) as node_id,
  count(*) as cnt_all,
  sum(if(t.run_tool = 'tcp-ping', 1, null)) as cnt,
  truncate(avg(if(t.run_tool = 'tcp-ping', t.rtt, null)), 6) as 'rtt [sec]',
  sum(if(t.run_tool = 'neat-tcp-ping', 1, null)) as cnt_neat,
  truncate(avg(if(t.run_tool = 'neat-tcp-ping', t.rtt, null)), 6) as 'rtt_neat [sec]',
  truncate(100 * sum(if(t.run_tool = 'neat-tcp-ping', 1, -1)) / sum(if(t.run_tool = 'tcp-ping', 1, 0)), 2) as 'cnt_gain [%]',
  truncate(100.0 * (avg(if(t.run_tool = 'neat-tcp-ping', t.rtt, null)) - avg(if(t.run_tool = 'tcp-ping', t.rtt, null))) / avg(if(t.run_tool = 'tcp-ping', t.rtt, null)), 2) as 'rtt_gain [%]'
from
  tcp_ping t
where
  t.exp_id in (26555, 26557, 26561, 26558, 26559, 26573) and
  t.ping_type = 'ECHO'
group by
  t.sched_id
;

-- 2) Download speed gain

-- a) Overall download speed gain

select
  count(*) as cnt_all,
  sum(if(t.run_tool = 'dwnl-test', 1, 0)) as cnt,
  sum(if(t.run_tool = 'dwnl-test', t.dwnl_size, 0)) / (1024 * 1024) as 'size [MB]',
  truncate(sum(if(t.run_tool = 'dwnl-test', t.dwnl_size, null)) / sum(if(t.run_tool = 'dwnl-test', t.total_time, null)), 1) / (1024 * 1024) as 'speed [MB/s]',
  sum(if(t.run_tool = 'neat-dwnl-test', 1, 0)) as cnt_neat,
  sum(if(t.run_tool = 'neat-dwnl-test', t.dwnl_size, 0)) / (1024 * 1024) as 'size_neat [MB]',
  truncate(sum(if(t.run_tool = 'neat-dwnl-test', t.dwnl_size, null)) / sum(if(t.run_tool = 'neat-dwnl-test', t.total_time, null)), 1) / (1024 * 1024) as 'speed_neat [MB/s]',
  truncate(100 * sum(if(t.run_tool = 'neat-dwnl-test', 1, -1)) / sum(if(t.run_tool = 'dwnl-test', 1, 0)), 2) as 'cnt_gain [%]',
  truncate(100 * (sum(if(t.run_tool = 'neat-dwnl-test', t.dwnl_size, 0)) - sum(if(t.run_tool = 'dwnl-test', t.dwnl_size, 0))) / sum(if(t.run_tool = 'dwnl-test', t.dwnl_size, 0)), 2) as 'size_gain [%]',
  truncate(100 * (sum(if(t.run_tool = 'neat-dwnl-test', t.dwnl_size, null)) / sum(if(t.run_tool = 'neat-dwnl-test', t.total_time, null)) - sum(if(t.run_tool = 'dwnl-test', t.dwnl_size, null)) / sum(if(t.run_tool = 'dwnl-test', t.total_time, null))) / (sum(if(t.run_tool = 'dwnl-test', t.dwnl_size, null)) / sum(if(t.run_tool = 'dwnl-test', t.total_time, null))), 2) as 'speed_gain [%]'
from
  dwnl_test t
where
  t.exp_id in (26555, 26557, 26561, 26558, 26559, 26573)
;

-- b) Download speed gain per experiment

select
  group_concat(distinct t.exp_id) as exp_id,
  count(*) as cnt_all,
  sum(if(t.run_tool = 'dwnl-test', 1, 0)) as cnt,
  sum(if(t.run_tool = 'dwnl-test', t.dwnl_size, 0)) / (1024 * 1024) as 'size [MB]',
  truncate(sum(if(t.run_tool = 'dwnl-test', t.dwnl_size, null)) / sum(if(t.run_tool = 'dwnl-test', t.total_time, null)), 1) / (1024 * 1024) as 'speed [MB/s]',
  sum(if(t.run_tool = 'neat-dwnl-test', 1, 0)) as cnt_neat,
  sum(if(t.run_tool = 'neat-dwnl-test', t.dwnl_size, 0)) / (1024 * 1024) as 'size_neat [MB]',
  truncate(sum(if(t.run_tool = 'neat-dwnl-test', t.dwnl_size, null)) / sum(if(t.run_tool = 'neat-dwnl-test', t.total_time, null)), 1) / (1024 * 1024) as 'speed_neat [MB/s]',
  truncate(100 * sum(if(t.run_tool = 'neat-dwnl-test', 1, -1)) / sum(if(t.run_tool = 'dwnl-test', 1, 0)), 2) as 'cnt_gain [%]',
  truncate(100 * (sum(if(t.run_tool = 'neat-dwnl-test', t.dwnl_size, 0)) - sum(if(t.run_tool = 'dwnl-test', t.dwnl_size, 0))) / sum(if(t.run_tool = 'dwnl-test', t.dwnl_size, 0)), 2) as 'size_gain [%]',
  truncate(100 * (sum(if(t.run_tool = 'neat-dwnl-test', t.dwnl_size, null)) / sum(if(t.run_tool = 'neat-dwnl-test', t.total_time, null)) - sum(if(t.run_tool = 'dwnl-test', t.dwnl_size, null)) / sum(if(t.run_tool = 'dwnl-test', t.total_time, null))) / (sum(if(t.run_tool = 'dwnl-test', t.dwnl_size, null)) / sum(if(t.run_tool = 'dwnl-test', t.total_time, null))), 2) as 'speed_gain [%]'
from
  dwnl_test t
where
  t.exp_id in (26555, 26557, 26561, 26558, 26559, 26573)
group by
  t.exp_id
;

-- c) Download speed gain per node

select
  group_concat(distinct t.exp_id) as exp_id,
  sched_id as sched_id,
  group_concat(distinct t.node_id) as node_id,
  count(*) as cnt_all,
  sum(if(t.run_tool = 'dwnl-test', 1, 0)) as cnt,
  sum(if(t.run_tool = 'dwnl-test', t.dwnl_size, 0)) / (1024 * 1024) as 'size [MB]',
  truncate(sum(if(t.run_tool = 'dwnl-test', t.dwnl_size, null)) / sum(if(t.run_tool = 'dwnl-test', t.total_time, null)), 1) / (1024 * 1024) as 'speed [MB/s]',
  sum(if(t.run_tool = 'neat-dwnl-test', 1, 0)) as cnt_neat,
  sum(if(t.run_tool = 'neat-dwnl-test', t.dwnl_size, 0)) / (1024 * 1024) as 'size_neat [MB]',
  truncate(sum(if(t.run_tool = 'neat-dwnl-test', t.dwnl_size, null)) / sum(if(t.run_tool = 'neat-dwnl-test', t.total_time, null)), 1) / (1024 * 1024) as 'speed_neat [MB/s]',
  truncate(100 * sum(if(t.run_tool = 'neat-dwnl-test', 1, -1)) / sum(if(t.run_tool = 'dwnl-test', 1, 0)), 2) as 'cnt_gain [%]',
  truncate(100 * (sum(if(t.run_tool = 'neat-dwnl-test', t.dwnl_size, 0)) - sum(if(t.run_tool = 'dwnl-test', t.dwnl_size, 0))) / sum(if(t.run_tool = 'dwnl-test', t.dwnl_size, 0)), 2) as 'size_gain [%]',
  truncate(100 * (sum(if(t.run_tool = 'neat-dwnl-test', t.dwnl_size, null)) / sum(if(t.run_tool = 'neat-dwnl-test', t.total_time, null)) - sum(if(t.run_tool = 'dwnl-test', t.dwnl_size, null)) / sum(if(t.run_tool = 'dwnl-test', t.total_time, null))) / (sum(if(t.run_tool = 'dwnl-test', t.dwnl_size, null)) / sum(if(t.run_tool = 'dwnl-test', t.total_time, null))), 2) as 'speed_gain [%]'
from
  dwnl_test t
where
  t.exp_id in (26555, 26557, 26561, 26558, 26559, 26573)
group by
  t.sched_id
;

