#!/bin/bash

DATE=`date +%Y%m%d-%H%M%S`
RESULT_DIR="/monroe/results/"

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
# export NEAT_CIB_SOCKET="/var/run/neat/neat_cib_socket" (Commented out to disable CIB caching) 

RUN_COUNT=`cat /monroe/config | jq .run_count`
RUN_INTERVAL=`cat /monroe/config | jq .run_interval`
RUN_INITIAL_WAIT=`cat /monroe/config | jq .run_initial_wait`

RUN_TCP_PING=`cat /monroe/config | jq -r .run_tcp_ping`
RUN_NEAT_TCP_PING=`cat /monroe/config | jq -r .run_neat_tcp_ping`
TCP_PING_SERVER=`cat /monroe/config | jq -r .ping_server`
TCP_PING_PORT=`cat /monroe/config | jq -r .ping_port`
TCP_PING_MODE=`cat /monroe/config | jq -r .ping_mode`
TCP_PING_COUNT=`cat /monroe/config | jq -r .ping_count`
TCP_PING_INTERVAL=`cat /monroe/config | jq -r .ping_interval`
TCP_PING_BIND_IFNAME=`cat /monroe/config | jq -r .ping_bind_ifname`
TCP_PING_VERBOSE=`cat /monroe/config | jq -r .ping_verbose`

RUN_DWNL_TEST=`cat /monroe/config | jq -r .run_dwnl_test`
RUN_NEAT_DWNL_TEST=`cat /monroe/config | jq -r .run_neat_dwnl_test`
DWNL_TEST_PORT=`cat /monroe/config | jq -r .dwnl_test_port`
DWNL_TEST_PATH=`cat /monroe/config | jq -r .dwnl_test_path`
DWNL_TEST_COUNT=`cat /monroe/config | jq -r .dwnl_test_count`
DWNL_TEST_INTERVAL=`cat /monroe/config | jq -r .dwnl_test_interval`
DWNL_TEST_SERVER=`cat /monroe/config | jq -r .dwnl_test_server`
DWNL_TEST_BIND_IFNAME=`cat /monroe/config | jq -r .dwnl_test_bind_ifname`
DWNL_TEST_VERBOSE=`cat /monroe/config | jq -r .dwnl_test_verbose`


# Sleep for a while to let metadata-exporter produce some CIBs
sleep ${RUN_INITIAL_WAIT}

