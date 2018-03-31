set @exp_batch = 'policy-3';

-- Overall RTT gain

select 
  concat(e.exp_name, ' (', e.exp_dtm, ')') as exp_name,
  sum(t.cnt_rtt) as cnt,
  round(sum(t.cnt_rtt * t.rtt) / sum(t.cnt_rtt) * 1000.0, 1) as `rtt [ms]`,
  round(sum(t.cnt * t.init_time) / sum(t.cnt) * 1000.0, 1) as `init [ms]`,
  sum(t.cnt_rtt_neat) as cnt_neat,
  round(sum(t.cnt_rtt_neat * t.rtt_neat) / sum(t.cnt_rtt_neat) * 1000.0, 1) as `rtt_neat [ms]`,
  round(sum(t.cnt_neat * t.init_time_neat) / sum(t.cnt_neat) * 1000.0, 1) as `init_neat [ms]`,
--  round(100.0 * sum(t.cnt_rtt_neat) / sum(t.cnt_rtt) - 100.0, 2) as `cnt_gain [%]`,
  round(100.0 * (sum(t.cnt_rtt_neat * t.rtt_neat) / sum(t.cnt_rtt_neat)) / (sum(t.cnt_rtt * t.rtt) / sum(t.cnt_rtt)) - 100.0, 2) as `rtt_gain [%]`,
  round(100.0 * avg(if(t.rtt_neat < t.rtt, 1, 0)), 2) as `win_cnt [%]`
--  round(100.0 * avg(t.rtt_gain), 2) as `avg_rtt_gain [%]`
from
  tcp_ping_avg t
  left join experiment e on t.exp_id = e.exp_id
where
  e.batch = @exp_batch
group by
  t.exp_id
union all select
  'TOTAL',
  sum(t.cnt_rtt),
  round(sum(t.cnt_rtt * t.rtt) / sum(t.cnt_rtt) * 1000.0, 1),
  round(sum(t.cnt * t.init_time) / sum(t.cnt) * 1000.0, 1),
  sum(t.cnt_rtt_neat),
  round(sum(t.cnt_rtt_neat * t.rtt_neat) / sum(t.cnt_rtt_neat) * 1000.0, 1),
  round(sum(t.cnt_neat * t.init_time_neat) / sum(t.cnt_neat) * 1000.0, 1),
--  round(100.0 * sum(t.cnt_rtt_neat) / sum(t.cnt_rtt) - 100.0, 2),
  round(100.0 * (sum(t.cnt_rtt_neat * t.rtt_neat) / sum(t.cnt_rtt_neat)) / (sum(t.cnt_rtt * t.rtt) / sum(t.cnt_rtt)) - 100.0, 2),
  round(100.0 * avg(if(t.rtt_neat < t.rtt, 1, 0)), 2)
--  round(100.0 * avg(t.rtt_gain), 2)
from
  tcp_ping_avg t
  left join experiment e on t.exp_id = e.exp_id
where
  e.batch = @exp_batch
;

-- Overall Download speed gain

select
  concat(e.exp_name, ' (', e.exp_dtm, ')') as exp_name,
  sum(t.cnt_dwnl) as cnt,
  round(sum(t.dwnl_size) / 1048576, 0) as `size [MB]`,
  round((sum(t.dwnl_size) / sum(t.dwnl_time)) / 1048576, 2) as `speed [MB/s]`,
  round(sum(t.cnt * t.init_time) / sum(t.cnt) * 1000.0, 1) as `init [ms]`,
  sum(t.cnt_dwnl_neat) as cnt_neat,
  round(sum(t.dwnl_size_neat) / 1048576, 0) as `size_neat [MB]`,
  round((sum(t.dwnl_size_neat) / sum(t.dwnl_time_neat)) / 1048576, 2) as `speed_neat [MB/s]`,
  round(sum(t.cnt_neat * t.init_time_neat) / sum(t.cnt_neat) * 1000.0, 1) as `init_neat [ms]`,
--  round(100.0 * sum(t.cnt_dwnl_neat) / sum(t.cnt_dwnl) - 100.0, 2) as `cnt_gain [%]`,
  round(100.0 * sum(t.dwnl_size_neat) / sum(t.dwnl_size) - 100.0, 2) as `size_gain [%]`,
  round(100.0 * (sum(t.dwnl_size_neat) / sum(t.dwnl_time_neat)) / (sum(t.dwnl_size) / sum(t.dwnl_time)) - 100.0, 2) as `speed_gain [%]`,
  round(100.0 * avg(if(t.dwnl_speed_neat > t.dwnl_speed, 1, 0)), 2) as `win_cnt [%]`
--  round(100.0 * avg(t.speed_gain), 2) as `avg_speed_gain [%]`
from
  dwnl_test_avg t
  left join experiment e on t.exp_id = e.exp_id
where
  e.batch = @exp_batch
group by
  t.exp_id
