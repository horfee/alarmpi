/*
 * AlarmSystemDAOSQLite.cpp
 *
 *  Created on: 24 juil. 2015
 *      Author: horfee
 */

#include "AlarmSystemDAOSQLite.h"

#include <strings.h>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <tuple>
#include <typeinfo>
#include <vector>

#include "actions/ActivateAction.h"
#include "actions/CallPhoneAction.h"
#include "actions/DelayAction.h"
#include "actions/RingBellAction.h"
#include "actions/SendMessageAction.h"
#include "devices/BellDevice.h"
#include "devices/MagneticDevice.h"
#include "devices/MotionDevice.h"
#include "devices/RemoteDevice.h"
#include "Mode.h"
#include "Property.h"


namespace alarmpi {


std::map<std::size_t, std::function<bool(AlarmSystemDAOSQLite&, Device*)>> devicePersisters;
std::map<std::size_t, std::function<bool(AlarmSystemDAOSQLite&, Device*)>> deviceUpdaters;
std::map<std::size_t, std::function<bool(AlarmSystemDAOSQLite&, Action*)>> actionPersisters;
std::map<std::size_t, std::function<bool(AlarmSystemDAOSQLite&, Action*)>> actionUpdaters;

void logSql(int i, const char* message) {
	std::cerr << i << " ] " << message << std::endl;
}

AlarmSystemDAOSQLite::AlarmSystemDAOSQLite(std::string sFileName) {
	this->sFileName = sFileName;

	sqlite3_open(sFileName.c_str(), &database);
	sqlite3_config(SQLITE_CONFIG_LOG, logSql);
	initializeSchema();

	devicePersisters[typeid(MotionDevice).hash_code()] 		= &AlarmSystemDAOSQLite::persistMotionDevice;
	devicePersisters[typeid(MagneticDevice).hash_code()] 	= &AlarmSystemDAOSQLite::persistMagneticDevice;
	devicePersisters[typeid(RemoteDevice).hash_code()] 		= &AlarmSystemDAOSQLite::persistRemoteDevice;
	devicePersisters[typeid(BellDevice).hash_code()] 		= &AlarmSystemDAOSQLite::persistBellDevice;

	deviceUpdaters[typeid(MotionDevice).hash_code()] 		= &AlarmSystemDAOSQLite::updateMotionDevice;
	deviceUpdaters[typeid(MagneticDevice).hash_code()] 	= &AlarmSystemDAOSQLite::updateMagneticDevice;
	deviceUpdaters[typeid(RemoteDevice).hash_code()] 		= &AlarmSystemDAOSQLite::updateRemoteDevice;
	deviceUpdaters[typeid(BellDevice).hash_code()] 		= &AlarmSystemDAOSQLite::updateBellDevice;

	actionPersisters[typeid(ActivateAction).hash_code()]	= &AlarmSystemDAOSQLite::persistActivateAction;
	actionPersisters[typeid(DelayAction).hash_code()]		= &AlarmSystemDAOSQLite::persistDelayAction;
	actionPersisters[typeid(RingBellAction).hash_code()]	= &AlarmSystemDAOSQLite::persistRingBellAction;
	actionPersisters[typeid(CallPhoneAction).hash_code()]	= &AlarmSystemDAOSQLite::persistCallPhoneAction;
	actionPersisters[typeid(SendMessageAction).hash_code()]	= &AlarmSystemDAOSQLite::persistSendMessageAction;

	actionUpdaters[typeid(ActivateAction).hash_code()]		= &AlarmSystemDAOSQLite::updateActivateAction;
	actionUpdaters[typeid(DelayAction).hash_code()]			= &AlarmSystemDAOSQLite::updateDelayAction;
	actionUpdaters[typeid(RingBellAction).hash_code()]		= &AlarmSystemDAOSQLite::updateRingBellAction;
	actionUpdaters[typeid(CallPhoneAction).hash_code()]		= &AlarmSystemDAOSQLite::updateCallPhoneAction;
	actionUpdaters[typeid(SendMessageAction).hash_code()]	= &AlarmSystemDAOSQLite::updateSendMessageAction;
}

AlarmSystemDAOSQLite::~AlarmSystemDAOSQLite() {
	sqlite3_close( database );
}

void AlarmSystemDAOSQLite::initializeSchema() {
	std::vector<std::string> queries;
	sqlite3_stmt *statement;

	queries.push_back("CREATE TABLE IF NOT EXISTS phonenumbers(phonenumber TEXT PRIMARY KEY, rank INTEGER)");
	queries.push_back("CREATE TABLE IF NOT EXISTS modes(mode TEXT PRIMARY KEY, description TEXT, type INTEGER)");
	queries.push_back("CREATE TABLE IF NOT EXISTS properties(name TEXT PRIMARY KEY, description TEXT, value TEXT, type TEXT)");

	queries.push_back("CREATE TABLE IF NOT EXISTS alarmdevices(deviceid INTEGER PRIMARY KEY, description TEXT)");
	queries.push_back("CREATE TABLE IF NOT EXISTS motiondevices(deviceid INTEGER PRIMARY KEY)");
	queries.push_back("CREATE TABLE IF NOT EXISTS magneticdevices(deviceid INTEGER PRIMARY KEY)");
	queries.push_back("CREATE TABLE IF NOT EXISTS belldevices(deviceid INTEGER PRIMARY KEY, onvalue INT, offvalue INT)");
	queries.push_back("CREATE TABLE IF NOT EXISTS remotedevices(deviceid INTEGER PRIMARY KEY, commands TEXT)");

	queries.push_back("CREATE TABLE IF NOT EXISTS actions(name TEXT PRIMARY KEY, nextaction TEXT)");
	queries.push_back("CREATE TABLE IF NOT EXISTS ringbellactions(name TEXT PRIMARY KEY, duration INT, params TEXT)");
	queries.push_back("CREATE TABLE IF NOT EXISTS sendmessageactions(name TEXT PRIMARY KEY, message TEXT)");
	queries.push_back("CREATE TABLE IF NOT EXISTS activateactions(name TEXT PRIMARY KEY, data INT, params TEXT)");
	queries.push_back("CREATE TABLE IF NOT EXISTS callphonesactions(name TEXT PRIMARY KEY)");
	queries.push_back("CREATE TABLE IF NOT EXISTS delayactions(name TEXT PRIMARY KEY, delay INT)");

	queries.push_back("CREATE TABLE IF NOT EXISTS devicesmodesactionsassociations(device INT, mode TEXT, action TEXT, PRIMARY KEY(device, mode, action))");
	for(auto it = queries.begin(); it != queries.end(); it++) {
		if(!(sqlite3_prepare_v2(database, it->c_str(), -1, &statement, 0) == SQLITE_OK && sqlite3_step(statement) == SQLITE_DONE)) {

		}
	}



}

std::vector<Mode*> AlarmSystemDAOSQLite::getModes() {

	sqlite3_stmt *statement;
	std::vector<Mode*> results;
	std::string query = "SELECT mode, description, type FROM modes ORDER BY mode";


	if( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		while ( sqlite3_step(statement) == SQLITE_ROW ) {
			std::string mode((const char*)sqlite3_column_text(statement, 0));
			std::string description((const char*)sqlite3_column_text(statement, 1));
			ModeType type;
			switch(sqlite3_column_int(statement, 2)) {
			case 1:
				type = ModeType::Active;
				break;
			case 2:
				type = ModeType::Inactive;
				break;
			case 3:
				type = ModeType::Configuration;
				break;
			default:
				type = ModeType::Active;

			}
			Mode* m = new Mode(mode, description, type);
			results.push_back(m);
		}
	}
	sqlite3_finalize(statement);
	return results;
}
std::vector<Device*> AlarmSystemDAOSQLite::getDevices() {
	sqlite3_stmt *statement;
	std::vector<Device*> results;
	std::string query = "SELECT m.deviceid, description FROM alarmdevices d INNER JOIN motiondevices m ON m.deviceid = d.deviceid ORDER BY m.deviceid";

	if(sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		while ( sqlite3_step(statement) == SQLITE_ROW ) {
			int deviceid = sqlite3_column_int(statement, 0);
			std::string description((const char*)sqlite3_column_text(statement, 1));
			Device* dev = new MotionDevice(deviceid, description);
			results.push_back(dev);
		}
	}
	sqlite3_finalize(statement);

	query = "SELECT m.deviceid, description FROM alarmdevices d INNER JOIN magneticdevices m ON m.deviceid = d.deviceid ORDER BY m.deviceid";
	if(sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		while ( sqlite3_step(statement) == SQLITE_ROW ) {
			int deviceid = sqlite3_column_int(statement, 0);
			std::string description((const char*)sqlite3_column_text(statement, 1));
			Device* dev = new MagneticDevice(deviceid, description);
			results.push_back(dev);
		}
	}
	sqlite3_finalize(statement);

	query = "SELECT m.deviceid, description FROM alarmdevices d INNER JOIN belldevices m ON m.deviceid = d.deviceid ORDER BY m.deviceid";
	if(sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		while ( sqlite3_step(statement) == SQLITE_ROW ) {
			int deviceid = sqlite3_column_int(statement, 0);
			std::string description((const char*)sqlite3_column_text(statement, 1));
			Device* dev = new BellDevice(deviceid, description);
			results.push_back(dev);
		}
	}
	sqlite3_finalize(statement);

	query = "SELECT m.deviceid, description, commands FROM alarmdevices d INNER JOIN remotedevices m ON m.deviceid = d.deviceid ORDER BY m.deviceid";
	if(sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		while ( sqlite3_step(statement) == SQLITE_ROW ) {
			int deviceid = sqlite3_column_int(statement, 0);
			std::string description((const char*)sqlite3_column_text(statement, 1));

			std::string commands;
			if ( sqlite3_column_type(statement, 2) != SQLITE_NULL ) {
				commands = ((const char*)sqlite3_column_text(statement, 2));
			}

			Device* dev = new RemoteDevice(deviceid, description, commands);
			results.push_back(dev);
		}
	}
	sqlite3_finalize(statement);

	return results;
}

std::vector<std::string> AlarmSystemDAOSQLite::getPhoneNumbers() {
	sqlite3_stmt *statement;
	std::vector<std::string> results;
	std::string query = "SELECT phonenumber FROM phonenumbers ORDER BY rank";


	if(sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		while ( sqlite3_step(statement) == SQLITE_ROW ) {
			std::string number((const char*)sqlite3_column_text(statement, 0));
			results.push_back(number);
		}
	}
	sqlite3_finalize(statement);
	return results;
}

bool AlarmSystemDAOSQLite::persistMode(Mode* mode) {

	sqlite3_stmt *statement;
	std::string query;
	int count = -1;
	query = "SELECT COUNT(*) FROM modes WHERE mode = ?";
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		sqlite3_bind_text(statement, 1, mode->getName().c_str(), -1, SQLITE_TRANSIENT);
		if ( sqlite3_step(statement) == SQLITE_ROW ) {
			count = sqlite3_column_int(statement, 0);
		}
		sqlite3_finalize(statement);
		if (count == 0 ) {
			query = "INSERT INTO modes(mode,description, type) values (?, ?, ?)";
			if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
				sqlite3_bind_text(statement, 1, mode->getName().c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_text(statement, 2, mode->getDescription().c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_int(statement, 3, mode->getType());
				int res = sqlite3_step(statement);
				if ( res == SQLITE_DONE) {
					sqlite3_finalize(statement);
					return true;
				}
			}
		} else if ( count == 1 ){
			query = "UPDATE modes set description = ? WHERE mode = ?";
			if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
				sqlite3_bind_text(statement, 1, mode->getDescription().c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_text(statement, 2, mode->getName().c_str(), -1, SQLITE_TRANSIENT);
				int res = sqlite3_step(statement);
				if ( res == SQLITE_DONE) {
					sqlite3_finalize(statement);
					return true;
				}
			}
		}
	}



	sqlite3_finalize(statement);
	return false;
}



bool AlarmSystemDAOSQLite::updateBellDevice(Device* device) {
	sqlite3_stmt *statement;
	std::string query = "UPDATE belldevices set onvalue = ?, offvalue = ? WHERE deviceid = ?";
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		sqlite3_bind_int(statement, 1, device->getId());
		sqlite3_bind_int(statement, 2, ((BellDevice*)device)->getOnValue());
		sqlite3_bind_int(statement, 3, ((BellDevice*)device)->getOffValue());
		if ( sqlite3_step(statement) == SQLITE_DONE) {
			sqlite3_finalize(statement);
			return true;
		}
	}
	sqlite3_finalize(statement);
	return false;
}

