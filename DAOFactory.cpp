/*
 * DAOFactory.cpp
 *
 *  Created on: 25 juil. 2015
 *      Author: horfee
 */

#include "DAOFactory.h"

namespace alarmpi {

DAOFactory::DAOFactory() {
	this->dao = NULL;
}

DAOFactory::~DAOFactory() {
	this->dao = NULL;
}

AlarmSystemDAO* DAOFactory::getDAO() {
	return dao;
}

void DAOFactory::setDAO(AlarmSystemDAO* dao) {
	this->dao = dao;
}

} /* namespace alarmpi */
