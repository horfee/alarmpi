/*
 * AlarmSystemDAOSQLLite.h
 *
 *  Created on: 24 juil. 2015
 *      Author: horfee
 */

#ifndef ALARMSYSTEMDAOSQLITE_H_
#define ALARMSYSTEMDAOSQLITE_H_

#include <sqlite3.h>

#include "AlarmSystemDAO.h"
#include "devices/AllDevices.h"
#include "actions/AllActions.h"


namespace alarmpi {

class AlarmSystemDAOSQLite: virtual public AlarmSystemDAO {
public:
	AlarmSystemDAOSQLite(std::string file);

	virtual ~AlarmSystemDAOSQLite();


	virtual std::vector<Property*> getProperties();

	virtual std::vector<Mode*>getModes();

	virtual std::vector<Device*> getDevices();

	virtual std::vector<std::string>getPhoneNumbers();

	virtual bool persistProperty(Property *property);

	virtual bool persistPhoneNumber(std::string phoneNumber, int order);

	virtual bool persistDevice(Device* device);

	virtual bool persistMode(Mode* mode);

	virtual bool deleteProperty(std::string property);

	virtual bool deleteMode(Mode* mode);

	virtual bool deletePhoneNumber(std::string phoneNumber);

	virtual bool deleteDevice(Device* device);

	virtual bool deleteAssociation(Device* device, Mode* mode);

	virtual bool deleteAction(Action* action);

	virtual std::vector<Action*> getActions();

	virtual bool persistAction(Action* action);

	virtual bool persistAssocation(Device* device, Mode* mode, Action* action);

	virtual std::vector<std::tuple<int,std::string,std::string>> getAssociations();

private:
	sqlite3 *database;
	std::string sFileName;

	void initializeSchema();
	bool existsDevice(Device* device);
	bool updateBellDevice(Device* dev);
	bool updateRemoteDevice(Device* dev);
	bool updateMotionDevice(Device* dev);
	bool updateMagneticDevice(Device* dev);
	bool persistBellDevice(Device* dev);
	bool persistRemoteDevice(Device* dev);
	bool persistMotionDevice(Device* dev);
	bool persistMagneticDevice(Device* dev);

	bool existsAction(Action* action);
	bool updateRingBellAction(Action* action);
	bool updateActivateAction(Action* action);
	bool updateDelayAction(Action* action);
	bool updateSendMessageAction(Action* action);
	bool updateCallPhoneAction(Action* action);

	bool persistRingBellAction(Action* action);
	bool persistActivateAction(Action* action);
	bool persistDelayAction(Action* action);
	bool persistSendMessageAction(Action* action);
	bool persistCallPhoneAction(Action* action);

	std::map<Action*, std::string> loadActivateActions() const;
	std::map<Action*, std::string> loadDelayActions() const;
	std::map<Action*, std::string> loadCallPhonesActions() const;
	std::map<Action*, std::string> loadRingBellActions() const;
	std::map<Action*, std::string> loadSendMessageActions() const;
};

} /* namespace alarmpi */

#endif /* ALARMSYSTEMDAOSQLITE_H_ */
