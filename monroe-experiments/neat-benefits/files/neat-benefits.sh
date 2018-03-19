#!/bin/bash

TS_START=`date +%s`
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

RUN_COMMANDS=`cat /monroe/config | jq -r .run_commands`
RUN_COUNT=`cat /monroe/config | jq -r .run_count`
RUN_INTERVAL=`cat /monroe/config | jq -r .run_interval`
RUN_TIMEOUT=`cat /monroe/config | jq -r .run_timeout`
RUN_INITIAL_WAIT=`cat /monroe/config | jq -r .run_initial_wait`

TCP_PING_SERVER=`cat /monroe/config | jq -r .ping_server`
TCP_PING_PORT=`cat /monroe/config | jq -r .ping_port`
TCP_PING_MODE=`cat /monroe/config | jq -r .ping_mode`
TCP_PING_COUNT=`cat /monroe/config | jq -r .ping_count`
TCP_PING_INTERVAL=`cat /monroe/config | jq -r .ping_interval`
TCP_PING_TIMEOUT=`cat /monroe/config | jq -r .ping_timeout`
TCP_PING_BIND_IFNAME=`cat /monroe/config | jq -r .ping_bind_ifname`
TCP_PING_VERBOSE=`cat /monroe/config | jq -r .ping_verbose`

DWNL_TEST_SERVER=`cat /monroe/config | jq -r .dwnl_test_server`
DWNL_TEST_PORT=`cat /monroe/config | jq -r .dwnl_test_port`
DWNL_TEST_PATH=`cat /monroe/config | jq -r .dwnl_test_path`
DWNL_TEST_COUNT=`cat /monroe/config | jq -r .dwnl_test_count`
DWNL_TEST_INTERVAL=`cat /monroe/config | jq -r .dwnl_test_interval`
DWNL_TEST_TIMEOUT=`cat /monroe/config | jq -r .dwnl_test_timeout`
DWNL_TEST_BIND_IFNAME=`cat /monroe/config | jq -r .dwnl_test_bind_ifname`
DWNL_TEST_VERBOSE=`cat /monroe/config | jq -r .dwnl_test_verbose`

function run_tcp_ping {
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
    --timeout=${TCP_PING_TIMEOUT} \
    --bind=${TCP_PING_BIND_IFNAME} \
    --verbose=${TCP_PING_VERBOSE} \
    ${TCP_PING_SERVER} \
    1>"${TMP_FNAME_STDOUT}" \
    2>"${TMP_FNAME_STDERR}"
  mv ${TMP_FNAME_STDOUT} ${FNAME_STDOUT}
  mv ${TMP_FNAME_STDERR} ${FNAME_STDERR}
}

function run_neat_tcp_ping {
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
    --timeout=${TCP_PING_TIMEOUT} \
    --verbose=${TCP_PING_VERBOSE} \
    ${TCP_PING_SERVER} \
    1>"${TMP_FNAME_STDOUT}" \
    2>"${TMP_FNAME_STDERR}"
  mv ${TMP_FNAME_STDOUT} ${FNAME_STDOUT}
  mv ${TMP_FNAME_STDERR} ${FNAME_STDERR}
}

function run_dwnl_test {
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
    --timeout=${DWNL_TEST_TIMEOUT} \
    --bind=${DWNL_TEST_BIND_IFNAME} \
    --verbose=${DWNL_TEST_VERBOSE} \
    ${DWNL_TEST_SERVER} \
    1>"${TMP_FNAME_STDOUT}" \
    2>"${TMP_FNAME_STDERR}"
  mv ${TMP_FNAME_STDOUT} ${FNAME_STDOUT}
  mv ${TMP_FNAME_STDERR} ${FNAME_STDERR}
}

function run_neat_dwnl_test {
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
    --timeout=${DWNL_TEST_TIMEOUT} \
    --verbose=${DWNL_TEST_VERBOSE} \
    ${DWNL_TEST_SERVER} \
    1>"${TMP_FNAME_STDOUT}" \
    2>"${TMP_FNAME_STDERR}"
  mv ${TMP_FNAME_STDOUT} ${FNAME_STDOUT}
  mv ${TMP_FNAME_STDERR} ${FNAME_STDERR}
}

# Sleep for a while to let metadata-exporter produce some CIBs
sleep ${RUN_INITIAL_WAIT}

COUNT=0
while [ $COUNT -lt ${RUN_COUNT} ]; do
  for CMD in ${RUN_COMMANDS}; do
    if [ "${CMD}" == "tcp-ping" ]; then
      run_tcp_ping
    elif [ "${CMD}" == "neat-tcp-ping" ]; then
      run_neat_tcp_ping
    elif [ "${CMD}" == "dwnl-test" ]; then
      run_dwnl_test
    elif [ "${CMD}" == "neat-dwnl-test" ]; then
      run_neat_dwnl_test
    fi
  done

  if [ "${RUN_TIMEOUT}" != "null" -a "${RUN_TIMEOUT}" != "0" ]; then
    if [ $((`date +%s` - ${TS_START})) -gt ${RUN_TIMEOUT} ]; then
      break
    fi
  fi

  COUNT=$((${COUNT} + 1))
done

#while true; do
#    sleep 15
#done
