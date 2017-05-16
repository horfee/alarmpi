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

namespace alarmpi {

RF433Module::RF433Module(int receivingPIN, int transmittingPIN) {
	this->transmittingPIN = transmittingPIN;
	this->receivingPIN = receivingPIN;
	started = false;
	tx = NULL;
	rx = NULL;
	pi = -1;

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
	}
}

void RF433Module::cbf(_433D_rx_data_t r){
	for(RF433MessageListener* listener : listeners) {
		listener->onSignalReceived(r.code);
	}
}

void RF433Module::stop() {
	_433D_tx_cancel(tx);
	_433D_rx_cancel(rx);
	pigpio_stop(pi);
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
	_433D_tx_send(tx, toSend);
}
} /* namespace alarmpi */
