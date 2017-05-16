/*
 * WifiModule.h
 *
 *  Created on: 4 avr. 2017
 *      Author: horfee
 */

#ifndef SOURCES_NETWORKMODULE_H_
#define SOURCES_NETWORKMODULE_H_

#include <string>
#include <vector>
#include "Singleton.h"

namespace alarmpi {

class NetworkListener {
public:
	virtual void onConnectionStateChanged(bool connected) = 0;
	virtual void onClientConnected(bool created) = 0;
	virtual ~NetworkListener() {};
};


typedef struct {
	std::string name;
	float strength;
} WifiDesc;

class NetworkModule : public Singleton<NetworkModule> {
	friend NetworkModule* Singleton<NetworkModule>::getInstance();
	friend void Singleton<NetworkModule>::kill();

public:
	NetworkModule();
	virtual ~NetworkModule();


	std::vector<WifiDesc> listWifi() const;

	std::string getCurrentESSI() const;

	std::vector<std::string> ipAddresses() const;

	int isConnectedToNetwork() const;

	bool connectToWifi(std::string essid, std::string password) const;

	bool createAccessPoint(std::string essid, std::string password);

private:



};

}

#endif /* SOURCES_NETWORKMODULE_H_ */
