/*
 * AlarmPIAssociationsServlet.h
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#ifndef SERVLETS_ALARMPIASSOCIATIONSSERVLET_H_
#define SERVLETS_ALARMPIASSOCIATIONSSERVLET_H_

#include "AlarmPIServlet.h"

namespace alarmpi {

class AlarmPIAssociationsServlet: public AlarmPIServlet {
public:
	AlarmPIAssociationsServlet(AlarmSystem *system);
	virtual ~AlarmPIAssociationsServlet();

	virtual void doGet(HTTPRequest &request, HTTPResponse &response);
	virtual void doPost(HTTPRequest &request, HTTPResponse &response);
	virtual void doPut(HTTPRequest &request, HTTPResponse &response);
	virtual void doDelete(HTTPRequest &request, HTTPResponse &response);
};

} /* namespace alarmpi */

#endif /* SERVLETS_ALARMPIASSOCIATIONSSERVLET_H_ */
