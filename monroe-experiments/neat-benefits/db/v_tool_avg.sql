create or replace view v_tool_avg as select
  t.exp_id,
  t.sched_id,
  t.node_id,
  t.cnt + d.cnt as cnt,
  t.cnt_op0 + d.cnt_op0 as cnt_op0,
  t.cnt_op1 + d.cnt_op1 as cnt_op1,
  t.cnt_lte + d.cnt_lte as cnt_lte,
  t.cnt_3g + d.cnt_3g as cnt_3g,
  t.cnt_2g + d.cnt_2g as cnt_2g,
  t.cnt_good + d.cnt_good as cnt_good,
  t.cnt_fair + d.cnt_fair as cnt_fair,
  t.cnt_poor + d.cnt_poor as cnt_poor,
  (t.cnt * t.init_time + d.cnt * d.init_time) / (t.cnt + d.cnt) as init_time,
  t.cnt_neat + d.cnt_neat as cnt_neat,
  t.cnt_op0_neat + d.cnt_op0_neat as cnt_op0_neat,
  t.cnt_op1_neat + d.cnt_op1_neat as cnt_op1_neat,
  t.cnt_lte_neat + d.cnt_lte_neat as cnt_lte_neat,
  t.cnt_3g_neat + d.cnt_3g_neat as cnt_3g_neat,
  t.cnt_2g_neat  + d.cnt_2g_neat as cnt_2g_neat,
  t.cnt_good_neat + d.cnt_good_neat as cnt_good_neat,
  t.cnt_fair_neat + d.cnt_fair_neat as cnt_fair_neat,
  t.cnt_poor_neat + d.cnt_poor_neat as cnt_poor_neat,
  (t.cnt_neat * t.init_time_neat + d.cnt_neat * d.init_time_neat) / (t.cnt_neat + d.cnt_neat) as init_time_neat
from 
  tcp_ping_avg t
  left join dwnl_test_avg d on t.sched_id = d.sched_id
;

/*
create or replace view v_tool_avg as select
  t.exp_id,
  t.sched_id,
  t.node_id,
  t.cnt,
  t.cnt_op0,
  t.cnt_op1,
  t.cnt_lte,
  t.cnt_3g,
  t.cnt_2g,
  t.cnt_good,
  t.cnt_fair,
  t.cnt_poor,
  t.init_time,
  t.cnt_neat,
  t.cnt_op0_neat,
  t.cnt_op1_neat,
  t.cnt_lte_neat,
  t.cnt_3g_neat,
  t.cnt_2g_neat,
  t.cnt_good_neat,
  t.cnt_fair_neat,
  t.cnt_poor_neat,
  t.init_time_neat
from 
  tcp_ping_avg t
union all select
  t.exp_id,
  t.sched_id,
  t.node_id,
  t.cnt,
  t.cnt_op0,
  t.cnt_op1,
  t.cnt_lte,
  t.cnt_3g,
  t.cnt_2g,
  t.cnt_good,
  t.cnt_fair,
  t.cnt_poor,
  t.init_time,
  t.cnt_neat,
  t.cnt_op0_neat,
  t.cnt_op1_neat,
  t.cnt_lte_neat,
  t.cnt_3g_neat,
  t.cnt_2g_neat,
  t.cnt_good_neat,
  t.cnt_fair_neat,
  t.cnt_poor_neat,
  t.init_time_neat
from
  dwnl_test_avg t
;
*/