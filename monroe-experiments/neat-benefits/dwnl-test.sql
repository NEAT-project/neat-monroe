create table dwnl_test (
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
  dwnl_path varchar(255) default null,
  dwnl_size integer default null,
  init_time double default null,
  total_time double default null,
  primary key (sched_id, run_tool, run_dtm, iter_nr)
) DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci;
