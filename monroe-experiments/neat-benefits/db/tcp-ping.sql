create table tcp_ping (
  exp_id integer default null,
  sched_id integer default null,
  node_id integer default null,
  run_tool varchar(16) default null,
  run_dtm datetime default null,
  iter_nr integer default null,
  local_ip varchar(16) default null,
  if_name varchar(16) default null,
  oper integer default null,
  remote_host varchar(255) default null,
  remote_port integer default null,
  ping_type varchar(16) default null,
  rtt double default null,
  m_ts datetime(6) default null,
  primary key (sched_id, run_tool, run_dtm, iter_nr)
) DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci;
