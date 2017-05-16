/*
 * MagneticDevice.h
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#ifndef MAGNETICDEVICE_H_
#define MAGNETICDEVICE_H_

#include "Device.h"

namespace alarmpi {

class MagneticDevice: public Device {
public:
	MagneticDevice(int id, std::string description);
	virtual ~MagneticDevice();

	virtual std::string getType() const;
};

} /* namespace alarmpi */

#endif /* MAGNETICDEVICE_H_ */
