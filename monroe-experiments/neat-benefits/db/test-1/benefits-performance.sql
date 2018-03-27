-- RTT by operator

select
  oper,
  count(*) as cnt,
  round(avg(rtt), 6) as 'rtt [sec]'
from
  tcp_ping
where
  ping_type = 'ECHO'
group by oper
;

-- Download speed by operator

select
  oper,
  count(*) as cnt,
  round(sum(dwnl_size) / sum(total_time) / (1024 * 1024) , 4) as 'speed [MB/s]'
from dwnl_test
group by oper
;

-- RTT by mode

select
  case m.dev_mode
    when 0 then 'UNKNOWN'
    when 1 then 'DISCONNECTED'
    when 2 then 'NO SERVICE'
    when 3 then '2G'
    when 4 then '3G'
    when 5 then 'LTE'
    else '?'
  end as dev_mode,
  count(*) as cnt,
  round(avg(t.rtt), 6) as 'rtt [sec]'
from
  tcp_ping t
  left join metadata m on t.sched_id = m.sched_id and t.m_ts = m.ts
where
  t.ping_type = 'ECHO'
group by m.dev_mode
;

-- Download speed by mode

select
  case m.dev_mode
    when 0 then 'UNKNOWN'
    when 1 then 'DISCONNECTED'
    when 2 then 'NO SERVICE'
    when 3 then '2G'
    when 4 then '3G'
    when 5 then 'LTE'
    else '?'
  end as dev_mode,
  count(*) as cnt,
  round(sum(t.dwnl_size) / sum(t.total_time) / (1024 * 1024) , 4) as 'speed [MB/s]'
from
  dwnl_test t
  left join metadata m on t.sched_id = m.sched_id and t.m_ts = m.ts
group by m.dev_mode
;
