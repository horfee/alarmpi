/*
 * DAOFactory.h
 *
 *  Created on: 25 juil. 2015
 *      Author: horfee
 */

#ifndef DAOFACTORY_H_
#define DAOFACTORY_H_

#include "Singleton.h"
#include "AlarmSystemDAO.h"

namespace alarmpi {

class DAOFactory: public Singleton<DAOFactory> {
	friend DAOFactory* Singleton<DAOFactory>::getInstance();
	friend void Singleton<DAOFactory>::kill();
public:

	DAOFactory();
	virtual ~DAOFactory();

	AlarmSystemDAO* getDAO();
	void setDAO(AlarmSystemDAO* dao);

protected:
private:

	AlarmSystemDAO* dao;
};

} /* namespace alarmpi */

#endif /* DAOFACTORY_H_ */
