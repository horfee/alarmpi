/*
 * SIM800Module.h
 *
 *  Created on: 8 août 2015
 *      Author: horfee
 */

#ifndef SIM800MODULE_H_
#define SIM800MODULE_H_

#include <string>
#include <exception>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <ctime>
#include <atomic>

namespace alarmpi {

class SIM800ModuleListener {

public:
	virtual void onMessageReceived(int messageId) = 0;
	virtual void onIncomingCall(std::string callingNumber) = 0;
	virtual ~SIM800ModuleListener() {} ;
};

class SIM800UARTException: public std::exception {};
class SIM800Exception: public std::exception {};
class SIM800TransmitException: public std::exception {};
class SIM800ReceiveException: public std::exception {};

typedef struct {
	short rssi;
	float bitErrorRate;
} SignalQuality;

typedef struct {
	std::string text;
	int status;
} SIM800Command;

typedef enum {
	READ = 1,
	UNREAD = 2,
	SENT = 3,
	UNSENT = 4,
	INBOX = 5,
	ALL = 6
} SMSStatus;

typedef struct {
	int msgId;
	std::string phoneNumber;
	time_t timestamp;
	std::string text;
	SMSStatus status;

} SMS;



class SIM800Module {
public:
	SIM800Module(int resetPin, std::string uartStream, int baudrate);
	virtual ~SIM800Module();

	void reset();

//	void sleep();
//
//	void wakeUp();

	void addListener(SIM800ModuleListener *listener);

	void removeListener(SIM800ModuleListener *listener);

	bool callPhone(std::string numberPhone);

	void hangUp();

	int sendMessage(std::string numberPhone, std::string message);

	std::string getModuleVersion();

	std::string getIMEI();

	bool isReady();

	int getNetworkStatus();

	SignalQuality getSignalQuality();

	std::string getOperator();

	std::vector<std::string> listOperators();

	bool needPinCode();

	void setPinCode(std::string pinCode);

	bool isCallingPhone();

	void answerIncommingCall();

	void toggleDisplayCalleeNumber();	//AT+CLIP=0|1

	void toggleBusy();

	std::vector<SMS> listSMS(SMSStatus status);

	void deleteSMS(int msgId);	//AT+CMGD

	void deleteSMS(SMSStatus status); // AT+CMGDA="DEL <TYPE>"

	bool getClipStatus();

	SMS getSms(int msgId);

private:

	bool callingPhone;

	std::thread* deamon;
	std::thread* unattendedCommands;

	std::vector<SIM800ModuleListener*> listeners;

//#if defined(RPI) and defined(WIRINGPI)
	int simUARTFileStream = 0;
//#endif

	std::atomic<bool> stop;

#ifdef RPI
	int resetPin;
#endif
//#if defined(RPI) and !defined(WIRINGPI)
//	int pi;
//#endif

	void receivingThreadCallBack();
	void unattendCommandsCallBack();
	void sendCommand(std::string command, bool includeRF = true);

	SIM800Command readCommand(std::string awaitedCommand);
	std::vector<std::string> readCommands;

	std::mutex mutex;
	std::mutex sendingMutex;
	std::condition_variable condition;

	bool clipFlag;

};

} /* namespace alarmpi */

#endif /* SIM800MODULE_H_ */
