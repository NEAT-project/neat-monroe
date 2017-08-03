#!/bin/bash

# Start policy manager
mkdir -p /var/run/neat/cib/
mkdir -p /var/run/neat/pib/
python3.5 /opt/celerway/neat/policy/neatpmd --sock /var/run/neat/ --cib /var/run/neat/cib/ --pib /var/run/neat/pib/ &
 
# Start neat metadata exporter
neat-metadata-exporter --cib-socket /var/run/neat/neat_cib_socket &

# Run experiment

CMD="/opt/celerway/neat/build/examples/client_http_get -v 1 celerway.com"

while true; do

    DATE=`date +%Y%m%d-%H%M%S.%N`
    FNAME=/monroe/results/neat_test-${DATE}.txt
    TMP_FNAME=/tmp/neat_test-${DATE}.txt

    echo -n "/usr/bin/time -f 'TIME-SEC: %e' ${CMD} 1>/dev/null 2> ${FNAME} ..."
    /usr/bin/time -f 'TIME-SEC: %e' ${CMD} 1>/dev/null 2> ${TMP_FNAME}
    mv $TMP_FNAME $FNAME
    echo " DONE"

    sleep 15

done
