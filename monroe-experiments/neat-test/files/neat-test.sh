#!/bin/bash

# Start policy manager
mkdir -p /var/run/neat/cib/
mkdir -p /var/run/neat/pib/
python3.5 /opt/celerway/neat/policy/neatpmd \
  --sock /var/run/neat/ \
  --cib /var/run/neat/cib/ \
  --pib /var/run/neat/pib/ \
  > /monroe/results/neatpmd-${DATE}.txt 2>&1 &
 
# Start neat metadata exporter
neat-metadata-exporter --cib-socket /var/run/neat/neat_cib_socket \
  > /monroe/results/metadata-exporter-${DATE}.txt 2>&1 &

# Run experiment

#CMD="neat_http_get -v 1 celerway.com"
export NEAT_PM_SOCKET="/var/run/neat/neat_pm_socket"
export NEAT_CIB_SOCKET="/var/run/neat/neat_cib_socket"

while true; do

    DATE=`date +%Y%m%d-%H%M%S.%N`
    CMD="wget celerway.com -O /monroe/results/celerway.com-${DATE}.html"
    FNAME=/monroe/results/neat-test-${DATE}.txt
    TMP_FNAME=/tmp/neat-test-${DATE}.txt

    echo -n "time -f 'TIME-SEC: %e' ${CMD} 1>/dev/null 2> ${FNAME} ..."
    /usr/bin/time -f 'TIME-SEC: %e' ${CMD} 1>/dev/null 2> ${TMP_FNAME}
    mv $TMP_FNAME $FNAME
    echo " DONE"

    sleep 15

done
