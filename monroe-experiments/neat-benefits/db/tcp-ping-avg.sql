drop table if exists tcp_ping_avg;

create table tcp_ping_avg (
  exp_id integer default null,
  sched_id integer default null,
  node_id integer default null,
  cnt_all integer default null,
  cnt_rtt integer default null,
  rtt double default null,
  cnt integer default null,
  cnt_op0 integer default null,
  cnt_op1 integer default null,
  cnt_lte integer default null,
  cnt_3g integer default null,
  cnt_2g integer default null,
  cnt_good integer default null,
  cnt_fair integer default null,
  cnt_poor integer default null,
  init_time double default null,
  cnt_rtt_neat integer default null,
  rtt_neat double default null,
  cnt_neat integer default null,
  cnt_op0_neat integer default null,
  cnt_op1_neat integer default null,
  cnt_lte_neat integer default null,
  cnt_3g_neat integer default null,
  cnt_2g_neat integer default null,
  cnt_good_neat integer default null,
  cnt_fair_neat integer default null,
  cnt_poor_neat integer default null,
  init_time_neat double default null,
  cnt_gain double default null,
  rtt_gain double default null,
  primary key (sched_id)
) DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci;

insert into tcp_ping_avg select
  t.exp_id,
  sched_id,
  t.node_id,
  count(*),
  sum(if(t.run_tool not like 'neat%' and t.ping_type = 'ECHO', 1, null)),
  avg(if(t.run_tool not like 'neat%' and t.ping_type = 'ECHO', t.rtt, null)),
  sum(if(t.run_tool not like 'neat%' and t.iter_nr = 1, 1, null)),
  sum(if(t.run_tool not like 'neat%' and t.iter_nr = 1 and t.if_name = 'op0', 1, 0)),
  sum(if(t.run_tool not like 'neat%' and t.iter_nr = 1 and t.if_name = 'op1', 1, 0)),
  sum(if(t.run_tool not like 'neat%' and t.iter_nr = 1 and t.device_mode = 'LTE', 1, 0)),
  sum(if(t.run_tool not like 'neat%' and t.iter_nr = 1 and t.device_mode = '3G', 1, 0)),
  sum(if(t.run_tool not like 'neat%' and t.iter_nr = 1 and t.device_mode = '2G', 1, 0)),
  sum(if(t.run_tool not like 'neat%' and t.iter_nr = 1 and t.signal_quality = 'GOOD', 1, 0)),
  sum(if(t.run_tool not like 'neat%' and t.iter_nr = 1 and t.signal_quality = 'FAIR', 1, 0)),
  sum(if(t.run_tool not like 'neat%' and t.iter_nr = 1 and t.signal_quality = 'POOR', 1, 0)),
  avg(if(t.run_tool not like 'neat%' and t.iter_nr = 1, t.rtt, null)),
  sum(if(t.run_tool like 'neat%' and t.ping_type = 'ECHO', 1, null)),
  avg(if(t.run_tool like 'neat%' and t.ping_type = 'ECHO', t.rtt, null)),
  sum(if(t.run_tool like 'neat%' and t.iter_nr = 1, 1, null)),
  sum(if(t.run_tool like 'neat%' and t.iter_nr = 1 and t.if_name = 'op0', 1, 0)),
  sum(if(t.run_tool like 'neat%' and t.iter_nr = 1 and t.if_name = 'op1', 1, 0)),
  sum(if(t.run_tool like 'neat%' and t.iter_nr = 1 and t.device_mode = 'LTE', 1, 0)),
  sum(if(t.run_tool like 'neat%' and t.iter_nr = 1 and t.device_mode = '3G', 1, 0)),
  sum(if(t.run_tool like 'neat%' and t.iter_nr = 1 and t.device_mode = '2G', 1, 0)),
  sum(if(t.run_tool like 'neat%' and t.iter_nr = 1 and t.signal_quality = 'GOOD', 1, 0)),
  sum(if(t.run_tool like 'neat%' and t.iter_nr = 1 and t.signal_quality = 'FAIR', 1, 0)),
  sum(if(t.run_tool like 'neat%' and t.iter_nr = 1 and t.signal_quality = 'POOR', 1, 0)),
  avg(if(t.run_tool like 'neat%' and t.iter_nr = 1, t.rtt, null)),
  null,
  null
from
  v_tcp_ping t
group by
  t.sched_id
;

update tcp_ping_avg set
  cnt_gain = cnt_rtt_neat / cnt_rtt - 1.0,
  rtt_gain = rtt_neat / rtt - 1.0
;
