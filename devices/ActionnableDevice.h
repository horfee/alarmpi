/*
 * ActionnableDevice.h
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#ifndef ACTIONNABLEDEVICE_H_
#define ACTIONNABLEDEVICE_H_

#include "Device.h"

namespace alarmpi {

class ActionnableDevice: public Device {
public:
	ActionnableDevice(int deviceId, std::string description);
	virtual ~ActionnableDevice();

	virtual std::string getType() const = 0;
};

} /* namespace alarmpi */

#endif /* ACTIONNABLEDEVICE_H_ */
