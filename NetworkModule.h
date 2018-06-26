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
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "Singleton.h"

#define	DEFAULT_ESSID	"AlarmPI"

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

	void startConnectionManager();

	void stopConnectionManager();

	std::vector<WifiDesc> listWifi() const;

	std::string getCurrentESSI() const;

	std::vector<std::string> ipAddresses() const;

	bool isConnectedToNetwork() const;

	bool connectToWifi(std::string essid, std::string password) const;

	bool createAccessPoint(std::string essid, std::string password);

	void addListener(NetworkListener *listener);

	void removeListener(NetworkListener *listener);

	void setEssidPassword(std::string password);

private:

	void deamonThreadCallback();

	std::thread* deamonThread;

	std::atomic<bool> askedToStop;

	std::mutex mutex;

	std::condition_variable conditionVariable;

	std::string ssidPassword;

	std::vector<NetworkListener*> listeners;

};

}

#endif /* SOURCES_NETWORKMODULE_H_ */
