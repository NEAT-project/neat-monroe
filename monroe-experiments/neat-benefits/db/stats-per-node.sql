set @exp_batch = 'policy-3';

-- RTT gain

select 
  t.exp_id,
  sched_id,
  t.node_id,
  t.cnt_rtt as cnt,
  round(t.rtt * 1000.0, 1) as `rtt [ms]`,
  round(t.init_time * 1000.0, 1) as `init [ms]`,
  t.cnt_rtt_neat as cnt_neat,
  round(t.rtt_neat * 1000.0, 1) as `rtt_neat [ms]`,
  round(t.init_time_neat * 1000.0, 1) as `init_neat [ms]`,
--  round(100.0 * t.cnt_gain, 2) as `cnt_gain [%]`,
  round(100.0 * t.rtt_gain, 2) as `rtt_gain [%]`
from
  tcp_ping_avg t
  left join experiment e on t.exp_id = e.exp_id
where
  e.batch = @exp_batch
;

-- Download speed gain

select
  t.exp_id,
  t.sched_id,
  t.node_id,
  t.cnt_dwnl as cnt,
  round(t.dwnl_size / 1048576, 1) as `size [MB]`,
  round(t.dwnl_speed / 1048576, 2) as `speed [MB/s]`,
  round(t.init_time * 1000.0, 1) as `init [ms]`,
  t.cnt_dwnl_neat as cnt_neat,
  round(t.dwnl_size_neat / 1048576, 1) as `size_neat [MB]`,
  round(t.dwnl_speed_neat / 1048576, 2) as `speed_neat [MB/s]`,
  round(t.init_time_neat * 1000.0, 1) as `init_neat [ms]`,
--  round(100.0 * t.cnt_gain, 2) as `cnt_gain [%]`,
  round(100.0 * t.size_gain, 2) as `size_gain [%]`,
  round(100.0 * t.speed_gain, 2) as `speed_gain [%]`
from
  dwnl_test_avg t
  left join experiment e on t.exp_id = e.exp_id
where
  e.batch = @exp_batch
;

-- Interface selected for download/ping non-neat vs neat

select
  e.exp_id,
  t.sched_id,
  t.node_id,
  t.cnt,
  round(100.0 * t.cnt_op0 / t.cnt, 2) as `op0 [%]`,
  round(100.0 * t.cnt_op1 / t.cnt, 2) as `op1 [%]`,
  t.cnt_neat,
  round(100.0 * t.cnt_op0_neat / t.cnt_neat, 2) as `op0_neat [%]`,
  round(100.0 * t.cnt_op1_neat / t.cnt_neat, 2) as `op1_neat [%]`
from
  v_tool_avg t
  left join experiment e on t.exp_id = e.exp_id
  left join v_tool_avg v on t.sched_id = v.sched_id
where
--  if(v.cnt_op0 = v.cnt, v.cnt_op1_neat, v.cnt_op0_neat) / v.cnt_neat > 0.2 and
  e.batch = @exp_batch
;

-- Mode selected for ping/download non-neat vs neat

select
  t.exp_id,
  t.sched_id,
  t.node_id,
  t.cnt,
  round(100.0 * t.cnt_lte / t.cnt, 2) as `lte [%]`,
  round(100.0 * t.cnt_3g / t.cnt, 2) as `3g [%]`,
  round(100.0 * t.cnt_2g / t.cnt, 2) as `2g [%]`,
  t.cnt_neat,
  round(100.0 * t.cnt_lte_neat / t.cnt_neat, 2) as `lte_neat [%]`,
  round(100.0 * t.cnt_3g_neat / t.cnt_neat, 2) as `3g_neat [%]`,
  round(100.0 * t.cnt_2g_neat / t.cnt_neat, 2) as `2g_neat [%]`
from
  v_tool_avg t
  left join experiment e on t.exp_id = e.exp_id
  left join v_tool_avg v on t.sched_id = v.sched_id
where
--  (v.cnt_3g != 0 or v.cnt_3g_neat != 0) and
  e.batch = @exp_batch  
;

-- Signal quality chosen for download/ping non-neat vs neat

select
  t.exp_id,
  t.sched_id,
  t.node_id,
  t.cnt,
  round(100.0 * t.cnt_good / t.cnt, 2) as `good [%]`,
  round(100.0 * t.cnt_fair / t.cnt, 2) as `fair [%]`,
  round(100.0 * t.cnt_poor / t.cnt, 2) as `poor [%]`,
  t.cnt_neat,
  round(100.0 * t.cnt_good_neat / t.cnt_neat, 2) as `good_neat [%]`,
  round(100.0 * t.cnt_fair_neat / t.cnt_neat, 2) as `fair_neat [%]`,
  round(100.0 * t.cnt_poor_neat / t.cnt_neat, 2) as `poor_neat [%]`
from
  v_tool_avg t
  left join experiment e on t.exp_id = e.exp_id
where
  e.batch = @exp_batch
;
