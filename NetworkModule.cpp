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
#include "StringUtils.h"

namespace alarmpi {

NetworkModule::NetworkModule() {
}

NetworkModule::~NetworkModule() {
}



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

std::string getWifiInterface() {
	return exec("sudo iw dev | sed -n '/^\\sInterface/p' | sed -r 's/\\sInterface\\s//'");
}

std::vector<WifiDesc> NetworkModule::listWifi() const {
	std::string interface = getWifiInterface();
	std::string cmd = "sudo iw dev " + interface + " scan | grep 'SSID\\|signal' | sed -r 's/^\\s.*: //g;s/ dBm//'";
	std::string list = exec(cmd);

//	std::cout << "Result : " << std::endl << list << std::endl << "---" << std::endl;
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

std::string NetworkModule::getCurrentESSI() const {
	return exec("iwgetid " + getWifiInterface() + " -r");
}

bool NetworkModule::connectToWifi(std::string essid, std::string password) const {
	//sudo su -c 'wpa_supplicant -D nl80211,wext -i wlan0 -c <(wpa_passphrase "Snubbyland" "Batrie59")'
	return false;
}

bool NetworkModule::createAccessPoint(std::string essid, std::string password) {
	return false;
}

std::vector<std::string> NetworkModule::ipAddresses() const {
#ifdef RPI
	return split(exec("hostname -I"), {" "});
#else
	return {};
#endif
}

int NetworkModule::isConnectedToNetwork() const {
	return ipAddresses().empty();
}

};
