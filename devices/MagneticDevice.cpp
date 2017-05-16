/*
 * MagneticDevice.cpp
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#include "MagneticDevice.h"

namespace alarmpi {

MagneticDevice::MagneticDevice(int id, std::string description): Device(id, description) {

}

MagneticDevice::~MagneticDevice() {
}

std::string MagneticDevice::getType() const {
	return "MAGNETIC";
}
} /* namespace alarmpi */
