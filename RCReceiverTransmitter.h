/*
 * RCReceiverTransmitter.h
 *
 *  Created on: 17 juil. 2015
 *      Author: horfee
 */

#ifndef RCRECEIVERTRANSMITTER_H_
#define RCRECEIVERTRANSMITTER_H_
#ifdef WIRINGPI

#include <map>
#include <thread>
#include <vector>
#include "RCSwitch.h"
#include <condition_variable>
#include <chrono>

using namespace std;

typedef struct {
	void* recipient;
	int data;
} RCMessage;


class AlreadyRunningException: public exception {};


class RCMessageListener {

public:
	virtual void onSignalReceived(int message) = 0;
	virtual ~RCMessageListener() {} ;
};


class RCReceiverTransmitter {
public:
	RCReceiverTransmitter(int receivingPIN, int transmittingPIN);
	virtual ~RCReceiverTransmitter();

	int getReceivingPIN();
	int getTransmittingPIN();

	void setReceivingPIN(int pin);
	void setTransmittingPIN(int pin);

	void start();
	void stop();

	bool isStarted() const;

	void addListener(RCMessageListener* listener);
	void removeListener(RCMessageListener* listener);

	void sendMessage(int toSend);

private:
	int receivingPIN;
	int transmittingPIN;

	thread* receivingThread;
	thread* transmittingThread;
	bool stopAsked;

	vector<RCMessageListener*> listeners;
	vector<int> messagesToSend;

	std::map<int, std::chrono::milliseconds> receivedTimes;

	void receivingThreadCallBack();
	void sendingThreadCallBack();

	RCSwitch *rcSwitch;

	std::mutex mutex;
	std::condition_variable condition;

};

#endif
#endif /* RCRECEIVERTRANSMITTER_H_ */