bool AlarmSystemDAOSQLite::persistBellDevice(Device* dev) {
	sqlite3_stmt *statement;
	std::string query = "INSERT INTO alarmdevices (deviceid, description) values (?, ?)";

	sqlite3_exec(database, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		sqlite3_bind_int(statement , 1, dev->getId());
		sqlite3_bind_text(statement, 2, dev->getDescription().c_str(), -1, SQLITE_TRANSIENT);
		if ( sqlite3_step(statement) == SQLITE_DONE) {
			sqlite3_finalize(statement);

			query = "INSERT INTO belldevices (deviceid, onvalue, offvalue ) values ( ?, ?, ? )";
			if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
				sqlite3_bind_int(statement, 1, dev->getId());
				sqlite3_bind_int(statement, 2, ((BellDevice*)dev)->getOnValue());
				sqlite3_bind_int(statement, 3, ((BellDevice*)dev)->getOffValue());
				if ( sqlite3_step(statement) == SQLITE_DONE) {
					sqlite3_finalize(statement);
					sqlite3_exec(database, "END TRANSACTION;", NULL, NULL, NULL);
					return true;
				}
			}
		}
	}

	sqlite3_finalize(statement);
	sqlite3_exec(database, "ROLLBACK TRANSACTION;", NULL, NULL, NULL);
	return false;

}


