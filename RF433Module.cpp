/*
 * RF433Module.cpp
 *
 *  Created on: 10 mai 2017
 *      Author: horfee
 */

#include "RF433Module.h"
#include <stdlib.h>
#include <algorithm>
#include <pigpiod_if2.h>
#include <functional>
#include <iostream>
#include "Utils.h"

#ifdef RPI
#ifndef WIRINGPI

namespace alarmpi {

RF433Module::RF433Module(int receivingPIN, int transmittingPIN) {
	this->transmittingPIN = transmittingPIN;
	this->receivingPIN = receivingPIN;
	started = false;
	sending = false;
#ifdef RPI
	tx = NULL;
	rx = NULL;

	pi = -1;
#endif


}

RF433Module::~RF433Module() {
	// TODO Auto-generated destructor stub
}

int RF433Module::getReceivingPIN() {
	return receivingPIN;
}
int RF433Module::getTransmittingPIN() {
	return transmittingPIN;
}

void RF433Module::setReceivingPIN(int pin) {
	receivingPIN = pin;
}
void RF433Module::setTransmittingPIN(int pin) {
	transmittingPIN = pin;
}

template <typename T>
struct Callback;

template <typename Ret, typename... Params>
struct Callback<Ret(Params...)> {
    template <typename... Args>
    static Ret callback(Args... args) { return func(args...); }
    static std::function<Ret(Params...)> func;
};

// Initialize the static member.
template <typename Ret, typename... Params>
std::function<Ret(Params...)> Callback<Ret(Params...)>::func;

void RF433Module::start() {

#ifdef RPI
	int optRx     = receivingPIN;
	int optTx     = transmittingPIN;

	int optBits = 24;
	int optRepeats = 6;
	int opt0 = 300;
	int opt1 = 900;
	int optGap = 9000;

	int optMinBits = 8;
	int optMaxBits = 32;
	int optGlitch = 150;


	char *optHost   = NULL;
	char *optPort   = NULL;

	pi = pigpio_start(optHost, optPort);
	started = pi >= 0;
	if ( started ) {
		logMessage(LOG_DEBUG, "PIGPIO successfully started");
		Callback<void(_433D_rx_data_t)>::func = std::bind(&RF433Module::cbf, this, std::placeholders::_1);
	    // Convert callback-function to c-pointer.
	    void (*c_func)(_433D_rx_data_t) = static_cast<decltype(c_func)>(Callback<void(_433D_rx_data_t)>::callback);

	    rx = _433D_rx(pi, optRx, c_func);

		_433D_rx_set_bits(rx, optMinBits, optMaxBits);
		_433D_rx_set_glitch(rx, optGlitch);

		tx = _433D_tx(pi, optTx);
		_433D_tx_set_bits(tx, optBits);
		_433D_tx_set_repeats(tx, optRepeats);
		_433D_tx_set_timings(tx, optGap, opt0, opt1);
	} else {
		logMessage(LOG_ERR, "Unable to connect to pigpio");
	}
#endif

}

#ifdef RPI
void RF433Module::cbf(_433D_rx_data_t r){
	if ( sending ) return;
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch());
	std::map<int, std::chrono::milliseconds>::iterator t = this->receivedTimes.find(r.code);
	if ( t == this->receivedTimes.end()) {
		for(RF433MessageListener* l : this->listeners) {
			l->onSignalReceived(r.code);
		}
	} else {
		if ( (ms - receivedTimes[r.code]).count() > 1000) {
			for(RF433MessageListener* l : this->listeners) {
				l->onSignalReceived(r.code);
			}
		}
	}
	this->receivedTimes[r.code] = ms;
}
#endif

void RF433Module::stop() {
#ifdef RPI
	_433D_tx_cancel(tx);
	_433D_rx_cancel(rx);
	pigpio_stop(pi);
#endif
	started = false;

}

bool RF433Module::isStarted() const {
	return started;
}

void RF433Module::addListener(RF433MessageListener* listener) {
	if ( listener == NULL ) return;
	listeners.push_back(listener);
}
void RF433Module::removeListener(RF433MessageListener* listener) {
	if ( listener == NULL ) return;
	std::vector<RF433MessageListener*>::iterator it = std::find(this->listeners.begin(), this->listeners.end(), listener);
	if (it != this->listeners.end()) this->listeners.erase(it);
}

void RF433Module::sendMessage(unsigned long long int toSend) {
#ifdef RPI
	if ( started ) {
		sending = true;
		for(int i = 0; i < sendRepeat; i++ ) {
			_433D_tx_send(tx, toSend);
		}
		sending = false;
	}
#endif
}
} /* namespace alarmpi */


#endif
#endif
