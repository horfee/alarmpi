/*
 * AlarmPIModesServlet.h
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#ifndef SERVLETS_ALARMPIMODESSERVLET_H_
#define SERVLETS_ALARMPIMODESSERVLET_H_

#include "AlarmPIServlet.h"

namespace alarmpi {

class AlarmPIModesServlet: public AlarmPIServlet {
public:
	AlarmPIModesServlet(AlarmSystem *system);
	virtual ~AlarmPIModesServlet();

	virtual void doGet(HTTPRequest &request, HTTPResponse &response);
	virtual void doPost(HTTPRequest &request, HTTPResponse &response);
	virtual void doPut(HTTPRequest &request, HTTPResponse &response);
	virtual void doDelete(HTTPRequest &request, HTTPResponse &response);

};

} /* namespace alarmpi */

#endif /* SERVLETS_ALARMPIMODESSERVLET_H_ */
