/*
 * RemoteDevice.h
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#ifndef REMOTEDEVICE_H_
#define REMOTEDEVICE_H_

#include "Device.h"

namespace alarmpi {

class RemoteDevice: public Device {
public:
	RemoteDevice(int deivceId, std::string description, std::string commands);
	virtual ~RemoteDevice();

	virtual std::string getType() const;
};

} /* namespace alarmpi */

#endif /* REMOTEDEVICE_H_ */
