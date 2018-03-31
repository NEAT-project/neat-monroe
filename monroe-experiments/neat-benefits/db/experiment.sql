drop table if exists experiment;

create table experiment (
  exp_id integer default null,
  exp_name varchar(255) default null,
  exp_dtm datetime default null,
  batch varchar(255) default null,
  primary key (exp_id)
) DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci;


insert into experiment (exp_id, exp_name, exp_dtm, batch) values
  (25943, 'Day-1', '2018-03-15 13:36', 'policy-1'),
  (26115, 'Day-2', '2018-03-16 15:38', 'policy-1'),
  (26555, 'Day-1', '2018-03-20 12:10', 'policy-2'),
  (26561, 'Day-1', '2018-03-20 15:01', 'policy-2'),
  (26557, 'Day-1', '2018-03-20 20:13', 'policy-2'),
  (26559, 'Day-2', '2018-03-21 14:10', 'policy-2'),
  (26573, 'Day-2', '2018-03-21 18:09', 'policy-2'),
  (26558, 'Day-2', '2018-03-21 20:13', 'policy-2'),
  (26759, 'Day-1', '2018-03-22 15:00', 'policy-2'),
  (27129, 'Day-1', '2018-03-26 13:51', 'policy-3'),
  (27130, 'Day-2', '2018-03-26 15:54', 'policy-3'),
  (27131, 'Day-1', '2018-03-26 19:10', 'policy-3'),
  (27132, 'Day-2', '2018-03-26 21:13', 'policy-3'),
  (27133, 'Day-1', '2018-03-27 07:11', 'policy-3'),
  (27135, 'Day-2', '2018-03-27 09:14', 'policy-3');
