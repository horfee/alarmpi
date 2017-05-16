/*
 * Sensor.cpp
 *
 *  Created on: 21 juil. 2015
 *      Author: horfee
 */

#include "Device.h"

#include <iostream>

using namespace std;

namespace alarmpi {

Device::Device(int iId, std::string sDescription) {

	id = iId;
	description = sDescription;
}

Device::~Device() {
}

int Device::getId() const {
	return id;
}

std::string Device::getDescription() const {
	return description;
}

void Device::setDescription(std::string description) {
	this->description = description;
}

bool Device::operator==(const int& deviceId) {
	return this->id == deviceId;
}

bool Device::operator==(const Device& device) {
	return *this == device.id;
}

bool Device::operator!=(const int& deviceId) {
	return this->id != deviceId;
}

bool Device::operator!=(const Device& device) {
	return *this != device.id;
}

Json::Value Device::toJSON() const {
	Json::Value res;
	res["deviceId"] = id;
	res["description"] = description;
	res["type"] = getType();
	return res;
}

} /* namespace alarmpi */
