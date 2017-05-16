/*
 * ActionnableDevice.cpp
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#include "ActionnableDevice.h"

namespace alarmpi {

ActionnableDevice::ActionnableDevice(int deviceId, std::string description): Device(deviceId, description) {

}

ActionnableDevice::~ActionnableDevice() {
}

} /* namespace alarmpi */