union all select
  'TOTAL',
  sum(t.cnt_dwnl),
  round(sum(t.dwnl_size) / 1048576, 0),
  round((sum(t.dwnl_size) / sum(t.dwnl_time)) / 1048576, 2),
  round(sum(t.cnt * t.init_time) / sum(t.cnt) * 1000.0, 1),
  sum(t.cnt_dwnl_neat),
  round(sum(t.dwnl_size_neat) / 1048576, 0),
  round((sum(t.dwnl_size_neat) / sum(t.dwnl_time_neat)) / 1048576, 2),
  round(sum(t.cnt_neat * t.init_time_neat) / sum(t.cnt_neat) * 1000.0, 1),
--  round(100.0 * sum(t.cnt_dwnl_neat) / sum(t.cnt_dwnl) - 100.0, 2),
  round(100.0 * sum(t.dwnl_size_neat) / sum(t.dwnl_size) - 100.0, 2),
  round(100.0 * (sum(t.dwnl_size_neat) / sum(t.dwnl_time_neat)) / (sum(t.dwnl_size) / sum(t.dwnl_time)) - 100.0, 2),
  round(100.0 * avg(if(t.dwnl_speed_neat > t.dwnl_speed, 1, 0)), 2)
--  round(100.0 * avg(t.speed_gain), 2)
from
  dwnl_test_avg t
  left join experiment e on t.exp_id = e.exp_id
where
  e.batch = @exp_batch
;

-- Interface selected by NEAT for download/ping in relation to default

select
  concat(e.exp_name, ' (', e.exp_dtm, ')') as exp_name,
  count(*) as cnt,
  round(100.0 * sum(if((if(t.cnt_op0 = t.cnt, t.cnt_op1_neat, t.cnt_op0_neat) / t.cnt_neat) between 0.0 and 0.0001, 1, 0)) / count(*), 2) as `no diff [%]`,
  round(100.0 * sum(if((if(t.cnt_op0 = t.cnt, t.cnt_op1_neat, t.cnt_op0_neat) / t.cnt_neat) between 0.0001 and 0.2 ,1, 0)) / count(*), 2) as `0%-20% [%]`,
  round(100.0 * sum(if((if(t.cnt_op0 = t.cnt, t.cnt_op1_neat, t.cnt_op0_neat) / t.cnt_neat) between 0.2001 and 0.4, 1, 0)) / count(*), 2) as `20%-40% [%]`,
  round(100.0 * sum(if((if(t.cnt_op0 = t.cnt, t.cnt_op1_neat, t.cnt_op0_neat) / t.cnt_neat) between 0.4001 and 0.6, 1, 0)) / count(*), 2) as `40%-60% [%]`,
  round(100.0 * sum(if((if(t.cnt_op0 = t.cnt, t.cnt_op1_neat, t.cnt_op0_neat) / t.cnt_neat) between 0.6001 and 0.8, 1, 0)) / count(*), 2) as `60%-80% [%]`,
  round(100.0 * sum(if((if(t.cnt_op0 = t.cnt, t.cnt_op1_neat, t.cnt_op0_neat) / t.cnt_neat) between 0.8001 and 1.0, 1, 0)) / count(*), 2) as `>80% [%]`
from
  v_tool_avg t
  left join experiment e on t.exp_id = e.exp_id
where
  e.batch = @exp_batch
group by
  t.exp_id  
union all select
  'TOTAL',
  count(*),
  round(100.0 * sum(if((if(t.cnt_op0 = t.cnt, t.cnt_op1_neat, t.cnt_op0_neat) / t.cnt_neat) between 0.0 and 0.0001, 1, 0)) / count(*), 2),
  round(100.0 * sum(if((if(t.cnt_op0 = t.cnt, t.cnt_op1_neat, t.cnt_op0_neat) / t.cnt_neat) between 0.0001 and 0.2 ,1, 0)) / count(*), 2),
  round(100.0 * sum(if((if(t.cnt_op0 = t.cnt, t.cnt_op1_neat, t.cnt_op0_neat) / t.cnt_neat) between 0.2001 and 0.4, 1, 0)) / count(*), 2),
  round(100.0 * sum(if((if(t.cnt_op0 = t.cnt, t.cnt_op1_neat, t.cnt_op0_neat) / t.cnt_neat) between 0.4001 and 0.6, 1, 0)) / count(*), 2),
  round(100.0 * sum(if((if(t.cnt_op0 = t.cnt, t.cnt_op1_neat, t.cnt_op0_neat) / t.cnt_neat) between 0.6001 and 0.8, 1, 0)) / count(*), 2),
  round(100.0 * sum(if((if(t.cnt_op0 = t.cnt, t.cnt_op1_neat, t.cnt_op0_neat) / t.cnt_neat) between 0.8001 and 1.0, 1, 0)) / count(*), 2)
from
  v_tool_avg t
  left join experiment e on t.exp_id = e.exp_id
where
  e.batch = @exp_batch
;

-- RTT gain for nodes on which at least 20% of NEAT downloads/pings on non-default interface

select 
  concat(e.exp_name, ' (', e.exp_dtm, ')') as exp_name,
  sum(t.cnt_rtt) as cnt,
  round(sum(t.cnt_rtt * t.rtt) / sum(t.cnt_rtt) * 1000.0, 1) as `rtt [ms]`,
  round(sum(t.cnt * t.init_time) / sum(t.cnt) * 1000.0, 1) as `init [ms]`,
  sum(t.cnt_rtt_neat) as cnt_neat,
  round(sum(t.cnt_rtt_neat * t.rtt_neat) / sum(t.cnt_rtt_neat) * 1000.0, 1) as `rtt_neat [ms]`,
  round(sum(t.cnt_neat * t.init_time_neat) / sum(t.cnt_neat) * 1000.0, 1) as `init_neat [ms]`,
