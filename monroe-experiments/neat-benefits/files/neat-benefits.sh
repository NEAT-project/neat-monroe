#!/bin/bash

DATE=`date +%Y%m%d-%H%M%S`

# Apply policies and start policy manager

mkdir -p /var/run/neat/cib/
mkdir -p /var/run/neat/pib/

cp /opt/celerway/policies/* /var/run/neat/pib/

python3.5 /opt/celerway/neat/policy/neatpmd \
  --sock /var/run/neat/ \
  --cib /var/run/neat/cib/ \
  --pib /var/run/neat/pib/ \
  &> /monroe/results/neatpmd-${DATE}.txt &
 
# Start neat metadata exporter

neat-metadata-exporter --cib-socket /var/run/neat/neat_cib_socket \
  --ifname-key InternalInterface \
  &> /monroe/results/metadata-exporter-${DATE}.txt &

# Run experiment

export NEAT_PM_SOCKET="/var/run/neat/neat_pm_socket"
export NEAT_CIB_SOCKET="/var/run/neat/neat_cib_socket"

RUN_COUNT=`cat /monroe/config | jq .run_count`
RUN_INTERVAL=`cat /monroe/config | jq .run_interval`

RUN_TCP_PING=`cat /monroe/config | jq -r .run_tcp_ping`
RUN_NEAT_TCP_PING=`cat /monroe/config | jq -r .run_neat_tcp_ping`
TCP_PING_SERVER=`cat /monroe/config | jq -r .ping_server`
TCP_PING_PORT=`cat /monroe/config | jq -r .ping_port`
TCP_PING_MODE=`cat /monroe/config | jq -r .ping_mode`
TCP_PING_COUNT=`cat /monroe/config | jq -r .ping_count`
TCP_PING_INTERVAL=`cat /monroe/config | jq -r .ping_interval`

RUN_DWNL_TEST=`cat /monroe/config | jq -r .run_dwnl_test`
RUN_NEAT_DWNL_TEST=`cat /monroe/config | jq -r .run_neat_dwnl_test`
DWNL_TEST_PORT=`cat /monroe/config | jq -r .dwnl_test_port`
DWNL_TEST_PATH=`cat /monroe/config | jq -r .dwnl_test_path`
DWNL_TEST_COUNT=`cat /monroe/config | jq -r .dwnl_test_count`
DWNL_TEST_INTERVAL=`cat /monroe/config | jq -r .dwnl_test_interval`
DWNL_TEST_SERVER=`cat /monroe/config | jq -r .dwnl_test_server`

COUNT=0
while [ $COUNT -lt ${RUN_COUNT} ]; do

  # tcp-ping
  if [ "${RUN_TCP_PING}" == "true" ]; then
    DATE=`date +%Y%m%d-%H%M%S`
    FNAME="/monroe/results/tcp-ping-${DATE}.txt"
    TMP_FNAME="/tmp/tcp-ping-${DATE}.txt"
    tcp-ping --port=${TCP_PING_PORT} \
      --mode=${TCP_PING_MODE} \
      --count=${TCP_PING_COUNT} \
      --interval=${TCP_PING_INTERVAL} \
      ${TCP_PING_SERVER} &> ${TMP_FNAME}
    mv ${TMP_FNAME} ${FNAME}
  fi

  # dwnl-test
  if [ "${RUN_DWNL_TEST}" == "true" ]; then
    DATE=`date +%Y%m%d-%H%M%S`
    FNAME="/monroe/results/dwnl-test-${DATE}.txt"
    TMP_FNAME="/tmp/dwnl-test-${DATE}.txt"
    dwnl-test --port=${DWNL_TEST_PORT} \
      --path=${DWNL_TEST_PATH} \
       --count=${DWNL_TEST_COUNT} \
       --interval=${DWNL_TEST_INTERVAL} \
      ${DWNL_TEST_SERVER} &> ${TMP_FNAME}
    mv ${TMP_FNAME} ${FNAME}
  fi

  # neat-tcp-ping
  if [ "${RUN_NEAT_TCP_PING}" == "true" ]; then
    DATE=`date +%Y%m%d-%H%M%S`
    FNAME="/monroe/results/neat-tcp-ping-${DATE}.txt"
    TMP_FNAME="/tmp/neat-tcp-ping-${DATE}.txt"
    neat-tcp-ping --port=${TCP_PING_PORT} \
      --mode=${TCP_PING_MODE} \
      --count=${TCP_PING_COUNT} \
      --interval=${TCP_PING_INTERVAL} \
      ${TCP_PING_SERVER} &> ${TMP_FNAME}
    mv ${TMP_FNAME} ${FNAME}
  fi

  # neat-dwnl-test
  if [ "${RUN_NEAT_DWNL_TEST}" == "true" ]; then
    DATE=`date +%Y%m%d-%H%M%S`
    FNAME="/monroe/results/neat-dwnl-test-${DATE}.txt"
    TMP_FNAME="/tmp/neat-dwnl-test-${DATE}.txt"
    neat-dwnl-test --port=${DWNL_TEST_PORT} \
      --path=${DWNL_TEST_PATH} \
       --count=${DWNL_TEST_COUNT} \
       --interval=${DWNL_TEST_INTERVAL} \
      ${DWNL_TEST_SERVER} &> ${TMP_FNAME}
    mv ${TMP_FNAME} ${FNAME}
  fi

  sleep ${RUN_INTERVAL}
  let COUNT=COUNT+1
done

while true; do
    sleep 15
done

