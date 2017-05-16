/*
 * MotionDevice.cpp
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#include "MotionDevice.h"

namespace alarmpi {

MotionDevice::MotionDevice(int id, std::string description): Device(id, description) {

}

MotionDevice::~MotionDevice() {
}

std::string MotionDevice::getType() const {
	return "MOTION";
}

} /* namespace alarmpi */
