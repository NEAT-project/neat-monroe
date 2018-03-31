create or replace view v_metadata as select
  m.*,
  case m.dev_mode
    when 0 then 'UNKNOWN'
    when 1 then 'DISCONNECTED'
    when 2 then 'NO SERVICE'
    when 3 then '2G'
    when 4 then '3G'
    when 5 then 'LTE'
    else '?'
  end as device_mode,
  case 
    when (m.dev_mode = 5 and m.lte_rsrq >= -7 and m.lte_rsrp >= -102) or
         (m.dev_mode in (4,3) and m.rssi >= -69) then 'GOOD'
    when (m.dev_mode = 5 and m.lte_rsrq >= -11 and m.lte_rsrp >= -111) or
         (m.dev_mode in (4,3) and m.rssi >= -94) then 'FAIR'
    else 'POOR'
  end as signal_quality
from
  metadata m
;

create or replace view v_tcp_ping as select
  t.*,
  m.dev_mode,
  m.rssi,
  m.lte_rsrq,
  m.lte_rsrp,
  case m.dev_mode
    when 0 then 'UNKNOWN'
    when 1 then 'DISCONNECTED'
    when 2 then 'NO SERVICE'
    when 3 then '2G'
    when 4 then '3G'
    when 5 then 'LTE'
    else '?'
  end as device_mode,
  case 
    when (m.dev_mode = 5 and m.lte_rsrq >= -7 and m.lte_rsrp >= -102) or
         (m.dev_mode in (4,3) and m.rssi >= -69) then 'GOOD'
    when (m.dev_mode = 5 and m.lte_rsrq >= -11 and m.lte_rsrp >= -111) or
         (m.dev_mode in (4,3) and m.rssi >= -94) then 'FAIR'
    else 'POOR'
  end as signal_quality
from
  tcp_ping t
  left join metadata m on t.sched_id = m.sched_id and t.m_ts = m.ts
;

create or replace view v_dwnl_test as select 
  t.*,
  m.dev_mode,
  m.rssi,
  m.lte_rsrq,
  m.lte_rsrp,
  case m.dev_mode
    when 0 then 'UNKNOWN'
    when 1 then 'DISCONNECTED'
    when 2 then 'NO SERVICE'
    when 3 then '2G'
    when 4 then '3G'
    when 5 then 'LTE'
    else '?'
  end as device_mode,
  case 
    when (m.dev_mode = 5 and m.lte_rsrq >= -7 and m.lte_rsrp >= -102) or
         (m.dev_mode in (4,3) and m.rssi >= -69) then 'GOOD'
    when (m.dev_mode = 5 and m.lte_rsrq >= -11 and m.lte_rsrp >= -111) or
         (m.dev_mode in (4,3) and m.rssi >= -94) then 'FAIR'
    else 'POOR'
  end as signal_quality
 from
  dwnl_test t
  left join metadata m on t.sched_id = m.sched_id and t.m_ts = m.ts
;