--  round(100.0 * sum(t.cnt_rtt_neat) / sum(t.cnt_rtt) - 100.0, 2) as `cnt_gain [%]`,
  round(100.0 * (sum(t.cnt_rtt_neat * t.rtt_neat) / sum(t.cnt_rtt_neat)) / (sum(t.cnt_rtt * t.rtt) / sum(t.cnt_rtt)) - 100.0, 2) as `rtt_gain [%]`,
  round(100.0 * avg(if(t.rtt_neat < t.rtt, 1, 0)), 2) as `win_cnt [%]`
--  round(100.0 * avg(t.rtt_gain), 2) as `avg_rtt_gain [%]`
from
  tcp_ping_avg t
  left join experiment e on t.exp_id = e.exp_id
  left join v_tool_avg v on t.sched_id = v.sched_id
where
  if(v.cnt_op0 = v.cnt, v.cnt_op1_neat, v.cnt_op0_neat) / v.cnt_neat > 0.2 and
  e.batch = @exp_batch
group by
  t.exp_id
union all select
  'TOTAL',
  sum(t.cnt_rtt),
  round(sum(t.cnt_rtt * t.rtt) / sum(t.cnt_rtt) * 1000.0, 1),
  round(sum(t.cnt * t.init_time) / sum(t.cnt) * 1000.0, 1),
  sum(t.cnt_rtt_neat),
  round(sum(t.cnt_rtt_neat * t.rtt_neat) / sum(t.cnt_rtt_neat) * 1000.0, 1),
  round(sum(t.cnt_neat * t.init_time_neat) / sum(t.cnt_neat) * 1000.0, 1),
--  round(100.0 * sum(t.cnt_rtt_neat) / sum(t.cnt_rtt) - 100.0, 2),
  round(100.0 * (sum(t.cnt_rtt_neat * t.rtt_neat) / sum(t.cnt_rtt_neat)) / (sum(t.cnt_rtt * t.rtt) / sum(t.cnt_rtt)) - 100.0, 2),
  round(100.0 * avg(if(t.rtt_neat < t.rtt, 1, 0)), 2)
--  round(100.0 * avg(t.rtt_gain), 2)
from
  tcp_ping_avg t
  left join experiment e on t.exp_id = e.exp_id
  left join v_tool_avg v on t.sched_id = v.sched_id
where
  if(v.cnt_op0 = v.cnt, v.cnt_op1_neat, v.cnt_op0_neat) / v.cnt_neat > 0.2 and
  e.batch = @exp_batch
;

-- Download speed gain for nodes on which at least 20% of NEAT downloads/pings on non-default interface

select
  concat(e.exp_name, ' (', e.exp_dtm, ')') as exp_name,
  sum(t.cnt_dwnl) as cnt,
  round(sum(t.dwnl_size) / 1048576, 0) as `size [MB]`,
  round((sum(t.dwnl_size) / sum(t.dwnl_time)) / 1048576, 2) as `speed [MB/s]`,
  round(sum(t.cnt * t.init_time) / sum(t.cnt) * 1000.0, 1) as `init [ms]`,
  sum(t.cnt_dwnl_neat) as cnt_neat,
  round(sum(t.dwnl_size_neat) / 1048576, 0) as `size_neat [MB]`,
  round((sum(t.dwnl_size_neat) / sum(t.dwnl_time_neat)) / 1048576, 2) as `speed_neat [MB/s]`,
  round(sum(t.cnt_neat * t.init_time_neat) / sum(t.cnt_neat) * 1000.0, 1) as `init_neat [ms]`,
--  round(100.0 * sum(t.cnt_dwnl_neat) / sum(t.cnt_dwnl) - 100.0, 2) as `cnt_gain [%]`,
  round(100.0 * sum(t.dwnl_size_neat) / sum(t.dwnl_size) - 100.0, 2) as `size_gain [%]`,
  round(100.0 * (sum(t.dwnl_size_neat) / sum(t.dwnl_time_neat)) / (sum(t.dwnl_size) / sum(t.dwnl_time)) - 100.0, 2) as `speed_gain [%]`,
  round(100.0 * avg(if(t.dwnl_speed_neat > t.dwnl_speed, 1, 0)), 2) as `win_cnt [%]`
--  round(100.0 * avg(t.speed_gain), 2) as `avg_speed_gain [%]`
from
  dwnl_test_avg t
  left join experiment e on t.exp_id = e.exp_id
  left join v_tool_avg v on t.sched_id = v.sched_id
where
  if(v.cnt_op0 = v.cnt, v.cnt_op1_neat, v.cnt_op0_neat) / v.cnt_neat > 0.2 and
  e.batch = @exp_batch
group by
  t.exp_id
