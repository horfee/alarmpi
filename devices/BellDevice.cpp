/*
 * BellDevice.cpp
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#include "BellDevice.h"

namespace alarmpi {

BellDevice::BellDevice(int id, std::string description, int onValue, int offValue): ActionnableDevice(id, description) {
	this->onValue = onValue;
	this->offValue = offValue;
}

BellDevice::~BellDevice() {
}

std::string BellDevice::getType() const {
	return "BELL";
}

int BellDevice::getOnValue() const {
	return onValue;
}

int BellDevice::getOffValue() const {
	return offValue;
}

Json::Value BellDevice::toJSON() const {
	Json::Value res = Device::toJSON();
	res["on"] = onValue;
	res["off"] = offValue;
	return res;
}

} /* namespace alarmpi */
