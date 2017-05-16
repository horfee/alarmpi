/*
 * AlarmSystem.h
 *
 *  Created on: 21 juil. 2015
 *      Author: horfee
 */

#ifndef ALARMSYSTEM_H_
#define ALARMSYSTEM_H_

#define VERSION "1.0.0"

#include <vector>
#include <ctime>
#include <thread>

#include "devices/AllDevices.h"
#include "Property.h"
#include "actions/Action.h"
#include "Mode.h"
#include "NetworkModule.h"
#include "SIM800Module.h"

#ifdef WIRINGPI
#include "RCReceiverTransmitter.h"
#else
#include "RF433Module.h"
#endif

namespace alarmpi {

class Action;


class NotInConfigModeException: public exception {};
class ModeAlreadyExistsException: public exception {};
class InvalidModeException: public exception{};
class PropertyAlreadyExistsException: public exception {};
class AlarmDeviceAlreadyExistsException: public exception {};
class ActionAlreadyExistsException: public exception {};
class InvalidPasswordException: public exception {};
class UnknownModeException: public exception {};

typedef std::tuple<Device*, Mode*, Action*> Association;

class AlarmSystem:
#ifdef WIRINGPI
		private RCMessageListener
#else
		private RF433MessageListener
#endif
{
public:
	AlarmSystem();
	virtual ~AlarmSystem();

	const std::vector<Action*> getActions() const;
	const std::vector<Device*> getDevices() const;
	const std::map<int, time_t> getDetectedDevices() const;
	const std::vector<std::string> availablePhoneNumbers() const;
	const std::vector<Mode*> getModes() const;
	const std::vector<std::string> getPhones() const;
	const std::vector<Property*> getProperties() const;
	const std::vector<Association> getAssociations() const;
	std::string activeMode() const;
	void activateMode(std::string mode, std::string password);

	void setConfigMode(std::string encPassword);
	bool isConfigMode() const;


	/*********** Available when in config mode */
	void addAction(Action* toAdd);
	void removeAction(Action* toRemove);

	void associateActionAndMode(Device* device, Action* action, Mode* mode);
	void removeAssociation(Device* device, Mode* mode);

	void addDevice(Device* toAdd);
	void removeDevice(Device* toRemove);

	void addMode(Mode* mode);
	void removeMode(Mode* toRemove);

	void addPhoneNumber(std::string phoneNumber, int order = -1);
	void removePhoneNumber(std::string phoneNumber);

	void addProperty(Property* toAdd);
	void removeProperty(Property* toRemove);

	std::vector<alarmpi::WifiDesc> listWifi() const;
	std::string getCurrentESSI() const;
	bool connectToWifi(std::string essid, std::string password) const;
	bool createAccessPoint(std::string essid, std::string password);
	std::vector<std::string> ipAddresses() const;
	int isConnectedToNetwork() const;
	/*********** End of available when in config mode */


	void sendRFMessage(ActionnableDevice* bell, int data) const;
	void sendSMS(std::string message) const;
	void callPhones() const;
	void sendMail(std::string recipient, std::string object, std::string message) const;

	Device* getDevice(int id) const;
	Property* getProperty(std::string propertyName) const;

	int getPhone(std::string phone) const;
	Mode* getMode(std::string mode) const;
	Action* getAction(std::string name) const;

	Action* getAssociation(int deviceId, std::string mode) const;


	bool testPassword(std::string password) const;
	void changePassword(std::string password, std::string newPassword);

	void simulateSignalReceived(std::string signal);

	std::string getVersion() const;
protected:
	virtual void loadConfiguration();
	virtual void onSignalReceived(int value);
	virtual void onMessageReceived(int messageId);
	virtual void onIncomingCall(std::string callingNumber);

	std::vector<Property*> properties;
	std::vector<std::string> phoneNumbers;



private:

	void cleanUpDetectedDevices();

	virtual void saveConfiguration();

	Property* getPassword() const;

	std::vector<Device*> devices;

	std::map<int, time_t> detectedDevices;

	std::vector<Mode*> modes;

	std::string currentMode;

#ifdef WIRINGPI
	RCReceiverTransmitter* wirelessModule;
#else
	RF433Module* rf433Module;
#endif

	std::vector<Action*> actions;

	std::map<std::tuple<Device*, std::string>, Action*> devicesModesActionsAssociations;

	std::thread* detectedDevicesCleanUpThread;

	SIM800Module* gsmModule;

	bool mustStopActionThreads();

	bool bMustStopActionThreads;

	std::vector<std::thread *> actionThreads;
	std::mutex actionThreadsMutex;

	std::mutex actionThreadMustStopMutex;

	NetworkModule wifi;

	void stopActionThreads() ;

	Mode* getConfigMode() const;
	Mode* getInactiveMode() const;
};

} /* namespace alarmpi */

#endif /* ALARMSYSTEM_H_ */
