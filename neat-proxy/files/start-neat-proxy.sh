#!/bin/bash


# Start the container

lxc-start -n neat-proxy || exit 1
LXC_PID=`lxc-info -n neat-proxy -pH`


# Make the container network namespace accessible by name

mkdir -p /var/run/netns
ln -sf /proc/${LXC_PID}/ns/net /var/run/netns/neat-proxy


# Enable packets forwarding

sysctl -w net.ipv4.ip_forward=1
lxc-attach -n neat-proxy -- sysctl -w net.ipv4.ip_forward=1


# Connect enp2s0 interface to the container

IF_NAME="enp2s0"
IF_ID="1"

#ip link set ${IF_NAME} up
ip route flush table main
dhclient -4 -v -pf /run/dhclient.${IF_NAME}.pid -lf /var/lib/dhcp/dhclient.${IF_NAME}.leases -I -df /var/lib/dhcp/dhclient6.${IF_NAME}.leases ${IF_NAME}
echo "nameserver 8.8.8.8" > /etc/resolv.conf

IF_ADDR=`ip addr show dev ${IF_NAME} | grep -o "inet [0-9.]*" | cut -d' ' -f2`
IF_SUBNET=`ip route | grep ${IF_NAME} | grep "^[0-9]" | cut -d' ' -f1`
IF_GW=`ip route | grep ${IF_NAME} | grep "^default" | cut -d' ' -f3`
IF_TABLEID=`echo 100 + ${IF_ID} | bc`

echo "Interface ${IF_NAME} (id=${IF_ID}): Addr ${IF_ADDR} Subnet ${IF_SUBNET} Gw ${IF_GW} TableId ${IF_TABLEID}"
ip link add lxc-${IF_NAME} type veth peer name tmp-${IF_NAME}
ip link set tmp-${IF_NAME} netns neat-proxy name ${IF_NAME}

ip link set lxc-${IF_NAME} up
ip addr add 192.168.${IF_ID}.1/24 dev lxc-${IF_NAME}
ip rule add iif ${IF_NAME} table ${IF_TABLEID}
ip rule add iif lxc-${IF_NAME} table ${IF_TABLEID}
ip route add ${IF_SUBNET} proto kernel scope link table ${IF_TABLEID} dev ${IF_NAME} src ${IF_ADDR}
ip route add 192.168.${IF_ID}.0/24 proto kernel scope link table ${IF_TABLEID} dev lxc-${IF_NAME} src 192.168.${IF_ID}.1
ip route add default via ${IF_GW} table ${IF_TABLEID}
iptables -t nat -A POSTROUTING -o ${IF_NAME} -j MASQUERADE

ip netns exec neat-proxy ip link set ${IF_NAME} up
ip netns exec neat-proxy ip addr add 192.168.${IF_ID}.2/24 dev ${IF_NAME}
ip netns exec neat-proxy ip route add default via 192.168.${IF_ID}.1
ip netns exec neat-proxy iptables -t nat -A POSTROUTING -o ${IF_NAME} -j MASQUERADE
ip netns exec neat-proxy ip rule add from 192.168.${IF_ID}.2 table ${IF_TABLEID}
ip netns exec neat-proxy ip route add default via 192.168.${IF_ID}.1 table ${IF_TABLEID}
ip netns exec neat-proxy ip route add 192.168.${IF_ID}.0/24 dev ${IF_NAME} proto kernel scope link src 192.168.${IF_ID}.2 table ${IF_TABLEID}


# Forward incomming ssh via neat-proxy to lxc-proxy inteface
#iptables -t nat -A PREROUTING -i ${IF_NAME} -p tcp --destination ${IF_ADDR} --dport 22 -j DNAT --to-destination 192.168.${IF_ID}.2
#ip netns exec neat-proxy iptables -t nat -A PREROUTING -i ${IF_NAME} -p tcp --destination 192.168.${IF_ID}.2 --dport 22 -j DNAT --to-destination 172.16.76.100
#exit

# Start wwan1 interface and Connect it to the container
IF_NAME="wwan1"
IF_ID="2"

qmi-network /dev/cdc-wdm1 start
ip route flush table main
dhclient -4 -v -pf /run/dhclient.${IF_NAME}.pid -lf /var/lib/dhcp/dhclient.${IF_NAME}.leases -I -df /var/lib/dhcp/dhclient6.${IF_NAME}.leases ${IF_NAME}
echo "nameserver 8.8.8.8" > /etc/resolv.conf
echo "Interface ${IF_NAME} (id=${IF_ID}): Addr ${IF_ADDR} Subnet ${IF_SUBNET} Gw ${IF_GW} TableId ${IF_TABLEID}"
ip link set dev wwan1 mtu 1500

IF_ADDR=`ip addr show dev ${IF_NAME} | grep -o "inet [0-9.]*" | cut -d' ' -f2`
IF_SUBNET=`ip route | grep ${IF_NAME} | grep "^[0-9]" | cut -d' ' -f1`
IF_GW=`ip route | grep ${IF_NAME} | grep "^default" | cut -d' ' -f3`
IF_TABLEID="102"

ip link add lxc-${IF_NAME} type veth peer name tmp-${IF_NAME}
ip link set tmp-${IF_NAME} netns neat-proxy name ${IF_NAME}

ip link set lxc-${IF_NAME} up
ip addr add 192.168.${IF_ID}.1/24 dev lxc-${IF_NAME}
ip rule add iif ${IF_NAME} table ${IF_TABLEID}
ip rule add iif lxc-${IF_NAME} table ${IF_TABLEID}
ip route add ${IF_SUBNET} proto kernel scope link table ${IF_TABLEID} dev ${IF_NAME} src ${IF_ADDR}
ip route add 192.168.${IF_ID}.0/24 proto kernel scope link table ${IF_TABLEID} dev lxc-${IF_NAME} src 192.168.${IF_ID}.1
ip route add default via ${IF_GW} table ${IF_TABLEID}
iptables -t nat -A POSTROUTING -o ${IF_NAME} -j MASQUERADE

ip netns exec neat-proxy ip link set ${IF_NAME} up
ip netns exec neat-proxy ip addr add 192.168.${IF_ID}.2/24 dev ${IF_NAME}
ip netns exec neat-proxy iptables -t nat -A POSTROUTING -o ${IF_NAME} -j MASQUERADE
ip netns exec neat-proxy ip rule add from 192.168.${IF_ID}.2 table ${IF_TABLEID}
ip netns exec neat-proxy ip route add default via 192.168.${IF_ID}.1 table ${IF_TABLEID}
ip netns exec neat-proxy ip route add 192.168.${IF_ID}.0/24 dev ${IF_NAME} proto kernel scope link src 192.168.${IF_ID}.2 table ${IF_TABLEID}

# Create and configure proxy interface

ip link add lxc-proxy type veth peer name proxy
ip link set proxy netns neat-proxy

ip link set lxc-proxy up
ip addr add 172.16.76.100/16 dev lxc-proxy
ip route flush table main
ip route add 172.16.0.0/16 proto kernel scope link dev lxc-proxy src 172.16.76.100
ip route add default via 172.16.0.1

ip netns exec neat-proxy ip link set proxy up
ip netns exec neat-proxy ip addr add 172.16.0.1/16 dev proxy

ip route add 192.168.76.0/24 proto kernel scope link dev enp1s0 src 192.168.76.23

echo "OK"

