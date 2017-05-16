/*
 * AlarmSystem.cpp
 *
 *  Created on: 21 juil. 2015
 *      Author: horfee
 */

#include "AlarmSystem.h"
#include "DAOFactory.h"
#include <algorithm>
#include "Properties.h"
#include <stdio.h>
#include <bitset>
#include <string.h>
#include <sstream>
#include <mutex>
#include <clocale>
#include <tuple>
#include <openssl/md5.h>

namespace alarmpi {

void logMessage(std::string msg) {
	std::cout << msg << std::endl;
}


bool cleanUpDetectedDevicesThreadMustStop = false;
std::mutex cleanUpDetectedDevicesThreadMutex;
std::mutex detectedDevicesMutex;
unsigned int nbActionThreads = 0;
std::condition_variable cv;


void AlarmSystem::cleanUpDetectedDevices() {

	while ( true ) {
		time_t now = time(0);
		std::vector<int> toRemove;
		detectedDevicesMutex.lock();
		for(auto detectedDevice = detectedDevices.begin(); detectedDevice != detectedDevices.end(); detectedDevice++) {
			int key = detectedDevice->first;
			time_t value = detectedDevice->second;
			if ( difftime(now, value) > 90.0) {
				toRemove.push_back(key);
			}
		}
		for(int key : toRemove) {
			detectedDevices.erase(key);
		}
		detectedDevicesMutex.unlock();

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		cleanUpDetectedDevicesThreadMutex.lock();
		bool mustStop = cleanUpDetectedDevicesThreadMustStop;
		cleanUpDetectedDevicesThreadMutex.unlock();
		if ( mustStop ) break;
	}
}

AlarmSystem::AlarmSystem() {
	currentMode = "";//InactiveMode;
#ifdef WIRINGPI
	wirelessModule = NULL;
#else
	rf433Module = NULL;
#endif

//
	gsmModule = NULL;

	loadConfiguration();

	detectedDevicesCleanUpThread = new std::thread(&AlarmSystem::cleanUpDetectedDevices, this);
//	detectedDevices[12345678] = time(0);
}

AlarmSystem::~AlarmSystem() {

	cleanUpDetectedDevicesThreadMutex.lock();
	cleanUpDetectedDevicesThreadMustStop = true;
	cleanUpDetectedDevicesThreadMutex.unlock();
	detectedDevicesCleanUpThread->join();
	delete detectedDevicesCleanUpThread;

	saveConfiguration();

	stopActionThreads();

#ifdef WIRINGPI
	if ( wirelessModule != NULL ) {
		wirelessModule->removeListener(this);
	}
	delete wirelessModule;
#else
	if ( rf433Module != NULL ) {
		rf433Module->removeListener(this);
	}
	delete rf433Module;
#endif

	if ( gsmModule != NULL ) {

	}
	delete gsmModule;

	for(auto device: devices) {
		delete device;
	}
	for(auto property : properties) {
		delete property;
	}
	for(auto mode : modes) {
		delete mode;
	}
	for(auto action : actions) {
		delete action;
	}

	DAOFactory::kill();
}

const std::vector<Property*> AlarmSystem::getProperties() const {
	std::vector<Property*>p(properties);
	p.erase(std::remove(p.begin(), p.end(), getProperty(PROPERTY_PASSWORD)), p.end());
	return p;
}

bool AlarmSystem::testPassword(std::string password) const {
	Property* p = getPassword();
	return password == p->getStringValue();
}

void AlarmSystem::changePassword(std::string password, std::string newPassword) {
	if ( this->testPassword(password) ) {
		Property* p = getPassword();
		p->setStringValue(newPassword);

		AlarmSystemDAO* dao = DAOFactory::getInstance()->getDAO();
		dao->persistProperty(p);
	}
}

Property* AlarmSystem::getPassword() const {
	for(Property *p : properties) {
		if ( *p == PROPERTY_PASSWORD ) return p;
	}

	return NULL;
}

Property* AlarmSystem::getProperty(string propertyName) const {
	if ( propertyName == PROPERTY_PASSWORD ) return NULL;
	for(Property *p : properties) {
		if ( *p == propertyName ) return p;
	}

	return NULL;
}


Action* AlarmSystem::getAction(std::string name) const {
	for(Action* a : actions) {
		if ( a->getName() == name ) return a;
	}
	return NULL;
}

const std::vector<std::tuple<Device*, Mode*, Action*>> AlarmSystem::getAssociations() const {
	std::vector<std::tuple<Device*, Mode*, Action*>> res;

	for(auto it : this->devicesModesActionsAssociations) {
		std::tuple<Device*, Mode*, Action*> t = std::make_tuple(std::get<0>(it.first),getMode(std::get<1>(it.first)), it.second);
		res.push_back(t);
	}
	return res;
}

void AlarmSystem::associateActionAndMode(Device* device, Action* action, Mode* mode) {
	std::tuple<Device*, std::string> key;
	key = std::make_tuple(device,mode->getName());

	AlarmSystemDAO* dao = DAOFactory::getInstance()->getDAO();

	if (devicesModesActionsAssociations.find(key) != devicesModesActionsAssociations.end() ) {
		dao->deleteAssociation(device, mode);
	}

	devicesModesActionsAssociations[key] = action;

	std::cout << "Creating association between " << device->getId() << ", " << mode->getName() << ", " << action->getName() << std::endl;
	dao->persistAssocation(device, mode, action);
}

void AlarmSystem::removeAssociation(Device* device, Mode* mode) {
	if ( device == NULL || mode == NULL ) return;
	std::tuple<Device*, std::string> key;
	key = std::make_tuple(device,mode->getName());
	this->devicesModesActionsAssociations.erase(key);

	AlarmSystemDAO* dao = DAOFactory::getInstance()->getDAO();
	dao->deleteAssociation(device, mode);
}

void AlarmSystem::loadConfiguration() {

	logMessage("Loading config");

	AlarmSystemDAO* dao = DAOFactory::getInstance()->getDAO();

	devices = dao->getDevices();
	modes = dao->getModes();
	phoneNumbers = dao->getPhoneNumbers();
	properties = dao->getProperties();
	actions = dao->getActions();
	for(Action* a : actions) {
		a->system = this;
	}

	vector<std::tuple<int, std::string, std::string>> assocs = dao->getAssociations();
	std::tuple<Device*, std::string> key;
	for(auto t : assocs) {
		Device* d = getDevice(std::get<0>(t));
		Mode* m = getMode(std::get<1>(t));

		if ( m == NULL ) continue;
		Action* a = getAction(std::get<2>(t));

		key = std::make_tuple(d,m->getName());
		this->devicesModesActionsAssociations[key] = a;
	}

	if ( getMode("Active") == NULL ) {
		Mode* active = new Mode("Active", "L'alarme est actuellement active.", ModeType::Active);
		modes.push_back(active);
		dao->persistMode(active);
	}


	if ( getInactiveMode() == NULL ) {
		Mode* inactive = new Mode("Inactive", "L'alarme est actuellement inactive.", ModeType::Inactive);
		modes.push_back(inactive);
		dao->persistMode(inactive);
	}

	if ( getConfigMode() == NULL ) {
		Mode* configuration = new Mode("Configuration","L'alarme est actuellement en cours de configuration.", ModeType::Configuration);
		modes.push_back(configuration);
		dao->persistMode(configuration);
	}


	Property* receivingPinProperty = getProperty(PROPERTY_RECEIVING_PIN);
	Property* transmittingPinProperty = getProperty(PROPERTY_TRANSMITTING_PIN);
	Property* sim800ResetPinProperty  = getProperty(PROPERTY_SIM800RESET_PIN);
	Property* sim800FileStreamProperty  = getProperty(PROPERTY_SIM800RESET_PIN);
	Property* sim800BaudRateProperty  = getProperty(PROPERTY_SIM800RESET_PIN);

	if ( receivingPinProperty == NULL ) {
		receivingPinProperty = new Property(PROPERTY_RECEIVING_PIN, PROPERTY_RECEIVING_PIN_DESCRIPTION, DEFAULT_VALUE_RECEIVING_PIN);
		properties.push_back(receivingPinProperty);
		dao->persistProperty(receivingPinProperty);
	}
	if ( transmittingPinProperty == NULL ) {
		transmittingPinProperty = new Property(PROPERTY_TRANSMITTING_PIN, PROPERTY_TRANSMITTING_PIN_DESCRIPTION, DEFAULT_VALUE_TRANSMITTING_PIN);
		properties.push_back(transmittingPinProperty);
		dao->persistProperty(transmittingPinProperty);
	}

	try {
		logMessage(std::string("Creating RF module on pin ") + std::to_string(receivingPinProperty->getIntValue()) + std::string(" / ") + std::to_string(transmittingPinProperty->getIntValue()));
#ifdef WIRINGPI
		wirelessModule = new RCReceiverTransmitter(receivingPinProperty->getIntValue(), transmittingPinProperty->getIntValue());
		wirelessModule->addListener(this);
#else
		rf433Module = new RF433Module(receivingPinProperty->getIntValue(), transmittingPinProperty->getIntValue());
		rf433Module->addListener(this);
#endif

	} catch ( std::exception &e ) {
#ifdef WIRINGPI
		wirelessModule = NULL;
#else
		rf433Module = NULL;
#endif
		std::cout << "Error catched : " << e.what() << std::endl;
	}

	if ( sim800ResetPinProperty == NULL ) {
		sim800ResetPinProperty  = new Property(PROPERTY_SIM800RESET_PIN, PROPERTY_SIM800RESET_PIN_DESCRIPTION, DEFAULT_VALUE_SIM800RESET_PIN);
		properties.push_back(sim800ResetPinProperty );
		dao->persistProperty(sim800ResetPinProperty );
	}

	if ( sim800FileStreamProperty == NULL ) {
		sim800FileStreamProperty  = new Property(PROPERTY_SIM800STREAM, PROPERTY_SIM800STREAM_DESCRIPTION, std::string(DEFAULT_VALUE_SIM800STREAM));
		properties.push_back(sim800FileStreamProperty );
		dao->persistProperty(sim800FileStreamProperty );
	}

	if ( sim800BaudRateProperty == NULL ) {
		sim800BaudRateProperty  = new Property(PROPERTY_SIM800BAUDRATE, PROPERTY_SIM800BAUDRATE_DESCRIPTION, DEFAULT_VALUE_SIM800BAUDRATE);
		properties.push_back(sim800BaudRateProperty );
		dao->persistProperty(sim800BaudRateProperty );
	}

#ifdef RPI
	try {
		logMessage(std::string("Creating gsm module on ") + sim800FileStreamProperty->getStringValue() + std::string("/ pin ") + std::to_string(sim800ResetPinProperty ->getIntValue()));
		gsmModule = new SIM800Module(sim800ResetPinProperty->getIntValue(), sim800FileStreamProperty->getStringValue(), sim800BaudRateProperty->getIntValue());
	} catch ( alarmpi::SIM800UARTException &e2 ) {
		gsmModule = NULL;
	} catch ( alarmpi::SIM800Exception &e3 ) {
		gsmModule = NULL;
	} catch ( std::runtime_error &re ) {
		std::cerr << re.what() << std::endl;
	}
#else
	gsmModule = NULL;
#endif

	Property* lastModeProperty = getProperty(PROPERTY_CURRENT_MODE);
	if ( lastModeProperty == NULL ) {
		lastModeProperty = new Property(PROPERTY_CURRENT_MODE, PROPERTY_CURRENT_MODE_DESCRIPTION, std::string(DEFAULT_VALUE_CURRENT_MODE));
		properties.push_back(lastModeProperty);
		dao->persistProperty(lastModeProperty);
	}

	Property* passwordProperty = getPassword();
	if ( passwordProperty == NULL ) {
		passwordProperty = new Property(PROPERTY_PASSWORD, PROPERTY_PASSWORD_DESCRIPTION, std::string(DEFAULT_VALUE_PASSWORD));
		properties.push_back(passwordProperty);
		dao->persistProperty(passwordProperty);

	}

	Property* localeProperty = getProperty(PROPERTY_LOCALE);
	if ( localeProperty == NULL ) {
		localeProperty = new Property(PROPERTY_LOCALE, PROPERTY_LOCALE_DESCRIPTION, std::string(DEFAULT_VALUE_LOCALE));
		properties.push_back(localeProperty);
		dao->persistProperty(localeProperty);
	}

	std::locale l = std::locale(localeProperty->getStringValue() + ".UTF-8");
	std::locale::global(l);

	this->activateMode(lastModeProperty->getStringValue(), passwordProperty->getStringValue());

	logMessage("config loaded");
}

void AlarmSystem::saveConfiguration() {
	logMessage("Saving configuration");

	AlarmSystemDAO* dao = DAOFactory::getInstance()->getDAO();
	for(Device *device : devices) {
		dao->persistDevice(device);
	}
	for(Action *action: actions) {
		dao->persistAction(action);
	}
	for(auto assoc : devicesModesActionsAssociations) {
		Device* device = std::get<0>(assoc.first);
		Mode* mode = getMode(std::get<1>(assoc.first));
		Action* action = assoc.second;
		dao->persistAssocation(device, mode, action);
	}

	for(Mode* mode : modes) {
		dao->persistMode(mode);
	}
	for(string phoneNumber : phoneNumbers) {
		dao->persistPhoneNumber(phoneNumber);
	}
	for(Property *property : properties) {
		dao->persistProperty(property);
	}

	logMessage("Configuration saved");
}


void AlarmSystem::setConfigMode(std::string encPassword) {
	activateMode(getConfigMode()->getName(), encPassword);
}

Mode* AlarmSystem::getMode(std::string mode) const {

	for(Mode* m : modes) {
		if ( *m == mode ) return m;
	}
	return NULL;

}

Mode* AlarmSystem::getConfigMode() const {
	for(Mode* m : modes) {
		if ( m->getType() == ModeType::Configuration ) return m;
	}
	return NULL;
}

Mode* AlarmSystem::getInactiveMode() const {
	for(Mode* m : modes) {
		if ( m->getType() == ModeType::Inactive ) return m;
	}
	return NULL;
}

void AlarmSystem::addMode(Mode* mode) { //string mode, string description) {
	if ( !isConfigMode() ) throw NotInConfigModeException();
	if ( mode == NULL ) return;

	if ( getMode(mode->getName()) != NULL ) throw ModeAlreadyExistsException();
	if ( mode->getType() != ModeType::Active ) throw InvalidModeException();
	modes.push_back(mode);
	DAOFactory::getInstance()->getDAO()->persistMode(mode);
}

void AlarmSystem::removeMode(Mode* mode) { //string mode) {
	if ( !isConfigMode() ) throw NotInConfigModeException();
	if( mode->getType() != ModeType::Active ) throw InvalidModeException();
	if ( mode == NULL ) return;

	std::map<std::tuple<Device*, std::string>, Action*>::iterator it = this->devicesModesActionsAssociations.end();
	std::map<std::tuple<Device*, std::string>, Action*>::iterator first = this->devicesModesActionsAssociations.begin();
	while (it != first ) {
		Mode* m = this->getMode(std::get<1>(it->first));
		if (m == mode ) {
			Device* device = std::get<0>(it->first);
			this->removeAssociation(device, mode);
		}
		it--;
	}
	modes.erase(std::remove(modes.begin(), modes.end(), mode), modes.end());
	DAOFactory::getInstance()->getDAO()->deleteMode(mode);

}

void AlarmSystem::removeAction(Action* toRemove) {
	if ( !isConfigMode() ) throw NotInConfigModeException();
	if ( toRemove == NULL ) return;

	std::map<std::tuple<Device*, std::string>, Action*>::iterator it = this->devicesModesActionsAssociations.end();
	std::map<std::tuple<Device*, std::string>, Action*>::iterator first = this->devicesModesActionsAssociations.begin();
	while (it != first ) {
		if (it->second == toRemove ) {
			Device* device = std::get<0>(it->first);
			Mode* mode = this->getMode(std::get<1>(it->first));
			this->removeAssociation(device, mode);
			//devicesModesActionsAssociations.erase(it->first);
		}
		it--;
	}

	for(Action * a : actions) {
		if ( a->getNextAction() == toRemove ) a->setNextAction(NULL);
	}

	actions.erase(std::remove(actions.begin(), actions.end(), toRemove),actions.end());
	DAOFactory::getInstance()->getDAO()->deleteAction(toRemove);
}

void AlarmSystem::addAction(Action* toAdd) {
	if ( !isConfigMode() ) throw NotInConfigModeException();
	if ( toAdd == NULL ) return;
	if ( getAction(toAdd->getName()) != NULL ) throw ActionAlreadyExistsException();
	actions.push_back(toAdd);
	toAdd->system = this;
	DAOFactory::getInstance()->getDAO()->persistAction(toAdd);
}

Action* AlarmSystem::getAssociation(int deviceId, std::string mode) const {

	Device* dev = getDevice(deviceId);
	std::tuple<Device*, std::string> key = std::make_tuple(dev, mode);

	auto it = devicesModesActionsAssociations.find(key);
	if ( it == devicesModesActionsAssociations.end() ) return NULL;
	return it->second;
}

void AlarmSystem::addDevice(Device* toAdd) {
	if ( !isConfigMode() ) throw NotInConfigModeException();
	if ( toAdd == NULL ) return;
	if ( getDevice(toAdd->getId()) != NULL ) throw AlarmDeviceAlreadyExistsException();
	devices.push_back(toAdd);

	std::cout << "Adding device " << toAdd->getId() << std::endl;
	DAOFactory::getInstance()->getDAO()->persistDevice(toAdd);
	detectedDevicesMutex.lock();
	auto d = detectedDevices.find(toAdd->getId());
	if ( d != detectedDevices.end() )
		detectedDevices.erase(d);

	detectedDevicesMutex.unlock();

}

void AlarmSystem::removeDevice(Device* toRemove) {
	if ( !isConfigMode() ) throw NotInConfigModeException();
	if ( toRemove == NULL ) return;

	std::map<std::tuple<Device*, std::string>, Action*>::iterator it = this->devicesModesActionsAssociations.end();
	std::map<std::tuple<Device*, std::string>, Action*>::iterator first = this->devicesModesActionsAssociations.begin();
	while (it != first ) {
		Device* device = std::get<0>(it->first);
		if (device == toRemove ) {
			Mode* mode = this->getMode(std::get<1>(it->first));
			this->removeAssociation(device, mode);
		}
		it--;
	}
	devices.erase(std::remove(devices.begin(), devices.end(), toRemove),devices.end());
	DAOFactory::getInstance()->getDAO()->deleteDevice(toRemove);

}

bool AlarmSystem::isConfigMode() const {
	return getMode(activeMode())->getType() == ModeType::Configuration;
}

bool AlarmSystem::mustStopActionThreads() {
	bool b;
	actionThreadMustStopMutex.lock();
	b = bMustStopActionThreads;
	actionThreadMustStopMutex.unlock();
	return b;
}

void AlarmSystem::stopActionThreads() {

	actionThreadMustStopMutex.lock();
	bMustStopActionThreads = true;
	actionThreadMustStopMutex.unlock();

	// TODO : wait for actionThreads is empty
	std::unique_lock<std::mutex> lk(actionThreadsMutex);
	cv.wait(lk, [&]{ return nbActionThreads == this->actionThreads.size();});

	for(std::thread* t : actionThreads) {
		t->join();
		delete t;
	}
	nbActionThreads = 0;
	this->actionThreads.clear();

	actionThreadMustStopMutex.lock();
	bMustStopActionThreads = false;
	actionThreadMustStopMutex.unlock();
}
void AlarmSystem::activateMode(string mode, string encPassword) {

	Mode* m = getMode(mode);
	if ( m != NULL ) {
		Property* p = getPassword();
		if ( p->getStringValue() != encPassword) throw InvalidPasswordException();

		currentMode = mode;
		if ( m->getType() == ModeType::Inactive ) {
#ifdef WIRINGPI
			if ( this->wirelessModule != NULL ) this->wirelessModule->stop();
#else
			if ( this->rf433Module != NULL ) this->rf433Module->stop();
#endif
		} else {
#ifdef WIRINGPI
			if ( this->wirelessModule != NULL  && !this->wirelessModule->isStarted() ) {
				std::cout << "Activating wireless module" << std::endl;
				try {

					this->wirelessModule->start();
				} catch (AlreadyRunningException &e) {

				}
			}
#else
			if ( this->rf433Module != NULL && !this->rf433Module->isStarted() ) {
				std::cout << "Activating wireless module" << std::endl;
				this->rf433Module->start();
			}
#endif
		}
		stopActionThreads();
		for(Device* dev : devices) {
			if ( typeid(*dev) == typeid(BellDevice)) {
				sendRFMessage((BellDevice*)dev, ((BellDevice*)dev)->getOffValue());
			}
		}

		Property* prop = getProperty(PROPERTY_CURRENT_MODE);
		if ( prop == NULL ) {
			prop = new Property(PROPERTY_CURRENT_MODE, PROPERTY_CURRENT_MODE_DESCRIPTION, mode);
			properties.push_back(prop);
		} else {
			prop->setStringValue(mode);
		}
		DAOFactory::getInstance()->getDAO()->persistProperty(prop);
		logMessage("Mode " + mode + " activated.");
	} else {
		throw UnknownModeException();
	}

}

Device* AlarmSystem::getDevice(int id) const {

	for(Device* device : devices ) {
		if ( *device == id ) return device;
	}

	return NULL;
}

string AlarmSystem::activeMode() const{
	return currentMode;
}

const vector<Mode*> AlarmSystem::getModes() const{
	return modes;
}

void AlarmSystem::sendRFMessage(ActionnableDevice* device, int data) const {
	cout << "I activate via RF the outlet ";
	cout << device->getId();
	cout << " with data ";
	cout << data;
	cout << " (finalword = " << (device->getId() | data) << ")" << endl;

#ifdef WIRINGPI
	if ( this->wirelessModule != NULL ) this->wirelessModule->sendMessage(device->getId() | data);
#else
	if ( this->rf433Module != NULL ) this->rf433Module->sendMessage(device->getId() | data);
#endif


}

void AlarmSystem::sendSMS(std::string message) const {
	if ( gsmModule == NULL ) {
		logMessage("I cannot send the message " + message + " to all phone numbers");
		return;
	}
	logMessage("I send a message " + message + " to all phone numbers");
	for(auto phone : getPhones()) {
		if ( !gsmModule->sendMessage(phone, message) ) {
			logMessage("Sending message failed");
		}
	}

	//AT+CMGF=1
	//AT+CMGS="+33662934347" +33614817996"
	//Mac mac mac
	//CTRL-Z


}

void AlarmSystem::callPhones() const {
	logMessage("I call all phone numbers");
}

const std::map<int, time_t> AlarmSystem::getDetectedDevices() const {
	return detectedDevices;
}

const std::vector<Device*> AlarmSystem::getDevices() const {
	return devices;
}

void AlarmSystem::addProperty(Property* toAdd) {
	if ( !isConfigMode() ) throw NotInConfigModeException();
	if ( toAdd == NULL ) return;

	if ( getProperty(toAdd->getName()) != NULL ) throw PropertyAlreadyExistsException();

	properties.push_back(toAdd);
	DAOFactory::getInstance()->getDAO()->persistProperty(toAdd);

}
void AlarmSystem::removeProperty(Property* toRemove) {
	if ( !isConfigMode() ) throw NotInConfigModeException();
	if ( toRemove == NULL ) return;

	properties.erase(std::remove(properties.begin(), properties.end(), toRemove),properties.end());
	DAOFactory::getInstance()->getDAO()->deleteProperty(toRemove->getName());
}

void AlarmSystem::simulateSignalReceived(std::string data) {

	std::cout << "Simulating device : " << data << std::endl;
	int v = atoi(data.c_str());
	std::thread(&AlarmSystem::onSignalReceived, this, v).detach();

}

void AlarmSystem::onMessageReceived(int messageId) {
	SMS sms = gsmModule->getSms(messageId);
	if ( std::find(phoneNumbers.begin(), phoneNumbers.end(), sms.phoneNumber) != phoneNumbers.end() ) {
		if ( getMode(sms.text) != NULL ) {
			this->activateMode(sms.text, getPassword()->getStringValue());
		}

	}

}

void AlarmSystem::onIncomingCall(std::string callingNumber) {

}

void AlarmSystem::onSignalReceived(int data) {
	Device* dev = getDevice(data);
	std::cout << "Message Received " << std::bitset<32>(data) << std::endl;

	if ( isConfigMode() ) {
		if ( dev == NULL ) {
			detectedDevicesMutex.lock();
			detectedDevices[data] = time(0);
			std::cout << "Creating device  " << std::bitset<32>(data) << std::endl;
			detectedDevicesMutex.unlock();
		}
	} else {
		Mode *currentMode = this->getMode(activeMode());
		if ( currentMode->getType() != ModeType::Inactive ) {
			if ( dev != NULL ) {
				std::cout << "Device found     : " << std::bitset<32>(dev->getId()) << " " << dev->getDescription() << std::endl;
				std::tuple<Device*, std::string> key = std::make_tuple(dev, currentMode->getName());
				Action* toExecute = this->devicesModesActionsAssociations[key];
				if ( toExecute != NULL ) {
//					Mode* mode = getMode(currentMode);
					actionThreadsMutex.lock();
//					std::thread *t = NULL;
					actionThreads.push_back(/*t = */new std::thread([=]{
						Action* a = toExecute;
						std::cout << "Start thread" << std::endl;
						while ( a != NULL && !mustStopActionThreads() ) {
							std::cout << "Executing " << a->getName() << std::endl;
							a->execute(dev, currentMode);
							a = a->getNextAction();
						}

						std::lock_guard<std::mutex> lk(actionThreadsMutex);
						nbActionThreads++;
						cv.notify_one();

					}));
					actionThreadsMutex.unlock();

				}
			} else {
				cout << "Device not found : " << std::bitset<32>(data) << endl;
			}
		}
	}
}

const std::vector<std::string> AlarmSystem::getPhones() const {
	return phoneNumbers;
}

void AlarmSystem::addPhoneNumber(std::string phoneNumber, int order) {

	int pos = order == -1 ? phoneNumbers.size() : min((int)order, (int)phoneNumbers.size());
	removePhoneNumber(phoneNumber);
	phoneNumbers.insert(phoneNumbers.begin() + pos, phoneNumber);
	DAOFactory::getInstance()->getDAO()->persistPhoneNumber(phoneNumber, pos);
}

void AlarmSystem::removePhoneNumber(std::string phoneNumber) {
	phoneNumbers.erase(std::remove(phoneNumbers.begin(), phoneNumbers.end(), phoneNumber), phoneNumbers.end());
	DAOFactory::getInstance()->getDAO()->deletePhoneNumber(phoneNumber);
}


int AlarmSystem::getPhone(std::string phone) const {
	int i = 1;
	for(auto it : phoneNumbers) {
		if ( phone == it ) return i;
		i++;
	}

	return -1;
}

const std::vector<Action*> AlarmSystem::getActions() const {
	return actions;
}

std::vector<alarmpi::WifiDesc> AlarmSystem::listWifi() const {
	return wifi.listWifi();
}

std::string AlarmSystem::getCurrentESSI() const {
	return wifi.getCurrentESSI();
}

bool AlarmSystem::connectToWifi(std::string essid, std::string password) const {
	return wifi.connectToWifi(essid, password);
}

bool AlarmSystem::createAccessPoint(std::string essid, std::string password) {
	return wifi.createAccessPoint(essid, password);
}

std::vector<std::string> AlarmSystem::ipAddresses() const {
	return wifi.ipAddresses();
}

int AlarmSystem::isConnectedToNetwork() const {
	return wifi.isConnectedToNetwork();
}


std::string AlarmSystem::getVersion() const {
	return VERSION;
}

} /* namespace alarmpi */
