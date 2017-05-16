

#ifndef ALARMSYSTEMDAO_H_
#define ALARMSYSTEMDAO_H_

#include <vector>
#include <string>
#include <tuple>

#include "AlarmSystem.h"
#include "Property.h"
#include "actions/Action.h"
#include "devices/Device.h"

namespace alarmpi {

typedef union {
		bool boolValue;
		int intvalue;
		long longValue;
		string stringValue;
		float floatValue;
		double doubleValue;
	} PropertyType;

class AlarmSystemDAO {



public:
	virtual ~AlarmSystemDAO() {};

	virtual std::vector<Mode*>getModes() = 0;

	virtual std::vector<Device*> getDevices() = 0;

	virtual std::vector<std::string>getPhoneNumbers() = 0;

	virtual std::vector<Property*>getProperties() = 0;

	virtual bool persistProperty(Property *property) = 0;

	virtual bool persistPhoneNumber(std::string phoneNumber, int order = -1) = 0;

	virtual bool persistDevice(Device* device) = 0;

	virtual bool persistMode(Mode* mode) = 0;

	virtual bool deleteMode(Mode* mode) = 0;

	virtual bool deletePhoneNumber(std::string phoneNumber) = 0;

	virtual bool deleteDevice(Device* device) = 0;

	virtual bool deleteProperty(std::string property) = 0;

	virtual bool deleteAction(Action* action) = 0;

	virtual bool deleteAssociation(Device* device, Mode* mode) = 0;

	virtual std::vector<Action*> getActions() = 0;

	virtual bool persistAction(Action* action) = 0;

	virtual bool persistAssocation(Device* device, Mode* mode, Action* action) = 0;

	virtual std::vector<std::tuple<int,std::string,std::string>> getAssociations() = 0;
};

}

#endif /* ALARMSYSTEMDAO_H_ */
