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
	std::cout << "Destroying receiver" << endl;
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
	std::cout << "Receiving Thread starting" << std::endl;
	while ( !this->stopAsked ) {
//		std::cout << "Receiving Waiting here" << std::endl;

		int val =this->rcSwitch->waitForValue();
		if ( !this->stopAsked && val != -1) {
			std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
			std::map<int, std::chrono::milliseconds>::iterator t = this->receivedTimes.find(val);
			if ( t == this->receivedTimes.end()) {
				this->receivedTimes[val] = ms;
				for(RCMessageListener* l : this->listeners) {
					l->onSignalReceived(val);
				}
			} else {
	//			cout << "\tNow   : " << ms.count() << endl;
	//			cout << "\tFound : " << receivedTimes[val].count() << endl;
	//			cout << "\tDiff  : " << (ms - receivedTimes[val]).count() << endl;
				if ( (ms - receivedTimes[val]).count() > 1000) {
					this->receivedTimes[val] = ms;
					for(RCMessageListener* l : this->listeners) {
						l->onSignalReceived(val);
					}
				}
			}
		}

		this->receivingThread = NULL;
//		delay(100);
//		if ( this->rcSwitch->available( ) ) {
//			int value = this->rcSwitch->getReceivedValue();
//			if ( value == -1 ) {
//				cout << "Error while waiting for value" << endl;
//			} else {

//				cout << micros() << " Received value " << value << endl;
//			}
//
//			this->rcSwitch->resetAvailable();
//		}
	}

	std::cout << "Receiving Thread ended" << std::endl;
}

void RCReceiverTransmitter::sendingThreadCallBack() {
	std::cout << "Sending Thread starting" << std::endl;
	while ( !this->stopAsked ) {

		std::cout << "Sending Waiting here" << std::endl;
		std::unique_lock<std::mutex> lock(mutex);
		condition.wait(lock);//, [=](){ return !this->messagesToSend.empty();});
		if ( !this->stopAsked ) {
			cout << "Getting new message ";
			int m = this->messagesToSend.back();
			this->messagesToSend.pop_back();
			cout << std::bitset<24>(m) << endl;
			this->rcSwitch->send(m, 24);
			cout << "Message sent " << endl;
//			delay(1000);
		}

	}

	this->transmittingThread = NULL;
	std::cout << "Sending Thread ended" << std::endl;
}

void RCReceiverTransmitter::stop() {
	std::cout << "Asked to stop" << std::endl;
	this->stopAsked = true;
	condition.notify_all();
	this->rcSwitch->disableReceive();
}

#endif