bool AlarmSystemDAOSQLite::updateMotionDevice(Device* device) {
	return true;
}

bool AlarmSystemDAOSQLite::persistMotionDevice(Device* dev) {
	sqlite3_stmt *statement;
	std::string query = "INSERT INTO alarmdevices (deviceid, description) values (?, ?)";

	sqlite3_exec(database, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		sqlite3_bind_int(statement , 1, dev->getId());
		sqlite3_bind_text(statement, 2, dev->getDescription().c_str(), -1, SQLITE_TRANSIENT);
		if ( sqlite3_step(statement) == SQLITE_DONE) {
			sqlite3_finalize(statement);

			query = "INSERT INTO motiondevices (deviceid ) values ( ? )";
			if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
				sqlite3_bind_int(statement, 1, dev->getId());
				if ( sqlite3_step(statement) == SQLITE_DONE) {
					sqlite3_finalize(statement);
					sqlite3_exec(database, "END TRANSACTION;", NULL, NULL, NULL);
					return true;
				}
			}
		}
	}

	sqlite3_finalize(statement);
	sqlite3_exec(database, "ROLLBACK TRANSACTION;", NULL, NULL, NULL);
	return false;

}


bool AlarmSystemDAOSQLite::updateMagneticDevice(Device* device) {
	return true;
}


bool AlarmSystemDAOSQLite::persistMagneticDevice(Device* dev) {
	sqlite3_stmt *statement;
	std::string query = "INSERT INTO alarmdevices (deviceid, description) values (?, ?)";

	sqlite3_exec(database, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		sqlite3_bind_int(statement , 1, dev->getId());
		sqlite3_bind_text(statement, 2, dev->getDescription().c_str(), -1, SQLITE_TRANSIENT);
		if ( sqlite3_step(statement) == SQLITE_DONE) {
			sqlite3_finalize(statement);

			query = "INSERT INTO magneticdevices (deviceid ) values ( ? )";
			if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
				sqlite3_bind_int(statement, 1, dev->getId());
				if ( sqlite3_step(statement) == SQLITE_DONE) {
					sqlite3_finalize(statement);
					sqlite3_exec(database, "END TRANSACTION;", NULL, NULL, NULL);
					return true;
				}
			}
		}
	}

	sqlite3_finalize(statement);
	sqlite3_exec(database, "ROLLBACK TRANSACTION;", NULL, NULL, NULL);
	return false;

}

bool AlarmSystemDAOSQLite::existsDevice(Device* device) {
	sqlite3_stmt *statement;
	std::string query = "SELECT COUNT(*) FROM alarmdevices WHERE deviceid = ?";
	int count = -1;
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		sqlite3_bind_int(statement, 1, device->getId());
		if ( sqlite3_step(statement) == SQLITE_ROW ) {
			count = sqlite3_column_int(statement, 0);
		}
	}
	sqlite3_finalize(statement);

	return count > 0;
}

bool updateDevice(sqlite3* database, Device* device) {
	sqlite3_stmt *statement;
	std::string query = "UPDATE alarmdevices set description = ? WHERE deviceid = ?";
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		sqlite3_bind_int(statement, 1, device->getId());
		sqlite3_bind_text(statement, 2, device->getDescription().c_str(), -1, SQLITE_TRANSIENT);
		if ( sqlite3_step(statement) == SQLITE_DONE) {
			sqlite3_finalize(statement);
			return true;
		}
	}
	sqlite3_finalize(statement);
	return false;
}

