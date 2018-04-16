/*
 * WifiModule.cpp
 *
 *  Created on: 4 avr. 2017
 *      Author: horfee
 */

#include "NetworkModule.h"
#include <cstdio>
#include <iostream>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <array>
#include <chrono>
#include "Utils.h"
#include <algorithm>
#include <unistd.h>

namespace alarmpi {

std::string exec(std::string cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != NULL)
            result += buffer.data();
    }
    return trim(result);
}

bool isConnectedToNetworkThroughInterface(std::string interface) {
	std::string state = exec("cat /sys/class/net/" + interface + "/operstate");
	return state == "up";
}

std::string getWifiInterface() {
	return exec("sudo iw dev | sed -n '/^\\sInterface/p' | sed -r 's/\\sInterface\\s//'");
}

NetworkModule::NetworkModule() {
	deamonThread = NULL;
	askedToStop = false;
}

NetworkModule::~NetworkModule() {
}

void NetworkModule::startDeamondWithDefaultPassword(std::string defaultPassword) {
	if ( deamonThread != NULL ) return;
	askedToStop = false;
	this->deamonThread= new std::thread(&NetworkModule::deamonThreadCallback, this, defaultPassword);
}

void notifyListeners(const NetworkModule &module) {

	//it = module.listeners.cbegin();

	//for(auto it = listeners.cbegin(); it != listeners.cend(); ++it) {
	//(*it)->onConnectionStateChanged(module.isConnectedToNetwork());
	//}
}

void NetworkModule::deamonThreadCallback(std::string defaultPassword) {
	while (!askedToStop ) {
		std::unique_lock<std::mutex> lck(this->mutex);
		this->conditionVariable.wait_for(lck, std::chrono::seconds(30));
		logMessage( LOG_INFO, "Checking if connected to a network");
		if ( !isConnectedToNetworkThroughInterface(getWifiInterface()) ) {
			this->createAccessPoint(DEFAULT_ESSID, defaultPassword);
		}

	}

}

void NetworkModule::stopDeamon() {
	if ( deamonThread == NULL ) return;
	askedToStop = true;

	deamonThread->join();
}


std::vector<WifiDesc> NetworkModule::listWifi() const {
	std::string interface = getWifiInterface();
	if ( interface == "" ) return {};
	std::string cmd = "sudo iw dev " + interface + " scan | grep 'SSID\\|signal' | sed -r 's/^\\s.*: //g;s/ dBm//'";
	std::string list = exec(cmd);

	std::istringstream iss(list);
	std::string line;
	int i = 0;
	float strength = 0;

	std::vector<WifiDesc> res;
	while (std::getline(iss, line))
	{
		if ( i % 2 == 0 ) {
			strength = atof(line.c_str());
		} else {
			res.push_back({line, strength});
		}
		i++;
	}
	return res;
}

std::string getWorkingDirectory() {
	char buf[2048];
	getcwd(buf, 2048 * sizeof(char));
	return std::string(buf) + "/";
}

std::string NetworkModule::getCurrentESSI() const {
	return exec("iwgetid " + getWifiInterface() + " -r");
}

void NetworkModule::addListener(NetworkListener *l) {
	if ( l )
		listeners.push_back(l);
}

void NetworkModule::removeListener(NetworkListener *l) {
	if ( l ) {
		listeners.erase(std::remove(listeners.begin(), listeners.end(), l), listeners.end());
	}
}


bool NetworkModule::connectToWifi(std::string essid, std::string password) const {
	logMessage( LOG_INFO, "Connecting to network " + essid + "\n");
	logMessage( LOG_DEBUG, exec(getWorkingDirectory() + "connectToWifi.sh -essid " + essid + " -pass " + password + " -interface " + getWifiInterface()));

	for(auto l : this->listeners) {
		l->onConnectionStateChanged(this->isConnectedToNetwork());
	}
	//notifyListeners(*this);
	//sudo su -c 'wpa_supplicant -D nl80211,wext -i wlan0 -c <(wpa_passphrase "Snubbyland" "Batrie59")'
	/*
	std::string wpaFile = "/etc/wpa_supplicant/wpa_supplicant.conf";
	logMessage( LOG_DEBUG, exec("rm " + wpaFile));
	logMessage( LOG_DEBUG, exec("rm " + wpaFile));
	logMessage( LOG_DEBUG, exec("echo country=GB>>" + wpaFile));
	logMessage( LOG_DEBUG, exec("echo ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev>>" + wpaFile));
	logMessage( LOG_DEBUG, exec("echo update_config=1>>" + wpaFile));
	logMessage( LOG_DEBUG, exec("echo>>" + wpaFile));
	logMessage( LOG_DEBUG, exec("wpa_passphrase  \"" + essid + "\" \"" + password + "\" | sed \"/#psk=.* /d\">>" + wpaFile));
	logMessage( LOG_DEBUG, exec("systemctl start wpa_supplicant"));
	logMessage( LOG_DEBUG, exec("ifdown " + getWifiInterface()));
	logMessage( LOG_DEBUG, exec("ifup " + getWifiInterface()));
	*/
	return true;
}

bool NetworkModule::createAccessPoint(std::string essid, std::string password) {
	logMessage( LOG_INFO, "creating access point\n");
	logMessage( LOG_DEBUG, exec(getWorkingDirectory() + "createAP.sh -essid AlarmPI -pass " + password + " -interface " + getWifiInterface()));
	for(auto l : this->listeners) {
		l->onConnectionStateChanged(this->isConnectedToNetwork());
	}
	//notifyListeners(*this);

	/*
	logMessage( LOG_DEBUG, exec("systemctl stop wpa_supplicant"));
	logMessage( LOG_DEBUG, exec("ifconfig " + getWifiInterface() + " down"));
	logMessage( LOG_DEBUG, exec("echo \"denyinterfaces " +getWifiInterface() + "\" >> /etc/dhcpcd.conf"));
	logMessage( LOG_DEBUG, exec("systemctl restart dhcpcd"));
	logMessage( LOG_DEBUG, exec("ifconfig " + getWifiInterface() + " 192.168.0.1 netmask 255.255.255.0 up"));
	logMessage( LOG_DEBUG, exec(
			"echo -e \"interface=" + getWifiInterface() + "\n"
			"driver=nl80211\n"
			"ssid=" + essid + "\n"
			"hw_mode=g\n"
			"ieee80211n=1\n"
			"wmm_enabled=1\n"
			"macaddr_acl=0\n"
			"channel=0"
			"auth_algs=1\n"
			"ignore_broadcast_ssid=0\n"
			"wpa=2\n"
			"wpa_key_mgmt=WPA-PSK\n"
			"wpa_passphrase=" + password + "\n"
			"rsn_pairwise=CCMP\" > /etc/hostapd/hostapd.conf"));

	logMessage( LOG_DEBUG, exec("hostapd -dd -B /etc/hostapd/hostapd.conf"));
	*/
	return true;
}

std::vector<std::string> NetworkModule::ipAddresses() const {
#ifdef RPI
	return split(exec("hostname -I"), {" "});
#else
	return {};
#endif
}

bool NetworkModule::isConnectedToNetwork() const {
	return ipAddresses().empty();
}

};
