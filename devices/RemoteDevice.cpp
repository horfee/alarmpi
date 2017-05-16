/*
 * RemoteDevice.cpp
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#include "RemoteDevice.h"

namespace alarmpi {

RemoteDevice::RemoteDevice(int deviceId, std::string description, std::string commands): Device(deviceId, description) {

}

RemoteDevice::~RemoteDevice() {
}

std::string RemoteDevice::getType() const {
	return "REMOTE";
}

} /* namespace alarmpi */