bool AlarmSystemDAOSQLite::updateRemoteDevice(Device* dev) {
	return true;
}

bool AlarmSystemDAOSQLite::persistRemoteDevice(Device* dev) {
	sqlite3_stmt *statement;
	std::string query = "INSERT INTO alarmdevices (deviceid, description) values (?, ?)";

	sqlite3_exec(database, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		sqlite3_bind_int(statement , 1, dev->getId());
		sqlite3_bind_text(statement, 2, dev->getDescription().c_str(), -1, SQLITE_TRANSIENT);
		if ( sqlite3_step(statement) == SQLITE_DONE) {
			sqlite3_finalize(statement);

			query = "INSERT INTO remotedevices (deviceid ) values ( ? )";
			if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
				sqlite3_bind_int(statement, 1, dev->getId());
				if ( sqlite3_step(statement) == SQLITE_DONE) {
					sqlite3_finalize(statement);
					sqlite3_exec(database, "END TRANSACTION;", NULL, NULL, NULL);
					return true;
				}
			}
		}
	}

	sqlite3_finalize(statement);
	sqlite3_exec(database, "ROLLBACK TRANSACTION;", NULL, NULL, NULL);
	return false;

}

bool AlarmSystemDAOSQLite::persistDevice(Device *device) {
	if ( existsDevice(device) ) {
		auto deviceUpdater = deviceUpdaters[typeid(*device).hash_code()];
		return deviceUpdater != NULL ? updateDevice(database, device) && deviceUpdater(*this, device) : false;
	} else {
		auto devicePersister = devicePersisters[typeid(*device).hash_code()];
		return devicePersister != NULL ? devicePersister(*this, device) : false;
	}
}

bool AlarmSystemDAOSQLite::persistPhoneNumber(std::string phoneNumber, int order) {
	sqlite3_stmt *statement;
	std::string query;
	if ( order == -1 ) {
		query = "SELECT max(rank) FROM phonenumbers";
		if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK && sqlite3_step(statement) == SQLITE_ROW) {
			order = sqlite3_column_int(statement, 0) + 1;
		}
		sqlite3_finalize(statement);
	}

	query = "UPDATE phonenumbers SET rank = rank + 1 WHERE rank >= ?";

	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		sqlite3_bind_int(statement, 1, order);
		sqlite3_step(statement);
	}
	sqlite3_finalize(statement);

	query = "INSERT INTO phonenumbers(phonenumber, rank) values (?, ?)";

	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		sqlite3_bind_text(statement, 1, phoneNumber.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(statement , 2, order);
		if ( sqlite3_step(statement) == SQLITE_DONE ) {
			sqlite3_finalize(statement);
			return true;
		}
	}
	sqlite3_finalize(statement);
	return false;
}

bool AlarmSystemDAOSQLite::deleteMode(Mode* mode) {
	sqlite3_stmt *statement;
	std::string query = "DELETE FROM modes WHERE mode = ?";

	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		sqlite3_bind_text(statement, 1, mode->getName().c_str(), -1, SQLITE_TRANSIENT);
		if ( sqlite3_step(statement) == SQLITE_DONE ) {
			sqlite3_finalize(statement);
			return true;
		}
	}
	sqlite3_finalize(statement);
	return false;
}

bool AlarmSystemDAOSQLite::deletePhoneNumber(std::string phoneNumber) {
	sqlite3_stmt *statement;
	std::string query = "DELETE FROM phonenumbers WHERE phonenumber = ?";

	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		sqlite3_bind_text(statement, 1, phoneNumber.c_str(), -1, SQLITE_TRANSIENT);
		if ( sqlite3_step(statement) == SQLITE_DONE ) {
			sqlite3_finalize(statement);
			return true;
		}
	}
	sqlite3_finalize(statement);
	return false;
}


std::map<std::string, std::string> deviceTypeTableMap = {
		{"BELL", "belldevices"},
		{"MAGNETIC", "magneticdevices"},
		{"MOTION", "motiondevices"},
		{"REMOTE", "remotedevices"},
};

bool AlarmSystemDAOSQLite::deleteDevice(Device* device) {
	sqlite3_stmt *statement;
	std::string query = "DELETE FROM alarmdevices WHERE deviceid = ?";
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		sqlite3_bind_int(statement, 1, device->getId());
		if ( sqlite3_step(statement) == SQLITE_DONE) {
			sqlite3_finalize(statement);

			query = "DELETE FROM " + deviceTypeTableMap[device->getType()] + " WHERE deviceid = ?";
			if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
					sqlite3_bind_int(statement, 1, device->getId());
					if ( sqlite3_step(statement) == SQLITE_DONE) {
						sqlite3_finalize(statement);
						return true;
					}
			}
		}
	}
	sqlite3_finalize(statement);
	sqlite3_exec(database, "ROLLBACK TRANSACTION;", NULL, NULL, NULL);
	return false;
}

bool AlarmSystemDAOSQLite::deleteAssociation(Device* device, Mode* mode) {
	sqlite3_stmt *statement;
	std::string query = "DELETE FROM devicesmodesactionsassociations WHERE device = ? and mode = ?";
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		sqlite3_bind_int(statement, 1, device->getId());
		sqlite3_bind_text(statement, 2, mode->getName().c_str(), -1, SQLITE_TRANSIENT);
		if ( sqlite3_step(statement) == SQLITE_DONE) {
			sqlite3_finalize(statement);
			return true;
		}
	}
	sqlite3_finalize(statement);
	return false;
}

