
-- a) Iface selected for download/ping per node

select
  group_concat(distinct t.exp_id) as exp_id,
  t.sched_id as sched_id,
  group_concat(distinct t.node_id) as node_id,
  '' as non_neat,
  sum(if(t.run_tool not like 'neat%', 1, 0)) as cnt,
  truncate(100 * sum(if(t.if_name = 'op0' and t.run_tool not like 'neat%', 1, 0)) / sum(if(t.run_tool not like 'neat%', 1, 0)), 2) as op0,
  truncate(100 * sum(if(t.if_name = 'op1' and t.run_tool not like 'neat%', 1, 0)) / sum(if(t.run_tool not like 'neat%', 1, 0)), 2) as op1,
  '' as neat,
  sum(if(t.run_tool like 'neat%', 1, 0)) as cnt,
  truncate(100 * sum(if(t.if_name = 'op0' and t.run_tool like 'neat%', 1, 0)) / sum(if(t.run_tool like 'neat%', 1, 0)), 2) as op0,
  truncate(100 * sum(if(t.if_name = 'op1' and t.run_tool like 'neat%', 1, 0)) / sum(if(t.run_tool like 'neat%', 1, 0)), 2) as op1
from
  (select exp_id, sched_id, node_id, run_tool, if_name from dwnl_test
   union all select exp_id, sched_id, node_id, run_tool, if_name from tcp_ping where iter_nr = 1) as t
where
  t.exp_id in (26555, 26557, 26561, 26558, 26559, 26573)
group by
  t.sched_id
;

-- b) Iface selected for download/ping per node - nodes with differences

select
  group_concat(distinct t.exp_id) as exp_id,
  t.sched_id as sched_id,
  group_concat(distinct t.node_id) as node_id,
  '' as non_neat,
  sum(if(t.run_tool not like 'neat%', 1, 0)) as cnt,
  truncate(100 * sum(if(t.if_name = 'op0' and t.run_tool not like 'neat%', 1, 0)) / sum(if(t.run_tool not like 'neat%', 1, 0)), 2) as op0,
  truncate(100 * sum(if(t.if_name = 'op1' and t.run_tool not like 'neat%', 1, 0)) / sum(if(t.run_tool not like 'neat%', 1, 0)), 2) as op1,
  '' as neat,
  sum(if(t.run_tool like 'neat%', 1, 0)) as cnt,
  truncate(100 * sum(if(t.if_name = 'op0' and t.run_tool like 'neat%', 1, 0)) / sum(if(t.run_tool like 'neat%', 1, 0)), 2) as nop0,
  truncate(100 * sum(if(t.if_name = 'op1' and t.run_tool like 'neat%', 1, 0)) / sum(if(t.run_tool like 'neat%', 1, 0)), 2) as nop1
from
  (select exp_id, sched_id, node_id, run_tool, if_name from dwnl_test
   union all select exp_id, sched_id, node_id, run_tool, if_name from tcp_ping where iter_nr = 1) as t
where
  t.exp_id in (26555, 26557, 26561, 26558, 26559, 26573)
group by
  t.sched_id
having op0 != nop0 or nop1 != nop1
;