union all select
  'TOTAL',
  sum(t.cnt_dwnl),
  round(sum(t.dwnl_size) / 1048576, 0),
  round((sum(t.dwnl_size) / sum(t.dwnl_time)) / 1048576, 2),
  round(sum(t.cnt * t.init_time) / sum(t.cnt) * 1000.0, 1),
  sum(t.cnt_dwnl_neat),
  round(sum(t.dwnl_size_neat) / 1048576, 0),
  round((sum(t.dwnl_size_neat) / sum(t.dwnl_time_neat)) / 1048576, 2),
  round(sum(t.cnt_neat * t.init_time_neat) / sum(t.cnt_neat) * 1000.0, 1),
--  round(100.0 * sum(t.cnt_dwnl_neat) / sum(t.cnt_dwnl) - 100.0, 2),
  round(100.0 * sum(t.dwnl_size_neat) / sum(t.dwnl_size) - 100.0, 2),
  round(100.0 * (sum(t.dwnl_size_neat) / sum(t.dwnl_time_neat)) / (sum(t.dwnl_size) / sum(t.dwnl_time)) - 100.0, 2),
  round(100.0 * avg(if(t.dwnl_speed_neat > t.dwnl_speed, 1, 0)), 2)
--  round(100.0 * avg(t.speed_gain), 2)
from
  dwnl_test_avg t
  left join experiment e on t.exp_id = e.exp_id
  left join v_tool_avg v on t.sched_id = v.sched_id
where
  if(v.cnt_op0 = v.cnt, v.cnt_op1_neat, v.cnt_op0_neat) / v.cnt_neat > 0.2 and
  e.batch = @exp_batch
;

-- RTT gain for nodes on which all NEAT downloads/pings on default interface

select 
  concat(e.exp_name, ' (', e.exp_dtm, ')') as exp_name,
  sum(t.cnt_rtt) as cnt,
  round(sum(t.cnt_rtt * t.rtt) / sum(t.cnt_rtt) * 1000.0, 1) as `rtt [ms]`,
  round(sum(t.cnt * t.init_time) / sum(t.cnt) * 1000.0, 1) as `init [ms]`,
  sum(t.cnt_rtt_neat) as cnt_neat,
  round(sum(t.cnt_rtt_neat * t.rtt_neat) / sum(t.cnt_rtt_neat) * 1000.0, 1) as `rtt_neat [ms]`,
  round(sum(t.cnt_neat * t.init_time_neat) / sum(t.cnt_neat) * 1000.0, 1) as `init_neat [ms]`,
--  round(100.0 * sum(t.cnt_rtt_neat) / sum(t.cnt_rtt) - 100.0, 2) as `cnt_gain [%]`,
  round(100.0 * (sum(t.cnt_rtt_neat * t.rtt_neat) / sum(t.cnt_rtt_neat)) / (sum(t.cnt_rtt * t.rtt) / sum(t.cnt_rtt)) - 100.0, 2) as `rtt_gain [%]`,
  round(100.0 * avg(if(t.rtt_neat < t.rtt, 1, 0)), 2) as `win_cnt [%]`
--  round(100.0 * avg(t.rtt_gain), 2) as `avg_rtt_gain [%]`
from
  tcp_ping_avg t
  left join experiment e on t.exp_id = e.exp_id
  left join v_tool_avg v on t.sched_id = v.sched_id
where
  if(v.cnt_op0 = v.cnt, v.cnt_op1_neat, v.cnt_op0_neat) / v.cnt_neat = 0.0 and
  e.batch = @exp_batch
group by
  t.exp_id
union all select
  'TOTAL',
  sum(t.cnt_rtt),
  round(sum(t.cnt_rtt * t.rtt) / sum(t.cnt_rtt) * 1000.0, 1),
  round(sum(t.cnt * t.init_time) / sum(t.cnt) * 1000.0, 1),
  sum(t.cnt_rtt_neat),
  round(sum(t.cnt_rtt_neat * t.rtt_neat) / sum(t.cnt_rtt_neat) * 1000.0, 1),
  round(sum(t.cnt_neat * t.init_time_neat) / sum(t.cnt_neat) * 1000.0, 1),
--  round(100.0 * sum(t.cnt_rtt_neat) / sum(t.cnt_rtt) - 100.0, 2),
  round(100.0 * (sum(t.cnt_rtt_neat * t.rtt_neat) / sum(t.cnt_rtt_neat)) / (sum(t.cnt_rtt * t.rtt) / sum(t.cnt_rtt)) - 100.0, 2),
  round(100.0 * avg(if(t.rtt_neat < t.rtt, 1, 0)), 2)
--  round(100.0 * avg(t.rtt_gain), 2)
from
  tcp_ping_avg t
  left join experiment e on t.exp_id = e.exp_id
  left join v_tool_avg v on t.sched_id = v.sched_id
where
  if(v.cnt_op0 = v.cnt, v.cnt_op1_neat, v.cnt_op0_neat) / v.cnt_neat = 0.0 and
  e.batch = @exp_batch
;

-- Download speed gain for nodes on which all NEAT downloads/pings on default interface