std::vector<Property*> AlarmSystemDAOSQLite::getProperties() {
	std::vector<Property*> res;
	sqlite3_stmt *statement;
	std::string query = "SELECT name, description, type, value FROM properties ORDER BY name";
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		while(sqlite3_step(statement) == SQLITE_ROW ) {
			string name = (char*)sqlite3_column_text(statement,0);
			string description = string((char*)sqlite3_column_text(statement,1));
			string type = string((char*)sqlite3_column_text(statement,2));
			string value = string((char*)sqlite3_column_text(statement,3));
			Property *p = new Property(name, description, type, value);//NULL;//("","",12);
			if ( p != NULL ) {
				res.push_back(p);
			} else {
//				 Log message
			}


		}
	}
	sqlite3_finalize(statement);
	return res;
}

bool AlarmSystemDAOSQLite::persistProperty(Property *property) {
	sqlite3_stmt *statement;
	std::string query;


	int count = -1;
	query = "SELECT COUNT(*) FROM properties WHERE name = ?";
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		sqlite3_bind_text(statement, 1, property->getName().c_str(), -1, SQLITE_TRANSIENT);
		if ( sqlite3_step(statement) == SQLITE_ROW ) {
			count = sqlite3_column_int(statement, 0);
		}
		sqlite3_finalize(statement);
		if ( count == 0 ) {
			query = "INSERT INTO properties(name, description, type, value) values (?, ?, ?, ?)";

			if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
				sqlite3_bind_text(statement, 1, property->getName().c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_text(statement, 2, property->getDescription().c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_text(statement, 3, property->getType().c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_text(statement, 4, property->getValueAsString().c_str(), -1, SQLITE_TRANSIENT);

				if ( sqlite3_step(statement) == SQLITE_DONE ) {
					sqlite3_finalize(statement);
					return true;
				}
			}
		} else if ( count == 1 ){
			query = "UPDATE properties SET description = ?, type = ?, value = ? WHERE name = ?";
			if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
				sqlite3_bind_text(statement, 4, property->getName().c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_text(statement, 1, property->getDescription().c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_text(statement, 2, property->getType().c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_text(statement, 3, property->getValueAsString().c_str(), -1, SQLITE_TRANSIENT);

				if ( sqlite3_step(statement) == SQLITE_DONE ) {
					sqlite3_finalize(statement);
					return true;
				}
			}
		}
	}

	sqlite3_finalize(statement);
	return false;
}

bool AlarmSystemDAOSQLite::deleteProperty(std::string property) {
	sqlite3_stmt *statement;
	std::string query = "DELETE FROM properties WHERE name = ?";

	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		sqlite3_bind_text(statement, 1, property.c_str(), -1, SQLITE_TRANSIENT);
		if ( sqlite3_step(statement) == SQLITE_DONE ) {
			sqlite3_finalize(statement);
			return true;
		}
	}
	sqlite3_finalize(statement);
	return false;
}


std::map<std::string, std::string> actionTypeTableMap = {
		{"ACTIVATEACTION", "activateactions"},
		{"CALLPHONEACTION", "callphonesactions"},
		{"DELAYACTION", "delayactions"},
		{"RINGBELLACTION", "ringbellactions"},
		{"SENDMESSAGEACTION", "sendmessageactions"},
};

bool AlarmSystemDAOSQLite::deleteAction(Action* action) {
	sqlite3_stmt *statement;
		std::string query = "DELETE FROM actions WHERE name = ?";
		if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
			sqlite3_bind_text(statement, 1, action->getName().c_str(), -1, SQLITE_TRANSIENT);
			if ( sqlite3_step(statement) == SQLITE_DONE) {
				sqlite3_finalize(statement);

				query = "DELETE FROM " + actionTypeTableMap[action->getType()] + " WHERE name = ?";
				if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
					sqlite3_bind_text(statement, 1, action->getName().c_str(), -1, SQLITE_TRANSIENT);
						if ( sqlite3_step(statement) == SQLITE_DONE) {
							sqlite3_finalize(statement);
							return true;
						}
				}
			}
		}
		sqlite3_finalize(statement);
		sqlite3_exec(database, "ROLLBACK TRANSACTION;", NULL, NULL, NULL);
		return false;
}

std::map<Action*, std::string> AlarmSystemDAOSQLite::loadDelayActions() const {
	std::map<Action*, std::string> results;

	sqlite3_stmt *statement;
	std::string query = "SELECT a.name, delay, nextaction FROM actions a inner join delayactions da on a.name = da.name";

	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		while(sqlite3_step(statement) == SQLITE_ROW ) {
			string name = (char*)sqlite3_column_text(statement,0);
			int delay = sqlite3_column_int(statement,1);
			string nextAction("");
			if ( sqlite3_column_type(statement, 2) != SQLITE_NULL ) {
				nextAction = string((char*)sqlite3_column_text(statement,2));
			}
			Action* a = new DelayAction(name, delay);
			results[a] = nextAction;
		}
	}
	sqlite3_finalize(statement);
	return results;
}

std::map<Action*, std::string> AlarmSystemDAOSQLite::loadActivateActions() const {
	std::map<Action*, std::string> results;

	sqlite3_stmt *statement;
	std::string query = "SELECT a.name, data, params, nextaction FROM actions a inner join activateactions aa on a.name = aa.name";

	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		while(sqlite3_step(statement) == SQLITE_ROW ) {
			string name = (char*)sqlite3_column_text(statement,0);
			int data = sqlite3_column_int(statement,1);
			string params = string((char*)sqlite3_column_text(statement,2));
			string nextAction("");
			if ( sqlite3_column_type(statement, 3) != SQLITE_NULL ) {
				nextAction = string((char*)sqlite3_column_text(statement,3));
			}
			Action* a = new ActivateAction(name, data, params, true);
			results[a] = nextAction;
		}
	}
	sqlite3_finalize(statement);
	return results;
}

