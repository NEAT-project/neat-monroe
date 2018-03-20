select
  case m.exp_id 
    when 25943 then 'NEAT Test Day 1'
    when 26115 then 'NEAT Test Day 2'
    else '?'
  end as experiment,
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
  100 * count(*) / (select count(*) from metadata x where x.exp_id = m.exp_id) as percent
from
  metadata m
group by
  m.exp_id, m.dev_mode;
