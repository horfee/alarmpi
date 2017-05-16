/*
 * MotionDevice.h
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#ifndef MOTIONDEVICE_H_
#define MOTIONDEVICE_H_

#include "Device.h"

namespace alarmpi {

class MotionDevice: public Device {
public:
	MotionDevice(int id, std::string description);
	virtual ~MotionDevice();

	virtual std::string getType() const;
};

} /* namespace alarmpi */

#endif /* MOTIONDEVICE_H_ */