std::map<Action*, std::string> AlarmSystemDAOSQLite::loadCallPhonesActions() const {
	std::map<Action*, std::string> results;

	sqlite3_stmt *statement;
	std::string query = "SELECT a.name, nextaction FROM actions a inner join callphonesactions aa on a.name = aa.name";

	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		while(sqlite3_step(statement) == SQLITE_ROW ) {
			string name = (char*)sqlite3_column_text(statement,0);
			string nextAction("");
			if ( sqlite3_column_type(statement, 1) != SQLITE_NULL ) {
				nextAction = string((char*)sqlite3_column_text(statement,1));
			}
			Action* a = new CallPhoneAction(name, true);
			results[a] = nextAction;
		}
	}
	sqlite3_finalize(statement);
	return results;
}

std::map<Action*, std::string> AlarmSystemDAOSQLite::loadRingBellActions() const {
	std::map<Action*, std::string> results;

	sqlite3_stmt *statement;
	std::string query = "SELECT a.name, duration, params, nextaction FROM actions a inner join ringbellactions aa on a.name = aa.name";

	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		while(sqlite3_step(statement) == SQLITE_ROW ) {
			string name = (char*)sqlite3_column_text(statement,0);
			int duration = sqlite3_column_int(statement,1);
			string params = string((char*)sqlite3_column_text(statement,2));
			string nextAction("");
			if ( sqlite3_column_type(statement, 3) != SQLITE_NULL) {
				nextAction = string((char*)sqlite3_column_text(statement,3));
			}
			Action* a = new RingBellAction(name, duration, params, true);
			results[a] = nextAction;
		}
	}
	sqlite3_finalize(statement);
	return results;
}

std::map<Action*, std::string> AlarmSystemDAOSQLite::loadSendMessageActions() const {
	std::map<Action*, std::string> results;

	sqlite3_stmt *statement;
	std::string query = "SELECT a.name, message, nextaction FROM actions a inner join sendmessageactions aa on a.name = aa.name";

	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		while(sqlite3_step(statement) == SQLITE_ROW ) {
			string name = (char*)sqlite3_column_text(statement,0);
			string message = (char*)sqlite3_column_text(statement,1);
			string nextAction("");
			if ( sqlite3_column_type(statement, 2) != SQLITE_NULL ) {
				nextAction = string((char*)sqlite3_column_text(statement,2));
			}
			Action* a = new SendMessageAction(name, message, true);
			results[a] = nextAction;
		}
	}
	sqlite3_finalize(statement);
	return results;
}

std::vector<Action*> AlarmSystemDAOSQLite::getActions() {
	std::vector<Action*> results;

	std::map<Action*, std::string> c1 = loadActivateActions();
	std::map<Action*, std::string> c2 = loadCallPhonesActions();
	std::map<Action*, std::string> c3 = loadRingBellActions();
	std::map<Action*, std::string> c4 = loadSendMessageActions();
	std::map<Action*, std::string> c5 = loadDelayActions();

	for(auto it = c1.begin(); it != c1.end(); ++it) results.push_back(it->first);
	for(auto it = c2.begin(); it != c2.end(); ++it) results.push_back(it->first);
	for(auto it = c3.begin(); it != c3.end(); ++it) results.push_back(it->first);
	for(auto it = c4.begin(); it != c4.end(); ++it) results.push_back(it->first);
	for(auto it = c5.begin(); it != c5.end(); ++it) results.push_back(it->first);

	std::string nextAction;
	for(Action* a : results) {

		       if ( c1.find(a) != c1.end() ) {
			nextAction = c1[a];
		} else if ( c2.find(a) != c2.end() ) {
			nextAction = c2[a];
		} else if ( c3.find(a) != c3.end() ) {
			nextAction = c3[a];
		} else if ( c4.find(a) != c4.end() ) {
			nextAction = c4[a];
		} else if ( c5.find(a) != c5.end() ) {
			nextAction = c5[a];
		}

		if ( nextAction != "" ) {
			for(Action* a2 : results) {
				if ( strcasecmp(a2->getName().c_str(), nextAction.c_str() ) == 0 ) {
					a->setNextAction(a2);
					break;
				}
			}
		}
	}

	return results;
}


bool AlarmSystemDAOSQLite::updateDelayAction(Action* action) {
	sqlite3_stmt *statement;
	std::string query = "UPDATE delayactions set delay = ? WHERE name = ?";
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		sqlite3_bind_int(statement, 1, ((DelayAction*)action)->getDelay());
		sqlite3_bind_text(statement, 2, action->getName().c_str(), -1, SQLITE_TRANSIENT);
		if ( sqlite3_step(statement) == SQLITE_DONE) {
			sqlite3_finalize(statement);
			return true;
		}
	}
	sqlite3_finalize(statement);
	return false;
}

bool AlarmSystemDAOSQLite::persistDelayAction(Action* action) {
	sqlite3_stmt *statement;
	std::string query = "INSERT INTO actions (name, nextaction) values (?, ?)";

	sqlite3_exec(database, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		sqlite3_bind_text(statement, 1, action->getName().c_str(), -1, SQLITE_TRANSIENT);
		if ( action->getNextAction() != NULL ) sqlite3_bind_text(statement, 2, action->getNextAction()->getName().c_str(), -1, SQLITE_TRANSIENT);
		else sqlite3_bind_null(statement, 2);

		if ( sqlite3_step(statement) == SQLITE_DONE) {
			sqlite3_finalize(statement);

			query = "INSERT INTO delayactions (name, delay ) values ( ?, ?)";
			if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
				sqlite3_bind_text(statement, 1, action->getName().c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_int(statement, 2, ((DelayAction*)action)->getDelay());

				if ( sqlite3_step(statement) == SQLITE_DONE) {
					sqlite3_finalize(statement);
					sqlite3_exec(database, "END TRANSACTION;", NULL, NULL, NULL);
					return true;
				}
			}
		}
	}

	sqlite3_finalize(statement);
	sqlite3_exec(database, "ROLLBACK TRANSACTION;", NULL, NULL, NULL);
	return false;
}



