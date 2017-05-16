/*
 * Sensor.h
 *
 *  Created on: 21 juil. 2015
 *      Author: horfee
 */

#ifndef ALARMDEVICE_H_
#define ALARMDEVICE_H_
#include <string>
#include "../json/json.h"

namespace alarmpi {

class Device {
public:
	Device(int id, std::string description);
	virtual ~Device();

	int getId() const;

	std::string getDescription() const;
	void setDescription(std::string description);

	virtual std::string getType() const = 0;

	virtual Json::Value toJSON() const;

	virtual bool operator==(const int& deviceId);
	virtual bool operator==(const Device& device);

	virtual bool operator!=(const int& deviceId);
	virtual bool operator!=(const Device& device);
private:

	int id;
	std::string description;

//	AlarmSystem* system;
};

} /* namespace alarmpi */

#endif /* ALARMDEVICE_H_ */