select
  concat(e.exp_name, ' (', e.exp_dtm, ')') as exp_name,
  sum(t.cnt_dwnl) as cnt,
  round(sum(t.dwnl_size) / 1048576, 0) as `size [MB]`,
  round((sum(t.dwnl_size) / sum(t.dwnl_time)) / 1048576, 2) as `speed [MB/s]`,
  round(sum(t.cnt * t.init_time) / sum(t.cnt) * 1000.0, 1) as `init [ms]`,
  sum(t.cnt_dwnl_neat) as cnt_neat,
  round(sum(t.dwnl_size_neat) / 1048576, 0) as `size_neat [MB]`,
  round((sum(t.dwnl_size_neat) / sum(t.dwnl_time_neat)) / 1048576, 2) as `speed_neat [MB/s]`,
  round(sum(t.cnt_neat * t.init_time_neat) / sum(t.cnt_neat) * 1000.0, 1) as `init_neat [ms]`,
--  round(100.0 * sum(t.cnt_dwnl_neat) / sum(t.cnt_dwnl) - 100.0, 2) as `cnt_gain [%]`,
  round(100.0 * sum(t.dwnl_size_neat) / sum(t.dwnl_size) - 100.0, 2) as `size_gain [%]`,
  round(100.0 * (sum(t.dwnl_size_neat) / sum(t.dwnl_time_neat)) / (sum(t.dwnl_size) / sum(t.dwnl_time)) - 100.0, 2) as `speed_gain [%]`,
  round(100.0 * avg(if(t.dwnl_speed_neat > t.dwnl_speed, 1, 0)), 2) as `win_cnt [%]`
--  round(100.0 * avg(t.speed_gain), 2) as `avg_speed_gain [%]`
from
  dwnl_test_avg t
  left join experiment e on t.exp_id = e.exp_id
  left join v_tool_avg v on t.sched_id = v.sched_id
where
  if(v.cnt_op0 = v.cnt, v.cnt_op1_neat, v.cnt_op0_neat) / v.cnt_neat = 0.0 and
  e.batch = @exp_batch
group by
  t.exp_id
union all select
  'TOTAL',
  sum(t.cnt_dwnl),
  round(sum(t.dwnl_size) / 1048576, 0),
  round((sum(t.dwnl_size) / sum(t.dwnl_time)) / 1048576, 2),
  round(sum(t.cnt * t.init_time) / sum(t.cnt) * 1000.0, 1),
  sum(t.cnt_dwnl_neat),
  round(sum(t.dwnl_size_neat) / 1048576, 0),
  round((sum(t.dwnl_size_neat) / sum(t.dwnl_time_neat)) / 1048576, 2),
  round(sum(t.cnt_neat * t.init_time_neat) / sum(t.cnt_neat) * 1000.0, 1),
--  round(100.0 * sum(t.cnt_dwnl_neat) / sum(t.cnt_dwnl) - 100.0, 2),
  round(100.0 * sum(t.dwnl_size_neat) / sum(t.dwnl_size) - 100.0, 2),
  round(100.0 * (sum(t.dwnl_size_neat) / sum(t.dwnl_time_neat)) / (sum(t.dwnl_size) / sum(t.dwnl_time)) - 100.0, 2),
  round(100.0 * avg(if(t.dwnl_speed_neat > t.dwnl_speed, 1, 0)), 2)
--  round(100.0 * avg(t.speed_gain), 2)
from
  dwnl_test_avg t
  left join experiment e on t.exp_id = e.exp_id
  left join v_tool_avg v on t.sched_id = v.sched_id
where
  if(v.cnt_op0 = v.cnt, v.cnt_op1_neat, v.cnt_op0_neat) / v.cnt_neat = 0.0 and
  e.batch = @exp_batch
;

-- Distibution of mode in metadata

select 
  concat(e.exp_name, ' (', e.exp_dtm, ')') as exp_name,
  round(100.0 * sum(if(m.device_mode = 'LTE', m.span, 0)) / sum(m.span), 2) as `lte [%]`,
  round(100.0 * sum(if(m.device_mode = '3G', m.span, 0)) / sum(m.span), 2) as `3g [%]`,
  round(100.0 * sum(if(m.device_mode = '2G', m.span, 0)) / sum(m.span), 2) as `2g [%]`,
  round(100.0 * sum(if(m.device_mode = 'NO SERVICE', m.span, 0)) / sum(m.span), 2) as `no_service [%]`,
  round(100.0 * sum(if(m.device_mode = 'DISCONNECTED', m.span, 0)) / sum(m.span), 2) as `disconnected [%]`,
  round(100.0 * sum(if(m.device_mode = 'UNKNOWN', m.span, 0)) / sum(m.span), 2) as `unknown [%]`
from
  v_metadata m
  left join experiment e on m.exp_id = e.exp_id
where
  e.batch = @exp_batch
group by
  m.exp_id
union all select 
  'TOTAL',
  round(100.0 * sum(if(m.device_mode = 'LTE', m.span, 0)) / sum(m.span), 2),
  round(100.0 * sum(if(m.device_mode = '3G', m.span, 0)) / sum(m.span), 2),
  round(100.0 * sum(if(m.device_mode = '2G', m.span, 0)) / sum(m.span), 2),
  round(100.0 * sum(if(m.device_mode = 'NO SERVICE', m.span, 0)) / sum(m.span), 2),
  round(100.0 * sum(if(m.device_mode = 'DISCONNECTED', m.span, 0)) / sum(m.span), 2),
  round(100.0 * sum(if(m.device_mode = 'UNKNOWN', m.span, 0)) / sum(m.span), 2)