bool AlarmSystemDAOSQLite::updateRingBellAction(Action* action) {
	sqlite3_stmt *statement;
	std::string query = "UPDATE ringbellactions set duration = ?, params = ? WHERE name = ?";
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		sqlite3_bind_int(statement, 1, ((RingBellAction*)action)->getDuration());
		sqlite3_bind_text(statement, 2, ((RingBellAction*)action)->getParams().c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(statement, 3, action->getName().c_str(), -1, SQLITE_TRANSIENT);
		if ( sqlite3_step(statement) == SQLITE_DONE) {
			sqlite3_finalize(statement);
			return true;
		}
	}
	sqlite3_finalize(statement);
	return false;
}

bool AlarmSystemDAOSQLite::persistRingBellAction(Action* action) {
	sqlite3_stmt *statement;
	std::string query = "INSERT INTO actions (name, nextaction) values (?, ?)";

	sqlite3_exec(database, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		sqlite3_bind_text(statement, 1, action->getName().c_str(), -1, SQLITE_TRANSIENT);
		if ( action->getNextAction() != NULL ) sqlite3_bind_text(statement, 2, action->getNextAction()->getName().c_str(), -1, SQLITE_TRANSIENT);
		else sqlite3_bind_null(statement, 2);

		if ( sqlite3_step(statement) == SQLITE_DONE) {
			sqlite3_finalize(statement);

			query = "INSERT INTO ringbellactions (name, duration, params ) values ( ?, ?, ? )";
			if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
				sqlite3_bind_text(statement, 1, action->getName().c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_int(statement, 2, ((RingBellAction*)action)->getDuration());
				sqlite3_bind_text(statement, 3, ((RingBellAction*)action)->getParams().c_str(), -1, SQLITE_TRANSIENT);

				if ( sqlite3_step(statement) == SQLITE_DONE) {
					sqlite3_finalize(statement);
					sqlite3_exec(database, "END TRANSACTION;", NULL, NULL, NULL);
					return true;
				}
			}
		}
	}

	sqlite3_finalize(statement);
	sqlite3_exec(database, "ROLLBACK TRANSACTION;", NULL, NULL, NULL);
	return false;
}


bool AlarmSystemDAOSQLite::updateSendMessageAction(Action* action) {
	sqlite3_stmt *statement;
	std::string query = "UPDATE sendmessageactions set message = ? WHERE name = ?";
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		sqlite3_bind_text(statement, 1, ((SendMessageAction*)action)->getMessage().c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(statement, 2, action->getName().c_str(), -1, SQLITE_TRANSIENT);
		if ( sqlite3_step(statement) == SQLITE_DONE) {
			sqlite3_finalize(statement);
			return true;
		}
	}
	sqlite3_finalize(statement);
	return false;
}

bool AlarmSystemDAOSQLite::persistSendMessageAction(Action* action) {
	sqlite3_stmt *statement;
	std::string query = "INSERT INTO actions (name, nextaction) values (?, ?)";

	sqlite3_exec(database, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		sqlite3_bind_text(statement, 1, action->getName().c_str(), -1, SQLITE_TRANSIENT);
		if ( action->getNextAction() != NULL ) sqlite3_bind_text(statement, 2, action->getNextAction()->getName().c_str(), -1, SQLITE_TRANSIENT);
		else sqlite3_bind_null(statement, 2);

		if ( sqlite3_step(statement) == SQLITE_DONE) {
			sqlite3_finalize(statement);

			query = "INSERT INTO sendmessageactions (name, message ) values ( ?, ? )";
			if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
				sqlite3_bind_text(statement, 1, action->getName().c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_text(statement, 2, ((SendMessageAction*)action)->getMessage().c_str(), -1, SQLITE_TRANSIENT);

				if ( sqlite3_step(statement) == SQLITE_DONE) {
					sqlite3_finalize(statement);
					sqlite3_exec(database, "END TRANSACTION;", NULL, NULL, NULL);
					return true;
				}
			}
		}
	}

	sqlite3_finalize(statement);
	sqlite3_exec(database, "ROLLBACK TRANSACTION;", NULL, NULL, NULL);
	return false;
}

bool AlarmSystemDAOSQLite::updateCallPhoneAction(Action* action) {
	return true;
}

bool AlarmSystemDAOSQLite::persistCallPhoneAction(Action* action) {
	sqlite3_stmt *statement;
	std::string query = "INSERT INTO actions (name, nextaction) values (?, ?)";

	sqlite3_exec(database, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		sqlite3_bind_text(statement, 1, action->getName().c_str(), -1, SQLITE_TRANSIENT);
		if ( action->getNextAction() != NULL ) sqlite3_bind_text(statement, 2, action->getNextAction()->getName().c_str(), -1, SQLITE_TRANSIENT);
		else sqlite3_bind_null(statement, 2);

		if ( sqlite3_step(statement) == SQLITE_DONE) {
			sqlite3_finalize(statement);

			query = "INSERT INTO callphonesactions (name ) values ( ? )";
			if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
				sqlite3_bind_text(statement, 1, action->getName().c_str(), -1, SQLITE_TRANSIENT);

				if ( sqlite3_step(statement) == SQLITE_DONE) {
					sqlite3_finalize(statement);
					sqlite3_exec(database, "END TRANSACTION;", NULL, NULL, NULL);
					return true;
				}
			}
		}
	}

	sqlite3_finalize(statement);
	sqlite3_exec(database, "ROLLBACK TRANSACTION;", NULL, NULL, NULL);
	return false;
}

