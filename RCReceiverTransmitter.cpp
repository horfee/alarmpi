/*
 * RCReceiverTransmitter.cpp
 *
 *  Created on: 17 juil. 2015
 *      Author: horfee
 */

#include "RCReceiverTransmitter.h"
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <bitset>
#include "Utils.h"

#ifdef WIRINGPI

RCReceiverTransmitter::RCReceiverTransmitter(int receivingPIN, int transmittingPIN) {

	this->receivingThread = NULL;
	this->transmittingThread = NULL;

	this->stopAsked = false;

#ifdef RPI
//	if ( wiringPiSetup() == -1 ) {
//		throw std::logic_error("unable to initialize wiring API");
//	}
#endif

	this->transmittingPIN = -1;
	this->receivingPIN = -1;

	this->rcSwitch = new RCSwitch();
	this->rcSwitch->setProtocol(2);
	this->setReceivingPIN(receivingPIN);
	this->setTransmittingPIN(transmittingPIN);


}


void RCReceiverTransmitter::sendMessage(int toSend) {
	this->messagesToSend.push_back(toSend);
	condition.notify_all();
}

RCReceiverTransmitter::~RCReceiverTransmitter() {
	logMessage( LOG_DEBUG, "Destroying receiver");
	this->stop();
	if ( this->receivingThread != NULL ) {
		this->receivingThread->join();
		delete this->receivingThread;
	}

	if ( this->transmittingThread != NULL ) {
		this->transmittingThread->join();
		delete this->transmittingThread;
	}

	this->listeners.clear();
	this->messagesToSend.clear();
	delete rcSwitch;
}

int RCReceiverTransmitter::getReceivingPIN() {
	return this->receivingPIN;
}

int RCReceiverTransmitter::getTransmittingPIN() {
	return this->transmittingPIN;
}

void RCReceiverTransmitter::setReceivingPIN(int pin) {
	this->receivingPIN = pin;
	if ( this->receivingPIN != -1) {
			this->rcSwitch->enableReceive(this->receivingPIN);
	} else {
		this->rcSwitch->disableReceive();
	}
}

void RCReceiverTransmitter::setTransmittingPIN(int pin) {
	this->transmittingPIN = pin;
	if ( this->transmittingPIN != -1 ) {
		this->rcSwitch->enableTransmit(this->transmittingPIN);
	} else {
		this->rcSwitch->disableTransmit();
	}
}

bool RCReceiverTransmitter::isStarted() const {
	return this->receivingThread != NULL && this->receivingThread->joinable();
}

void RCReceiverTransmitter::start() {
	if ( this->receivingThread != NULL || this->transmittingThread != NULL ) {
		throw AlreadyRunningException();
	}

	stopAsked = false;
	if ( this->receivingThread == NULL ) {
		this->receivingThread = new thread(&RCReceiverTransmitter::receivingThreadCallBack, this);
		this->setReceivingPIN(this->receivingPIN);
	}
	if ( this->transmittingThread == NULL ) {
		this->transmittingThread = new thread(&RCReceiverTransmitter::sendingThreadCallBack, this);
	}
}

void RCReceiverTransmitter::addListener(RCMessageListener* listener) {
	this->listeners.push_back(listener);
}

void RCReceiverTransmitter::removeListener(RCMessageListener* listener) {
	std::vector<RCMessageListener*>::iterator it = std::find(this->listeners.begin(), this->listeners.end(), listener);
	if (it != this->listeners.end()) this->listeners.erase(it);

}

void RCReceiverTransmitter::receivingThreadCallBack() {
	logMessage( LOG_DEBUG, "Receiving Thread starting");
	while ( !this->stopAsked ) {

		int val =this->rcSwitch->waitForValue();
		logMessage(LOG_DEBUG,"Received : %d", val);
		if ( !this->stopAsked && val != -1) {
			std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
			std::map<int, std::chrono::milliseconds>::iterator t = this->receivedTimes.find(val);
			if ( t == this->receivedTimes.end()) {
				this->receivedTimes[val] = ms;
				for(RCMessageListener* l : this->listeners) {
					l->onSignalReceived(val);
				}
			} else {
				if ( (ms - receivedTimes[val]).count() > 1000) {
					this->receivedTimes[val] = ms;
					for(RCMessageListener* l : this->listeners) {
						l->onSignalReceived(val);
					}
				}
			}
		}

		this->receivingThread = NULL;
	}

	logMessage( LOG_DEBUG, "Receiving Thread ended");
}

void RCReceiverTransmitter::sendingThreadCallBack() {
	logMessage( LOG_DEBUG, "Sending Thread starting");
	while ( !this->stopAsked ) {
		logMessage( LOG_DEBUG, "Sending Waiting here");
		std::unique_lock<std::mutex> lock(mutex);
		condition.wait(lock);//, [=](){ return !this->messagesToSend.empty();});
		if ( !this->stopAsked ) {
			logMessage( LOG_DEBUG, "Getting new message");
			int m = this->messagesToSend.back();
			this->messagesToSend.pop_back();
			logMessage( LOG_DEBUG, std::bitset<24>(m).to_string().c_str());
			this->rcSwitch->send(m, 24);
			logMessage( LOG_DEBUG, "Message sent");
//			delay(1000);
		}

	}

	this->transmittingThread = NULL;
	logMessage( LOG_DEBUG, "Sending Thread ended");
}

void RCReceiverTransmitter::stop() {
	logMessage( LOG_DEBUG, "Asked to stop");
	this->stopAsked = true;
	condition.notify_all();
	this->rcSwitch->disableReceive();
}

#endif