from
  v_metadata m
  left join experiment e on m.exp_id = e.exp_id
where
  e.batch = @exp_batch
;

-- Mode selected for download/ping non-neat vs neat

select
  concat(e.exp_name, ' (', e.exp_dtm, ')') as exp_name,
  sum(t.cnt) as cnt,
  round(100.0 * sum(t.cnt_lte) / sum(t.cnt), 2) as `lte [%]`,
  round(100.0 * sum(t.cnt_3g) / sum(t.cnt), 2) as `3g [%]`,
  round(100.0 * sum(t.cnt_2g) / sum(t.cnt), 2) as `2g [%]`,
  sum(t.cnt_neat) as cnt_neat,
  round(100.0 * sum(t.cnt_lte_neat) / sum(t.cnt_neat), 2) as `lte_neat [%]`,
  round(100.0 * sum(t.cnt_3g_neat) / sum(t.cnt_neat), 2) as `3g_neat [%]`,
  round(100.0 * sum(t.cnt_2g_neat) / sum(t.cnt_neat), 2) as `2g_neat [%]`
from
  v_tool_avg t
  left join experiment e on t.exp_id = e.exp_id
where
  e.batch = @exp_batch
group by
  t.exp_id
union all select
  'TOTAL',
  sum(t.cnt),
  round(100.0 * sum(t.cnt_lte) / sum(t.cnt), 2),
  round(100.0 * sum(t.cnt_3g) / sum(t.cnt), 2),
  round(100.0 * sum(t.cnt_2g) / sum(t.cnt), 2),
  sum(t.cnt_neat),
  round(100.0 * sum(t.cnt_lte_neat) / sum(t.cnt_neat), 2),
  round(100.0 * sum(t.cnt_3g_neat) / sum(t.cnt_neat), 2),
  round(100.0 * sum(t.cnt_2g_neat) / sum(t.cnt_neat), 2)
from
  v_tool_avg t
  left join experiment e on t.exp_id = e.exp_id
where
  e.batch = @exp_batch
;

-- RTT gain for nodes on which at least one download/ping in non-LTE mode

select 
  concat(e.exp_name, ' (', e.exp_dtm, ')') as exp_name,
  sum(t.cnt_rtt) as cnt,
  round(sum(t.cnt_rtt * t.rtt) / sum(t.cnt_rtt) * 1000.0, 1) as `rtt [ms]`,
  round(sum(t.cnt * t.init_time) / sum(t.cnt) * 1000.0, 1) as `init [ms]`,
  sum(t.cnt_rtt_neat) as cnt_neat,
  round(sum(t.cnt_rtt_neat * t.rtt_neat) / sum(t.cnt_rtt_neat) * 1000.0, 1) as `rtt_neat [ms]`,
  round(sum(t.cnt_neat * t.init_time_neat) / sum(t.cnt_neat) * 1000.0, 1) as `init_neat [ms]`,
--  round(100.0 * sum(t.cnt_rtt_neat) / sum(t.cnt_rtt) - 100.0, 2) as `cnt_gain [%]`,
  round(100.0 * (sum(t.cnt_rtt_neat * t.rtt_neat) / sum(t.cnt_rtt_neat)) / (sum(t.cnt_rtt * t.rtt) / sum(t.cnt_rtt)) - 100.0, 2) as `rtt_gain [%]`,
  round(100.0 * avg(if(t.rtt_neat < t.rtt, 1, 0)), 2) as `win_cnt [%]`
--  round(100.0 * avg(t.rtt_gain), 2) as `avg_rtt_gain [%]`
from
  tcp_ping_avg t
  left join experiment e on t.exp_id = e.exp_id
  left join v_tool_avg v on t.sched_id = v.sched_id
where
  (v.cnt_lte != v.cnt or v.cnt_lte_neat != v.cnt_neat) and
  e.batch = @exp_batch
group by
  t.exp_id
union all select
  'TOTAL',
  sum(t.cnt_rtt),
  round(sum(t.cnt_rtt * t.rtt) / sum(t.cnt_rtt) * 1000.0, 1),
  round(sum(t.cnt * t.init_time) / sum(t.cnt) * 1000.0, 1),
  sum(t.cnt_rtt_neat),
  round(sum(t.cnt_rtt_neat * t.rtt_neat) / sum(t.cnt_rtt_neat) * 1000.0, 1),
  round(sum(t.cnt_neat * t.init_time_neat) / sum(t.cnt_neat) * 1000.0, 1),
--  round(100.0 * sum(t.cnt_rtt_neat) / sum(t.cnt_rtt) - 100.0, 2),
  round(100.0 * (sum(t.cnt_rtt_neat * t.rtt_neat) / sum(t.cnt_rtt_neat)) / (sum(t.cnt_rtt * t.rtt) / sum(t.cnt_rtt)) - 100.0, 2),
  round(100.0 * avg(if(t.rtt_neat < t.rtt, 1, 0)), 2)
--  round(100.0 * avg(t.rtt_gain), 2)
from
  tcp_ping_avg t
  left join experiment e on t.exp_id = e.exp_id
  left join v_tool_avg v on t.sched_id = v.sched_id
