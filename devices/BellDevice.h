/*
 * BellDevice.h
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#ifndef BELLDEVICE_H_
#define BELLDEVICE_H_

#include "ActionnableDevice.h"

namespace alarmpi {

class BellDevice: public ActionnableDevice {
public:
	BellDevice(int id, std::string description, int onValue = 8, int offValue = 0);
	virtual ~BellDevice();

	virtual std::string getType() const;

	int getOnValue() const;

	int getOffValue() const;

	virtual Json::Value toJSON() const;

private:
	int onValue;
	int offValue;
};

} /* namespace alarmpi */

#endif /* BELLDEVICE_H_ */
