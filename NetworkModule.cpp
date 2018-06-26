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

void NetworkModule::startConnectionManager() {
	if ( deamonThread != NULL ) return;
	askedToStop = false;
	this->deamonThread= new std::thread(&NetworkModule::deamonThreadCallback, this);
}

//void notifyListeners(const NetworkModule &module) {
//
//	for(auto it = module.listeners.cbegin(); it != module.listeners.cend(); ++it) {
//		(*it)->onConnectionStateChanged(module.isConnectedToNetwork());
//	}
//}

void NetworkModule::setEssidPassword(std::string password){
	this->ssidPassword = password;
}

void NetworkModule::deamonThreadCallback() {
	while (!askedToStop ) {
		std::unique_lock<std::mutex> lck(this->mutex);
		this->conditionVariable.wait_for(lck, std::chrono::seconds(30));
		logMessage( LOG_INFO, "Checking if connected to a network on wifi");
		if ( !isConnectedToNetworkThroughInterface(getWifiInterface()) ) {
			this->createAccessPoint(DEFAULT_ESSID, this->ssidPassword);
		}

	}

}

void NetworkModule::stopConnectionManager() {
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

	return true;
}

bool NetworkModule::createAccessPoint(std::string essid, std::string password) {
	logMessage( LOG_INFO, "creating access point\n");
	logMessage( LOG_DEBUG, exec(getWorkingDirectory() + "createAP.sh -essid AlarmPI -pass " + password + " -interface " + getWifiInterface()));
	for(auto l : this->listeners) {
		l->onConnectionStateChanged(this->isConnectedToNetwork());
	}

	return true;
}

std::vector<std::string> NetworkModule::ipAddresses() const {
#ifdef RPI
	return split(exec("hostname -i"), {" "});
#else
	return {};
#endif
}

bool NetworkModule::isConnectedToNetwork() const {
	return ipAddresses().empty();
}

};