where
  (v.cnt_lte != v.cnt or v.cnt_lte_neat != v.cnt_neat) and
  e.batch = @exp_batch
;

-- Download speed gain for nodes on which at least one download/ping in non-LTE mode

select
  concat(e.exp_name, ' (', e.exp_dtm, ')') as exp_name,
  sum(t.cnt_dwnl) as cnt,
  round(sum(t.dwnl_size) / 1048576, 0) as `size [MB]`,
  round((sum(t.dwnl_size) / sum(t.dwnl_time)) / 1048576, 2) as `speed [MB/s]`,
  round(sum(t.cnt * t.init_time) / sum(t.cnt) * 1000.0, 1) as `init [ms]`,
  sum(t.cnt_dwnl_neat) as cnt_neat,
  round(sum(t.dwnl_size_neat) / 1048576, 0) as `size_neat [MB]`,
  round((sum(t.dwnl_size_neat) / sum(t.dwnl_time_neat)) / 1048576, 2) as `speed_neat [MB/s]`,
  round(sum(t.cnt_neat * t.init_time_neat) / sum(t.cnt_neat) * 1000.0, 1) as `init_neat [ms]`,
--  round(100.0 * sum(t.cnt_dwnl_neat) / sum(t.cnt_dwnl) - 100.0, 2) as `cnt_gain [%]`,
  round(100.0 * sum(t.dwnl_size_neat) / sum(t.dwnl_size) - 100.0, 2) as `size_gain [%]`,
  round(100.0 * (sum(t.dwnl_size_neat) / sum(t.dwnl_time_neat)) / (sum(t.dwnl_size) / sum(t.dwnl_time)) - 100.0, 2) as `speed_gain [%]`,
  round(100.0 * avg(if(t.dwnl_speed_neat > t.dwnl_speed, 1, 0)), 2) as `win_cnt [%]`
--  round(100.0 * avg(t.speed_gain), 2) as `avg_speed_gain [%]`
from
  dwnl_test_avg t
  left join experiment e on t.exp_id = e.exp_id
  left join v_tool_avg v on t.sched_id = v.sched_id
where
  (v.cnt_lte != v.cnt or v.cnt_lte_neat != v.cnt_neat) and
  e.batch = @exp_batch
group by
  t.exp_id
union all select
  'TOTAL',
  sum(t.cnt_dwnl),
  round(sum(t.dwnl_size) / 1048576, 0),
  round((sum(t.dwnl_size) / sum(t.dwnl_time)) / 1048576, 2),
  round(sum(t.cnt * t.init_time) / sum(t.cnt) * 1000.0, 1),
  sum(t.cnt_dwnl_neat),
  round(sum(t.dwnl_size_neat) / 1048576, 0),
  round((sum(t.dwnl_size_neat) / sum(t.dwnl_time_neat)) / 1048576, 2),
  round(sum(t.cnt_neat * t.init_time_neat) / sum(t.cnt_neat) * 1000.0, 1),
--  round(100.0 * sum(t.cnt_dwnl_neat) / sum(t.cnt_dwnl) - 100.0, 2),
  round(100.0 * sum(t.dwnl_size_neat) / sum(t.dwnl_size) - 100.0, 2),
  round(100.0 * (sum(t.dwnl_size_neat) / sum(t.dwnl_time_neat)) / (sum(t.dwnl_size) / sum(t.dwnl_time)) - 100.0, 2),
  round(100.0 * avg(if(t.dwnl_speed_neat > t.dwnl_speed, 1, 0)), 2)
--  round(100.0 * avg(t.speed_gain), 2)
from
  dwnl_test_avg t
  left join experiment e on t.exp_id = e.exp_id
  left join v_tool_avg v on t.sched_id = v.sched_id
where
  (v.cnt_lte != v.cnt or v.cnt_lte_neat != v.cnt_neat) and
  e.batch = @exp_batch
;

-- Distribution of signal quality in metadata

select 
  concat(e.exp_name, ' (', e.exp_dtm, ')') as exp_name,
  round(100.0 * sum(if(m.signal_quality = 'GOOD', m.span, 0)) / sum(m.span), 2) as `good [%]`,
  round(100.0 * sum(if(m.signal_quality = 'FAIR', m.span, 0)) / sum(m.span), 2) as `fair [%]`,
  round(100.0 * sum(if(m.signal_quality = 'POOR', m.span, 0)) / sum(m.span), 2) as `poor [%]`
from
  v_metadata m
  left join experiment e on m.exp_id = e.exp_id
where
  e.batch = @exp_batch
group by
  m.exp_id
union all select
  'TOTAL',
  round(100.0 * sum(if(m.signal_quality = 'GOOD', m.span, 0)) / sum(m.span), 2),
  round(100.0 * sum(if(m.signal_quality = 'FAIR', m.span, 0)) / sum(m.span), 2),
  round(100.0 * sum(if(m.signal_quality = 'POOR', m.span, 0)) / sum(m.span), 2)
from
  v_metadata m
  left join experiment e on m.exp_id = e.exp_id
where
  e.batch = @exp_batch
;

