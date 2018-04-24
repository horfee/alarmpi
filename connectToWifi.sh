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

systemctl stop isc-dhcp-server

sed -i "/denyinterfaces $wlan/Id" /etc/dhcpcd.conf
systemctl restart dhcpcd

#mkdir -p /var/run/wpa_supplicant
rm /etc/wpa_supplicant/wpa_supplicant.conf
echo "country=FR" > /etc/wpa_supplicant/wpa_supplicant.conf
echo "ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev" >> /etc/wpa_supplicant/wpa_supplicant.conf

wpa_passphrase $essid $password | sed "/.*#psk=.*/d" >> /etc/wpa_supplicant/wpa_supplicant.conf
ifconfig $wlan down
ifconfig $wlan up

wpa_cli reconfigure