COUNT=0
while [ $COUNT -lt ${RUN_COUNT} ]; do

  # tcp-ping
  if [ "${RUN_TCP_PING}" == "true" ]; then
    sleep ${RUN_INTERVAL}
    APP_NAME="tcp-ping"
    DATE=`date +%Y%m%d-%H%M%S`
    FNAME_STDOUT="${RESULT_DIR}${APP_NAME}-${DATE}.txt"
    FNAME_STDERR="${RESULT_DIR}${APP_NAME}-stderr-${DATE}.txt"
    TMP_FNAME_STDOUT="${TMP_DIR}${APP_NAME}-${DATE}.txt"
    TMP_FNAME_STDERR="${TMP_DIR}${APP_NAME}-stderr-${DATE}.txt"
    tcp-ping --port=${TCP_PING_PORT} \
      --mode=${TCP_PING_MODE} \
      --count=${TCP_PING_COUNT} \
      --interval=${TCP_PING_INTERVAL} \
      --bind=${TCP_PING_BIND_IFNAME} \
      --verbose=${TCP_PING_VERBOSE} \
      ${TCP_PING_SERVER} \
      1>"${TMP_FNAME_STDOUT}" \
      2>"${TMP_FNAME_STDERR}"
    mv ${TMP_FNAME_STDOUT} ${FNAME_STDOUT}
    mv ${TMP_FNAME_STDERR} ${FNAME_STDERR}
  fi

  # dwnl-test
  if [ "${RUN_DWNL_TEST}" == "true" ]; then
    sleep ${RUN_INTERVAL}
    APP_NAME="dwnl-test"
    DATE=`date +%Y%m%d-%H%M%S`
    FNAME_STDOUT="${RESULT_DIR}${APP_NAME}-${DATE}.txt"
    FNAME_STDERR="${RESULT_DIR}${APP_NAME}-stderr-${DATE}.txt"
    TMP_FNAME_STDOUT="${TMP_DIR}${APP_NAME}-${DATE}.txt"
    TMP_FNAME_STDERR="${TMP_DIR}${APP_NAME}-stderr-${DATE}.txt"
    dwnl-test --port=${DWNL_TEST_PORT} \
      --path=${DWNL_TEST_PATH} \
      --count=${DWNL_TEST_COUNT} \
      --interval=${DWNL_TEST_INTERVAL} \
      --bind=${DWNL_TEST_BIND_IFNAME} \
      --verbose=${DWNL_TEST_VERBOSE} \
      ${DWNL_TEST_SERVER} \
      1>"${TMP_FNAME_STDOUT}" \
      2>"${TMP_FNAME_STDERR}"
    mv ${TMP_FNAME_STDOUT} ${FNAME_STDOUT}
    mv ${TMP_FNAME_STDERR} ${FNAME_STDERR}
  fi

  # neat-tcp-ping
  if [ "${RUN_NEAT_TCP_PING}" == "true" ]; then
    sleep ${RUN_INTERVAL}
    APP_NAME="neat-tcp-ping"
    DATE=`date +%Y%m%d-%H%M%S`
    FNAME_STDOUT="${RESULT_DIR}${APP_NAME}-${DATE}.txt"
    FNAME_STDERR="${RESULT_DIR}${APP_NAME}-stderr-${DATE}.txt"
    TMP_FNAME_STDOUT="${TMP_DIR}${APP_NAME}-${DATE}.txt"
    TMP_FNAME_STDERR="${TMP_DIR}${APP_NAME}-stderr-${DATE}.txt"
    neat-tcp-ping --port=${TCP_PING_PORT} \
      --mode=${TCP_PING_MODE} \
      --count=${TCP_PING_COUNT} \
      --interval=${TCP_PING_INTERVAL} \
      --verbose=${TCP_PING_VERBOSE} \
      ${TCP_PING_SERVER} \
      1>"${TMP_FNAME_STDOUT}" \
      2>"${TMP_FNAME_STDERR}"
    mv ${TMP_FNAME_STDOUT} ${FNAME_STDOUT}
    mv ${TMP_FNAME_STDERR} ${FNAME_STDERR}
  fi

  # neat-dwnl-test
  if [ "${RUN_NEAT_DWNL_TEST}" == "true" ]; then
    sleep ${RUN_INTERVAL}
    APP_NAME="neat-dwnl-test"
    DATE=`date +%Y%m%d-%H%M%S`
    FNAME_STDOUT="${RESULT_DIR}${APP_NAME}-${DATE}.txt"
    FNAME_STDERR="${RESULT_DIR}${APP_NAME}-stderr-${DATE}.txt"
    TMP_FNAME_STDOUT="${TMP_DIR}${APP_NAME}-${DATE}.txt"
    TMP_FNAME_STDERR="${TMP_DIR}${APP_NAME}-stderr-${DATE}.txt"
    neat-dwnl-test --port=${DWNL_TEST_PORT} \
      --path=${DWNL_TEST_PATH} \
      --count=${DWNL_TEST_COUNT} \
      --interval=${DWNL_TEST_INTERVAL} \
      --verbose=${DWNL_TEST_VERBOSE} \
      ${DWNL_TEST_SERVER} \
      1>"${TMP_FNAME_STDOUT}" \
      2>"${TMP_FNAME_STDERR}"
    mv ${TMP_FNAME_STDOUT} ${FNAME_STDOUT}
    mv ${TMP_FNAME_STDERR} ${FNAME_STDERR}
  fi

  let COUNT=COUNT+1
done

while true; do
    sleep 15
done