-- Signal quality chosen for download/ping non-neat vs neat

 select
  concat(e.exp_name, ' (', e.exp_dtm, ')') as exp_name,
  sum(t.cnt) as cnt,
  round(100.0 * sum(t.cnt_good) / sum(t.cnt), 2) as `good [%]`,
  round(100.0 * sum(t.cnt_fair) / sum(t.cnt), 2) as `fair [%]`,
  round(100.0 * sum(t.cnt_poor) / sum(t.cnt), 2) as `poor [%]`,
  sum(t.cnt_neat) as cnt_neat,
  round(100.0 * sum(t.cnt_good_neat) / sum(t.cnt_neat), 2) as `good_neat [%]`,
  round(100.0 * sum(t.cnt_fair_neat) / sum(t.cnt_neat), 2) as `fair_neat [%]`,
  round(100.0 * sum(t.cnt_poor_neat) / sum(t.cnt_neat), 2) as `poor_neat [%]`
from
  v_tool_avg t
  left join experiment e on t.exp_id = e.exp_id
where
  e.batch = @exp_batch
group by
  t.exp_id
union all select
  'TOTAL',
  sum(t.cnt),
  round(100.0 * sum(t.cnt_good) / sum(t.cnt), 2),
  round(100.0 * sum(t.cnt_fair) / sum(t.cnt), 2),
  round(100.0 * sum(t.cnt_poor) / sum(t.cnt), 2),
  sum(t.cnt_neat),
  round(100.0 * sum(t.cnt_good_neat) / sum(t.cnt_neat), 2),
  round(100.0 * sum(t.cnt_fair_neat) / sum(t.cnt_neat), 2),
  round(100.0 * sum(t.cnt_poor_neat) / sum(t.cnt_neat), 2)
from
  v_tool_avg t
  left join experiment e on t.exp_id = e.exp_id
where
  e.batch = @exp_batch
;

-- RTT by operator

select
  t.oper,
  count(*) as cnt,
  round(avg(t.rtt) * 1000.0, 1) as `rtt [ms]`
from
  tcp_ping t
  left join experiment e on t.exp_id = e.exp_id
where
  t.ping_type = 'ECHO' and
  e.batch = @exp_batch
group by
  t.oper
order by
  `rtt [ms]`
;

-- RTT by mode

select
  t.device_mode,
  count(*) as cnt,
  round(avg(t.rtt) * 1000.0, 1) as `rtt [ms]`
from
  v_tcp_ping t
  left join experiment e on t.exp_id = e.exp_id
where
  t.ping_type = 'ECHO' and
  e.batch = @exp_batch
group by
  t.device_mode
order by
  `rtt [ms]`
;

-- RTT by signal quality

select
  t.signal_quality,
  count(*) as cnt,
  round(avg(t.rtt) * 1000.0, 1) as `rtt [ms]`
from
  v_tcp_ping t
  left join experiment e on t.exp_id = e.exp_id
where
  t.ping_type = 'ECHO' and
  e.batch = @exp_batch
group by
  t.signal_quality
order by
  `rtt [ms]`
;

-- RTT by mode + signal quality

select
  t.device_mode,
  t.signal_quality,
  count(*) as cnt,
  round(avg(t.rtt) * 1000.0, 1) as `rtt [ms]`
from
  v_tcp_ping t
  left join experiment e on t.exp_id = e.exp_id
where
  t.ping_type = 'ECHO' and
  e.batch = @exp_batch
group by
  t.device_mode,
  t.signal_quality
order by
  `rtt [ms]`
;


-- Download speed by operator

select
  t.oper,
  count(*) as cnt,
  round((sum(t.dwnl_size) / sum(t.total_time - t.init_time)) / 1048576, 2) as `speed [MB/s]`
from
  dwnl_test t
  left join experiment e on t.exp_id = e.exp_id
where
  e.batch = @exp_batch
group by
  t.oper
order by
  `speed [MB/s]` desc
;

-- Download speed by mode

select
  t.device_mode,
  count(*) as cnt,
  round((sum(t.dwnl_size) / sum(t.total_time - t.init_time)) / 1048576, 2) as `speed [MB/s]`
from
  v_dwnl_test t
  left join experiment e on t.exp_id = e.exp_id
where
  e.batch = @exp_batch
group by
  t.device_mode
order by
  `speed [MB/s]` desc
;

-- Download speed by signal quality

select
  t.signal_quality,
  count(*) as cnt,
  round((sum(t.dwnl_size) / sum(t.total_time - t.init_time)) / 1048576, 2) as `speed [MB/s]`
from
  v_dwnl_test t
  left join experiment e on t.exp_id = e.exp_id
where
  e.batch = @exp_batch
group by
  t.signal_quality
order by
  `speed [MB/s]` desc
;

-- Download speed by mode + signal quality

select
  t.device_mode,
  t.signal_quality,
  count(*) as cnt,
  round((sum(t.dwnl_size) / sum(t.total_time - t.init_time)) / 1048576, 2) as `speed [MB/s]`
from
  v_dwnl_test t
  left join experiment e on t.exp_id = e.exp_id
where
  e.batch = @exp_batch
group by
  t.device_mode,
  t.signal_quality
order by
  `speed [MB/s]` desc
;
