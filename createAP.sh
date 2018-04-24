#!/bin/bash

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -essid)
    essid="$2"
    shift # past argument
    shift # past value
    ;;
    -pass)
    password="$2"
    shift # past argument
    shift # past value
    ;;
    -interface)
    wlan="$2"
    shift # past argument
    shift # past value
    ;;
    *)    # unknown option
    POSITIONAL+=("$1") # save it in an array for later
    shift # past argument
    ;;
esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters


if ! grep -q "denyinterfaces $wlan" /etc/dhcpcd.conf; then
	echo "denyinterfaces $wlan" >> /etc/dhcpcd.conf
fi
systemctl restart dhcpcd

if ! grep -q "#configured=true" /etc/dhcp/dhcpd.conf; then
	rm /etc/dhcp/dhcpd.conf
	echo "default-lease-time 600;" >> /etc/dhcp/dhcpd.conf
	echo "max-lease-time 7200;" >> /etc/dhcp/dhcpd.conf
	echo "option subnet-mask 255.255.255.0;" >> /etc/dhcp/dhcpd.conf
	echo "option broadcast-address 192.168.6.255;" >> /etc/dhcp/dhcpd.conf
	echo "option routers 192.168.6.1;" >> /etc/dhcp/dhcpd.conf
	echo "option domain-name \"alarmpi.wifi\";" >> /etc/dhcp/dhcpd.conf
	echo "option ntp-servers 192.168.6.1;" >> /etc/dhcp/dhcpd.conf	
	echo "subnet 192.168.6.0 netmask 255.255.255.0 {" >> /etc/dhcp/dhcpd.conf
	echo "	range 192.168.6.10 192.168.6.20;" >> /etc/dhcp/dhcpd.conf
	echo "}" >> /etc/dhcp/dhcpd.conf
	echo "#configured=true" >> /etc/dhcp/dhcpd.conf
	
	sed -i "/INTERFACES=.*/Id" /etc/default/isc-dhcp-server
	echo "INTERFACES=$wlan" >>  /etc/default/isc-dhcp-server
fi

sudo mkdir -p /var/run/dhcp
sudo touch /var/lib/dhcp/dhcpcd.leases

if [ $(systemctl -q is-active isc-dhcp-server ) ] ; then
	systemctl restart isc-dhcp-server
else
	systemctl start isc-dhcp-server
fi

#mkdir -p /var/run/wpa_supplicant
echo "country=FR" > /etc/wpa_supplicant/wpa_supplicant.conf
echo "ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev" >> /etc/wpa_supplicant/wpa_supplicant.conf
wpa_passphrase $essid $password | sed "/.*#psk=.*/d" | sed "/.*}.*/d" >> /etc/wpa_supplicant/wpa_supplicant.conf
echo "	mode=2" >> /etc/wpa_supplicant/wpa_supplicant.conf
echo "	key_mgmt=WPA-PSK" >> /etc/wpa_supplicant/wpa_supplicant.conf
echo "	proto=RSN" >> /etc/wpa_supplicant/wpa_supplicant.conf
echo "	pairwise=CCMP" >> /etc/wpa_supplicant/wpa_supplicant.conf
echo "}" >> /etc/wpa_supplicant/wpa_supplicant.conf

ifconfig $wlan down
ifconfig $wlan 192.168.6.1/24 up

wpa_cli reconfigure