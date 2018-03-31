drop table if exists dwnl_test_avg;

create table dwnl_test_avg (
  exp_id integer default null,
  sched_id integer default null,
  node_id integer default null,
  cnt_all integer default null,
  cnt_dwnl integer default null,
  dwnl_size bigint default null,
  dwnl_time double default null,
  dwnl_speed double default null,
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
  cnt_dwnl_neat integer default null,
  dwnl_size_neat bigint default null,
  dwnl_time_neat double default null,
  dwnl_speed_neat double default null,
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
  size_gain double default null,
  speed_gain double default null,  
  primary key (sched_id)
) DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci;

insert into dwnl_test_avg select
  t.exp_id,
  sched_id,
  t.node_id,
  count(*),
  sum(if(t.run_tool not like 'neat%', 1, null)),
  sum(if(t.run_tool not like 'neat%', t.dwnl_size, null)),
  sum(if(t.run_tool not like 'neat%', t.total_time - t.init_time, null)),
  null,
  sum(if(t.run_tool not like 'neat%' and t.iter_nr = 1, 1, null)),
  sum(if(t.run_tool not like 'neat%' and t.iter_nr = 1 and t.if_name = 'op0', 1, 0)),
  sum(if(t.run_tool not like 'neat%' and t.iter_nr = 1 and t.if_name = 'op1', 1, 0)),
  sum(if(t.run_tool not like 'neat%' and t.iter_nr = 1 and t.device_mode = 'LTE', 1, 0)),
  sum(if(t.run_tool not like 'neat%' and t.iter_nr = 1 and t.device_mode = '3G', 1, 0)),
  sum(if(t.run_tool not like 'neat%' and t.iter_nr = 1 and t.device_mode = '2G', 1, 0)),
  sum(if(t.run_tool not like 'neat%' and t.iter_nr = 1 and t.signal_quality = 'GOOD', 1, 0)),
  sum(if(t.run_tool not like 'neat%' and t.iter_nr = 1 and t.signal_quality = 'FAIR', 1, 0)),
  sum(if(t.run_tool not like 'neat%' and t.iter_nr = 1 and t.signal_quality = 'POOR', 1, 0)),
  avg(if(t.run_tool not like 'neat%' and t.iter_nr = 1, t.init_time, null)),
  sum(if(t.run_tool like 'neat%', 1, null)),
  sum(if(t.run_tool like 'neat%', t.dwnl_size, null)),
  sum(if(t.run_tool like 'neat%', t.total_time - t.init_time, null)),
  null,
  sum(if(t.run_tool like 'neat%' and t.iter_nr = 1, 1, null)),
  sum(if(t.run_tool like 'neat%' and t.iter_nr = 1 and t.if_name = 'op0', 1, 0)),
  sum(if(t.run_tool like 'neat%' and t.iter_nr = 1 and t.if_name = 'op1', 1, 0)),
  sum(if(t.run_tool like 'neat%' and t.iter_nr = 1 and t.device_mode = 'LTE', 1, 0)),
  sum(if(t.run_tool like 'neat%' and t.iter_nr = 1 and t.device_mode = '3G', 1, 0)),
  sum(if(t.run_tool like 'neat%' and t.iter_nr = 1 and t.device_mode = '2G', 1, 0)),
  sum(if(t.run_tool like 'neat%' and t.iter_nr = 1 and t.signal_quality = 'GOOD', 1, 0)),
  sum(if(t.run_tool like 'neat%' and t.iter_nr = 1 and t.signal_quality = 'FAIR', 1, 0)),
  sum(if(t.run_tool like 'neat%' and t.iter_nr = 1 and t.signal_quality = 'POOR', 1, 0)),
  avg(if(t.run_tool like 'neat%' and t.iter_nr = 1, t.init_time, null)),
  null,
  null,
  null
from
  v_dwnl_test t
group by
  t.sched_id
;

update dwnl_test_avg set
  dwnl_speed = dwnl_size / dwnl_time,
  dwnl_speed_neat = dwnl_size_neat / dwnl_time_neat
;

update dwnl_test_avg set
  cnt_gain = cnt_dwnl_neat / cnt_dwnl - 1.0,
  size_gain = dwnl_size_neat / dwnl_size - 1.0,
  speed_gain = dwnl_speed_neat / dwnl_speed - 1.0
;
