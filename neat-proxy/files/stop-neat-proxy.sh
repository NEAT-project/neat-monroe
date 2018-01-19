#!/bin/bash


# Stop the container

lxc-stop -n neat-proxy

# Stop modem

qmi-network /dev/cdc-wdm1 stop

# Disconnect enp1s0 interface from the container

IF_NAME="enp2s0"
IF_ADDR_WITH_MASK=`ip addr show dev ${IF_NAME} | grep -o "inet [0-9./]*" | cut -d' ' -f2`
IF_ID="1"
IF_TABLEID="101"

dhclient -x -pf /run/dhclient.${IF_NAME}.pid
ip rule del iif ${IF_NAME} table ${IF_TABLEID}
ip rule del iif lxc-${IF_NAME} table ${IF_TABLEID}
ip route flush table ${IF_TABLEID}
ip addr del ${IF_ADDR_WITH_MASK} dev ${IF_NAME}

# Disconnect wwan1 interface from the container

IF_NAME="wwan1"
IF_ADDR_WITH_MASK=`ip addr show dev ${IF_NAME} | grep -o "inet [0-9./]*" | cut -d' ' -f2`
IF_GW=`ip route | grep ${IF_NAME} | grep "^default" | cut -d' ' -f3`

IF_ID="2"
IF_TABLEID="102"

dhclient -x -pf /run/dhclient.${IF_NAME}.pid
ip rule del iif ${IF_NAME} table ${IF_TABLEID}
ip rule del iif lxc-${IF_NAME} table ${IF_TABLEID}
ip route flush table ${IF_TABLEID}
ip addr del ${IF_ADDR_WITH_MASK} dev ${IF_NAME}

# Clean firewall

iptables -t nat -F

# Restore default routing

ip route flush table main
ip route add 192.168.76.0/24 proto kernel scope link dev enp1s0 src 192.168.76.23
ip route add default via 192.168.76.1

echo "OK"