bool AlarmSystemDAOSQLite::existsAction(Action* action) {
	sqlite3_stmt *statement;
	std::string query = "SELECT COUNT(*) FROM action WHERE name = ?";
	int count = -1;
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		sqlite3_bind_text(statement, 1, action->getName().c_str(), -1, SQLITE_TRANSIENT);
		if ( sqlite3_step(statement) == SQLITE_ROW ) {
			count = sqlite3_column_int(statement, 0);
		}
	}
	sqlite3_finalize(statement);

	return count > 0;
}


bool updateAction(sqlite3* database, Action* action) {
	sqlite3_stmt *statement;
	std::string query = "UPDATE actions set nextaction = ? WHERE name = ?";
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		if ( action->getNextAction() == NULL) sqlite3_bind_null(statement, 1);
		else  sqlite3_bind_text(statement, 1, action->getNextAction()->getName().c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(statement, 3, action->getName().c_str(), -1, SQLITE_TRANSIENT);
		if ( sqlite3_step(statement) == SQLITE_DONE) {
			sqlite3_finalize(statement);
			return true;
		}
	}
	sqlite3_finalize(statement);
	return false;
}


bool AlarmSystemDAOSQLite::updateActivateAction(Action* action) {
	sqlite3_stmt *statement;
	std::string query = "UPDATE activateactions set data = ?, params = ? WHERE name = ?";
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		sqlite3_bind_int(statement, 1, ((ActivateAction*)action)->getValue());
		sqlite3_bind_text(statement, 2, ((ActivateAction*)action)->getTargets().c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(statement, 3, action->getName().c_str(), -1, SQLITE_TRANSIENT);
		if ( sqlite3_step(statement) == SQLITE_DONE) {
			sqlite3_finalize(statement);
			return true;
		}
	}
	sqlite3_finalize(statement);
	return false;
}

bool AlarmSystemDAOSQLite::persistActivateAction(Action* action) {
	sqlite3_stmt *statement;
	std::string query = "INSERT INTO actions (name, nextaction) values (?, ?)";

	sqlite3_exec(database, "BEGIN TRANSACTION;", NULL, NULL, NULL);
	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		sqlite3_bind_text(statement, 1, action->getName().c_str(), -1, SQLITE_TRANSIENT);
		if ( action->getNextAction() != NULL ) sqlite3_bind_text(statement, 2, action->getNextAction()->getName().c_str(), -1, SQLITE_TRANSIENT);
		else sqlite3_bind_null(statement, 2);

		if ( sqlite3_step(statement) == SQLITE_DONE) {
			sqlite3_finalize(statement);

			query = "INSERT INTO activateactions (name, data, params ) values ( ?, ?, ? )";
			if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
				sqlite3_bind_text(statement, 1, action->getName().c_str(), -1, SQLITE_TRANSIENT);
				sqlite3_bind_int(statement , 2, ((ActivateAction*)action)->getValue());
				sqlite3_bind_text(statement, 3, ((ActivateAction*)action)->getTargets().c_str(), -1, SQLITE_TRANSIENT);

				if ( sqlite3_step(statement) == SQLITE_DONE) {
					sqlite3_finalize(statement);
					sqlite3_exec(database, "END TRANSACTION;", NULL, NULL, NULL);
					return true;
				}
			}
		}
	}

	sqlite3_finalize(statement);
	sqlite3_exec(database, "ROLLBACK TRANSACTION;", NULL, NULL, NULL);
	return false;

}

bool AlarmSystemDAOSQLite::persistAction(Action* action) {
	if ( existsAction(action) ) {
		auto actionUpdater = actionUpdaters[typeid(*action).hash_code()];
		return actionUpdater != NULL ? updateAction(database, action) && actionUpdater(*this, action) : false;
	} else {
		auto actionPersister = actionPersisters[typeid(*action).hash_code()];
		return actionPersister != NULL ? actionPersister(*this, action) : false;
	}
}


bool AlarmSystemDAOSQLite::persistAssocation(Device* device, Mode* mode, Action* action) {
	sqlite3_stmt *statement;
	std::string query = "INSERT INTO devicesmodesactionsassociations (device, mode, action) values (?, ?, ?)";

	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, NULL) == SQLITE_OK) {
		sqlite3_bind_int(statement , 1, device->getId());
		sqlite3_bind_text(statement, 2, mode->getName().c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(statement, 3, action->getName().c_str(), -1, SQLITE_TRANSIENT);

		if ( sqlite3_step(statement) == SQLITE_DONE) {
			sqlite3_finalize(statement);
			return true;
		}
	}
	sqlite3_finalize(statement);

	return false;
}

std::vector<std::tuple<int,std::string,std::string>> AlarmSystemDAOSQLite::getAssociations() {
	std::vector<std::tuple<int,std::string,std::string>> results;

	sqlite3_stmt *statement;
	std::string query = "SELECT device, mode, action FROM devicesmodesactionsassociations";

	if ( sqlite3_prepare_v2(database, query.c_str(), -1, &statement, 0) == SQLITE_OK) {
		while(sqlite3_step(statement) == SQLITE_ROW ) {
			int device= sqlite3_column_int(statement,0);
			string mode = (char*)sqlite3_column_text(statement,1);
			string action = string((char*)sqlite3_column_text(statement,2));
			std::tuple<int,std::string,std::string> tu(device, mode, action);//std::tuple<std::string, std::string, std::string>make_tuple(device, mode, action);
			results.push_back(tu);
		}
	}
	sqlite3_finalize(statement);
	return results;
}
} /* namespace alarmpi */
