
-- a) Overall mode selected for download/ping

select
  '' as non_neat,
  sum(if(t.run_tool not like 'neat%', 1, 0)) as cnt,
  truncate(100 * sum(if(m.dev_mode = 5 and t.run_tool not like 'neat%', 1, 0)) / sum(if(t.run_tool not like 'neat%', 1, 0)), 2) as lte,
  truncate(100 * sum(if(m.dev_mode = 4 and t.run_tool not like 'neat%', 1, 0)) / sum(if(t.run_tool not like 'neat%', 1, 0)), 2) as 3g,
  truncate(100 * sum(if(m.dev_mode = 3 and t.run_tool not like 'neat%', 1, 0)) / sum(if(t.run_tool not like 'neat%', 1, 0)), 2) as 2g,
  '' as neat,
  sum(if(t.run_tool like 'neat%', 1, 0)) as cnt,
  truncate(100 * sum(if(m.dev_mode = 5 and t.run_tool like 'neat%', 1, 0)) / sum(if(t.run_tool like 'neat%', 1, 0)), 2) as lte,
  truncate(100 * sum(if(m.dev_mode = 4 and t.run_tool like 'neat%', 1, 0)) / sum(if(t.run_tool like 'neat%', 1, 0)), 2) as 3g,
  truncate(100 * sum(if(m.dev_mode = 3 and t.run_tool like 'neat%', 1, 0)) / sum(if(t.run_tool like 'neat%', 1, 0)), 2) as 2g
from
  (select exp_id, sched_id, node_id, run_tool, m_ts from dwnl_test
   union all select exp_id, sched_id, node_id, run_tool, m_ts from tcp_ping where iter_nr = 1) as t
  left join metadata m on t.sched_id = m.sched_id and t.m_ts = m.ts
where
  t.exp_id in (26555, 26557, 26561, 26558, 26559, 26573)
;

-- b) Mode selected for download/ping per experiment

select
  t.exp_id as exp_id,
  '' as non_neat,
  sum(if(t.run_tool not like 'neat%', 1, 0)) as cnt,
  truncate(100 * sum(if(m.dev_mode = 5 and t.run_tool not like 'neat%', 1, 0)) / sum(if(t.run_tool not like 'neat%', 1, 0)), 2) as lte,
  truncate(100 * sum(if(m.dev_mode = 4 and t.run_tool not like 'neat%', 1, 0)) / sum(if(t.run_tool not like 'neat%', 1, 0)), 2) as 3g,
  truncate(100 * sum(if(m.dev_mode = 3 and t.run_tool not like 'neat%', 1, 0)) / sum(if(t.run_tool not like 'neat%', 1, 0)), 2) as 2g,
  '' as neat,
  sum(if(t.run_tool like 'neat%', 1, 0)) as cnt,
  truncate(100 * sum(if(m.dev_mode = 5 and t.run_tool like 'neat%', 1, 0)) / sum(if(t.run_tool like 'neat%', 1, 0)), 2) as lte,
  truncate(100 * sum(if(m.dev_mode = 4 and t.run_tool like 'neat%', 1, 0)) / sum(if(t.run_tool like 'neat%', 1, 0)), 2) as 3g,
  truncate(100 * sum(if(m.dev_mode = 3 and t.run_tool like 'neat%', 1, 0)) / sum(if(t.run_tool like 'neat%', 1, 0)), 2) as 2g
from
  (select exp_id, sched_id, node_id, run_tool, m_ts from dwnl_test
   union all select exp_id, sched_id, node_id, run_tool, m_ts from tcp_ping where iter_nr = 1) as t
  left join metadata m on t.sched_id = m.sched_id and t.m_ts = m.ts
where
  t.exp_id in (26555, 26557, 26561, 26558, 26559, 26573)
group by
  t.exp_id
;

-- c) Mode selected for download/ping per node

select
  group_concat(distinct t.exp_id) as exp_id,
  t.sched_id as sched_id,
  group_concat(distinct t.node_id) as node_id,
  '' as non_neat,
  sum(if(t.run_tool not like 'neat%', 1, 0)) as cnt,
  truncate(100 * sum(if(m.dev_mode = 5 and t.run_tool not like 'neat%', 1, 0)) / sum(if(t.run_tool not like 'neat%', 1, 0)), 2) as lte,
  truncate(100 * sum(if(m.dev_mode = 4 and t.run_tool not like 'neat%', 1, 0)) / sum(if(t.run_tool not like 'neat%', 1, 0)), 2) as 3g,
  truncate(100 * sum(if(m.dev_mode = 3 and t.run_tool not like 'neat%', 1, 0)) / sum(if(t.run_tool not like 'neat%', 1, 0)), 2) as 2g,
  '' as neat,
  sum(if(t.run_tool like 'neat%', 1, 0)) as cnt,
  truncate(100 * sum(if(m.dev_mode = 5 and t.run_tool like 'neat%', 1, 0)) / sum(if(t.run_tool like 'neat%', 1, 0)), 2) as lte,
  truncate(100 * sum(if(m.dev_mode = 4 and t.run_tool like 'neat%', 1, 0)) / sum(if(t.run_tool like 'neat%', 1, 0)), 2) as 3g,
  truncate(100 * sum(if(m.dev_mode = 3 and t.run_tool like 'neat%', 1, 0)) / sum(if(t.run_tool like 'neat%', 1, 0)), 2) as 2g
from
  (select exp_id, sched_id, node_id, run_tool, m_ts from dwnl_test
   union all select exp_id, sched_id, node_id, run_tool, m_ts from tcp_ping where iter_nr = 1) as t
  left join metadata m on t.sched_id = m.sched_id and t.m_ts = m.ts
where
  t.exp_id in (26555, 26557, 26561, 26558, 26559, 26573)
group by
  t.sched_id
;

-- d) Overall mode in metadata

select 
  round(100.0 * sum(if(m.dev_mode = 5, m.span, 0)) / sum(m.span), 2) as lte,
  round(100.0 * sum(if(m.dev_mode = 4, m.span, 0)) / sum(m.span), 2) as 3g,
  round(100.0 * sum(if(m.dev_mode = 3, m.span, 0)) / sum(m.span), 2) as 2g,
  round(100.0 * sum(if(m.dev_mode = 2, m.span, 0)) / sum(m.span), 2) as no_service,
  round(100.0 * sum(if(m.dev_mode = 1, m.span, 0)) / sum(m.span), 2) as disconnected,
  round(100.0 * sum(if(m.dev_mode = 0, m.span, 0)) / sum(m.span), 2) as unknown
from
  metadata m
where
  m.exp_id in (26555, 26557, 26561, 26558, 26559, 26573)
;

-- d) Mode in metadata per experiment

select 
  m.exp_id,
  round(100.0 * sum(if(m.dev_mode = 5, m.span, 0)) / sum(m.span), 2) as lte,
  round(100.0 * sum(if(m.dev_mode = 4, m.span, 0)) / sum(m.span), 2) as 3g,
  round(100.0 * sum(if(m.dev_mode = 3, m.span, 0)) / sum(m.span), 2) as 2g,
  round(100.0 * sum(if(m.dev_mode = 2, m.span, 0)) / sum(m.span), 2) as no_service,
  round(100.0 * sum(if(m.dev_mode = 1, m.span, 0)) / sum(m.span), 2) as disconnected,
  round(100.0 * sum(if(m.dev_mode = 0, m.span, 0)) / sum(m.span), 2) as unknown
from
  metadata m
where
  m.exp_id in (26555, 26557, 26561, 26558, 26559, 26573)
group by
  m.exp_id
;

